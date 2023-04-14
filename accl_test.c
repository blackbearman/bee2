#include "bee2/core/word.h"
#include "bee2/core/u32.h"

#include <stdio.h>
#include <bee2/core/prng.h>
#include <bee2/core/tm.h>
#include <bee2/core/util.h>
#include <bee2/crypto/bash.h>
#include <bee2/crypto/belt.h>
#include <bee2/math/pp.h>
#include <bee2/math/ww.h>

//#undef B_PER_W
//#define B_PER_W 16

#if (B_PER_W == 16)

#define beltBlockSetZero(block)\
	((word*)(block))[0] = 0,\
	((word*)(block))[1] = 0,\
	((word*)(block))[2] = 0,\
	((word*)(block))[3] = 0,\
	((word*)(block))[4] = 0,\
	((word*)(block))[5] = 0,\
	((word*)(block))[6] = 0,\
	((word*)(block))[7] = 0\

#define beltBlockRevW(block)\
	((word*)(block))[0] = wordRev(((word*)(block))[0]),\
	((word*)(block))[1] = wordRev(((word*)(block))[1]),\
	((word*)(block))[2] = wordRev(((word*)(block))[2]),\
	((word*)(block))[3] = wordRev(((word*)(block))[3]),\
	((word*)(block))[4] = wordRev(((word*)(block))[4]),\
	((word*)(block))[5] = wordRev(((word*)(block))[5]),\
	((word*)(block))[6] = wordRev(((word*)(block))[6]),\
	((word*)(block))[7] = wordRev(((word*)(block))[7])\

#define beltHalfBlockIsZero(block)\
	(((word*)(block))[0] == 0 && ((word*)(block))[1] == 0 &&\
		((word*)(block))[2] == 0 && ((word*)(block))[3] == 0)

#define beltBlockNeg(dest, src)\
	((word*)(dest))[0] = ~((const word*)(src))[0],\
	((word*)(dest))[1] = ~((const word*)(src))[1],\
	((word*)(dest))[2] = ~((const word*)(src))[2],\
	((word*)(dest))[3] = ~((const word*)(src))[3],\
	((word*)(dest))[4] = ~((const word*)(src))[4],\
	((word*)(dest))[5] = ~((const word*)(src))[5],\
	((word*)(dest))[6] = ~((const word*)(src))[6],\
	((word*)(dest))[7] = ~((const word*)(src))[7]\

#define beltBlockXor(dest, src1, src2)\
	((word*)(dest))[0] = ((const word*)(src1))[0] ^ ((const word*)(src2))[0],\
	((word*)(dest))[1] = ((const word*)(src1))[1] ^ ((const word*)(src2))[1],\
	((word*)(dest))[2] = ((const word*)(src1))[2] ^ ((const word*)(src2))[2],\
	((word*)(dest))[3] = ((const word*)(src1))[3] ^ ((const word*)(src2))[3],\
	((word*)(dest))[4] = ((const word*)(src1))[4] ^ ((const word*)(src2))[4],\
	((word*)(dest))[5] = ((const word*)(src1))[5] ^ ((const word*)(src2))[5],\
	((word*)(dest))[6] = ((const word*)(src1))[6] ^ ((const word*)(src2))[6],\
	((word*)(dest))[7] = ((const word*)(src1))[7] ^ ((const word*)(src2))[7]\

#define beltBlockXor2(dest, src)\
	((word*)(dest))[0] ^= ((const word*)(src))[0],\
	((word*)(dest))[1] ^= ((const word*)(src))[1],\
	((word*)(dest))[2] ^= ((const word*)(src))[2],\
	((word*)(dest))[3] ^= ((const word*)(src))[3],\
	((word*)(dest))[4] ^= ((const word*)(src))[4],\
	((word*)(dest))[5] ^= ((const word*)(src))[5],\
	((word*)(dest))[6] ^= ((const word*)(src))[6],\
	((word*)(dest))[7] ^= ((const word*)(src))[7]\

#define beltBlockCopy(dest, src)\
	((word*)(dest))[0] = ((const word*)(src))[0],\
	((word*)(dest))[1] = ((const word*)(src))[1],\
	((word*)(dest))[2] = ((const word*)(src))[2],\
	((word*)(dest))[3] = ((const word*)(src))[3],\
	((word*)(dest))[4] = ((const word*)(src))[4],\
	((word*)(dest))[5] = ((const word*)(src))[5],\
	((word*)(dest))[6] = ((const word*)(src))[6],\
	((word*)(dest))[7] = ((const word*)(src))[7]\

#elif (B_PER_W == 32)

#define beltBlockSetZero(block)\
	((word*)(block))[0] = 0,\
	((word*)(block))[1] = 0,\
	((word*)(block))[2] = 0,\
	((word*)(block))[3] = 0\

#define beltBlockRevW(block)\
	((word*)(block))[0] = wordRev(((word*)(block))[0]),\
	((word*)(block))[1] = wordRev(((word*)(block))[1]),\
	((word*)(block))[2] = wordRev(((word*)(block))[2]),\
	((word*)(block))[3] = wordRev(((word*)(block))[3])\

#define beltHalfBlockIsZero(block)\
	(((word*)(block))[0] == 0 && ((word*)(block))[1] == 0)\

#define beltBlockNeg(dest, src)\
	((word*)(dest))[0] = ~((const word*)(src))[0],\
	((word*)(dest))[1] = ~((const word*)(src))[1],\
	((word*)(dest))[2] = ~((const word*)(src))[2],\
	((word*)(dest))[3] = ~((const word*)(src))[3]\

#define beltBlockXor(dest, src1, src2)\
	((word*)(dest))[0] = ((const word*)(src1))[0] ^ ((const word*)(src2))[0],\
	((word*)(dest))[1] = ((const word*)(src1))[1] ^ ((const word*)(src2))[1],\
	((word*)(dest))[2] = ((const word*)(src1))[2] ^ ((const word*)(src2))[2],\
	((word*)(dest))[3] = ((const word*)(src1))[3] ^ ((const word*)(src2))[3]\

#define beltBlockXor2(dest, src)\
	((word*)(dest))[0] ^= ((const word*)(src))[0],\
	((word*)(dest))[1] ^= ((const word*)(src))[1],\
	((word*)(dest))[2] ^= ((const word*)(src))[2],\
	((word*)(dest))[3] ^= ((const word*)(src))[3]\

#define beltBlockCopy(dest, src)\
	((word*)(dest))[0] = ((const word*)(src))[0],\
	((word*)(dest))[1] = ((const word*)(src))[1],\
	((word*)(dest))[2] = ((const word*)(src))[2],\
	((word*)(dest))[3] = ((const word*)(src))[3]\

#elif (B_PER_W == 64)

#define beltBlockSetZero(block)\
	((word*)(block))[0] = 0,\
	((word*)(block))[1] = 0\

#define beltBlockRevW(block)\
	((word*)(block))[0] = wordRev(((word*)(block))[0]),\
	((word*)(block))[1] = wordRev(((word*)(block))[1])\

#define beltHalfBlockIsZero(block)\
	(((word*)(block))[0] == 0)\

#define beltBlockNeg(dest, src)\
	((word*)(dest))[0] = ~((const word*)(src))[0],\
	((word*)(dest))[1] = ~((const word*)(src))[1]\

#define beltBlockXor(dest, src1, src2)\
	((word*)(dest))[0] = ((const word*)(src1))[0] ^ ((const word*)(src2))[0],\
	((word*)(dest))[1] = ((const word*)(src1))[1] ^ ((const word*)(src2))[1];\

#define beltBlockXor2(dest, src)\
	((word*)(dest))[0] ^= ((const word*)(src))[0],\
	((word*)(dest))[1] ^= ((const word*)(src))[1]\

#define beltBlockCopy(dest, src)\
	((word*)(dest))[0] = ((const word*)(src))[0],\
	((word*)(dest))[1] = ((const word*)(src))[1]\

#else
	#error "Unsupported word size"
#endif // B_PER_W

#define beltBlockRevU32(block)\
	((u32*)(block))[0] = u32Rev(((u32*)(block))[0]),\
	((u32*)(block))[1] = u32Rev(((u32*)(block))[1]),\
	((u32*)(block))[2] = u32Rev(((u32*)(block))[2]),\
	((u32*)(block))[3] = u32Rev(((u32*)(block))[3])\

#define beltBlockIncU32(block)\
	if ((((u32*)(block))[0] += 1) == 0 &&\
		(((u32*)(block))[1] += 1) == 0 &&\
		(((u32*)(block))[2] += 1) == 0)\
		((u32*)(block))[3] += 1\

union _block{
    u64 b8[2];
    u32 b4[4];
    octet b1[16];
    word w[W_OF_B(128)];
};
typedef union _block block_t;

#define XbeltBlockSetZero(block)\
	((block_t*)(block))->w[0] = 0,\
	((block_t*)(block))->w[1] = 0\

#define WbeltBlockSetZero(block)\
	for(int _i = 0; _i < W_OF_B(128); _i++) \
        ((block_t*)(block))->w[_i] = 0;

#define XbeltBlockRevW(block)\
	((block_t*)(block))->w[0] = wordRev(((block_t*)(block))->w[0]),\
	((block_t*)(block))->w[1] = wordRev(((block_t*)(block))->w[1])\

#define WbeltBlockRevW(block)\
	for(int _i = 0; _i < W_OF_B(128); _i++) \
	    ((block_t*)(block))->w[_i] = wordRev(((block_t*)(block))->w[_i]);

#define XbeltBlockNeg(dest, src)\
	((block_t*)(dest))->w[0] = ~((const block_t*)(src))->w[0],\
	((block_t*)(dest))->w[1] = ~((const block_t*)(src))->w[1]\

#define WbeltBlockNeg(dest, src)\
	for(int _i = 0; _i < W_OF_B(128); _i++) \
	    ((block_t*)(dest))->w[_i] = ~((const block_t*)(src))->w[_i];

#define XbeltBlockXor(dest, src1, src2)\
	((block_t*)(dest))->w[0] = ((const block_t*)(src1))->w[0] ^ ((const block_t*)(src2))->w[0],\
	((block_t*)(dest))->w[1] = ((const block_t*)(src1))->w[1] ^ ((const block_t*)(src2))->w[1];\

#define WbeltBlockXor(dest, src1, src2)\
	for(int _i = 0; _i < W_OF_B(128); _i++) \
	    ((block_t*)(dest))->w[_i] = ((const block_t*)(src1))->w[_i] ^ ((const block_t*)(src2))->w[_i];

#define XbeltHalfBlockIsZero(block)\
	(((block_t*)(block))->w[0] == 0)\

#define WbeltHalfBlockIsZero(block)\
    ({ \
        int _x = 0; \
        for(int _i = 0; _i < W_OF_B(128)/2; _i++) \
            _x |= (((block_t*)(block))->w[_i] == 0);\
        _x;\
    }) 

#define WbeltBlockXor2(dest, src)\
    for(int _i = 0; _i < W_OF_B(128); _i++) \
	    ((block_t*)(dest))->w[_i] ^= ((const block_t*)(src))->w[_i];

#define WbeltBlockCopy(dest, src)\
    for(int _i = 0; _i < W_OF_B(128); _i++) \
	    ((block_t*)(dest))->w[_i] = ((const block_t*)(src))->w[_i];

bool_t acclBench()
{
	const size_t reps = 500000000;
	octet combo_state[256];
	octet buf[1024];
	size_t i;
	tm_ticks_t ticks;
    char* name;
	// подготовить стек
	if (sizeof(combo_state) < prngCOMBO_keep())
		return FALSE;
	// псевдослучайная генерация объектов
	prngCOMBOStart(combo_state, utilNonce32());
	prngCOMBOStepR(buf, sizeof(buf), combo_state);
    //////////////////////////////////////////////////////
    name = "HalfBlockIsZero";
    //beltHalfBlockIsZero(buf) == 0;
    i=0;
    
    //WbeltHalfBlockIsZero(buf) == 0;
    //ASSERT(beltHalfBlockIsZero(buf) != WbeltHalfBlockIsZero(buf));
    // cкорость belt
    int res = 0;
	for (i = 0, ticks = tmTicks(); i < reps; ++i) {
        octet *round_buf = buf + i * 128 % (sizeof(buf) - 128);
        res |= beltHalfBlockIsZero(round_buf);
    }
	ticks = tmTicks() - ticks;
	printf("acclBench::belt:%s:  %3u cpi [%5u ops/sec]\n", name,
		(unsigned)(ticks / reps),
		(unsigned)tmSpeed(reps, ticks));
	// cкорость Xbelt 
	for (i = 0, ticks = tmTicks(); i < reps; ++i) {
        octet *round_buf = buf + i * 128 % (sizeof(buf) - 128);
        res |= beltHalfBlockIsZero(round_buf);
    }
	ticks = tmTicks() - ticks;
	printf("acclBench::Xbelt:%s:  %3u cpi [%5u ops/sec]\n", name,
		(unsigned)(ticks / reps),
		(unsigned)tmSpeed(reps, ticks));
	// cкорость Wbelt 
	for (i = 0, ticks = tmTicks(); i < reps; ++i) {
        octet *round_buf = buf + i * 128 % (sizeof(buf) - 128);
        res |= WbeltHalfBlockIsZero(round_buf);
    }
	ticks = tmTicks() - ticks;
	printf("acclBench::Wbelt:%s:  %3u cpi [%5u ops/sec]\n", name,
		(unsigned)(ticks / reps),
		(unsigned)tmSpeed(reps, ticks));

    //////////////////////////////////////////////////////
    name = "SetZero";
	// cкорость belt
	for (i = 0, ticks = tmTicks(); i < reps; ++i) {
        octet *round_buf = buf + i * 128 % (sizeof(buf) - 128);
        beltBlockSetZero(round_buf);
    }
	ticks = tmTicks() - ticks;
	printf("acclBench::belt:%s:  %3u cpi [%5u ops/sec]\n", name,
		(unsigned)(ticks / reps),
		(unsigned)tmSpeed(reps, ticks));
	// cкорость Xbelt 
	for (i = 0, ticks = tmTicks(); i < reps; ++i) {
        octet *round_buf = buf + i * 128 % (sizeof(buf) - 128);
        XbeltBlockSetZero(round_buf);
    }
	ticks = tmTicks() - ticks;
	printf("acclBench::Xbelt:%s:  %3u cpi [%5u ops/sec]\n", name,
		(unsigned)(ticks / reps),
		(unsigned)tmSpeed(reps, ticks));
	// cкорость Wbelt 
	for (i = 0, ticks = tmTicks(); i < reps; ++i) {
        octet *round_buf = buf + i * 128 % (sizeof(buf) - 128);
        WbeltBlockSetZero(round_buf);
    }
	ticks = tmTicks() - ticks;
	printf("acclBench::Wbelt:%s:  %3u cpi [%5u ops/sec]\n", name,
		(unsigned)(ticks / reps),
		(unsigned)tmSpeed(reps, ticks));
///////////////////////////////////////////////////////////////
    name = "RevW";
	// cкорость belt
	for (i = 0, ticks = tmTicks(); i < reps; ++i) {
        octet *round_buf = buf + i * 128 % (sizeof(buf) - 128);
        beltBlockRevW(round_buf);
    }
	ticks = tmTicks() - ticks;
	printf("acclBench::belt:%s:  %3u cpi [%5u ops/sec]\n", name,
		(unsigned)(ticks / reps),
		(unsigned)tmSpeed(reps, ticks));
	// cкорость Xbelt 
	for (i = 0, ticks = tmTicks(); i < reps; ++i) {
        octet *round_buf = buf + i * 128 % (sizeof(buf) - 128);
        XbeltBlockRevW(round_buf);
    }
	ticks = tmTicks() - ticks;
	printf("acclBench::Xbelt:%s:  %3u cpi [%5u ops/sec]\n", name,
		(unsigned)(ticks / reps),
		(unsigned)tmSpeed(reps, ticks));
	// cкорость Wbelt 
	for (i = 0, ticks = tmTicks(); i < reps; ++i) {
        octet *round_buf = buf + i * 128 % (sizeof(buf) - 128);
        WbeltBlockRevW(round_buf);
    }
	ticks = tmTicks() - ticks;
	printf("acclBench::Wbelt:%s:  %3u cpi [%5u ops/sec]\n", name,
		(unsigned)(ticks / reps),
		(unsigned)tmSpeed(reps, ticks));
/////////////////////////////////////////////////////////////
    name = "Neg";
	// cкорость belt
	for (i = 0, ticks = tmTicks(); i < reps; ++i) {
        octet *round_buf = buf + i * 128 % (sizeof(buf) - 2*128);
        beltBlockNeg(round_buf, round_buf + 128);
    }
	ticks = tmTicks() - ticks;
	printf("acclBench::belt:%s:  %3u cpi [%5u ops/sec]\n", name,
		(unsigned)(ticks / reps),
		(unsigned)tmSpeed(reps, ticks));
	// cкорость Xbelt 
	for (i = 0, ticks = tmTicks(); i < reps; ++i) {
        octet *round_buf = buf + i * 128 % (sizeof(buf) - 2*128);
        XbeltBlockNeg(round_buf, round_buf + 128);
    }
	ticks = tmTicks() - ticks;
	printf("acclBench::Xbelt:%s:  %3u cpi [%5u ops/sec]\n", name,
		(unsigned)(ticks / reps),
		(unsigned)tmSpeed(reps, ticks));
	// cкорость Wbelt 
	for (i = 0, ticks = tmTicks(); i < reps; ++i) {
        octet *round_buf = buf + i * 128 % (sizeof(buf) - 2*128);
        WbeltBlockNeg(round_buf, round_buf + 128);
    }
	ticks = tmTicks() - ticks;
	printf("acclBench::Wbelt:%s:  %3u cpi [%5u ops/sec]\n", name,
		(unsigned)(ticks / reps),
		(unsigned)tmSpeed(reps, ticks));
/////////////////////////////////////////////////////////////
    name = "Xor";
	// cкорость belt
	for (i = 0, ticks = tmTicks(); i < reps; ++i) {
        octet *round_buf = buf + i * 128 % (sizeof(buf) - 3*128);
        beltBlockXor(round_buf, round_buf + 128, round_buf + 256);
    }
	ticks = tmTicks() - ticks;
	printf("acclBench::belt:%s:  %3u cpi [%5u ops/sec]\n", name,
		(unsigned)(ticks / reps),
		(unsigned)tmSpeed(reps, ticks));
	// cкорость Xbelt 
	for (i = 0, ticks = tmTicks(); i < reps; ++i) {
        octet *round_buf = buf + i * 128 % (sizeof(buf) - 3*128);
        XbeltBlockXor(round_buf, round_buf + 128, round_buf + 256);
    }
	ticks = tmTicks() - ticks;
	printf("acclBench::Xbelt:%s:  %3u cpi [%5u ops/sec]\n", name,
		(unsigned)(ticks / reps),
		(unsigned)tmSpeed(reps, ticks));
	// cкорость Wbelt 
	for (i = 0, ticks = tmTicks(); i < reps; ++i) {
        octet *round_buf = buf + i * 128 % (sizeof(buf) - 3*128);
        WbeltBlockXor(round_buf, round_buf + 128, round_buf + 256);
    }
	ticks = tmTicks() - ticks;
	printf("acclBench::Wbelt:%s:  %3u cpi [%5u ops/sec]\n", name,
		(unsigned)(ticks / reps),
		(unsigned)tmSpeed(reps, ticks));
	// все нормально
	return TRUE;
}

int main() {
    acclBench();
    return 0;
}

