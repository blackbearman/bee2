/*
*******************************************************************************
\file belt_lcl.h
\brief STB 34.101.31 (belt): local definitions
\project bee2 [cryptographic library]
\created 2012.12.18
\version 2020.03.20
\copyright The Bee2 authors
\license Licensed under the Apache License, Version 2.0 (see LICENSE.txt).
*******************************************************************************
*/

#ifndef __BELT_LCL_H
#define __BELT_LCL_H

#include "bee2/core/word.h"
#include "bee2/core/u32.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
*******************************************************************************
Замечание по интерфейсам

Состояния некоторых связок (например, beltHash) содержат память, которую 
не обязательно поддерживать постоянной между обращениями к функциям связки. 
Это -- дополнительный управляемый стек. Можно передавать указатель на эту 
память через дополнительный параметр (stack, а не state), описав предварительно
глубину стека с помощью функций типа _deep. Мы не стали так делать, чтобы 
не усложнять излишне интерфейсы.
*******************************************************************************
*/

/*
*******************************************************************************
Ускорители

Реализованы быстрые операции над блоками и полублоками belt. Блок
представляется либо как [16]octet, либо как [4]u32,
либо как [W_OF_B(128)]word.

Суффикс U32 в именах макросов и функций означает, что данные интерпретируются
как массив u32. Суффикс W означает, что данные интерпретируются как
массив word.
*******************************************************************************
*/

union _block{
    octet b1[16];
    word w[W_OF_B(128)];
};
typedef union _block block_t;

#define beltBlockSetZero(block)\
	for(int _i = 0; _i < W_OF_B(128); _i++) \
        ((block_t*)(block))->w[_i] = 0;

#define beltBlockRevW(block)\
	for(int _i = 0; _i < W_OF_B(128); _i++) \
	    ((block_t*)(block))->w[_i] = wordRev(((block_t*)(block))->w[_i]);

#define beltBlockNeg(dest, src)\
	for(int _i = 0; _i < W_OF_B(128); _i++) \
	    ((block_t*)(dest))->w[_i] = ~((const block_t*)(src))->w[_i];

#define beltBlockXor(dest, src1, src2)\
	for(int _i = 0; _i < W_OF_B(128); _i++) \
	    ((block_t*)(dest))->w[_i] = ((const block_t*)(src1))->w[_i] ^ ((const block_t*)(src2))->w[_i];

#define beltHalfBlockIsZero(block)\
    ({ \
        int _x = 0; \
        for(int _i = 0; _i < W_OF_B(128)/2; _i++) \
            _x |= (((block_t*)(block))->w[_i] == 0);\
        _x;\
    }) 
/*
#define beltBlockXor2(dest, src)\
    for(int _i = 0; _i < W_OF_B(128); _i++) \
	    ((block_t*)(dest))->w[_i] ^= ((const block_t*)(src))->w[_i];
*/
#define beltBlockXor2(dest, src)\
    for(int _i = 0; _i < 16; _i++) \
	    ((octet*)(dest))[_i] ^= ((const octet*)(src))[_i];

/*
#define beltBlockCopy(dest, src)\
    for(int _i = 0; _i < W_OF_B(128); _i++) \
	    ((block_t*)(dest))->w[_i] = ((const block_t*)(src))->w[_i];
*/

#define beltBlockCopy(dest, src)\
    memCopy(dest, src, 16);

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

/*
*******************************************************************************
Состояния CTR и WBL (используются в DWP, KWP и FMT)
*******************************************************************************
*/

typedef struct
{
	u32 key[8];			/*< форматированный ключ */
	u32 ctr[4];			/*< счетчик */
	octet block[16];	/*< блок гаммы */
	size_t reserved;	/*< резерв октетов гаммы */
} belt_ctr_st;

typedef struct
{
	u32 key[8];			/*< форматированный ключ */
	octet block[16];	/*< вспомогательный блок */
	octet sum[16];		/*< вспомогательная сумма блоков */
	word round;			/*< номер такта */
} belt_wbl_st;

/*
*******************************************************************************
Вспомогательные функции
*******************************************************************************
*/

void beltBlockAddBitSizeU32(u32 block[4], size_t count);
void beltHalfBlockAddBitSizeW(word block[W_OF_B(64)], size_t count);
void beltPolyMul(word c[], const word a[], const word b[], void* stack);
size_t beltPolyMul_deep();
void beltBlockMulC(u32 block[4]);



#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __BELT_LCL_H */
