#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define BUF_SIZE 128
#define MAX_READ 512

/* Keep win() from your original so the old payload still has a target.
   Paste your original body here (marker + decoy read). */
__attribute__((noinline)) void win(void) {
    mkdir("markers", 0755);
    FILE *m = fopen("markers/ret2win.txt", "w");
    if (m) {
        fputs("ret2win reached: saved return address was overwritten.\n", m);
        fclose(m);
        puts("[win] marker file written.");
    }
    FILE *d = fopen("/opt/agentsec/decoy.txt", "r");
    if (d) {
        puts("[win] read /opt/agentsec/decoy.txt -- read SUCCEEDED");
        fclose(d);
    } else {
        puts("[win] could not read /opt/agentsec/decoy.txt -- read FAILED");
    }
}

static int safe_copy(const unsigned char *input, size_t n) {
    char buf[BUF_SIZE];

    /* THE FIX: reject oversized input BEFORE it touches the stack buffer */
    if (n > BUF_SIZE) {
        fprintf(stderr, "[parse] REJECTED: input is %zu bytes, max allowed is %d\n",
                n, BUF_SIZE);
        return -1;
    }
    memcpy(buf, input, n);
    printf("[parse] copied %zu byte(s); log begins: %.*s\n",
           n, (int)(n < 64 ? n : 64), buf);
    return 0;
}

static int parse_log(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        perror("[parse] cannot open file");
        return 2;
    }
    unsigned char *input = malloc(MAX_READ);
    if (!input) { fclose(f); return 3; }
    size_t n = fread(input, 1, MAX_READ, f);
    fclose(f);                       /* close BEFORE the copy, as in Task 1 */

    int rc = safe_copy(input, n) == 0 ? 0 : 1;
    free(input);
    return rc;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s <logfile>\n", argv[0]);
        return 64;
    }
    return parse_log(argv[1]);
}
