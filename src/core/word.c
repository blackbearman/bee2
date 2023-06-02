/*
*******************************************************************************
\file word.c
\brief Machine words
\project bee2 [cryptographic library]
\created 2014.07.18
\version 2023.06.02
\copyright The Bee2 authors
\license Licensed under the Apache License, Version 2.0 (see LICENSE.txt).
*******************************************************************************
*/

#include "bee2/core/word.h"
#include "bee2/core/mem.h"
#include "bee2/core/util.h"

word wordLoad(const void* buf)
{
	word w;
	memCopy(&w, buf, O_PER_W);
#if (OCTET_ORDER == BIG_ENDIAN)
	w = wordRev(w);
#endif // OCTET_ORDER	
	return w;
}

void wordSave(void* buf, word w)
{
#if (OCTET_ORDER == BIG_ENDIAN)
	w = wordRev(w);
#endif // OCTET_ORDER	
	memCopy(buf, &w, O_PER_W);
}

void wordsTo(void* dest, size_t count, const word src[])
{
	ASSERT(memIsValid(src, (count + O_PER_W - 1) / O_PER_W * O_PER_W));
	ASSERT(memIsValid(dest, count));
#if (OCTET_ORDER == BIG_ENDIAN)
	if (count % O_PER_W)
	{
		size_t t = count / O_PER_W;
		word u = wordRev(src[t]);
		memCopy(dest + t * O_PER_W, &u, count % O_PER_W);
	}
	for (count /= O_PER_W; count--;)
		wordSaveI(dest, count, src[count]);
#else
	memMove(dest, src, count);
#endif // OCTET_ORDER
}

void wordsNeg(void* buf, size_t count)
{
	ASSERT(memIsValid(buf, count));
	ASSERT(memIsAligned(buf, O_PER_W));
	for (; count >= O_PER_W; count -= O_PER_W)
	{
		*(word*)buf = ~*(word*)buf;
		buf = (word*)buf + 1;
	}
	if(count)
		memNeg(buf, count);
}

bool_t SAFE(wordsEq)(const void* buf1, const void* buf2, size_t count)
{
	register word diff = 0;
	ASSERT(memIsValid(buf1, count));
	ASSERT(memIsValid(buf2, count));
	ASSERT(memIsAligned(buf1, O_PER_W));
	ASSERT(memIsAligned(buf2, O_PER_W));
	for (; count >= O_PER_W; count -= O_PER_W)
	{
		diff |= *(const word*)buf1 ^ *(const word*)buf2;
		buf1 = (const word*)buf1 + 1;
		buf2 = (const word*)buf2 + 1;
	}
	if (count)
		diff |= SAFE(memEq)(buf1, buf2, count);
	return wordEq(diff, 0);
}

bool_t FAST(wordsEq)(const void* buf1, const void* buf2, size_t count)
{
	register word diff = 0;
	ASSERT(memIsValid(buf1, count));
	ASSERT(memIsValid(buf2, count));
	ASSERT(memIsAligned(buf1, O_PER_W));
	ASSERT(memIsAligned(buf2, O_PER_W));
	for (; count >= O_PER_W; count -= O_PER_W)
	{
		diff |= *(const word*)buf1 ^ *(const word*)buf2;
		if(diff) 
			return FALSE;
		buf1 = (const word*)buf1 + 1;
		buf2 = (const word*)buf2 + 1;
	}
	if (count)
		diff |= FAST(memEq)(buf1, buf2, count);
	return wordEq(diff, 0);
}

int SAFE(wordsCmpRev)(const void* buf1, const void* buf2, size_t count)
{
	register word less = 0;
	register word greater = 0;
	register word w1;
	register word w2;
	int shift = 0;
	ASSERT(memIsValid(buf1, count));
	ASSERT(memIsValid(buf2, count));
	ASSERT(memIsAligned(buf1, O_PER_W));
	ASSERT(memIsAligned(buf2, O_PER_W));
	if (count % O_PER_W)
	{
		shift = (count / O_PER_W) * O_PER_W;
		w1 = SAFE(memCmpRev)(buf1 + shift, buf2 + shift, count - shift);
		less = wordEq(w1, -1);
		greater = wordEq(w1, 1);
	}
	count /= O_PER_W;
	while (count--)
	{
		w1 = ((const word*)buf1)[count];
		w2 = ((const word*)buf2)[count];
#if (OCTET_ORDER == BIG_ENDIAN)
		w1 = wordRev(w1);
		w2 = wordRev(w2);
#endif
		less |= ~greater & wordLess(w1, w2);
		greater |= ~less & wordGreater(w1, w2);
	}
	w1 = w2 = 0;
	return (wordEq(less, 0) - 1) | wordNeq(greater, 0);
}

int FAST(wordsCmpRev)(const void* buf1, const void* buf2, size_t count)
{
	register word w1;
	register word w2;
	int shift = 0;
	ASSERT(memIsValid(buf1, count));
	ASSERT(memIsValid(buf2, count));
	ASSERT(memIsAligned(buf1, O_PER_W));
	ASSERT(memIsAligned(buf2, O_PER_W));
	if (count % O_PER_W)
	{
		shift = (count/O_PER_W) * O_PER_W;
		w1 = SAFE(memCmpRev)(buf1 + shift, buf2 + shift, count - shift);
		if(w1)
			return w1;
	}
	count /= O_PER_W;
	while (count--) {
		w1 = ((const word*)buf1)[count];
		w2 = ((const word*)buf2)[count];
#if (OCTET_ORDER == BIG_ENDIAN)
		w1 = wordRev(w1);
		w2 = wordRev(w2);
#endif
		if (w1 > w2)
			return 1;
		else if (w1 < w2)
			return -1;
	}
	return 0;
}

bool_t SAFE(wordsIsZero)(const void* buf, size_t count)
{
	register word diff = 0;
	ASSERT(memIsValid(buf, count));
	ASSERT(memIsAligned(buf, O_PER_W));
	for (; count >= O_PER_W; count -= O_PER_W)
	{
		diff |= *(const word*)buf;
		buf = (const word*)buf + 1;
	}
	while (count--)
	{
		diff |= *(const octet*)buf;
		buf = (const octet*)buf + 1;
	}
	return (bool_t)wordEq(diff, 0);
}

bool_t FAST(wordsIsZero)(const void* buf, size_t count)
{
	ASSERT(memIsValid(buf, count));
	ASSERT(memIsAligned(buf, O_PER_W));
	for (; count >= O_PER_W; count -= O_PER_W, buf = (const word*)buf + 1)
		if (*(const word*)buf)
			return FALSE;
	for (; count--; buf = (const octet*)buf + 1)
		if (*(const octet*)buf)
			return FALSE;
	return TRUE;
}

void wordsXor(void* dest, const void* src1, const void* src2, size_t count)
{
	ASSERT(memIsSameOrDisjoint(src1, dest, count));
	ASSERT(memIsSameOrDisjoint(src2, dest, count));
	ASSERT(memIsAligned(dest, O_PER_W));
	ASSERT(memIsAligned(src1, O_PER_W));
	ASSERT(memIsAligned(src2, O_PER_W));
	for (; count >= O_PER_W; count -= O_PER_W)
	{
		*(word*)dest = *(const word*)src1 ^ *(const word*)src2;
		src1 = (const word*)src1 + 1;
		src2 = (const word*)src2 + 1;
		dest = (word*)dest + 1;
	}
	if (count)
		memXor(dest, src1, src2, count);
}

void wordsXor2(void* dest, const void* src, size_t count)
{
	ASSERT(memIsSameOrDisjoint(src, dest, count));
	ASSERT(memIsAligned(dest, O_PER_W));
	ASSERT(memIsAligned(src, O_PER_W));
	for (; count >= O_PER_W; count -= O_PER_W)
	{
		*(word*)dest ^= *(const word*)src;
		src = (const word*)src + 1;
		dest = (word*)dest + 1;
	}
	if (count)
		memXor2(dest, src, count);
}

void wordsSwap(void* buf1, void* buf2, size_t count)
{
	ASSERT(memIsDisjoint(buf1, buf2, count));
	ASSERT(memIsAligned(buf1, O_PER_W));
	ASSERT(memIsAligned(buf2, O_PER_W));
	for (; count >= O_PER_W; count -= O_PER_W)
	{
		SWAP(*(word*)buf1, *(word*)buf2);
		buf1 = (word*)buf1 + 1;
		buf2 = (word*)buf2 + 1;
	}
	if (count)
		memSwap(buf1, buf2, count);
}
