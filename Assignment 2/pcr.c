/* pcr.c */
#include <stdio.h>
#include <time.h>
#include <errno.h>

int main(void) {
    struct timespec res;
    int ret;

    // Retrieve resolution for CLOCK_REALTIME
    ret = clock_getres(CLOCK_REALTIME, &res);
    if (ret == 0) {
        printf("POSIX Clock Resolution for CLOCK_REALTIME: %ld seconds and %ld nanoseconds\n",
               res.tv_sec, res.tv_nsec);
    } else {
        perror("clock_getres failed");
        return errno;
    }
    return 0;
}
