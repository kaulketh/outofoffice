#include <Adafruit_LittleFS.h>
#include <Adafruit_TinyUSB.h>
#include <Adafruit_GFX.h>
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeMono12pt7b.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSansBold18pt7b.h>
#include <GxEPD2_BW.h>
#include <InternalFileSystem.h>
#include <bluefruit.h>
#include "configuration.h"

#define FONT_NAME_IMPL(family, size) family##size##pt7b
#define FONT_NAME(family, size) FONT_NAME_IMPL(family, size)

using namespace Adafruit_LittleFS_Namespace;

constexpr const char* STATE_FILE = "/sign.txt";

constexpr uint8_t EPD_CS = D1;
constexpr uint8_t EPD_DC = D2;
constexpr uint8_t EPD_RST = D3;
constexpr uint8_t EPD_BUSY = D4;
constexpr uint8_t EPD_PWR = D5;

// Waveshare 5.83" V2, Schwarz/Weiss, 648 x 480
GxEPD2_BW<GxEPD2_583_T8, GxEPD2_583_T8::HEIGHT / 2> display(
GxEPD2_583_T8(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY));

BLEUart bleuart(512);

String rxBuffer;
bool rxPending = false;
bool rxOverflow = false;
unsigned long lastRxMillis = 0;

constexpr size_t RX_MAX_LENGTH = 512;
constexpr unsigned long LEGACY_RX_TIMEOUT_MS = 60;

constexpr size_t BLE_TX_CHUNK_SIZE = 16;
constexpr uint16_t BLE_TX_PAUSE_MS = 25;
constexpr uint8_t BLE_TX_MAX_RETRIES = 20;

struct PersonState {
    String status;
    String text;
};

PersonState personState[PERSON_COUNT];

PersonState pendingState[PERSON_COUNT];

bool transactionActive = false;
volatile bool disconnectPending = false;

unsigned long transactionLastActivity = 0;
constexpr unsigned long TRANSACTION_TIMEOUT_MS = 30000;

int findPersonIndex(const String& id) {
    for (size_t i = 0; i < PERSON_COUNT; i++) {
        if (id == PERSONS[i].id) {
            return i;
        }
    }
    return -1;
}

bool saveState(const PersonState* states = personState) {
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
        file.print(states[i].status);
        file.print("|");
        file.println(states[i].text);
    }
    file.close();
    Serial.println("Status im Flash gespeichert");
    return true;
}

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

float readBatteryVoltage() {
    pinMode(VBAT_ENABLE, OUTPUT);
    digitalWrite(VBAT_ENABLE, LOW);
    delay(10);

    // nRF52 ADC bewusst auf 3,0 V und 12 Bit einstellen.
    analogReference(AR_INTERNAL_3_0);
    analogReadResolution(12);
    // Der hochohmige Spannungsteiler benötigt eine längere Sample-Zeit.
    analogSampleTime(40);
    // Die erste Messung nach der ADC-Konfiguration verwerfen.
    analogRead(PIN_VBAT);

    constexpr uint8_t SAMPLE_COUNT = 8;
    uint32_t sum = 0;
    for (uint8_t i = 0; i < SAMPLE_COUNT; i++) {
        sum += analogRead(PIN_VBAT);
        delay(2);
    }
    const float raw = static_cast<float>(sum) / SAMPLE_COUNT;

    // XIAO-Spannungsteiler: 1 MOhm + 510 kOhm, Faktor 1510 / 510 ≈ 2,9608.
    constexpr float ADC_REF = 3.0f;
    constexpr float ADC_MAX = 4095.0f;
    constexpr float VBAT_DIVIDER = 1510.0f / 510.0f;
    const float adcVoltage = raw * ADC_REF / ADC_MAX;
    const float batteryVoltage = adcVoltage * VBAT_DIVIDER;

    pinMode(VBAT_ENABLE, INPUT);
    analogReference(AR_DEFAULT);
    analogReadResolution(10);
    analogSampleTime(3);
    return batteryVoltage;
}


struct LipoPoint {
    float voltage;
    float percent;
};

const LipoPoint lipoTable[] = {
    {3.50f, 0.0f},
    {3.60f, 5.0f},
    {3.70f, 15.0f},
    {3.75f, 25.0f},
    {3.80f, 40.0f},
    {3.84f, 50.0f},
    {3.87f, 60.0f},
    {3.95f, 70.0f},
    {4.02f, 80.0f},
    {4.11f, 90.0f},
    {4.15f, 95.0f},
    {4.20f, 100.0f}
};

constexpr int lipoTableSize = sizeof(lipoTable) / sizeof(lipoTable[0]);

/**
 * Converts a measured 1S LiPo battery voltage into a rounded state-of-charge
 * percentage (0..100) using piecewise linear interpolation over `lipoTable`.
 *
 * - Clamps values at or below the lowest voltage point to 0%, and at or above
 *   the highest voltage point to 100%.
 * - Assumes `lipoTable` is sorted in ascending order by voltage with distinct
 *   voltage values (no duplicate voltages, which would cause division by zero).
 * - Interpolates linearly between adjacent points (a, b) enclosing `voltage`,
 *   then rounds to the nearest integer via `+ 0.5f`.
 * - Falls back to 0 if no interval matches (e.g., if `voltage` is NaN).
 */
int batteryPercent(float voltage) {
    if (voltage <= lipoTable[0].voltage) return 0;
    if (voltage >= lipoTable[lipoTableSize - 1].voltage) return 100;

    for (int i = 0; i < lipoTableSize - 1; i++) {
        const LipoPoint& a = lipoTable[i];
        const LipoPoint& b = lipoTable[i + 1];

        if (voltage >= a.voltage && voltage <= b.voltage) {
            const float percent = a.percent +
            (voltage - a.voltage) *
            (b.percent - a.percent) /
            (b.voltage - a.voltage);

            return static_cast<int>(percent + 0.5f);
        }
    }
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

uint16_t textWidth(const String& text) {
    int16_t x1;
    int16_t y1;
    uint16_t w;
    uint16_t h;
    display.getTextBounds(text.c_str(), 0, 0, &x1, &y1, &w, &h);
    return w;
}

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


void drawPerson(size_t index, int16_t top, int16_t height) {
    constexpr int16_t LEFT = 28;
    constexpr int16_t RIGHT = 28;
    const uint16_t usableWidth = display.width() - LEFT - RIGHT;

    display.setFont(&FreeSansBold18pt7b);
    display.setCursor(LEFT, top + 56);
    display.print(PERSONS[index].name);

    String status = personState[index].status;
    if (status.length() > 0) {
        status.toUpperCase();
        display.setFont(&FONT_NAME(STATUS_FONT_FAMILY, STATUS_FONT_SIZE));
        display.setCursor(LEFT, top + STATUS_Y_OFFSET);
        display.print(status);
    }

    if (personState[index].text.length() > 0) {
        display.setFont(&FONT_NAME(FREETEXT_FONT_FAMILY, FREETEXT_FONT_SIZE));
        drawWrappedText(
        personState[index].text,
        LEFT,
        top + FREETEXT_Y_OFFSET,
        usableWidth,
        2,
        FREETEXT_LINE_HEIGHT
        );
    }

    const int16_t y = top + height - 1;
    const uint8_t lineThickness = 3;
    display.fillRect(LEFT, y, usableWidth, lineThickness, GxEPD_BLACK);
}


void drawFooter(int16_t top) {
    constexpr int16_t LEFT = 28;
    uint16_t w, h;
    display.drawLine(LEFT, top, display.width() - LEFT, top, GxEPD_BLACK);
    display.drawLine(LEFT, top + 1, display.width() - LEFT, top + 1, GxEPD_BLACK);

    const char* line1 = DEPARTMENT;
    display.setFont(&FreeSansBold18pt7b);
    drawCentered(line1, top + 70);

    const char* line2 = ORGANIZATION;
    display.setFont(&FreeSansBold12pt7b);
    drawCentered(line2, top + 105);
}

void drawRoomSign() {
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    constexpr int16_t FOOTER_HEIGHT = 160;
    const int16_t personsHeight = display.height() - FOOTER_HEIGHT;
    const int16_t personHeight = personsHeight / PERSON_COUNT;
    for (size_t i = 0; i < PERSON_COUNT; i++) {
        drawPerson(i, i * personHeight, personHeight);
    }
    drawFooter(personsHeight);
}

void updateDisplay() {
    Serial.println("E-Paper wird aktualisiert ...");
    pinMode(EPD_PWR, OUTPUT);
    digitalWrite(EPD_PWR, HIGH);
    delay(100);
    display.init(115200, true, 10, false);
    display.setRotation(2);  // 180°
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
        // Dem Notification-Puffer Zeit zum Leeren geben.
        delay(BLE_TX_PAUSE_MS);
    }
    return true;
}

void sendLine(const String& text) {
    Serial.println(text);
    if (!Bluefruit.connected()) {
        return;
    }
    String line = text + "\n";
    sendBleData(line);
}

void sendInfo() {
    sendLine("INFO_BEGIN");
    sendLine(String("ROOM|") + ROOM_NUMBER);
    sendLine(String("ORG|") + ORGANIZATION);
    sendLine(String("DEPT|") + DEPARTMENT);
    for (size_t i = 0; i < PERSON_COUNT; i++) {
        sendLine(String("PERSON|") + PERSONS[i].id + "|" + PERSONS[i].name);
    }
    sendBatteryStatus();
    sendLine("CAP|TRANSACTION");
    sendLine("INFO_END");
}

void sendStates() {
    sendLine("STATE_BEGIN");
    for (size_t i = 0; i < PERSON_COUNT; i++) {
        sendLine(String("STATE|") + PERSONS[i].id + "|" + PERSONS[i].name + "|" + personState[i].status + "|" + personState[i].text);
    }
    sendLine("STATE_END");
}

void abortTransaction() {
    transactionActive = false;
    transactionLastActivity = 0;

    for (size_t i = 0; i < PERSON_COUNT; i++) {
        pendingState[i].status = "";
        pendingState[i].text = "";
    }
}

void beginTransaction() {
    if (transactionActive) {
        sendLine("ERR|TRANSACTION_ACTIVE");
        return;
    }

    for (size_t i = 0; i < PERSON_COUNT; i++) {
        pendingState[i] = personState[i];
    }

    transactionActive = true;
    transactionLastActivity = millis();

    sendLine("OK|BEGIN");
}

void commitTransaction() {
    if (!transactionActive) {
        sendLine("ERR|NO_TRANSACTION");
        return;
    }

    if (disconnectPending || !Bluefruit.connected()) {
        abortTransaction();
        return;
    }

    if (!saveState(pendingState)) {
        abortTransaction();
        sendLine("ERR|SAVE_FAILED");
        return;
    }

    for (size_t i = 0; i < PERSON_COUNT; i++) {
        personState[i] = pendingState[i];
    }

    abortTransaction();

    sendLine("OK|COMMIT");
    sendLine("REFRESHING");
    updateDisplay();
    sendLine("READY");
}

void processCommand(String command) {
    command.trim();
    if (command.length() == 0) {
        return;
    }
    Serial.print("Kommando: ");
    Serial.println(command);
    if (command == "INFO") {
        sendInfo();
        return;
    }
    if (command == "GET") {
        sendStates();
        return;
    }
    if (command == "BAT") {
        sendBatteryStatus();
        return;
    }
    if (command == "BEGIN") {
        beginTransaction();
        return;
    }
    if (command == "COMMIT") {
        commitTransaction();
        return;
    }
    if (command == "ABORT") {
        abortTransaction();
        sendLine("OK|ABORT");
        return;
    }
    if (command == "REDRAW") {
        sendLine("REFRESHING");
        updateDisplay();
        sendLine("READY");
        return;
    }
    // Format: SET|P1|Urlaub|bis 18.11.
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
    if (transactionActive) {
        pendingState[index].status = status;
        pendingState[index].text = text;
        transactionLastActivity = millis();

        sendLine(String("OK|STAGED|") + personId);
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



void connectCallback(uint16_t connHandle) {
    BLEConnection* connection = Bluefruit.Connection(connHandle);
    char centralName[32] = { 0 };
    connection->getPeerName(centralName, sizeof(centralName));
    Serial.print("Verbunden mit: ");
    Serial.println(centralName);
}

void disconnectCallback(uint16_t connHandle, uint8_t reason) {
    (void)connHandle;
    (void)reason;
    Serial.println("BLE getrennt");
    disconnectPending = true;
}

void startAdvertising() {
    Bluefruit.Advertising.stop();
    Bluefruit.Advertising.clearData();
    Bluefruit.ScanResponse.clearData();
    Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
    Bluefruit.Advertising.addTxPower();
    Bluefruit.Advertising.addService(bleuart);
    Bluefruit.ScanResponse.addName();
    Bluefruit.Advertising.restartOnDisconnect(true);
    // 1000 ms reduzieren die Senderaktivität; das Smartphone findet das Schild ggf. etwas später.
    Bluefruit.Advertising.setIntervalMS(1000, 1000);
    Bluefruit.Advertising.start(0);
    Serial.println("BLE Advertising gestartet (1000 ms)");
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println();
    Serial.println("================================");
    Serial.println(" Raumschild");
    Serial.println("================================");

    for (size_t i = 0; i < PERSON_COUNT; i++) {
        personState[i].status = DEFAULT_STATUS;
        personState[i].text = DEFAULT_TEXT;
    }

    Serial.println("Initialisiere internes Dateisystem ...");
    if (!InternalFS.begin()) {
        Serial.println("FEHLER: InternalFS konnte nicht initialisiert werden");
    } else {
        Serial.println("InternalFS bereit");
        loadState();
    }

    // Muss vor Bluefruit.begin() gesetzt werden.
    Bluefruit.configPrphBandwidth(BANDWIDTH_MAX);
    Bluefruit.begin();
    Bluefruit.autoConnLed(false);
    Bluefruit.setTxPower(0);
    Bluefruit.setName(BLE_DEVICE_NAME);
    Bluefruit.Periph.setConnectCallback(connectCallback);
    Bluefruit.Periph.setDisconnectCallback(disconnectCallback);
    bleuart.begin();

    startAdvertising();

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

void resetRxBuffer() {
    rxBuffer = "";
    rxPending = false;
    rxOverflow = false;
}

void processReceivedCommand(bool framed) {
    String command = rxBuffer;
    bool overflow = rxOverflow;

    resetRxBuffer();

    if (!Bluefruit.connected() || disconnectPending) {
        return;
    }

    if (overflow) {
        abortTransaction();
        sendLine("ERR|RX_OVERFLOW");
        return;
    }

    if (command.startsWith("TX|")) {
        if (!framed) {
            abortTransaction();
            sendLine("ERR|INCOMPLETE_COMMAND");
            return;
        }

        command = command.substring(3);
    } else if (transactionActive ||
    command == "BEGIN" ||
    command == "COMMIT" ||
    command == "ABORT") {
        abortTransaction();
        sendLine("ERR|TRANSACTION_FRAME_REQUIRED");
        return;
    }

    processCommand(command);
}

void loop() {
    if (disconnectPending) {
        disconnectPending = false;

        resetRxBuffer();

        while (bleuart.available()) {
            bleuart.read();
        }

        if (transactionActive) {
            abortTransaction();
            Serial.println("Transaktion wegen BLE-Abbruch verworfen");
        }
    }

    if (transactionActive &&
    millis() - transactionLastActivity > TRANSACTION_TIMEOUT_MS) {
        abortTransaction();
        resetRxBuffer();
        sendLine("ERR|TRANSACTION_TIMEOUT");
    }

    while (bleuart.available() &&
    Bluefruit.connected() &&
    !disconnectPending) {
        int value = bleuart.read();

        if (value < 0) {
            break;
        }

        char c = static_cast<char>(value);

        if (c == '\n') {
            processReceivedCommand(true);
            continue;
        }

        if (rxBuffer.length() < RX_MAX_LENGTH) {
            rxBuffer += c;
        } else {
            rxOverflow = true;
        }

        rxPending = true;
        lastRxMillis = millis();
    }

    if (rxPending &&
    millis() - lastRxMillis > LEGACY_RX_TIMEOUT_MS &&
    !rxBuffer.startsWith("TX|")) {
        processReceivedCommand(false);
    }
}
