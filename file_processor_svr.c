#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#define FIFO_C2S "/tmp/fp_c2s.fifo"
#define FIFO_S2C "/tmp/fp_s2c.fifo"
#define BUFSZ 1024

static int word_count(const char *s) {
    int cnt = 0, inword = 0;
    for (const char *p = s; *p; ++p) {
        if (!isspace((unsigned char)*p)) {
            if (!inword) { cnt++; inword = 1; }
        } else {
            inword = 0;
        }
    }
    return cnt;
}

static void str_upper(char *s) {
    for (; *s; ++s) *s = (char)toupper((unsigned char)*s);
}

static void str_lower(char *s) {
    for (; *s; ++s) *s = (char)tolower((unsigned char)*s);
}

static void str_reverse(char *s) {
    size_t n = strlen(s);
    for (size_t i = 0; i < n/2; ++i) {
        char t = s[i]; s[i] = s[n-1-i]; s[n-1-i] = t;
    }
}

static void chomp(char *s) {
    size_t n = strlen(s);
    if (n && (s[n-1] == '\n' || s[n-1] == '\r')) s[n-1] = '\0';
}

int main(void) {
    mkfifo(FIFO_C2S, 0666);
    mkfifo(FIFO_S2C, 0666);

    int fd_in  = open(FIFO_C2S, O_RDONLY);
    int fd_out = open(FIFO_S2C, O_WRONLY);

    FILE *in  = fdopen(fd_in,  "r");
    FILE *out = fdopen(fd_out, "w");

    char buf[BUFSZ];
    char mode[32] = {0};

    if (fgets(mode, sizeof(mode), in) == NULL) return 0;
    chomp(mode);

    int line_no = 0;
    while (fgets(buf, sizeof(buf), in)) {
        chomp(buf);
        if (strcmp(buf, "END") == 0) {
            break;
        }

        line_no++;
        printf("%d번째 줄 처리 중...\n", line_no);

        char outbuf[BUFSZ];
        if (strcmp(mode, "count") == 0) {
            int chars = (int)strlen(buf);
            int words = word_count(buf);
            snprintf(outbuf, sizeof(outbuf), "Line %d: %d chars, %d words", line_no, chars, words);
        }
        else if (strcmp(mode, "upper") == 0) {
            char tmp[BUFSZ]; strncpy(tmp, buf, sizeof(tmp)); tmp[sizeof(tmp)-1] = '\0';
            str_upper(tmp);
            snprintf(outbuf, sizeof(outbuf), "%s", tmp);
        }
        else if (strcmp(mode, "lower") == 0) {
            char tmp[BUFSZ]; strncpy(tmp, buf, sizeof(tmp)); tmp[sizeof(tmp)-1] = '\0';
            str_lower(tmp);
            snprintf(outbuf, sizeof(outbuf), "%s", tmp);
        }
        else if (strcmp(mode, "reverse") == 0) {
            char tmp[BUFSZ]; strncpy(tmp, buf, sizeof(tmp)); tmp[sizeof(tmp)-1] = '\0';
            str_reverse(tmp);
            snprintf(outbuf, sizeof(outbuf), "%s", tmp);
        }
        fprintf(out, "%s\n", outbuf);
        fflush(out);
    }
    return 0;
}
