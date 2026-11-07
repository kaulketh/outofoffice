#pragma once

// ============================================================
// Raumschild - lokale Konfiguration
// ============================================================

// Raum
// Wird derzeit nicht auf dem Display dargestellt,
// sondern z. B. für den BLE-Gerätenamen verwendet.
#define ROOM_NUMBER "123"

// BLE
#define BLE_DEVICE_NAME "ble-device-name"

// Organisation
#define ORGANIZATION "organization"

#define DEPARTMENT "department"

// Standardzustand, falls noch kein gespeicherter Zustand existiert
#define DEFAULT_STATUS "out of office"
#define DEFAULT_TEXT ""

// ------------------------------------------------------------
// Personen
// ------------------------------------------------------------

struct PersonConfig {
  const char* id;
  const char* name;
};

static const PersonConfig PERSONS[] = {
  { "P1", "person 1" },
  { "P2", "person 2" }
};

static const size_t PERSON_COUNT =
  sizeof(PERSONS) / sizeof(PERSONS[0]);