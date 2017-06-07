#include<stdio.h>
#include<stdlib.h>
#include<sys/mman.h>
#include<sys/stat.h>
#include<fcntl.h>

void pigrep_search_stream(FILE *fs, char *search, long max, int extra) {
    long i, j;
    int k, l, len, buflen, indexlen;
    char *buffer;

    if(search == NULL) return;
    
    for(len = 0; search[len] != '\0'; len++);

    if(len == 0) return;

    buflen = len + extra * 2;
    buffer = malloc(buflen);

    for(indexlen = 1, i = max; i > 9; indexlen++, i /= 10);

    // Load buffer with data
    for(l = 0; l < len + extra && (max == 0 || l < max); l++) {
        k = fgetc(fs);
        if(k == EOF) max = l;
        else buffer[l] = k;
    }

    for(i = 0; max == 0 || i <= max - len; i++) {
        for(l = 0; l < len; l++) {
            if(search[l] != '#' && search[l] != buffer[(i + l) % buflen]) break;
            if(l + 1 == len) {
                printf("%*ld:", indexlen, i - 1);
                for(j = i - extra; j < i + len + extra; j++) {
                    if(j < 0 || j >= max) printf(" ");
                    else printf("%c", buffer[j % buflen]);
                }
                printf("\n");
            }
        }

        j = i + len + extra;
        if(max > j) {
            k = fgetc(fs);
            if(k == EOF) max = j;
            else buffer[j % buflen] = k;
        }
    }

    free(buffer);
}

void pigrep_search_file(char *fname, char *search, long max, int extra) {
    char *buffer;
    long i, j;
    int l, len, indexlen, fd;
    struct stat st;

    if(search == NULL) return;
    for(len = 0; search[len] != '\0'; len++);
    if(len == 0) return;

    fd = open(fname, O_RDONLY);
    if(fd == -1) return;

    if(fstat(fd, &st) == -1) return;
    if(max == 0 || max > st.st_size) max = st.st_size;

    buffer = mmap(NULL, max, PROT_READ, MAP_PRIVATE, fd, 0);
    if(buffer == MAP_FAILED) return;

    close(fd);

    for(indexlen = 1, i = max; i > 9; indexlen++, i /= 10);

    for(i = 0; i < max - len; i++) {
        for(l = 0; l < len; l++) {
            if(search[l] != '#' && search[l] != buffer[i + l]) break;
            if(l+1 == len) {
                printf("%*ld:", indexlen, i - 1);
                for(j = i - extra; j < i + len + extra; j++) {
                    if(j < 0 || j >= max) printf(" ");
                    else printf("%c", buffer[j]);
                }
                printf("\n");
            }
        }
    }
}

long getint(char *str) {
    long ret = 0;
    int i;

    for(i = 0; str[i] >= '0' && str[i] <= '9'; i++) ret = ret * 10 + str[i] - '0';

    return ret;
}

int main(int argc, char *argv[]) {
    int argi = 1;
    int i;
    long max = 0;
    int extra = 3;
    char *fname = NULL;
    char *search = NULL;

    while(argv[argi] != NULL) {
        if(argv[argi][0] == '-') {
            if(argv[argi][1] == 'c' && argv[argi+1] != NULL) {
                max = getint(argv[argi+1]);
                argi += 2;
            } else if(argv[argi][1] == 'e' && argv[argi+1] != NULL) {
                extra = getint(argv[argi+1]);
                argi += 2;
            } else if(argv[argi][1] == 'f' && argv[argi+1] != NULL) {
                fname = argv[argi+1];
                argi += 2;
            } else {
                fprintf(stderr, "Invalid option %s\n", argv[argi]);
                return -1;
            }
        } else {
            search = argv[argi];
            break;
        }
    }

    if(fname) pigrep_search_file(fname, search, max, extra);
    else pigrep_search_stream(stdin, search, max, extra);

    return 0;
}
