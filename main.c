// the following definition is required to enable POSIX.1b features such as nanosleep
#define _POSIX_C_SOURCE 199309L

#include <notcurses/notcurses.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    notcurses_options opts = {};
    struct notcurses *nc = notcurses_core_init(&opts, NULL);
    if (nc == NULL) {
        fprintf(stderr, "Error initializing Notcurses\n"); // NOLINT
        return EXIT_FAILURE;
    }

    struct ncplane *std = notcurses_stdplane(nc);

    ncplane_putstr(std, "Hello, world!\n");
    notcurses_render(nc);

    struct timespec ts = {.tv_sec = 2, .tv_nsec = 0};
    nanosleep(&ts, NULL);

    notcurses_stop(nc);

    return EXIT_SUCCESS;
}
