#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <memory.h>
#undef memcpy
#include <string.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/poll.h>
#include <pthread.h>
#include <math.h>
#include "utils.h"

#ifdef __cplusplus
extern "C" {
#endif

int utils_debug = 0;
#define utils_printf(x...) do { if (utils_debug)printf(x); } while (0)

int lzo1x_decode(void *out, int *outlen, void *in, int *inlen) {
    int state= 0;
    int x;
    LZOContext c;
    c.in = in;
    c.in_end = (uint8_t *)in + *inlen;
    c.out = c.out_start = out;
    c.out_end = (uint8_t *)out + * outlen;
    c.error = 0;
    x = GETB(c);
    if (x > 17) {
        copy(&c, x - 17);
        x = GETB(c);
        if (x < 16) c.error |= LZO_ERROR;
    }
    if (c.in > c.in_end)
        c.error |= LZO_INPUT_DEPLETED;
    while (!c.error) {
        int cnt, back;
        if (x > 15) {
            if (x > 63) {
                cnt = (x >> 5) - 1;
                back = (GETB(c) << 3) + ((x >> 2) & 7) + 1;
            } else if (x > 31) {
                cnt = get_len(&c, x, 31);
                x = GETB(c);
                back = (GETB(c) << 6) + (x >> 2) + 1;
            } else {
                cnt = get_len(&c, x, 7);
                back = (1 << 14) + ((x & 8) << 11);
                x = GETB(c);
                back += (GETB(c) << 6) + (x >> 2);
                if (back == (1 << 14)) {
                    if (cnt != 1)
                        c.error |= LZO_ERROR;
                    break;
                }
            }
        } else if(!state){
                cnt = get_len(&c, x, 15);
                copy(&c, cnt + 3);
                x = GETB(c);
                if (x > 15)
                    continue;
                cnt = 1;
                back = (1 << 11) + (GETB(c) << 2) + (x >> 2) + 1;
        } else {
                cnt = 0;
                back = (GETB(c) << 2) + (x >> 2) + 1;
        }
        copy_backptr(&c, back, cnt + 2);
        state=
        cnt = x & 3;
        copy(&c, cnt);
        x = GETB(c);
    }
    *inlen = c.in_end - c.in;
    if (c.in > c.in_end)
        *inlen = 0;
    *outlen = c.out_end - c.out;
    return c.error;
}

double av_int2dbl(int64_t v){
    if(v+v > 0xFFEULL<<52)
        return 0.0/0.0;
    return ldexp(((v&((1LL<<52)-1)) + (1LL<<52)) * (v>>63|1), (v>>52&0x7FF)-1075);
}

float av_int2flt(int32_t v){
    if(v+v > 0xFF000000U)
        return 0.0/0.0;
    return ldexp(((v&0x7FFFFF) + (1<<23)) * (v>>31|1), (v>>23&0xFF)-150);
}
