#pragma once


#define ROOM_NUMBER "123"

#define BLE_DEVICE_NAME "ble-device-name room " ROOM_NUMBER

#define ORGANIZATION "organization"

#define DEPARTMENT "department"

#define DEFAULT_STATUS "out of office"
#define DEFAULT_TEXT ""

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