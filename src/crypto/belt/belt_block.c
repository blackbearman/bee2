/*
*******************************************************************************
\file belt_block.c
\brief STB 34.101.31 (belt): block encryption
\project bee2 [cryptographic library]
\created 2012.12.18
\version 2024.01.25
\copyright The Bee2 authors
\license Licensed under the Apache License, Version 2.0 (see LICENSE.txt).
*******************************************************************************
*/

#include "bee2/core/mem.h"
#include "bee2/core/u32.h"
#include "bee2/core/util.h"
#include "bee2/crypto/belt.h"
#include "belt_lcl.h"

/*
*******************************************************************************
H-блок

\remark H-блок можно построить с помощью следующей функции:
\code
	void beltHGen(octet H[256])
	{
		size_t x, i;
		H[10] = 0, H[11] = 0x8E;
		for (x = 12; x < 10 + 256; ++x)
		{
			word t = H[(x - 1) % 256];
			for (i = 0; i < 116; ++i)
				t = t >> 1 | wordParity(t & 0x63) << 7;
			H[x % 256] = (octet)t;
		}
	}
\endcode
*******************************************************************************
*/

#define H16(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p)\
	0x##a, 0x##b, 0x##c, 0x##d, 0x##e, 0x##f, 0x##g, 0x##h,\
	0x##i, 0x##j, 0x##k, 0x##l, 0x##m, 0x##n, 0x##o, 0x##p

static const octet H[256] = {
	H16(B1,94,BA,C8,0A,08,F5,3B,36,6D,00,8E,58,4A,5D,E4),
	H16(85,04,FA,9D,1B,B6,C7,AC,25,2E,72,C2,02,FD,CE,0D),
	H16(5B,E3,D6,12,17,B9,61,81,FE,67,86,AD,71,6B,89,0B),
	H16(5C,B0,C0,FF,33,C3,56,B8,35,C4,05,AE,D8,E0,7F,99),
	H16(E1,2B,DC,1A,E2,82,57,EC,70,3F,CC,F0,95,EE,8D,F1),
	H16(C1,AB,76,38,9F,E6,78,CA,F7,C6,F8,60,D5,BB,9C,4F),
	H16(F3,3C,65,7B,63,7C,30,6A,DD,4E,A7,79,9E,B2,3D,31),
	H16(3E,98,B5,6E,27,D3,BC,CF,59,1E,18,1F,4C,5A,B7,93),
	H16(E9,DE,E7,2C,8F,0C,0F,A6,2D,DB,49,F4,6F,73,96,47),
	H16(06,07,53,16,ED,24,7A,37,39,CB,A3,83,03,A9,8B,F6),
	H16(92,BD,9B,1C,E5,D1,41,01,54,45,FB,C9,5E,4D,0E,F2),
	H16(68,20,80,AA,22,7D,64,2F,26,87,F9,34,90,40,55,11),
	H16(BE,32,97,13,43,FC,9A,48,A0,2A,88,5F,19,4B,09,A1),
	H16(7E,CD,A4,D0,15,44,AF,8C,A5,84,50,BF,66,D2,E8,8A),
	H16(A2,D7,46,52,42,A8,DF,B3,69,74,C5,51,EB,23,29,21),
	H16(D4,EF,D9,B4,3A,62,28,75,91,14,10,EA,77,6C,DA,1D),
};

const octet* beltH()
{
	return H;
}

/*
*******************************************************************************
Расширение ключа
*******************************************************************************
*/

void beltKeyExpand(octet key_[32], const octet key[], size_t len)
{
	ASSERT(memIsValid(key_, 32));
	ASSERT(len == 16 || len == 24 || len == 32);
	ASSERT(memIsValid(key, len));
	memMove(key_, key, len);
	if (len == 16)
		memCopy(key_ + 16, key_, 16);
	else if (len == 24)
	{
		u32* w = (u32*)key_;
		w[6] = w[0] ^ w[1] ^ w[2];
		w[7] = w[3] ^ w[4] ^ w[5];
	}
}

void beltKeyExpand2(u32 key_[8], const octet key[], size_t len)
{
	ASSERT(memIsValid(key_, 32));
	ASSERT(len == 16 || len == 24 || len == 32);
	ASSERT(memIsValid(key, len));
	u32From(key_, key, len);
	if (len == 16)
	{
		key_[4] = key_[0];
		key_[5] = key_[1];
		key_[6] = key_[2];
		key_[7] = key_[3];
	}
	else if (len == 24)
	{
		key_[6] = key_[0] ^ key_[1] ^ key_[2];
		key_[7] = key_[3] ^ key_[4] ^ key_[5];
	}
}

/*
*******************************************************************************
Расширенные H-блоки
*******************************************************************************
*/

#define HEx(x, r) ((u32)0x##x << r | (u32)0x##x >> (32 - r))
#define HEx16(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p, r)\
	HEx(a, r), HEx(b, r), HEx(c, r), HEx(d, r),\
	HEx(e, r), HEx(f, r), HEx(g, r), HEx(h, r),\
	HEx(i, r), HEx(j, r), HEx(k, r), HEx(l, r),\
	HEx(m, r), HEx(n, r), HEx(o, r), HEx(p, r)

static const u32 H5[256] = {
	HEx16(B1,94,BA,C8,0A,08,F5,3B,36,6D,00,8E,58,4A,5D,E4, 5),
	HEx16(85,04,FA,9D,1B,B6,C7,AC,25,2E,72,C2,02,FD,CE,0D, 5),
	HEx16(5B,E3,D6,12,17,B9,61,81,FE,67,86,AD,71,6B,89,0B, 5),
	HEx16(5C,B0,C0,FF,33,C3,56,B8,35,C4,05,AE,D8,E0,7F,99, 5),
	HEx16(E1,2B,DC,1A,E2,82,57,EC,70,3F,CC,F0,95,EE,8D,F1, 5),
	HEx16(C1,AB,76,38,9F,E6,78,CA,F7,C6,F8,60,D5,BB,9C,4F, 5),
	HEx16(F3,3C,65,7B,63,7C,30,6A,DD,4E,A7,79,9E,B2,3D,31, 5),
	HEx16(3E,98,B5,6E,27,D3,BC,CF,59,1E,18,1F,4C,5A,B7,93, 5),
	HEx16(E9,DE,E7,2C,8F,0C,0F,A6,2D,DB,49,F4,6F,73,96,47, 5),
	HEx16(06,07,53,16,ED,24,7A,37,39,CB,A3,83,03,A9,8B,F6, 5),
	HEx16(92,BD,9B,1C,E5,D1,41,01,54,45,FB,C9,5E,4D,0E,F2, 5),
	HEx16(68,20,80,AA,22,7D,64,2F,26,87,F9,34,90,40,55,11, 5),
	HEx16(BE,32,97,13,43,FC,9A,48,A0,2A,88,5F,19,4B,09,A1, 5),
	HEx16(7E,CD,A4,D0,15,44,AF,8C,A5,84,50,BF,66,D2,E8,8A, 5),
	HEx16(A2,D7,46,52,42,A8,DF,B3,69,74,C5,51,EB,23,29,21, 5),
	HEx16(D4,EF,D9,B4,3A,62,28,75,91,14,10,EA,77,6C,DA,1D, 5),
};

static const u32 H13[256] = {
	HEx16(B1,94,BA,C8,0A,08,F5,3B,36,6D,00,8E,58,4A,5D,E4, 13),
	HEx16(85,04,FA,9D,1B,B6,C7,AC,25,2E,72,C2,02,FD,CE,0D, 13),
	HEx16(5B,E3,D6,12,17,B9,61,81,FE,67,86,AD,71,6B,89,0B, 13),
	HEx16(5C,B0,C0,FF,33,C3,56,B8,35,C4,05,AE,D8,E0,7F,99, 13),
	HEx16(E1,2B,DC,1A,E2,82,57,EC,70,3F,CC,F0,95,EE,8D,F1, 13),
	HEx16(C1,AB,76,38,9F,E6,78,CA,F7,C6,F8,60,D5,BB,9C,4F, 13),
	HEx16(F3,3C,65,7B,63,7C,30,6A,DD,4E,A7,79,9E,B2,3D,31, 13),
	HEx16(3E,98,B5,6E,27,D3,BC,CF,59,1E,18,1F,4C,5A,B7,93, 13),
	HEx16(E9,DE,E7,2C,8F,0C,0F,A6,2D,DB,49,F4,6F,73,96,47, 13),
	HEx16(06,07,53,16,ED,24,7A,37,39,CB,A3,83,03,A9,8B,F6, 13),
	HEx16(92,BD,9B,1C,E5,D1,41,01,54,45,FB,C9,5E,4D,0E,F2, 13),
	HEx16(68,20,80,AA,22,7D,64,2F,26,87,F9,34,90,40,55,11, 13),
	HEx16(BE,32,97,13,43,FC,9A,48,A0,2A,88,5F,19,4B,09,A1, 13),
	HEx16(7E,CD,A4,D0,15,44,AF,8C,A5,84,50,BF,66,D2,E8,8A, 13),
	HEx16(A2,D7,46,52,42,A8,DF,B3,69,74,C5,51,EB,23,29,21, 13),
	HEx16(D4,EF,D9,B4,3A,62,28,75,91,14,10,EA,77,6C,DA,1D, 13),
};

static const u32 H21[256] = {
	HEx16(B1,94,BA,C8,0A,08,F5,3B,36,6D,00,8E,58,4A,5D,E4, 21),
	HEx16(85,04,FA,9D,1B,B6,C7,AC,25,2E,72,C2,02,FD,CE,0D, 21),
	HEx16(5B,E3,D6,12,17,B9,61,81,FE,67,86,AD,71,6B,89,0B, 21),
	HEx16(5C,B0,C0,FF,33,C3,56,B8,35,C4,05,AE,D8,E0,7F,99, 21),
	HEx16(E1,2B,DC,1A,E2,82,57,EC,70,3F,CC,F0,95,EE,8D,F1, 21),
	HEx16(C1,AB,76,38,9F,E6,78,CA,F7,C6,F8,60,D5,BB,9C,4F, 21),
	HEx16(F3,3C,65,7B,63,7C,30,6A,DD,4E,A7,79,9E,B2,3D,31, 21),
	HEx16(3E,98,B5,6E,27,D3,BC,CF,59,1E,18,1F,4C,5A,B7,93, 21),
	HEx16(E9,DE,E7,2C,8F,0C,0F,A6,2D,DB,49,F4,6F,73,96,47, 21),
	HEx16(06,07,53,16,ED,24,7A,37,39,CB,A3,83,03,A9,8B,F6, 21),
	HEx16(92,BD,9B,1C,E5,D1,41,01,54,45,FB,C9,5E,4D,0E,F2, 21),
	HEx16(68,20,80,AA,22,7D,64,2F,26,87,F9,34,90,40,55,11, 21),
	HEx16(BE,32,97,13,43,FC,9A,48,A0,2A,88,5F,19,4B,09,A1, 21),
	HEx16(7E,CD,A4,D0,15,44,AF,8C,A5,84,50,BF,66,D2,E8,8A, 21),
	HEx16(A2,D7,46,52,42,A8,DF,B3,69,74,C5,51,EB,23,29,21, 21),
	HEx16(D4,EF,D9,B4,3A,62,28,75,91,14,10,EA,77,6C,DA,1D, 21),
};

static const u32 H29[256] = {
	HEx16(B1,94,BA,C8,0A,08,F5,3B,36,6D,00,8E,58,4A,5D,E4, 29),
	HEx16(85,04,FA,9D,1B,B6,C7,AC,25,2E,72,C2,02,FD,CE,0D, 29),
	HEx16(5B,E3,D6,12,17,B9,61,81,FE,67,86,AD,71,6B,89,0B, 29),
	HEx16(5C,B0,C0,FF,33,C3,56,B8,35,C4,05,AE,D8,E0,7F,99, 29),
	HEx16(E1,2B,DC,1A,E2,82,57,EC,70,3F,CC,F0,95,EE,8D,F1, 29),
	HEx16(C1,AB,76,38,9F,E6,78,CA,F7,C6,F8,60,D5,BB,9C,4F, 29),
	HEx16(F3,3C,65,7B,63,7C,30,6A,DD,4E,A7,79,9E,B2,3D,31, 29),
	HEx16(3E,98,B5,6E,27,D3,BC,CF,59,1E,18,1F,4C,5A,B7,93, 29),
	HEx16(E9,DE,E7,2C,8F,0C,0F,A6,2D,DB,49,F4,6F,73,96,47, 29),
	HEx16(06,07,53,16,ED,24,7A,37,39,CB,A3,83,03,A9,8B,F6, 29),
	HEx16(92,BD,9B,1C,E5,D1,41,01,54,45,FB,C9,5E,4D,0E,F2, 29),
	HEx16(68,20,80,AA,22,7D,64,2F,26,87,F9,34,90,40,55,11, 29),
	HEx16(BE,32,97,13,43,FC,9A,48,A0,2A,88,5F,19,4B,09,A1, 29),
	HEx16(7E,CD,A4,D0,15,44,AF,8C,A5,84,50,BF,66,D2,E8,8A, 29),
	HEx16(A2,D7,46,52,42,A8,DF,B3,69,74,C5,51,EB,23,29,21, 29),
	HEx16(D4,EF,D9,B4,3A,62,28,75,91,14,10,EA,77,6C,DA,1D, 29),
};

/*
*******************************************************************************
G-блоки
*******************************************************************************
*/
#define G5(x)\
	H5[(x) & 255] ^ H13[(x) >> 8 & 255] ^ H21[(x) >> 16 & 255] ^ H29[(x) >> 24]
#define G13(x)\
	H13[(x) & 255] ^ H21[(x) >> 8 & 255] ^ H29[(x) >> 16 & 255] ^ H5[(x) >> 24]
#define G21(x)\
	H21[(x) & 255] ^ H29[(x) >> 8 & 255] ^ H5[(x) >> 16 & 255] ^ H13[(x) >> 24]

/*
*******************************************************************************
Тактовая подстановка

Макрос R реализует шаги 2.1-2.9 алгоритмов зашифрования и расшифрования.

На шагах 2.4-2.6 дополнительный регистр е не используется.
Нужные данные сохраняются в регистрах b и c.

Параметр-макрос subkey задает порядок использования тактовых ключей:
порядок subkey = subkey_e используется при зашифровании,
порядок subkey = subkey_d -- при расшифровании.
*******************************************************************************
*/
#define R(a, b, c, d, K, i, subkey)\
	*b ^= G5(*a + subkey(K, i, 0));\
	*c ^= G21(*d + subkey(K, i, 1));\
	*a -= G13(*b + subkey(K, i, 2));\
	*c += *b;\
	*b += G21(*c + subkey(K, i, 3)) ^ i;\
	*c -= *b;\
	*d += G13(*c + subkey(K, i, 4));\
	*b ^= G21(*a + subkey(K, i, 5));\
	*c ^= G5(*d + subkey(K, i, 6));\

#define subkey_e(K, i, j) K[(7 * (i) - 7 + (j)) % 8]
#define subkey_d(K, i, j) K[(7 * (i) - 1 - (j)) % 8]

/*
*******************************************************************************
Такты зашифрования

Перестановка содержимого регистров a, b, c, d реализуется перестановкой
параметров макроса R. После выполнения последнего макроса R и шагов 2.10-2.12
алгоритма зашифрования в регистрах a, b, c, d будут находиться значения,
соответствующие спецификации belt.

Окончательная перестановка abcd -> bdac реализуется инверсиями:
a <-> b, c <-> d, b <-> c.
*******************************************************************************
*/
#define E(a, b, c, d, K)\
	R(a, b, c, d, K, 1, subkey_e);\
	R(b, d, a, c, K, 2, subkey_e);\
	R(d, c, b, a, K, 3, subkey_e);\
	R(c, a, d, b, K, 4, subkey_e);\
	R(a, b, c, d, K, 5, subkey_e);\
	R(b, d, a, c, K, 6, subkey_e);\
	R(d, c, b, a, K, 7, subkey_e);\
	R(c, a, d, b, K, 8, subkey_e);\
	*a ^= *b, *b ^= *a, *a ^= *b;\
	*c ^= *d, *d ^= *c, *c ^= *d;\
	*b ^= *c, *c ^= *b, *b ^= *c;\

/*
*******************************************************************************
Такты расшифрования

Перестановка содержимого регистров a, b, c, d реализуется перестановкой
параметров макроса R. После выполнения последнего макроса R и шагов 2.10-2.12
алгоритма расшифрования в регистрах a, b, c, d будут находиться значения,
соответствующие спецификации belt.

Окончательная перестановка abcd -> cadb реализуется инверсиями:
a <-> b, c <-> d, a <-> d.
*******************************************************************************
*/
#define D(a, b, c, d, K)\
	R(a, b, c, d, K, 8, subkey_d);\
	R(c, a, d, b, K, 7, subkey_d);\
	R(d, c, b, a, K, 6, subkey_d);\
	R(b, d, a, c, K, 5, subkey_d);\
	R(a, b, c, d, K, 4, subkey_d);\
	R(c, a, d, b, K, 3, subkey_d);\
	R(d, c, b, a, K, 2, subkey_d);\
	R(b, d, a, c, K, 1, subkey_d);\
	*a ^= *b, *b ^= *a, *a ^= *b;\
	*c ^= *d, *d ^= *c, *c ^= *d;\
	*a ^= *d, *d ^= *a, *a ^= *d;\

/*
*******************************************************************************
Зашифрование блока
*******************************************************************************
*/
void beltBlockEncr(octet block[16], const u32 key[8])
{
	u32* t = (u32*)block;
	ASSERT(memIsDisjoint2(block, 16, key, 32));
#if (OCTET_ORDER == BIG_ENDIAN)
	t[0] = u32Rev(t[0]);
	t[1] = u32Rev(t[1]);
	t[2] = u32Rev(t[2]);
	t[3] = u32Rev(t[3]);
#endif
	E((t + 0), (t + 1), (t + 2), (t + 3), key);
#if (OCTET_ORDER == BIG_ENDIAN)
	t[3] = u32Rev(t[3]);
	t[2] = u32Rev(t[2]);
	t[1] = u32Rev(t[1]);
	t[0] = u32Rev(t[0]);
#endif
}

void beltBlockEncr2(u32 block[4], const u32 key[8])
{
	E((block + 0), (block + 1), (block + 2), (block + 3), key);
}

void beltBlockEncr3(u32* a, u32* b, u32* c, u32* d, const u32 key[8])
{
	E(a, b, c, d, key);
}

/*
*******************************************************************************
Расшифрование блока
*******************************************************************************
*/
void beltBlockDecr(octet block[16], const u32 key[8])
{
	u32* t = (u32*)block;
	ASSERT(memIsDisjoint2(block, 16, key, 32));
#if (OCTET_ORDER == BIG_ENDIAN)
	t[0] = u32Rev(t[0]);
	t[1] = u32Rev(t[1]);
	t[2] = u32Rev(t[2]);
	t[3] = u32Rev(t[3]);
#endif
	D((t + 0), (t + 1), (t + 2), (t + 3), key);
#if (OCTET_ORDER == BIG_ENDIAN)
	t[3] = u32Rev(t[3]);
	t[2] = u32Rev(t[2]);
	t[1] = u32Rev(t[1]);
	t[0] = u32Rev(t[0]);
#endif
}

void beltBlockDecr2(u32 block[4], const u32 key[8])
{
	D((block + 0), (block + 1), (block + 2), (block + 3), key);
}

void beltBlockDecr3(u32* a, u32* b, u32* c, u32* d, const u32 key[8])
{
	D(a, b, c, d, key);
}
