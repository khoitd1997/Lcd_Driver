
#ifndef _BIT_MAP_MANIPULATION
#define _BIT_MAP_MANIPULATION

#ifdef __cplusplus
extern "C" {
#endif

#define bit_get(p, m) ((p) & (m))
#define bit_set(p, m) ((p) |= (m))     // bit_set(foo, BIT(5));
#define bit_clear(p, m) ((p) &= ~(m))  // bit_clear(bar, BIT(0));
#define bit_flip(p, m) ((p) ^= (m))    // bit_flip(foo, BIT(0));
#define bit_write(c, p, m) (c ? bit_set(p, m) : bit_clear(p, m))
#define BIT(x) (0x01 << (x))
#define LONGBIT(x) ((unsigned long)0x00000001 << (x))
// From < http://
// www.avrfreaks.net/forum/tut-c-bit-manipulation-aka-programming-101?name=PNphpBB2&file=viewtopic&t=37871>

#ifdef __cplusplus
}
#endif
#endif