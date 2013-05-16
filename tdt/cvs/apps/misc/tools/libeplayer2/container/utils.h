#ifndef _UTILS_H
#define _UTILS_H

#include <stdint.h>
#include <byteswap.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Defines used by various modules */

#ifndef __cplusplus
#ifndef bool
#define bool    unsigned char
#define false   0
#define true    1
#endif
#endif

#define fast_memcpy(a,b,c) memcpy(a,b,c)
#define FFMAX(a,b) ((a) > (b) ? (a) : (b))
#ifndef SIZE_MAX
#define SIZE_MAX ((size_t)-1)
#endif

#define LZO_INPUT_DEPLETED 1
#define LZO_OUTPUT_FULL 2
#define LZO_INVALID_BACKPTR 4
#define LZO_ERROR 8

#define LZO_INPUT_PADDING 8
#define LZO_OUTPUT_PADDING 12

typedef struct LZOContext {
    uint8_t *in, *in_end;
    uint8_t *out_start, *out, *out_end;
    int error;
} LZOContext;
static inline int get_byte(LZOContext *c) {
    if (c->in < c->in_end)
        return *c->in++;
    c->error |= LZO_INPUT_DEPLETED;
    return 1;
}

#ifdef INBUF_PADDED
#define GETB(c) (*(c).in++)
#else
#define GETB(c) get_byte(&(c))
#endif
static inline int get_len(LZOContext *c, int x, int mask) {
    int cnt = x & mask;
    if (!cnt) {
        while (!(x = get_byte(c))) cnt += 255;
        cnt += mask + x;
    }
    return cnt;
}

//#define UNALIGNED_LOADSTORE
#define BUILTIN_MEMCPY
#ifdef UNALIGNED_LOADSTORE
#define COPY2(d, s) *(uint16_t *)(d) = *(uint16_t *)(s);
#define COPY4(d, s) *(uint32_t *)(d) = *(uint32_t *)(s);
#elif defined(BUILTIN_MEMCPY)
#define COPY2(d, s) memcpy(d, s, 2);
#define COPY4(d, s) memcpy(d, s, 4);
#else
#define COPY2(d, s) (d)[0] = (s)[0]; (d)[1] = (s)[1];
#define COPY4(d, s) (d)[0] = (s)[0]; (d)[1] = (s)[1]; (d)[2] = (s)[2]; (d)[3] = (s)[3];
#endif

/**
 * \brief copy bytes from input to output buffer with checking
 * \param cnt number of bytes to copy, must be >= 0
 */
static inline void copy(LZOContext *c, int cnt) {
    register uint8_t *src = c->in;
    register uint8_t *dst = c->out;
    if (cnt > c->in_end - src) {
        cnt = FFMAX(c->in_end - src, 0);
        c->error |= LZO_INPUT_DEPLETED;
    }
    if (cnt > c->out_end - dst) {
        cnt = FFMAX(c->out_end - dst, 0);
        c->error |= LZO_OUTPUT_FULL;
    }
#if defined(INBUF_PADDED) && defined(OUTBUF_PADDED)
    COPY4(dst, src);
    src += 4;
    dst += 4;
    cnt -= 4;
    if (cnt > 0)
#endif
        memcpy(dst, src, cnt);
    c->in = src + cnt;
    c->out = dst + cnt;
}

/**
 * \brief copy previously decoded bytes to current position
 * \param back how many bytes back we start
 * \param cnt number of bytes to copy, must be >= 0
 *
 * cnt > back is valid, this will copy the bytes we just copied,
 * thus creating a repeating pattern with a period length of back.
 */
static inline void copy_backptr(LZOContext *c, int back, int cnt) {
    register uint8_t *src = &c->out[-back];
    register uint8_t *dst = c->out;
    if (src < c->out_start || src > dst) {
        c->error |= LZO_INVALID_BACKPTR;
        return;
    }
    if (cnt > c->out_end - dst) {
        cnt = FFMAX(c->out_end - dst, 0);
        c->error |= LZO_OUTPUT_FULL;
    }
    if (back == 1) {
        memset(dst, *src, cnt);
        dst += cnt;
    } else {
#ifdef OUTBUF_PADDED
        COPY2(dst, src);
        COPY2(dst + 2, src + 2);
        src += 4;
        dst += 4;
        cnt -= 4;
        if (cnt > 0) {
            COPY2(dst, src);
            COPY2(dst + 2, src + 2);
            COPY2(dst + 4, src + 4);
            COPY2(dst + 6, src + 6);
            src += 8;
            dst += 8;
            cnt -= 8;
        }
#endif
        if (cnt > 0) {
            int blocklen = back;
            while (cnt > blocklen) {
                memcpy(dst, src, blocklen);
                dst += blocklen;
                cnt -= blocklen;
                blocklen <<= 1;
            }
            memcpy(dst, src, cnt);
        }
        dst += cnt;
    }
    c->out = dst;
}

int lzo1x_decode(void *out, int *outlen, void *in, int *inlen);
double av_int2dbl(int64_t v);
float av_int2flt(int32_t v);

// Endianness
#define AF_FORMAT_BE		(0<<0) // Big Endian
#define AF_FORMAT_LE		(1<<0) // Little Endian
#define AF_FORMAT_END_MASK	(1<<0)

#if WORDS_BIGENDIAN	       	// Native endian of cpu
#define	AF_FORMAT_NE		AF_FORMAT_BE
#else
#define	AF_FORMAT_NE		AF_FORMAT_LE
#endif

// Signed/unsigned
#define AF_FORMAT_SI		(0<<1) // Signed
#define AF_FORMAT_US		(1<<1) // Unsigned
#define AF_FORMAT_SIGN_MASK	(1<<1)

// Fixed or floating point
#define AF_FORMAT_I		(0<<2) // Int
#define AF_FORMAT_F		(1<<2) // Foating point
#define AF_FORMAT_POINT_MASK	(1<<2)

// Bits used
#define AF_FORMAT_8BIT		(0<<3)
#define AF_FORMAT_16BIT		(1<<3)
#define AF_FORMAT_24BIT		(2<<3)
#define AF_FORMAT_32BIT		(3<<3)
#define AF_FORMAT_40BIT		(4<<3)
#define AF_FORMAT_48BIT		(5<<3)
#define AF_FORMAT_BITS_MASK	(7<<3)

// Special flags refering to non pcm data
#define AF_FORMAT_MU_LAW	(1<<6)
#define AF_FORMAT_A_LAW		(2<<6)
#define AF_FORMAT_MPEG2		(3<<6) // MPEG(2) audio
#define AF_FORMAT_AC3		(4<<6) // Dolby Digital AC3
#define AF_FORMAT_IMA_ADPCM	(5<<6)
#define AF_FORMAT_SPECIAL_MASK	(7<<6)

// PREDEFINED formats

#define AF_FORMAT_U8		(AF_FORMAT_I|AF_FORMAT_US|AF_FORMAT_8BIT|AF_FORMAT_NE)
#define AF_FORMAT_S8		(AF_FORMAT_I|AF_FORMAT_SI|AF_FORMAT_8BIT|AF_FORMAT_NE)
#define AF_FORMAT_U16_LE	(AF_FORMAT_I|AF_FORMAT_US|AF_FORMAT_16BIT|AF_FORMAT_LE)
#define AF_FORMAT_U16_BE	(AF_FORMAT_I|AF_FORMAT_US|AF_FORMAT_16BIT|AF_FORMAT_BE)
#define AF_FORMAT_S16_LE	(AF_FORMAT_I|AF_FORMAT_SI|AF_FORMAT_16BIT|AF_FORMAT_LE)
#define AF_FORMAT_S16_BE	(AF_FORMAT_I|AF_FORMAT_SI|AF_FORMAT_16BIT|AF_FORMAT_BE)
#define AF_FORMAT_U24_LE	(AF_FORMAT_I|AF_FORMAT_US|AF_FORMAT_24BIT|AF_FORMAT_LE)
#define AF_FORMAT_U24_BE	(AF_FORMAT_I|AF_FORMAT_US|AF_FORMAT_24BIT|AF_FORMAT_BE)
#define AF_FORMAT_S24_LE	(AF_FORMAT_I|AF_FORMAT_SI|AF_FORMAT_24BIT|AF_FORMAT_LE)
#define AF_FORMAT_S24_BE	(AF_FORMAT_I|AF_FORMAT_SI|AF_FORMAT_24BIT|AF_FORMAT_BE)
#define AF_FORMAT_U32_LE	(AF_FORMAT_I|AF_FORMAT_US|AF_FORMAT_32BIT|AF_FORMAT_LE)
#define AF_FORMAT_U32_BE	(AF_FORMAT_I|AF_FORMAT_US|AF_FORMAT_32BIT|AF_FORMAT_BE)
#define AF_FORMAT_S32_LE	(AF_FORMAT_I|AF_FORMAT_SI|AF_FORMAT_32BIT|AF_FORMAT_LE)
#define AF_FORMAT_S32_BE	(AF_FORMAT_I|AF_FORMAT_SI|AF_FORMAT_32BIT|AF_FORMAT_BE)

#define AF_FORMAT_FLOAT_LE	(AF_FORMAT_F|AF_FORMAT_32BIT|AF_FORMAT_LE)
#define AF_FORMAT_FLOAT_BE	(AF_FORMAT_F|AF_FORMAT_32BIT|AF_FORMAT_BE)

#ifdef WORDS_BIGENDIAN
#define AF_FORMAT_U16_NE AF_FORMAT_U16_BE
#define AF_FORMAT_S16_NE AF_FORMAT_S16_BE
#define AF_FORMAT_U24_NE AF_FORMAT_U24_BE
#define AF_FORMAT_S24_NE AF_FORMAT_S24_BE
#define AF_FORMAT_U32_NE AF_FORMAT_U32_BE
#define AF_FORMAT_S32_NE AF_FORMAT_S32_BE
#define AF_FORMAT_FLOAT_NE AF_FORMAT_FLOAT_BE
#else
#define AF_FORMAT_U16_NE AF_FORMAT_U16_LE
#define AF_FORMAT_S16_NE AF_FORMAT_S16_LE
#define AF_FORMAT_U24_NE AF_FORMAT_U24_LE
#define AF_FORMAT_S24_NE AF_FORMAT_S24_LE
#define AF_FORMAT_U32_NE AF_FORMAT_U32_LE
#define AF_FORMAT_S32_NE AF_FORMAT_S32_LE
#define AF_FORMAT_FLOAT_NE AF_FORMAT_FLOAT_LE
#endif

#define AF_FORMAT_UNKNOWN (-1)

// be2me ... BigEndian to MachineEndian
// le2me ... LittleEndian to MachineEndian

#ifdef WORDS_BIGENDIAN
#define be2me_16(x) (x)
#define be2me_32(x) (x)
#define be2me_64(x) (x)
#define le2me_16(x) bswap_16(x)
#define le2me_32(x) bswap_32(x)
#define le2me_64(x) bswap_64(x)
#else
#define be2me_16(x) bswap_16(x)
#define be2me_32(x) bswap_32(x)
#define be2me_64(x) bswap_64(x)
#define le2me_16(x) (x)
#define le2me_32(x) (x)
#define le2me_64(x) (x)
#endif

#ifdef __GNUC__

struct unaligned_64 { uint64_t l; } __attribute__((packed));
struct unaligned_32 { uint32_t l; } __attribute__((packed));
struct unaligned_16 { uint16_t l; } __attribute__((packed));

#define AV_RN16(a) (((const struct unaligned_16 *) (a))->l)
#define AV_RN32(a) (((const struct unaligned_32 *) (a))->l)
#define AV_RN64(a) (((const struct unaligned_64 *) (a))->l)

#define AV_WN16(a, b) (((struct unaligned_16 *) (a))->l) = (b)
#define AV_WN32(a, b) (((struct unaligned_32 *) (a))->l) = (b)
#define AV_WN64(a, b) (((struct unaligned_64 *) (a))->l) = (b)

#else /* __GNUC__ */

#define AV_RN16(a) (*((uint16_t*)(a)))
#define AV_RN32(a) (*((uint32_t*)(a)))
#define AV_RN64(a) (*((uint64_t*)(a)))

#define AV_WN16(a, b) *((uint16_t*)(a)) = (b)
#define AV_WN32(a, b) *((uint32_t*)(a)) = (b)
#define AV_WN64(a, b) *((uint64_t*)(a)) = (b)

#endif /* !__GNUC__ */

/* endian macros */
#define AV_RB8(x)     (((uint8_t*)(x))[0])
#define AV_WB8(p, d)  do { ((uint8_t*)(p))[0] = (d); } while(0)

#define AV_RL8(x)     AV_RB8(x)
#define AV_WL8(p, d)  AV_WB8(p, d)

#ifdef HAVE_FAST_UNALIGNED
# ifdef WORDS_BIGENDIAN
#  define AV_RB16(x)    AV_RN16(x)
#  define AV_WB16(p, d) AV_WN16(p, d)

#  define AV_RL16(x)    bswap_16(AV_RN16(x))
#  define AV_WL16(p, d) AV_WN16(p, bswap_16(d))
# else /* WORDS_BIGENDIAN */
#  define AV_RB16(x)    bswap_16(AV_RN16(x))
#  define AV_WB16(p, d) AV_WN16(p, bswap_16(d))

#  define AV_RL16(x)    AV_RN16(x)
#  define AV_WL16(p, d) AV_WN16(p, d)
# endif
#else /* HAVE_FAST_UNALIGNED */
#define AV_RB16(x)  ((((uint8_t*)(x))[0] << 8) | ((uint8_t*)(x))[1])
#define AV_WB16(p, d) do { \
                    ((uint8_t*)(p))[1] = (d); \
                    ((uint8_t*)(p))[0] = (d)>>8; } while(0)

#define AV_RL16(x)  ((((uint8_t*)(x))[1] << 8) | \
                      ((uint8_t*)(x))[0])
#define AV_WL16(p, d) do { \
                    ((uint8_t*)(p))[0] = (d); \
                    ((uint8_t*)(p))[1] = (d)>>8; } while(0)
#endif

#define AV_RB24(x)  ((((uint8_t*)(x))[0] << 16) | \
                     (((uint8_t*)(x))[1] <<  8) | \
                      ((uint8_t*)(x))[2])
#define AV_WB24(p, d) do { \
                    ((uint8_t*)(p))[2] = (d); \
                    ((uint8_t*)(p))[1] = (d)>>8; \
                    ((uint8_t*)(p))[0] = (d)>>16; } while(0)

#define AV_RL24(x)  ((((uint8_t*)(x))[2] << 16) | \
                     (((uint8_t*)(x))[1] <<  8) | \
                      ((uint8_t*)(x))[0])
#define AV_WL24(p, d) do { \
                    ((uint8_t*)(p))[0] = (d); \
                    ((uint8_t*)(p))[1] = (d)>>8; \
                    ((uint8_t*)(p))[2] = (d)>>16; } while(0)

#ifdef HAVE_FAST_UNALIGNED
# ifdef WORDS_BIGENDIAN
#  define AV_RB32(x)    AV_RN32(x)
#  define AV_WB32(p, d) AV_WN32(p, d)

#  define AV_RL32(x)    bswap_32(AV_RN32(x))
#  define AV_WL32(p, d) AV_WN32(p, bswap_32(d))
# else /* WORDS_BIGENDIAN */
#  define AV_RB32(x)    bswap_32(AV_RN32(x))
#  define AV_WB32(p, d) AV_WN32(p, bswap_32(d))

#  define AV_RL32(x)    AV_RN32(x)
#  define AV_WL32(p, d) AV_WN32(p, d)
# endif
#else /* HAVE_FAST_UNALIGNED */
#define AV_RB32(x)  ((((uint8_t*)(x))[0] << 24) | \
                     (((uint8_t*)(x))[1] << 16) | \
                     (((uint8_t*)(x))[2] <<  8) | \
                      ((uint8_t*)(x))[3])
#define AV_WB32(p, d) do { \
                    ((uint8_t*)(p))[3] = (d); \
                    ((uint8_t*)(p))[2] = (d)>>8; \
                    ((uint8_t*)(p))[1] = (d)>>16; \
                    ((uint8_t*)(p))[0] = (d)>>24; } while(0)

#define AV_RL32(x) ((((uint8_t*)(x))[3] << 24) | \
                    (((uint8_t*)(x))[2] << 16) | \
                    (((uint8_t*)(x))[1] <<  8) | \
                     ((uint8_t*)(x))[0])
#define AV_WL32(p, d) do { \
                    ((uint8_t*)(p))[0] = (d); \
                    ((uint8_t*)(p))[1] = (d)>>8; \
                    ((uint8_t*)(p))[2] = (d)>>16; \
                    ((uint8_t*)(p))[3] = (d)>>24; } while(0)
#endif

#ifdef HAVE_FAST_UNALIGNED
# ifdef WORDS_BIGENDIAN
#  define AV_RB64(x)    AV_RN64(x)
#  define AV_WB64(p, d) AV_WN64(p, d)

#  define AV_RL64(x)    bswap_64(AV_RN64(x))
#  define AV_WL64(p, d) AV_WN64(p, bswap_64(d))
# else /* WORDS_BIGENDIAN */
#  define AV_RB64(x)    bswap_64(AV_RN64(x))
#  define AV_WB64(p, d) AV_WN64(p, bswap_64(d))

#  define AV_RL64(x)    AV_RN64(x)
#  define AV_WL64(p, d) AV_WN64(p, d)
# endif
#else /* HAVE_FAST_UNALIGNED */
#define AV_RB64(x)  (((uint64_t)((uint8_t*)(x))[0] << 56) | \
                     ((uint64_t)((uint8_t*)(x))[1] << 48) | \
                     ((uint64_t)((uint8_t*)(x))[2] << 40) | \
                     ((uint64_t)((uint8_t*)(x))[3] << 32) | \
                     ((uint64_t)((uint8_t*)(x))[4] << 24) | \
                     ((uint64_t)((uint8_t*)(x))[5] << 16) | \
                     ((uint64_t)((uint8_t*)(x))[6] <<  8) | \
                      (uint64_t)((uint8_t*)(x))[7])
#define AV_WB64(p, d) do { \
                    ((uint8_t*)(p))[7] = (d);     \
                    ((uint8_t*)(p))[6] = (d)>>8;  \
                    ((uint8_t*)(p))[5] = (d)>>16; \
                    ((uint8_t*)(p))[4] = (d)>>24; \
                    ((uint8_t*)(p))[3] = (d)>>32; \
                    ((uint8_t*)(p))[2] = (d)>>40; \
                    ((uint8_t*)(p))[1] = (d)>>48; \
                    ((uint8_t*)(p))[0] = (d)>>56; } while(0)

#define AV_RL64(x)  (((uint64_t)((uint8_t*)(x))[7] << 56) | \
                     ((uint64_t)((uint8_t*)(x))[6] << 48) | \
                     ((uint64_t)((uint8_t*)(x))[5] << 40) | \
                     ((uint64_t)((uint8_t*)(x))[4] << 32) | \
                     ((uint64_t)((uint8_t*)(x))[3] << 24) | \
                     ((uint64_t)((uint8_t*)(x))[2] << 16) | \
                     ((uint64_t)((uint8_t*)(x))[1] <<  8) | \
                      (uint64_t)((uint8_t*)(x))[0])
#define AV_WL64(p, d) do { \
                    ((uint8_t*)(p))[0] = (d);     \
                    ((uint8_t*)(p))[1] = (d)>>8;  \
                    ((uint8_t*)(p))[2] = (d)>>16; \
                    ((uint8_t*)(p))[3] = (d)>>24; \
                    ((uint8_t*)(p))[4] = (d)>>32; \
                    ((uint8_t*)(p))[5] = (d)>>40; \
                    ((uint8_t*)(p))[6] = (d)>>48; \
                    ((uint8_t*)(p))[7] = (d)>>56; } while(0)
#endif

static inline uint8_t av_clip_uint8(int a)
{
    if (a&(~255)) return (-a)>>31;
    else          return a;
}

#endif /* UTILS_H */ 
