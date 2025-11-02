#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/time.h>

#define FIFO_C2S "/tmp/fp_c2s.fifo"
#define FIFO_S2C "/tmp/fp_s2c.fifo"
#define BUFSZ 1024

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "사용법: %s <input_file> <mode>\n", argv[0]);
        return 1;
    }

    const char *infile = argv[1];
    const char *mode   = argv[2];

    mkfifo(FIFO_C2S, 0666);
    mkfifo(FIFO_S2C, 0666);

    int fd_out = open(FIFO_C2S, O_WRONLY);
    int fd_in  = open(FIFO_S2C, O_RDONLY);
    if (fd_out == -1 || fd_in == -1) {
        perror("FIFO open 실패");
        return 1;
    }

    FILE *to_srv   = fdopen(fd_out, "w");
    FILE *from_srv = fdopen(fd_in,  "r");
    FILE *fp = fopen(infile, "r");
    if (!fp) {
        perror("입력 파일 열기 실패");
        return 1;
    }

    fprintf(to_srv, "%s\n", mode);
    fflush(to_srv);

    struct timeval t0, t1;
    gettimeofday(&t0, NULL);

    char line[BUFSZ];
    char resp[BUFSZ];
    int count = 0;

    while (fgets(line, sizeof(line), fp)) {
        count++;
        printf("%d번째 줄 전송...\n", count);

        size_t len = strlen(line);
        if (len == 0 || line[len - 1] != '\n') {
            fprintf(to_srv, "%s\n", line);
        } else {
            fputs(line, to_srv);
        }
        fflush(to_srv);

        if (fgets(resp, sizeof(resp), from_srv)) {
            size_t rlen = strlen(resp);
            if (rlen > 0 && resp[rlen - 1] == '\n') {
                printf("%d번째 줄 결과 수신: %s", count, resp);
            } else {
                printf("%d번째 줄 결과 수신: %s\n", count, resp);
            }
        }
    }

    fprintf(to_srv, "END\n");
    fflush(to_srv);

    gettimeofday(&t1, NULL);
    double elapsed = (t1.tv_sec - t0.tv_sec) + (t1.tv_usec - t0.tv_usec) / 1000000.0;

    printf("=== 처리 통계 ===\n");
    printf("처리 모드: %s\n", mode);
    printf("처리한 줄 수: %d줄\n", count);
    printf("소요 시간: %.2f초\n", elapsed);

    fclose(fp);
    fclose(to_srv);
    fclose(from_srv);
    return 0;
}
