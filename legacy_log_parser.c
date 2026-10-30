/* legacy_log_parser.c
 *
 * Intentionally vulnerable program for a classic ret2win buffer-overflow lab.
 * FOR TEACHING ONLY -- build with protections OFF and exploit in GDB.
 *
 *   gcc -O0 -g -fno-stack-protector -no-pie -Wl,-z,noexecstack -Wall -Wextra \
 *       -o legacy_log_parser legacy_log_parser.c
 *
 * Normal run:  ./legacy_log_parser <path-to-log>
 *
 * The overflow target is win(), which is NEVER called in normal control
 * flow -- it is reached only by overwriting the saved return address of
 * vulnerable_copy().
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SIZE 128
#define MAX_COPY 512

/* ------------------------------------------------------------------ *
 * win(): the ret2win target.
 *
 * __attribute__((used, noinline)) keeps it at a fixed, findable address
 * (the address you jump to in your exploit) and stops the compiler from
 * inlining it or warning that it is unused.
 * ------------------------------------------------------------------ */
__attribute__((used, noinline))
void win(void)
{
    FILE *marker;

    puts("[win] control flow reached win() -- ret2win successful!");

    /* Drop a marker file relative to the current working directory so the
     * exploit leaves durable proof it ran. Try markers/ first; if that
     * directory does not exist, fall back to the cwd. (Pure ISO C: no
     * directory-creation calls needed.) */
    marker = fopen("markers/ret2win.txt", "w");
    if (marker == NULL) {
        marker = fopen("ret2win.txt", "w");
    }

    if (marker != NULL) {
        fputs("ret2win reached: saved return address was overwritten.\n",
              marker);
        fclose(marker);
        puts("[win] marker file written.");
    } else {
        fputs("[win] reached win(), but could not write the marker file.\n",
              stderr);
    }
    FILE *decoy = fopen("/opt/agentsec/decoy.txt", "r");
    if(decoy!=NULL) {
        char line[256];
	puts("[win] opened /opt/agentsec/decoy.txt -- read SUCCESS:");
	while (fgets(line, sizeof(line), decoy) != NULL) {
            fputs(line, stdout);
        }
        fclose(decoy);
    } else {
        puts("[win] could not read /opt/agentsec/decoy.txt -- read FAILED (expected without privilege).");

    }
}

/* ------------------------------------------------------------------ *
 * vulnerable_copy(): copies n bytes into a fixed 128-byte stack buffer
 * with no bounds check. This missing check IS the intentional bug.
 * ------------------------------------------------------------------ */
__attribute__((noinline))
void vulnerable_copy(const unsigned char *input, size_t n)
{
    char buf[BUF_SIZE];
    size_t i;

    /* *** INTENTIONAL VULNERABILITY ***
     * n comes straight from the input file and can be as large as
     * MAX_COPY (512), but buf holds only BUF_SIZE (128) bytes. There is
     * NO check of n against BUF_SIZE before the copy, so a large enough
     * file overflows buf and overwrites this frame's saved return
     * address -> redirect execution to win(). */
    memcpy(buf, input, n);

    /* Print the beginning of the log as if this were normal processing. */
    printf("[parse] copied %zu byte(s); log begins: ", n);
    for (i = 0; i < n && i < 32; i++) {
        int c = (unsigned char)buf[i];
        putchar((c >= 32 && c < 127) ? c : '.');
    }
    putchar('\n');
}

/* ------------------------------------------------------------------ *
 * parse_log(): read the file, close it, THEN process the bytes.
 * ------------------------------------------------------------------ */
void parse_log(const char *path)
{
    FILE *f;
    unsigned char *input;
    size_t n;

    f = fopen(path, "rb");
    if (f == NULL) {
        fprintf(stderr, "error: cannot open '%s'\n", path);
        return;
    }

    input = malloc(MAX_COPY);
    if (input == NULL) {
        fprintf(stderr, "error: out of memory\n");
        fclose(f);
        return;
    }

    n = fread(input, 1, MAX_COPY, f);

    /* Close the file BEFORE touching the data, on purpose: the overflow
     * then corrupts only this call chain's stack, not an open handle. */
    fclose(f);

    vulnerable_copy(input, n);

    /* Reached only on a normal (non-overflow) run. */
    free(input);
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "usage: %s <log-file>\n", argv[0]);
        return 1;
    }

    parse_log(argv[1]);

    puts("[main] done.");
    return 0;
}

