#ifndef __DEBUG_HPP__
#define __DEBUG_HPP__

#include <sys/time.h>
#define log_printf(...) { \
    struct timeval t; \
    gettimeofday(&t, NULL); \
    printf("%ld.%06ld [%d] ", t.tv_sec, t.tv_usec, world_.rank()); \
    printf(__VA_ARGS__); \
  }

#endif
