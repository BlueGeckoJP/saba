#ifndef METADATA_UTILS_H
#define METADATA_UTILS_H

#include <stddef.h>
#include <stdint.h>

typedef struct
{
  char app_name[256];
  char title[512];
  char artist[256];
  int64_t duration_ms;
  int64_t position_ms;
} TrackInfo;

int
get_playing_app(char* app_name, size_t len);
void
get_metadata(const char* app_name, TrackInfo* info);

#endif // METADATA_UTILS_H
