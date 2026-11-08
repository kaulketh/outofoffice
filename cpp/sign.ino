#include <Adafruit_LittleFS.h>
#include <Adafruit_TinyUSB.h>
#include <Fonts/FreeMono12pt7b.h>
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSansBold18pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include <GxEPD2_BW.h>
#include <InternalFileSystem.h>
#include <bluefruit.h>

#include "config.h"

using namespace Adafruit_LittleFS_Namespace;

// ============================================================
// Persistenz
// ============================================================

constexpr const char* STATE_FILE = "/raumschild.txt";

// ============================================================
// E-Paper
// ============================================================

constexpr uint8_t EPD_CS = D1;
constexpr uint8_t EPD_DC = D2;
constexpr uint8_t EPD_RST = D3;
constexpr uint8_t EPD_BUSY = D4;
constexpr uint8_t EPD_PWR = D5;

// Waveshare 5.83" V2, Schwarz/Weiss, 648 x 480
GxEPD2_BW<GxEPD2_583_T8, GxEPD2_583_T8::HEIGHT / 2> display(
  GxEPD2_583_T8(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY));

// ============================================================
// BLE
// ============================================================

BLEUart bleuart;

String rxBuffer;

bool rxPending = false;
unsigned long lastRxMillis = 0;

// ------------------------------------------------------------
// BLE TX
//
// Wir senden bewusst kleine Pakete.
// Damit vermeiden wir überlaufende Notification-Puffer.
// ------------------------------------------------------------

constexpr size_t BLE_TX_CHUNK_SIZE = 16;
constexpr uint16_t BLE_TX_PAUSE_MS = 25;
constexpr uint8_t BLE_TX_MAX_RETRIES = 20;

// ============================================================
// Laufzeitdaten
// ============================================================

struct PersonState {
  String status;
  String text;
};

PersonState personState[PERSON_COUNT];

// ============================================================
// Person anhand ID suchen
// ============================================================

int findPersonIndex(const String& id) {
  for (size_t i = 0; i < PERSON_COUNT; i++) {
    if (id == PERSONS[i].id) {
      return i;
    }
  }

  return -1;
}

// ============================================================
// Zustand speichern
// ============================================================

bool saveState() {
  File file(InternalFS);

  if (!file.open(STATE_FILE, FILE_O_WRITE)) {
    Serial.println("Fehler: Statusdatei kann nicht geoeffnet werden");

    return false;
  }

  file.truncate();
  file.seek(0);

  file.println("V1");

  for (size_t i = 0; i < PERSON_COUNT; i++) {
    file.print(PERSONS[i].id);
    file.print("|");

    file.print(personState[i].status);
    file.print("|");

    file.println(personState[i].text);
  }

  file.close();

  Serial.println("Status im Flash gespeichert");

  return true;
}

// ============================================================
// Zustand laden
// ============================================================

bool loadState() {
  File file(InternalFS);

  if (!file.open(STATE_FILE, FILE_O_READ)) {
    Serial.println("Keine gespeicherten Statusdaten vorhanden");

    return false;
  }

  String version = file.readStringUntil('\n');

  version.trim();

  if (version != "V1") {
    Serial.print("Unbekannte Statusdatei-Version: ");

    Serial.println(version);

    file.close();

    return false;
  }

  while (file.available()) {
    String line = file.readStringUntil('\n');

    line.trim();

    if (line.length() == 0) {
      continue;
    }

    int separator1 = line.indexOf('|');

    int separator2 = line.indexOf('|', separator1 + 1);

    if (separator1 < 0 || separator2 < 0) {
      Serial.print("Ungueltige Statuszeile: ");

      Serial.println(line);

      continue;
    }

    String personId = line.substring(0, separator1);

    String status = line.substring(separator1 + 1, separator2);

    String text = line.substring(separator2 + 1);

    personId.trim();
    status.trim();
    text.trim();

    int index = findPersonIndex(personId);

    if (index < 0) {
      Serial.print("Unbekannte Person im Flash: ");

      Serial.println(personId);

      continue;
    }

    personState[index].status = status;

    personState[index].text = text;
  }

  file.close();

  Serial.println("Status aus Flash geladen");

  return true;
}

// ============================================================
// Akku
// ============================================================

float readBatteryVoltage() {
  // Spannungsteiler für VBAT aktivieren
  pinMode(VBAT_ENABLE, OUTPUT);
  digitalWrite(VBAT_ENABLE, LOW);

  delay(10);

  // nRF52 ADC bewusst auf 3.0 V / 12 Bit einstellen
  analogReference(AR_INTERNAL_3_0);
  analogReadResolution(12);

  // Bei hochohmigem Spannungsteiler etwas längere Sample-Zeit
  analogSampleTime(40);

  // Erste Messung verwerfen
  analogRead(PIN_VBAT);

  constexpr uint8_t SAMPLE_COUNT = 8;

  uint32_t sum = 0;

  for (uint8_t i = 0; i < SAMPLE_COUNT; i++) {
    sum += analogRead(PIN_VBAT);
    delay(2);
  }

  const float raw = static_cast<float>(sum) / SAMPLE_COUNT;

  /*
   * XIAO Spannungsteiler:
   *
   * 1 MOhm + 510 kOhm
   *
   * Faktor:
   * 1510 / 510 = ca. 2.9608
   */

  constexpr float ADC_REF = 3.0f;
  constexpr float ADC_MAX = 4095.0f;
  constexpr float VBAT_DIVIDER = 1510.0f / 510.0f;

  const float adcVoltage = raw * ADC_REF / ADC_MAX;

  const float batteryVoltage = adcVoltage * VBAT_DIVIDER;

  // Spannungsteiler wieder deaktivieren
  pinMode(VBAT_ENABLE, INPUT);

  // ADC auf Standard zurücksetzen
  analogReference(AR_DEFAULT);
  analogReadResolution(10);
  analogSampleTime(3);

  return batteryVoltage;
}

int batteryPercent(float voltage) {
  /*
   * Grobe LiPo-Schätzung.
   * Bewusst nur Näherungswert.
   */

  if (voltage >= 4.20f) return 100;
  if (voltage >= 4.10f) return 90;
  if (voltage >= 4.00f) return 80;
  if (voltage >= 3.90f) return 70;
  if (voltage >= 3.80f) return 55;
  if (voltage >= 3.70f) return 40;
  if (voltage >= 3.60f) return 20;
  if (voltage >= 3.50f) return 10;

  return 0;
}

void sendBatteryStatus() {
  const float voltage = readBatteryVoltage();

  if (voltage < 2.5f || voltage > 4.5f) {
    sendLine("BAT|NA");

    return;
  }

  const int percent = batteryPercent(voltage);

  String line = "BAT|";

  line += String(voltage, 2);
  line += "|";
  line += String(percent);

  sendLine(line);
}

// ============================================================
// Textbreite
// ============================================================

uint16_t textWidth(const String& text) {
  int16_t x1;
  int16_t y1;

  uint16_t w;
  uint16_t h;

  display.getTextBounds(text.c_str(), 0, 0, &x1, &y1, &w, &h);

  return w;
}

// ============================================================
// Text zentriert
// ============================================================

void drawCentered(const String& text, int16_t y) {
  int16_t x1;
  int16_t y1;

  uint16_t w;
  uint16_t h;

  display.getTextBounds(text.c_str(), 0, 0, &x1, &y1, &w, &h);

  int16_t x = (display.width() - w) / 2 - x1;

  display.setCursor(x, y);

  display.print(text);
}

// ============================================================
// Wortumbruch
// ============================================================

void drawWrappedText(const String& text, int16_t x, int16_t y,
                     uint16_t maxWidth, uint8_t maxLines, uint16_t lineHeight) {
  if (text.length() == 0) {
    return;
  }

  String remaining = text;
  String line;

  uint8_t lines = 0;

  while (remaining.length() > 0 && lines < maxLines) {
    int spacePos = remaining.indexOf(' ');

    String word;

    if (spacePos < 0) {
      word = remaining;
      remaining = "";

    } else {
      word = remaining.substring(0, spacePos);

      remaining = remaining.substring(spacePos + 1);
    }

    String candidate = line;

    if (candidate.length() > 0) {
      candidate += " ";
    }

    candidate += word;

    if (textWidth(candidate) <= maxWidth) {
      line = candidate;
      continue;
    }

    if (line.length() > 0) {
      display.setCursor(x, y + lines * lineHeight);

      display.print(line);

      lines++;
    }

    line = word;
  }

  if (line.length() > 0 && lines < maxLines) {
    display.setCursor(x, y + lines * lineHeight);

    display.print(line);
  }
}

// ============================================================
// Eine Person zeichnen
// ============================================================

void drawPerson(size_t index, int16_t top, int16_t height) {
  constexpr int16_t LEFT = 28;
  constexpr int16_t RIGHT = 28;

  const uint16_t usableWidth = display.width() - LEFT - RIGHT;

  // --------------------------------------------------------
  // Name
  // --------------------------------------------------------

  display.setFont(&FreeSansBold18pt7b);

  display.setCursor(LEFT, top + 56);

  display.print(PERSONS[index].name);

  // --------------------------------------------------------
  // Status
  // --------------------------------------------------------

  String status = personState[index].status;

  if (status.length() > 0) {
    status.toUpperCase();

    display.setFont(&FreeSansBold12pt7b);

    display.setCursor(LEFT, top + 93);

    display.print(status);
  }

  // --------------------------------------------------------
  // Freitext
  // --------------------------------------------------------

  if (personState[index].text.length() > 0) {
    display.setFont(&FreeSansBold9pt7b);

    drawWrappedText(personState[index].text, LEFT, top + 125, usableWidth, 2,
                    25);
  }

  // --------------------------------------------------------
  // Trennlinie
  // etwas kräftiger
  // --------------------------------------------------------

  const int16_t y = top + height - 1;

  display.drawLine(LEFT, y, display.width() - RIGHT, y, GxEPD_BLACK);

  display.drawLine(LEFT, y + 1, display.width() - RIGHT, y + 1, GxEPD_BLACK);
}

// ============================================================
// Footer
// ============================================================

void drawFooter(int16_t top) {
  constexpr int16_t LEFT = 28;

  // kräftige obere Begrenzung
  display.drawLine(LEFT, top, display.width() - LEFT, top, GxEPD_BLACK);

  display.drawLine(LEFT, top + 1, display.width() - LEFT, top + 1, GxEPD_BLACK);

  // --------------------------------------------------------
  // Sachgebiet
  // --------------------------------------------------------

  display.setFont(&FreeSansBold12pt7b);

  display.setCursor(LEFT, top + 60);

  display.print(DEPARTMENT);

  // --------------------------------------------------------
  // Organisation
  // --------------------------------------------------------

  display.setFont(&FreeSansBold12pt7b);

  display.setCursor(LEFT, top + 91);

  display.print(ORGANIZATION);
}

// ============================================================
// Schild zeichnen
// ============================================================

void drawRoomSign() {
  display.fillScreen(GxEPD_WHITE);

  display.setTextColor(GxEPD_BLACK);

  // Footer = exakt 1/3 der Displayhöhe
  constexpr int16_t FOOTER_HEIGHT = 160;

  const int16_t personsHeight = display.height() - FOOTER_HEIGHT;

  const int16_t personHeight = personsHeight / PERSON_COUNT;

  for (size_t i = 0; i < PERSON_COUNT; i++) {
    drawPerson(i, i * personHeight, personHeight);
  }

  drawFooter(personsHeight);
}

// ============================================================
// Display aktualisieren
// ============================================================

void updateDisplay() {
  Serial.println("E-Paper wird aktualisiert ...");

  pinMode(EPD_PWR, OUTPUT);

  digitalWrite(EPD_PWR, HIGH);

  delay(100);

  display.init(115200, true, 10, false);

  display.setRotation(2); //0 none, 2 180°
  display.setFullWindow();

  display.firstPage();

  do {
    drawRoomSign();

  } while (display.nextPage());

  display.hibernate();

  delay(100);

  digitalWrite(EPD_PWR, LOW);

  Serial.println("E-Paper aktualisiert; HAT aus");
}

// ============================================================
// Robustes BLE-Senden
// ============================================================

bool sendBleData(const String& data) {
  if (!Bluefruit.connected()) {
    return false;
  }

  if (!bleuart.notifyEnabled()) {
    return false;
  }

  const uint8_t* bytes = reinterpret_cast<const uint8_t*>(data.c_str());

  size_t total = data.length();

  size_t offset = 0;

  while (offset < total) {
    size_t remaining = total - offset;

    size_t chunkSize =
      remaining > BLE_TX_CHUNK_SIZE ? BLE_TX_CHUNK_SIZE : remaining;

    size_t sent = 0;
    uint8_t retries = 0;

    while (sent < chunkSize && retries < BLE_TX_MAX_RETRIES) {
      size_t written = bleuart.write(bytes + offset + sent, chunkSize - sent);

      if (written > 0) {
        sent += written;

      } else {
        retries++;

        delay(BLE_TX_PAUSE_MS);
      }
    }

    if (sent != chunkSize) {
      Serial.println("WARNUNG: BLE TX konnte nicht vollstaendig senden");

      return false;
    }

    offset += chunkSize;

    // Notification-Puffer Zeit zum Leeren geben

    delay(BLE_TX_PAUSE_MS);
  }

  return true;
}

// ============================================================
// BLE-Zeile senden
// ============================================================

void sendLine(const String& text) {
  // Immer ins serielle Log

  Serial.println(text);

  if (!Bluefruit.connected()) {
    return;
  }

  String line = text + "\n";

  sendBleData(line);
}

// ============================================================
// Konfiguration senden
// ============================================================

void sendInfo() {
  sendLine("INFO_BEGIN");

  sendLine(String("ROOM|") + ROOM_NUMBER);

  sendLine(String("ORG|") + ORGANIZATION);

  sendLine(String("DEPT|") + DEPARTMENT);

  for (size_t i = 0; i < PERSON_COUNT; i++) {
    sendLine(String("PERSON|") + PERSONS[i].id + "|" + PERSONS[i].name);
  }

  sendBatteryStatus();

  sendLine("INFO_END");
}

// ============================================================
// Zustände senden
// ============================================================

void sendStates() {
  sendLine("STATE_BEGIN");

  for (size_t i = 0; i < PERSON_COUNT; i++) {
    sendLine(String("STATE|") + PERSONS[i].id + "|" + PERSONS[i].name + "|" + personState[i].status + "|" + personState[i].text);
  }

  sendLine("STATE_END");
}

// ============================================================
// Kommando verarbeiten
// ============================================================

void processCommand(String command) {
  command.trim();

  if (command.length() == 0) {
    return;
  }

  Serial.print("Kommando: ");

  Serial.println(command);

  // --------------------------------------------------------
  // INFO
  // --------------------------------------------------------

  if (command == "INFO") {
    sendInfo();

    return;
  }

  // --------------------------------------------------------
  // GET
  // --------------------------------------------------------

  if (command == "GET") {
    sendStates();

    return;
  }

  // --------------------------------------------------------
  // BAT
  // --------------------------------------------------------

  if (command == "BAT") {
    sendBatteryStatus();

    return;
  }

  // --------------------------------------------------------
  // REDRAW
  // --------------------------------------------------------

  if (command == "REDRAW") {
    sendLine("REFRESHING");

    updateDisplay();

    sendLine("READY");

    return;
  }

  // --------------------------------------------------------
  // SET
  //
  // SET|P1|Urlaub|bis 18.11.
  // --------------------------------------------------------

  if (!command.startsWith("SET|")) {
    sendLine("ERR|UNKNOWN_COMMAND");

    return;
  }

  int separator1 = command.indexOf('|');

  int separator2 = command.indexOf('|', separator1 + 1);

  int separator3 = command.indexOf('|', separator2 + 1);

  if (separator2 < 0 || separator3 < 0) {
    sendLine("ERR|INVALID_FORMAT");

    return;
  }

  String personId = command.substring(separator1 + 1, separator2);

  String status = command.substring(separator2 + 1, separator3);

  String text = command.substring(separator3 + 1);

  personId.trim();
  status.trim();
  text.trim();

  int index = findPersonIndex(personId);

  if (index < 0) {
    sendLine(String("ERR|UNKNOWN_PERSON|") + personId);

    return;
  }

  personState[index].status = status;

  personState[index].text = text;

  if (!saveState()) {
    sendLine("ERR|SAVE_FAILED");

    return;
  }

  sendLine(String("OK|") + personId);

  sendLine("REFRESHING");

  updateDisplay();

  sendLine("READY");
}

// ============================================================
// BLE RX
// ============================================================

void bleuartRxCallback(uint16_t connHandle) {
  (void)connHandle;

  while (bleuart.available()) {
    char c = static_cast<char>(bleuart.read());

    if (rxBuffer.length() < 512) {
      rxBuffer += c;
    }
  }

  rxPending = true;

  lastRxMillis = millis();
}

// ============================================================
// BLE verbunden
// ============================================================

void connectCallback(uint16_t connHandle) {
  BLEConnection* connection = Bluefruit.Connection(connHandle);

  char centralName[32] = { 0 };

  connection->getPeerName(centralName, sizeof(centralName));

  Serial.print("Verbunden mit: ");

  Serial.println(centralName);
}

// ============================================================
// BLE getrennt
// ============================================================

void disconnectCallback(uint16_t connHandle, uint8_t reason) {
  (void)connHandle;
  (void)reason;

  Serial.println("BLE getrennt");
}

// ============================================================
// Advertising
// ============================================================

void startAdvertising() {
  Bluefruit.Advertising.stop();

  Bluefruit.Advertising.clearData();
  Bluefruit.ScanResponse.clearData();

  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);

  Bluefruit.Advertising.addTxPower();

  Bluefruit.Advertising.addService(bleuart);

  Bluefruit.ScanResponse.addName();

  Bluefruit.Advertising.restartOnDisconnect(true);

  /*
   * Türschild:
   *
   * 1000 ms reichen völlig.
   *
   * Das Smartphone braucht eventuell einen Moment
   * länger zum Finden des Schildes, dafür muss der
   * Sender nur noch ungefähr einmal pro Sekunde aktiv
   * werden.
   */

  Bluefruit.Advertising.setIntervalMS(1000, 1000);

  Bluefruit.Advertising.start(0);

  Serial.println("BLE Advertising gestartet (1000 ms)");
}

// ============================================================
// Setup
// ============================================================

void setup() {
  Serial.begin(115200);

  delay(1000);

  Serial.println();

  Serial.println("================================");

  Serial.println(" DWD Raumschild");

  Serial.println("================================");

  // --------------------------------------------------------
  // Defaults
  // --------------------------------------------------------

  for (size_t i = 0; i < PERSON_COUNT; i++) {
    personState[i].status = DEFAULT_STATUS;

    personState[i].text = DEFAULT_TEXT;
  }

  // --------------------------------------------------------
  // Flash
  // --------------------------------------------------------

  Serial.println("Initialisiere internes Dateisystem ...");

  if (!InternalFS.begin()) {
    Serial.println("FEHLER: InternalFS konnte nicht initialisiert werden");

  } else {
    Serial.println("InternalFS bereit");

    loadState();
  }

  // --------------------------------------------------------
  // BLE
  // --------------------------------------------------------

  // Adafruit verwendet dies auch im offiziellen
  // BLE-UART-Beispiel.
  //
  // Muss VOR Bluefruit.begin() gesetzt werden.

  Bluefruit.configPrphBandwidth(BANDWIDTH_MAX);

  Bluefruit.begin();

  // automatisches blaues BLE-Blinken abschalten
  Bluefruit.autoConnLed(false);

  // 0 dBm reicht für die paar Meter am Türschild völlig
  Bluefruit.setTxPower(0);

  Bluefruit.setName(BLE_DEVICE_NAME);

  Bluefruit.Periph.setConnectCallback(connectCallback);

  Bluefruit.Periph.setDisconnectCallback(disconnectCallback);

  bleuart.begin();

  bleuart.setRxCallback(bleuartRxCallback);

  startAdvertising();

  // --------------------------------------------------------
  // Log
  // --------------------------------------------------------

  Serial.println();

  Serial.print("Raum : ");

  Serial.println(ROOM_NUMBER);

  for (size_t i = 0; i < PERSON_COUNT; i++) {
    Serial.print(PERSONS[i].id);

    Serial.print(" : ");

    Serial.print(PERSONS[i].name);

    Serial.print(" -> ");

    Serial.print(personState[i].status);

    if (personState[i].text.length() > 0) {
      Serial.print(" / ");

      Serial.print(personState[i].text);
    }

    Serial.println();
  }

  Serial.println();

  Serial.println("Warte auf BLE ...");
}

// ============================================================
// Loop
// ============================================================

void loop() {
  /*
   * Ein BLE-Kommando kann auf mehrere Pakete verteilt sein.
   * Solange innerhalb von 60 ms weitere Bytes kommen,
   * sammeln wir weiter.
   */

  if (rxPending && millis() - lastRxMillis > 60) {
    String command = rxBuffer;

    rxBuffer = "";

    rxPending = false;

    processCommand(command);
  }
}