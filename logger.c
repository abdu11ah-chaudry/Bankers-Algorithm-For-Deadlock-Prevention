#include <stdio.h>
#include <time.h>
#include "logger.h"

#define LOG_FILE "banker_log.txt"

static FILE *log_fp = NULL;

void log_open(void)
{
    log_fp = fopen(LOG_FILE, "a");
}

void log_close(void)
{
    if(log_fp)
        fclose(log_fp);
}

void log_write(const char *msg)
{
    if(!log_fp) return;

    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);

    char buf[32];

    strftime(buf, sizeof(buf),
             "%Y-%m-%d %H:%M:%S",
             tm_info);

    fprintf(log_fp, "[%s] %s\n", buf, msg);

    fflush(log_fp);
}
