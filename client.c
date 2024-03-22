#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: sudo ./print_pmap <process_pid>\n");
        return -1;
    }

    int pid = atoi(argv[1]);

    if (!pid) {
        printf("Invalid PID: %s\n", argv[1]);
        return -1;
    }

    FILE *fptr;

    fptr = fopen("/sys/kernel/debug/pmap/pid_here", "w");

    if (fptr == NULL) {
        printf("Error! pid_here not found or permissions denied.\n");
        return -1;
    }

    fprintf(fptr, "%d", pid);
    fclose(fptr);

    fptr = fopen("/sys/kernel/debug/pmap/pmap_output", "r");

    if (fptr == NULL) {
        printf("Error! pmap_output not found or permissions denied.\n");
        return -1;
    }

    
    char line[256];
    while (fgets(line, sizeof(line), fptr)) {
        printf("%s", line);
    }

    fclose(fptr);

    return 0;
}


