/*
 * Toucan session store — global in-process store.
 *
 * A linked list of sessions, each holding a linked list of key-value pairs.
 * Using a module-global pointer avoids any concerns about passing opaque
 * handles across the Donna FFI boundary.
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

typedef struct kv {
    char      *key;
    char      *value;
    struct kv *next;
} kv;

typedef struct session {
    char           *id;
    kv             *data;
    struct session *next;
} session;

static session *g_sessions = NULL;

static char *xs(const char *s) {
    return strdup(s ? s : "");
}

void donna_session_init(void) {
    g_sessions = NULL;
}

char *donna_session_generate_id(void) {
    unsigned char buf[16];
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd >= 0) {
        ssize_t n = read(fd, buf, sizeof(buf));
        close(fd);
        if (n != (ssize_t)sizeof(buf)) {
            srand((unsigned)time(NULL));
            for (int i = 0; i < 16; i++) buf[i] = (unsigned char)(rand() & 0xff);
        }
    } else {
        srand((unsigned)time(NULL));
        for (int i = 0; i < 16; i++) buf[i] = (unsigned char)(rand() & 0xff);
    }
    char *out = (char *)malloc(33);
    if (!out) return xs("");
    for (int i = 0; i < 16; i++) sprintf(out + i * 2, "%02x", buf[i]);
    out[32] = '\0';
    return out;
}

static session *find_session(const char *id) {
    session *s = g_sessions;
    while (s) {
        if (strcmp(s->id, id) == 0) return s;
        s = s->next;
    }
    return NULL;
}

static session *get_or_create(const char *id) {
    session *s = find_session(id);
    if (s) return s;
    s = (session *)calloc(1, sizeof(session));
    s->id   = xs(id);
    s->next = g_sessions;
    g_sessions = s;
    return s;
}

char *donna_session_get(const char *sid, const char *key) {
    if (!sid || !key) return xs("");
    session *s = find_session(sid);
    if (!s) return xs("");
    kv *k = s->data;
    while (k) {
        if (strcmp(k->key, key) == 0) return xs(k->value);
        k = k->next;
    }
    return xs("");
}

long donna_session_put(const char *sid, const char *key, const char *value) {
    if (!sid || !key || !value) return -1;
    session *s = get_or_create(sid);
    kv *k = s->data;
    while (k) {
        if (strcmp(k->key, key) == 0) {
            free(k->value);
            k->value = xs(value);
            return 0;
        }
        k = k->next;
    }
    kv *nk    = (kv *)calloc(1, sizeof(kv));
    nk->key   = xs(key);
    nk->value = xs(value);
    nk->next  = s->data;
    s->data   = nk;
    return 0;
}

long donna_session_destroy(const char *sid) {
    if (!sid) return -1;
    session **prev = &g_sessions;
    session  *s    = g_sessions;
    while (s) {
        if (strcmp(s->id, sid) == 0) {
            *prev = s->next;
            kv *k = s->data;
            while (k) {
                kv *nk = k->next;
                free(k->key);
                free(k->value);
                free(k);
                k = nk;
            }
            free(s->id);
            free(s);
            return 0;
        }
        prev = &s->next;
        s    = s->next;
    }
    return 0;
}
