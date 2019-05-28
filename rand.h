//#include <stdint.h>
#include <string.h>


#ifdef __cplusplus
# define EXTERN_C extern "C" {
# define EXTERN_C_END }
#else
# define EXTERN_C
# define EXTERN_C_END
#endif

#define A 1
#define B 0xc05aa5704efbc79ful
#define C 0x05fd8167b327f30ful

//#define uint64_t uint64_t
#define STATE_SIZE 8
#define INV_POWER 6  //2**6==64

EXTERN_C
static union {
        uint64_t v;
        uint8_t a[STATE_SIZE];
    } st = {
        .v = C,
    };
    static int pos = 0;
    
static inline uint64_t inv(uint64_t x) 
{
        if (x) {
            int shift = 0;
            while (x % 2 == 0) {
                ++shift; 
                x /= 2;
            }
            uint64_t r = 1;
            /* when x i sodd -2*((x-1)/2) == 1-x */
            uint64_t q = 1-x;
            for (int i = 0; i < INV_POWER; ++i) {
                r *= 1+q; q*=q;
            }
            return r<<shift;
        } else {
            return 0;
        }
}

static uint64_t next_value(uint64_t val) 
{
        return A*inv(val) + B;
}
    
int rand_bytes(uint8_t *buf, int num) 
{
        while (num) {
            int l = STATE_SIZE - pos;
            if (l > num) l = num;
            
            memcpy(buf, &st.a[pos], l);

            buf += l;
            num -= l;
            pos += l;
            
            if (pos == STATE_SIZE) {
                st.v = next_value(st.v);
                pos = 0;
            }           
        }
        return 1;
}
EXTERN_C_END