// the following definition is required to enable POSIX.1b features such as
// nanosleep
#define _POSIX_C_SOURCE 199309L

#include <dbus-1.0/dbus/dbus.h>
#include <notcurses/notcurses.h>
#include <stdio.h>
#include <stdlib.h>

#include "metadata_utils.h"

/*
int
main(void)
{
  notcurses_options opts = {};
  struct notcurses* nc = notcurses_core_init(&opts, nullptr);
  if (nc == nullptr)
  {
    fprintf(stderr, "Error initializing Notcurses\n"); // NOLINT
    return EXIT_FAILURE;
  }

  struct ncplane* std = notcurses_stdplane(nc);

  ncplane_putstr(std, "Hello, world!\n");
  notcurses_render(nc);

  struct timespec ts = { .tv_sec = 2, .tv_nsec = 0 };
  nanosleep(&ts, nullptr);

  notcurses_stop(nc);

  return EXIT_SUCCESS;
}
*/

int
main(void)
{
  TrackInfo info = { 0 };
  char app_name[256] = { 0 };

  if (get_playing_app(app_name, sizeof(app_name)) == 0)
  {
    strlcpy(info.app_name, app_name, sizeof(info.app_name));
    get_metadata(app_name, &info);

    printf("App: %s\n", info.app_name);
    printf("Title: %s\n", info.title[0] ? info.title : "N/A");
    printf("Artist: %s\n", info.artist[0] ? info.artist : "N/A");
    printf("Duration: %lld ms\n", (long long)info.duration_ms);
    printf("Position: %lld ms\n", (long long)info.position_ms);
  }
  else
  {
    printf("No media player is currently playing.\n");
  }

  return EXIT_SUCCESS;
}
