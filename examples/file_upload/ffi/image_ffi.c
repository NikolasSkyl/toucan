#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int b64_value(char c) {
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    if (c == '=') return -2;
    return -1;
}

static const char *data_url_payload(const char *data_url) {
    const char *comma = strchr(data_url, ',');
    return comma ? comma + 1 : NULL;
}

int donna_save_data_url_file(const char *path, const char *data_url) {
    const char *src = data_url_payload(data_url);
    if (!path || !src) return -1;

    FILE *fp = fopen(path, "wb");
    if (!fp) return -1;

    int vals[4];
    int count = 0;
    int done = 0;

    for (const char *p = src; *p && !done; p++) {
        int v = b64_value(*p);
        if (v == -1) continue;

        vals[count++] = v;
        if (count != 4) continue;

        if (vals[0] < 0 || vals[1] < 0) {
            fclose(fp);
            return -1;
        }

        unsigned char out0 = (unsigned char)((vals[0] << 2) | (vals[1] >> 4));
        fwrite(&out0, 1, 1, fp);

        if (vals[2] == -2) {
            done = 1;
        } else if (vals[2] >= 0) {
            unsigned char out1 = (unsigned char)(((vals[1] & 15) << 4) | (vals[2] >> 2));
            fwrite(&out1, 1, 1, fp);

            if (vals[3] == -2) {
                done = 1;
            } else if (vals[3] >= 0) {
                unsigned char out2 = (unsigned char)(((vals[2] & 3) << 6) | vals[3]);
                fwrite(&out2, 1, 1, fp);
            } else {
                fclose(fp);
                return -1;
            }
        } else {
            fclose(fp);
            return -1;
        }

        count = 0;
    }

    fclose(fp);
    return 0;
}

static const char *mime_for_path(const char *path) {
    const char *dot = strrchr(path, '.');
    if (!dot) return "application/octet-stream";
    if (strcasecmp(dot, ".png") == 0) return "image/png";
    if (strcasecmp(dot, ".jpg") == 0) return "image/jpeg";
    if (strcasecmp(dot, ".jpeg") == 0) return "image/jpeg";
    if (strcasecmp(dot, ".gif") == 0) return "image/gif";
    if (strcasecmp(dot, ".webp") == 0) return "image/webp";
    return "application/octet-stream";
}

char *donna_file_data_url(const char *path) {
    static const char table[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    FILE *fp = fopen(path, "rb");
    if (!fp) return strdup("");

    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return strdup("");
    }
    long size = ftell(fp);
    if (size < 0) {
        fclose(fp);
        return strdup("");
    }
    rewind(fp);

    unsigned char *buf = malloc((size_t)size);
    if (!buf && size > 0) {
        fclose(fp);
        return strdup("");
    }

    size_t got = fread(buf, 1, (size_t)size, fp);
    fclose(fp);
    if (got != (size_t)size) {
        free(buf);
        return strdup("");
    }

    const char *mime = mime_for_path(path);
    size_t prefix_len = strlen("data:") + strlen(mime) + strlen(";base64,");
    size_t b64_len = ((got + 2) / 3) * 4;
    char *out = malloc(prefix_len + b64_len + 1);
    if (!out) {
        free(buf);
        return strdup("");
    }

    sprintf(out, "data:%s;base64,", mime);
    char *dst = out + prefix_len;
    for (size_t i = 0; i < got; i += 3) {
        unsigned int triple = (unsigned int)buf[i] << 16;
        int remain = (int)(got - i);
        if (remain > 1) triple |= (unsigned int)buf[i + 1] << 8;
        if (remain > 2) triple |= (unsigned int)buf[i + 2];

        *dst++ = table[(triple >> 18) & 63];
        *dst++ = table[(triple >> 12) & 63];
        *dst++ = remain > 1 ? table[(triple >> 6) & 63] : '=';
        *dst++ = remain > 2 ? table[triple & 63] : '=';
    }
    *dst = '\0';

    free(buf);
    return out;
}
