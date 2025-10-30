// the following definition is required to enable POSIX.1b features such as
// nanosleep
#define _POSIX_C_SOURCE 199309L

#include <dbus-1.0/dbus/dbus.h>
#include <notcurses/notcurses.h>
#include <stdio.h>
#include <stdlib.h>

#include "metadata_utils.h"

void
render_progress_bar(char* buffer,
                    size_t len,
                    int64_t position_ms,
                    int64_t duration_ms,
                    int width)
{
  if (duration_ms <= 0 || len < 10)
    return;

  int filled = (position_ms * width) / duration_ms;
  if (filled > width)
    filled = width;
  if (filled < 0)
    filled = 0;

  int pos_sec = position_ms / 1000;
  int dur_sec = duration_ms / 1000;
  int pos_min = pos_sec / 60;
  int pos_s = pos_sec % 60;
  int dur_min = dur_sec / 60;
  int dur_s = dur_sec % 60;

  // int percent = (position_ms * 100) / duration_ms;

  int pos = 0;
  pos += snprintf(buffer + pos, len - pos, "[");

  for (int i = 0; i < width; i++)
  {
    if (pos >= (int)len - 1)
      break;
    pos += snprintf(buffer + pos, len - pos, i < filled ? "█" : "░");
  }

  pos += snprintf(buffer + pos,
                  len - pos,
                  "] (%02d:%02d / %02d:%02d)",
                  pos_min,
                  pos_s,
                  dur_min,
                  dur_s);
}

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
  notcurses_inputready_fd(nc);

  TrackInfo info = { 0 };
  char app_name[256] = { 0 };
  char progress_bar[256] = { 0 };

  while (true)
  {
    ncplane_erase(std);

    if (get_playing_app(app_name, sizeof(app_name)) == 0)
    {
      strlcpy(info.app_name, app_name, sizeof(info.app_name));
      get_metadata(app_name, &info);

      ncplane_cursor_move_yx(std, 0, 0);
      ncplane_putstr(std, "Now Playing:");

      ncplane_cursor_move_yx(std, 1, 0);
      ncplane_printf(std, "App: %s", info.app_name);

      ncplane_cursor_move_yx(std, 2, 0);
      ncplane_printf(std, "Title: %s", info.title[0] ? info.title : "N/A");

      ncplane_cursor_move_yx(std, 3, 0);
      ncplane_printf(std, "Artist: %s", info.artist[0] ? info.artist : "N/A");

      render_progress_bar(progress_bar,
                          sizeof(progress_bar),
                          info.position_ms,
                          info.duration_ms,
                          30);
      ncplane_cursor_move_yx(std, 4, 0);
      ncplane_putstr(std, progress_bar);

      notcurses_render(nc);
    }
    else
    {
      ncplane_erase(std);
      ncplane_putstr(std, "No media player is currently playing.\n");
      notcurses_render(nc);
    }

    struct timespec ts = { .tv_sec = 1, .tv_nsec = 0 };
    unsigned ch;
    ncinput ni;
    if ((ch = notcurses_get(nc, &ts, &ni)) != (unsigned)-1)
    {
      if (ch == 'q')
        break;
    }
    nanosleep(&ts, NULL);
  }

  notcurses_stop(nc);
  return EXIT_SUCCESS;
}
