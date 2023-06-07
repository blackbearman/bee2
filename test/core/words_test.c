/*
*******************************************************************************
\file mem_test.c
\brief Tests for memory functions
\project bee2/test
\created 2023.06.07
\version 2023.06.07
\copyright The Bee2 authors
\license Licensed under the Apache License, Version 2.0 (see LICENSE.txt).
*******************************************************************************
*/

#include <bee2/core/hex.h>
#include <bee2/core/mem.h>
#include <bee2/core/str.h>
#include <bee2/core/util.h>
#include <bee2/core/word.h>

/*
*******************************************************************************
Тестирование

Функции требуют выровненные на границу слова буферы памяти.

\remark Аналогичные функции для невыровненной памяти тестируются в mem_test.
*******************************************************************************
*/

bool_t wordsTest()
{
	const size_t n = 16;
	word memBuf[W_OF_O(n * 3)];
	octet* buf = (octet*)memBuf;
	octet* buf1 = (octet*)memBuf + 16;
	octet* buf2 = (octet*)memBuf + 32;
	// neg
	hexTo(buf1, "F0F1F2F3F4F5F6F7F8");
	memCopy(buf2, buf1, n);
	memNeg(buf1, n);
	wordsNeg(buf2, n);
	if (!memEq(buf1, buf2, n))
		return FALSE;
	if (!wordsEq(buf1, buf2, n))
		return FALSE;
	// cmpRev
	if (FAST(memCmpRev)(buf1, buf2, n) != 0 ||
		SAFE(memCmpRev)(buf1, buf2, n) != 0)
		return FALSE;
	if (FAST(wordsCmpRev)(buf1, buf2, n) != 0 ||
		SAFE(wordsCmpRev)(buf1, buf2, n) != 0)
		return FALSE;
	// xor
	hexTo(buf, "000102030405060708");
	hexTo(buf1, "F0F1F2F3F4F5F6F7F8");
	wordsXor(buf2, buf, buf1, 9);
	if (!memIsRep(buf2, 9, 0xF0))
		return FALSE;
	wordsXor2(buf2, buf1, 9);
	wordsXor2(buf2, buf, 8);
	if (!memIsRep(buf2, 8, 0) || buf2[8] != 0x08)
		return FALSE;
	// все нормально
	return TRUE;
}