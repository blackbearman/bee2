/*
*******************************************************************************
\file word.c
\brief Machine words
\project bee2 [cryptographic library]
\created 2014.07.18
\version 2023.06.01
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

void wordTo(void* dest, size_t count, const word src[])
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