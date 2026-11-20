#pragma once

#define ROOM_NUMBER "123"
#define BLE_DEVICE_NAME "ble-device-name room " ROOM_NUMBER
#define ORGANIZATION "organization"
#define DEPARTMENT "department"

#define STATUS_FONT_FAMILY FreeSansBold
#define STATUS_FONT_SIZE 12
#define STATUS_Y_OFFSET 93

#define FREETEXT_FONT_FAMILY FreeSansBold
#define FREETEXT_FONT_SIZE 9
#define FREETEXT_Y_OFFSET 125
#define FREETEXT_LINE_HEIGHT 25

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