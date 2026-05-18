#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

char *donna_random_slug(void) {
    static const char chars[] = "abcdefghijklmnopqrstuvwxyz0123456789";
    unsigned char buf[6];
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd >= 0) {
        ssize_t n = read(fd, buf, sizeof(buf));
        close(fd);
        if (n != (ssize_t)sizeof(buf)) {
            srand((unsigned)time(NULL));
            for (int i = 0; i < 6; i++) buf[i] = (unsigned char)(rand() & 0xff);
        }
    } else {
        srand((unsigned)time(NULL));
        for (int i = 0; i < 6; i++) buf[i] = (unsigned char)(rand() & 0xff);
    }
    char *out = (char *)malloc(7);
    if (!out) return (char *)malloc(1);
    for (int i = 0; i < 6; i++) out[i] = chars[buf[i] % 36];
    out[6] = '\0';
    return out;
}
