#define _GNU_SOURCE
#include <unistd.h>   // read, write
#include <string.h>   // strlen, memmem, memmove
#include <stdlib.h>   // malloc, realloc, free
#include <stdio.h>    // perror
#include <errno.h>

#define READ_CHUNK 4096

static int write_all(int fd, const void *buf, size_t len) {
    const char *p = (const char *)buf;
    while (len > 0) {
        ssize_t w = write(fd, p, len);
        if (w < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        p += (size_t)w;
        len -= (size_t)w;
    }
    return 0;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        return 1;
    }
    const char *pat = argv[1];
    size_t m = strlen(pat);
    if (m == 0) {
        return 1;
    }

    char *acc = NULL;      // accumulation buffer holding unflushed data
    size_t acc_len = 0;    // bytes currently in acc
    size_t acc_cap = 0;    // capacity of acc

    char inbuf[READ_CHUNK];

    for (;;) {
        ssize_t r = read(STDIN_FILENO, inbuf, sizeof(inbuf));
        if (r < 0) {
            perror("Error");
            free(acc);
            return 1;
        }

        if (r == 0) {
            // EOF: process any remaining matches and flush all
            // First, replace remaining matches entirely within acc
            for (;;) {
                void *match = memmem(acc, acc_len, pat, m);
                if (!match) break;
                size_t pos = (size_t)((char *)match - acc);
                // write bytes before match
                if (pos > 0) {
                    if (write_all(STDOUT_FILENO, acc, pos) < 0) {
                        // Not specified, but treat as error
                        perror("Error");
                        free(acc);
                        return 1;
                    }
                }
                // write replacement asterisks
                for (size_t i = 0; i < m; ++i) {
                    if (write(STDOUT_FILENO, "*", 1) < 0) {
                        perror("Error");
                        free(acc);
                        return 1;
                    }
                }
                // remove processed part [0 .. pos+m)
                size_t remain = acc_len - (pos + m);
                memmove(acc, acc + pos + m, remain);
                acc_len = remain;
            }
            // flush the rest
            if (acc_len > 0) {
                if (write_all(STDOUT_FILENO, acc, acc_len) < 0) {
                    perror("Error");
                    free(acc);
                    return 1;
                }
            }
            free(acc);
            return 0;
        }

        // Append read bytes to accumulation buffer
        size_t need = acc_len + (size_t)r;
        if (need > acc_cap) {
            size_t new_cap = acc_cap ? acc_cap : (READ_CHUNK + m + 16);
            while (new_cap < need) new_cap *= 2;
            char *tmp = (char *)realloc(acc, new_cap);
            if (!tmp) {
                perror("Error");
                free(acc);
                return 1;
            }
            acc = tmp;
            acc_cap = new_cap;
        }
        memmove(acc + acc_len, inbuf, (size_t)r);
        acc_len += (size_t)r;

        // Replace matches present in acc; after each replacement, compact acc
        for (;;) {
            void *match = memmem(acc, acc_len, pat, m);
            if (!match) break;
            size_t pos = (size_t)((char *)match - acc);
            // write preceding bytes
            if (pos > 0) {
                if (write_all(STDOUT_FILENO, acc, pos) < 0) {
                    perror("Error");
                    free(acc);
                    return 1;
                }
            }
            // write replacement asterisks for the match
            for (size_t i = 0; i < m; ++i) {
                if (write(STDOUT_FILENO, "*", 1) < 0) {
                    perror("Error");
                    free(acc);
                    return 1;
                }
            }
            // Remove processed data up to end of match
            size_t remain = acc_len - (pos + m);
            memmove(acc, acc + pos + m, remain);
            acc_len = remain;
        }

        // After consuming all full matches, keep at most (m-1) trailing bytes in acc
        if (acc_len > m - 1) {
            size_t flush_len = acc_len - (m - 1);
            if (flush_len > 0) {
                if (write_all(STDOUT_FILENO, acc, flush_len) < 0) {
                    perror("Error");
                    free(acc);
                    return 1;
                }
                // Keep only the last (m-1) bytes
                memmove(acc, acc + flush_len, m - 1);
                acc_len = m - 1;
            }
        }
    }
}
