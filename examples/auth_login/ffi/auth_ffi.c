/*
 * auth_ffi.c — password hashing for the auth_login example.
 *
 * Uses a simple DJB2-based hash with a random salt and 10 000 iterations.
 * Good enough for a demo; use bcrypt/argon2 in production.
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static unsigned long djb2(const char *s) {
    unsigned long h = 5381;
    while (*s) h = h * 33 ^ (unsigned char)*s++;
    return h;
}

char *donna_generate_salt(void) {
    unsigned char buf[8];
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd >= 0) {
        ssize_t n = read(fd, buf, sizeof(buf));
        close(fd);
        if (n != (ssize_t)sizeof(buf)) {
            srand((unsigned)time(NULL));
            for (int i = 0; i < 8; i++) buf[i] = (unsigned char)(rand() & 0xff);
        }
    } else {
        srand((unsigned)time(NULL));
        for (int i = 0; i < 8; i++) buf[i] = (unsigned char)(rand() & 0xff);
    }
    char *out = (char *)malloc(17);
    if (!out) return strdup("");
    for (int i = 0; i < 8; i++) sprintf(out + i * 2, "%02x", buf[i]);
    out[16] = '\0';
    return out;
}

char *donna_hash_password(const char *password, const char *salt) {
    char buf[1024];
    snprintf(buf, sizeof(buf), "%s%s", salt, password);
    unsigned long h = djb2(buf);
    for (int i = 0; i < 9999; i++) {
        snprintf(buf, sizeof(buf), "%lu%s%s", h, salt, password);
        h = djb2(buf);
    }
    char *out = (char *)malloc(17);
    if (!out) return strdup("");
    snprintf(out, 17, "%016lx", h);
    return out;
}
