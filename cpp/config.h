#pragma once

// ============================================================
// DWD Raumschild - lokale Konfiguration
// ============================================================

// Raum
// Wird derzeit nicht auf dem Display dargestellt,
// sondern z. B. für den BLE-Gerätenamen verwendet.
#define ROOM_NUMBER "205"

// BLE
#define BLE_DEVICE_NAME "RAUMSCHILD-205"

// Organisation
#define ORGANIZATION \
  "Meteorologisches Observatorium Lindenberg"

#define DEPARTMENT \
  "Sachgebiet 2 - Grenzschichtprozesse"

// Standardzustand, falls noch kein gespeicherter Zustand existiert
#define DEFAULT_STATUS "Anwesend"
#define DEFAULT_TEXT ""

// ------------------------------------------------------------
// Personen
// ------------------------------------------------------------

struct PersonConfig {
  const char* id;
  const char* name;
};

static const PersonConfig PERSONS[] = {
  { "P1", "Kathleen Dix" },
  { "P2", "Thomas Kaulke" }
};

static const size_t PERSON_COUNT =
  sizeof(PERSONS) / sizeof(PERSONS[0]);