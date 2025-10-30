// the following definition is required to enable POSIX.1b features such as
// nanosleep
#define _POSIX_C_SOURCE 199309L

#include <dbus-1.0/dbus/dbus.h>
#include <notcurses/notcurses.h>
#include <stdio.h>
#include <stdlib.h>

#include "metadata_utils.h"

int
main(void)
{
  notcurses_options opts = { 0 };
  struct notcurses* nc = notcurses_core_init(&opts, NULL);
  if (nc == nullptr)
  {
    fprintf(stderr, "Error initializing Notcurses\n");
    return EXIT_FAILURE;
  }

  struct ncplane* std = notcurses_stdplane(nc);

  TrackInfo info = { 0 };
  char app_name[256] = { 0 };

  if (get_playing_app(app_name, sizeof(app_name)) == 0)
  {
    strlcpy(info.app_name, app_name, sizeof(info.app_name));
    get_metadata(app_name, &info);

    ncplane_putstr(std, "Now Playing:");
    ncplane_cursor_move_yx(std, 0, 0);

    ncplane_printf(std, "App: %s", info.app_name);
    ncplane_cursor_move_yx(std, 1, 0);

    ncplane_printf(std, "Title: %s", info.title[0] ? info.title : "N/A");
    ncplane_cursor_move_yx(std, 2, 0);

    ncplane_printf(std, "Artist: %s", info.artist[0] ? info.artist : "N/A");
    ncplane_cursor_move_yx(std, 3, 0);

    ncplane_printf(std, "Duration: %lld ms", (long long)info.duration_ms);
    ncplane_cursor_move_yx(std, 4, 0);

    ncplane_printf(std, "Position: %lld ms", (long long)info.position_ms);

    notcurses_render(nc);
  }
  else
  {
    printf("No media player is currently playing.\n");
  };

  struct timespec ts = { .tv_sec = 5, .tv_nsec = 0 };
  nanosleep(&ts, NULL);

  notcurses_stop(nc);
  return EXIT_SUCCESS;
}
