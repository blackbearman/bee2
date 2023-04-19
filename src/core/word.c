/*
*******************************************************************************
\file word.c
\brief Machine words
\project bee2 [cryptographic library]
\created 2014.07.18
\version 2019.07.08
\copyright The Bee2 authors
\license Licensed under the Apache License, Version 2.0 (see LICENSE.txt).
*******************************************************************************
*/

#include "bee2/core/word.h"
#include<string.h>

word wordLoad(const void* buf)
{
	word w;
	memcpy(&w, buf, O_PER_W);
#if (OCTET_ORDER == BIG_ENDIAN)
	w = wordRev(w);
#endif // OCTET_ORDER	
	return w;
}