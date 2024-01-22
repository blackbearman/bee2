/*
*******************************************************************************
\file g12s.c
\brief GOST R 34.10-94 (Russia): digital signature algorithms
\project bee2 [cryptographic library]
\created 2012.07.09
\version 2023.11.08
\copyright The Bee2 authors
\license Licensed under the Apache License, Version 2.0 (see LICENSE.txt).
*******************************************************************************
*/

#include "bee2/core/blob.h"
#include "bee2/core/err.h"
#include "bee2/core/mem.h"
#include "bee2/core/str.h"
#include "bee2/core/util.h"
#include "bee2/crypto/g12s.h"
#include "bee2/math/ecp.h"
#include "bee2/math/gfp.h"
#include "bee2/math/ww.h"
#include "bee2/math/zz.h"

/*
*******************************************************************************
Глубина стека

Высокоуровневые функции сообщают о потребностях в стековой памяти через 
функции интерфейса g12s_deep_i. Потребности не должны учитывать память для 
размещения описаний базового поля и эллиптической кривой.
*******************************************************************************
*/

typedef size_t (*g12s_deep_i)(
	size_t n,					/* размерность (в машинных словах) */
	size_t f_deep,				/* глубина стека базового поля */
	size_t ec_d,				/* число проективных координат */
	size_t ec_deep				/* глубина стека эллиптической кривой */
);

/*
*******************************************************************************
Стандартные параметры: проверочный пример A.1 из ГОСТ Р 34.10-2012
*******************************************************************************
*/

static const char _a1_name[] = "1.2.643.2.2.35.0";

static const u32 _a1_l = 256;

static const octet _a1_p[] = {
	0x31, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
};

static const octet _a1_a[] = {
	0x07,
};

static const octet _a1_b[] = {
	0x7E, 0x3B, 0xE2, 0xDA, 0xE9, 0x0C, 0x4C, 0x51,
	0x2A, 0xFC, 0x72, 0x34, 0x6A, 0x6E, 0x3F, 0x56,
	0x40, 0xEF, 0xAF, 0xFB, 0x22, 0xE0, 0xB8, 0x39,
	0xE7, 0x8C, 0x93, 0xAA, 0x98, 0xF4, 0xBF, 0x5F,
};

static const octet _a1_q[] = {
	0xB3, 0xF5, 0xCC, 0x3A, 0x19, 0xFC, 0x9C, 0xC5,
	0x54, 0x61, 0x97, 0x92, 0x18, 0x8A, 0xFE, 0x50,
	0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
};

static const octet _a1_n = 1;

static const octet _a1_xP[] = {
	0x02,
};

static const octet _a1_yP[] = {
	0xC8, 0x8F, 0x7E, 0xEA, 0xBC, 0xAB, 0x96, 0x2B,
	0x12, 0x67, 0xA2, 0x9C, 0x0A, 0x7F, 0xC9, 0x85,
	0x9C, 0xD1, 0x16, 0x0E, 0x03, 0x16, 0x63, 0xBD,
	0xD4, 0x47, 0x51, 0xE6, 0xA0, 0xA8, 0xE2, 0x08,
};

/*
*******************************************************************************
Параметры КриптоПро,  набор A (1.2.643.2.2.35.1)
*******************************************************************************
*/

static const char _cryptoproA_name[] = "1.2.643.2.2.35.1";

static const u32 _cryptoproA_l = 256;

static const octet _cryptoproA_p[] = {
	0x97, 0xFD, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
};

static const octet _cryptoproA_a[] = {
	0x94, 0xFD, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
};

static const octet _cryptoproA_b[] = {
	0xA6,
};

static const octet _cryptoproA_q[] = {
	0x93, 0xB8, 0x61, 0xB7, 0x09, 0x1B, 0x84, 0x45,
	0x00, 0xD1, 0x5A, 0x99, 0x70, 0x10, 0x61, 0x6C,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
};

static const octet _cryptoproA_n = 1;

static const octet _cryptoproA_xP[] = {
	0x01,
};

static const octet _cryptoproA_yP[] = {
	0x14, 0x1E, 0x9F, 0x9E, 0x9C, 0xC9, 0xAC, 0x22,
	0xB1, 0xE3, 0x23, 0xDF, 0x2D, 0x4F, 0x29, 0x35,
	0x76, 0x2B, 0x3F, 0x45, 0x5A, 0x50, 0xDF, 0x27,
	0xDA, 0x9C, 0x98, 0xE0, 0x71, 0xE4, 0x91, 0x8D,
};

/*
*******************************************************************************
Параметры КриптоПро,  набор B (1.2.643.2.2.35.2)
*******************************************************************************
*/

static const char _cryptoproB_name[] = "1.2.643.2.2.35.2";

static const u32 _cryptoproB_l = 256;

static const octet _cryptoproB_p[] = {
	0x99, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
};

static const octet _cryptoproB_a[] = {
	0x96, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
};

static const octet _cryptoproB_b[] = {
	0x8B, 0xBC, 0x1B, 0x7E, 0xCE, 0xD4, 0x49, 0x2F,
	0x18, 0x2B, 0xFF, 0x73, 0x93, 0x25, 0x79, 0xE9,
	0x0A, 0xF8, 0x3D, 0x5C, 0xC2, 0xD3, 0xA7, 0x66,
	0xF8, 0xA5, 0x69, 0xA2, 0x19, 0xF4, 0x1A, 0x3E,
};

static const octet _cryptoproB_q[] = {
	0x8F, 0x19, 0x8A, 0xCC, 0x1B, 0x16, 0x97, 0xE4, 
	0xE5, 0x24, 0xA6, 0xF1, 0xFF, 0x0C, 0x70, 0x5F, 
	0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
};

static const octet _cryptoproB_n = 1;

static const octet _cryptoproB_xP[] = {
	0x01,
};

static const octet _cryptoproB_yP[] = {
	0xFC, 0x7E, 0x71, 0x17, 0xD7, 0xF8, 0x4B, 0x74,
	0xFB, 0xEC, 0x03, 0x8D, 0x85, 0xC9, 0x45, 0xC5,
	0xE5, 0x70, 0xC0, 0xB2, 0x3E, 0x1C, 0x3D, 0xB8,
	0x80, 0x66, 0xF9, 0x59, 0x43, 0x12, 0xA8, 0x3F,
};

/*
*******************************************************************************
Параметры КриптоПро,  набор C (1.2.643.2.2.35.3)
*******************************************************************************
*/

static const char _cryptoproC_name[] = "1.2.643.2.2.35.3";

static const u32 _cryptoproC_l = 256;

static const octet _cryptoproC_p[] = {
	0x9B, 0x75, 0x2D, 0x02, 0xB9, 0xF7, 0x98, 0x79,
	0xD3, 0x51, 0x90, 0x78, 0x86, 0x6E, 0x84, 0xCF,
	0xAA, 0xC8, 0x41, 0x6B, 0x5E, 0xC8, 0x1E, 0xAB,
	0x07, 0x81, 0x85, 0x5A, 0x5F, 0x60, 0x9F, 0x9B,
};

static const octet _cryptoproC_a[] = {
	0x98, 0x75, 0x2D, 0x02, 0xB9, 0xF7, 0x98, 0x79,
	0xD3, 0x51, 0x90, 0x78, 0x86, 0x6E, 0x84, 0xCF,
	0xAA, 0xC8, 0x41, 0x6B, 0x5E, 0xC8, 0x1E, 0xAB,
	0x07, 0x81, 0x85, 0x5A, 0x5F, 0x60, 0x9F, 0x9B,
};

static const octet _cryptoproC_b[] = {
	0x5A, 0x80,
};

static const octet _cryptoproC_q[] = {
	0xB9, 0x0B, 0x98, 0x98, 0x65, 0x3A, 0x2F, 0xF0,
	0x74, 0xFB, 0xDD, 0x1E, 0x51, 0xA3, 0x2C, 0x58,
	0xAA, 0xC8, 0x41, 0x6B, 0x5E, 0xC8, 0x1E, 0xAB,
	0x07, 0x81, 0x85, 0x5A, 0x5F, 0x60, 0x9F, 0x9B,
};

static const octet _cryptoproC_n = 1;

static const octet _cryptoproC_xP[] = {
	0x00,
};

static const octet _cryptoproC_yP[] = {
	0x67, 0xBB, 0xB3, 0xFD, 0x0D, 0x55, 0x6E, 0x36,
	0x8F, 0x1A, 0x64, 0xD4, 0x40, 0xC4, 0x4D, 0x4D,
	0xEE, 0xC0, 0x08, 0xCD, 0x83, 0x37, 0xBF, 0x3C,
	0x8C, 0x1A, 0x71, 0x43, 0x57, 0xE5, 0xEC, 0x41,
};

/*
*******************************************************************************
Параметры CryptoCom (1.2.643.2.9.1.8.1)
*******************************************************************************
*/

static const char _cryptocom_name[] = "1.2.643.2.9.1.8.1";

static const u32 _cryptocom_l = 256;

static const octet _cryptocom_p[] = {
	0xC7, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0,
};

static const octet _cryptocom_a[] = {
	0xC4, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0,
};

static const octet _cryptocom_b[] = {
	0x0C, 0x08, 0x55, 0xE9, 0x07, 0xE4, 0xD5, 0x87,
	0x77, 0x4B, 0xD4, 0x8F, 0x08, 0xE9, 0x32, 0x16,
	0xE8, 0x32, 0x82, 0xF8, 0xF1, 0xF1, 0xD0, 0xF7,
	0x9F, 0x74, 0xBC, 0x5E, 0x26, 0xB4, 0x06, 0x2D,
};

static const octet _cryptocom_q[] = {
	0x85, 0x7B, 0xE8, 0xB6, 0x54, 0x8A, 0x45, 0xB7,
	0x28, 0xE4, 0xBD, 0xF4, 0xA2, 0x17, 0x61, 0x60,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x5F,
};

static const octet _cryptocom_n = 2;

static const octet _cryptocom_xP[] = {
	0x02,
};

static const octet _cryptocom_yP[] = {
	0x2C, 0xB2, 0xB6, 0xCC, 0xC8, 0xED, 0x0B, 0x44,
	0x6F, 0x70, 0xE9, 0x4A, 0x26, 0x8B, 0x24, 0xEB,
	0x17, 0x6A, 0x72, 0x5E, 0x10, 0x01, 0x8D, 0xC1,
	0xF5, 0x3E, 0x81, 0xF8, 0x4B, 0x03, 0x0E, 0xA2,
};

/*
*******************************************************************************
Стандартные параметры: проверочный пример A.2 из ГОСТ Р 34.10-2012
*******************************************************************************
*/

static const char _a2_name[] = "1.2.643.7.1.2.1.2.0";

static const u32 _a2_l = 512;

static const octet _a2_p[] = {
	0x73, 0x63, 0xBE, 0x28, 0xF5, 0xBB, 0x64, 0x16, 
	0xD8, 0x4D, 0x22, 0xAC, 0x6F, 0x33, 0xB8, 0x35, 
	0x6D, 0x54, 0xE4, 0x80, 0x7E, 0x04, 0x58, 0x04, 
	0x4A, 0x70, 0xF4, 0x1A, 0x74, 0x52, 0xD8, 0xF1, 
	0x5D, 0xD1, 0xD2, 0xB5, 0x09, 0x7C, 0xEB, 0xD4, 
	0x04, 0x0F, 0xB9, 0xFF, 0xB2, 0x14, 0x2B, 0x92, 
	0x80, 0xEE, 0x2F, 0x6B, 0x7B, 0x26, 0x0D, 0x55, 
	0xC7, 0x23, 0x00, 0xFE, 0xD1, 0xAC, 0x31, 0x45, 
};

static const octet _a2_a[] = {
	0x07, 
};

static const octet _a2_b[] = {
	0xDC, 0x2A, 0x30, 0x4F, 0x08, 0xA3, 0xD0, 0xFA, 
	0x97, 0x68, 0xDD, 0x2A, 0x0C, 0x54, 0x9E, 0xBC, 
	0x74, 0xCF, 0xE0, 0x58, 0xCA, 0x89, 0x0A, 0x48, 
	0x22, 0x73, 0xAD, 0xB2, 0x13, 0x40, 0x83, 0x61, 
	0x43, 0xAC, 0xA1, 0xEC, 0x49, 0xB6, 0x88, 0xD7, 
	0xFD, 0x00, 0x94, 0xE4, 0x77, 0xF3, 0xC5, 0x8B, 
	0x74, 0xEB, 0x57, 0x4E, 0xA5, 0xCF, 0xD8, 0x29, 
	0xDA, 0x16, 0x11, 0xA3, 0x06, 0x08, 0xFF, 0x1C, 
};

static const octet _a2_q[] = {
	0xDF, 0xE6, 0xE6, 0x87, 0xF1, 0xAA, 0x44, 0xD6, 
	0x95, 0xC5, 0x23, 0xBE, 0xED, 0x25, 0x6E, 0xD8, 
	0xF1, 0x23, 0xC4, 0xEC, 0x5E, 0x5C, 0x90, 0x19, 
	0xC7, 0xBA, 0x1D, 0xCB, 0x7E, 0x2D, 0x2F, 0xA8, 
	0x5D, 0xD1, 0xD2, 0xB5, 0x09, 0x7C, 0xEB, 0xD4, 
	0x04, 0x0F, 0xB9, 0xFF, 0xB2, 0x14, 0x2B, 0x92, 
	0x80, 0xEE, 0x2F, 0x6B, 0x7B, 0x26, 0x0D, 0x55, 
	0xC7, 0x23, 0x00, 0xFE, 0xD1, 0xAC, 0x31, 0x45, 
};

static const octet _a2_n = 1;

static const octet _a2_xP[] = {
	0x9A, 0x8A, 0x24, 0x20, 0xB1, 0xF1, 0x30, 0xB5, 
	0xB4, 0x33, 0xAC, 0x7F, 0x97, 0x49, 0xC8, 0x8B, 
	0xE2, 0x04, 0xE8, 0xEE, 0xA7, 0x0A, 0xB6, 0xC6, 
	0x8D, 0x83, 0xCD, 0x62, 0x12, 0x61, 0x60, 0xFD, 
	0x62, 0xD7, 0x8C, 0xA6, 0x93, 0x10, 0xF9, 0x25, 
	0xC8, 0x7C, 0x05, 0xD7, 0xB3, 0xB3, 0x13, 0x52, 
	0x6C, 0x7A, 0xFD, 0xBB, 0x6E, 0xBF, 0x96, 0xF3, 
	0x30, 0xEE, 0x72, 0x45, 0xC6, 0x9C, 0xD1, 0x24, 
};

static const octet _a2_yP[] = {
	0x1E, 0x37, 0xDD, 0x1A, 0xCB, 0x92, 0xBB, 0x6D, 
	0x0B, 0x64, 0x24, 0x1B, 0xB9, 0x18, 0x1A, 0xDC, 
	0x43, 0x4E, 0xEE, 0xE1, 0x51, 0x33, 0xEB, 0xF7, 
	0x6B, 0x49, 0xF1, 0x77, 0x6D, 0x15, 0xAB, 0x83, 
	0x2C, 0x9B, 0xF3, 0x59, 0xC2, 0x47, 0x24, 0xF3, 
	0xC3, 0xF2, 0xE5, 0x91, 0x1E, 0x06, 0xBF, 0xCF, 
	0xDD, 0xAC, 0x57, 0xC8, 0x13, 0x06, 0x02, 0x0D, 
	0x6E, 0xCE, 0xD2, 0x3B, 0xA4, 0x12, 0xB3, 0x2B, 
};

/*
*******************************************************************************
Стандартные параметры: набор iD-tC26-gost-3410-12-512-pArAmSEtA 
из Методических рекомендаций
*******************************************************************************
*/

static const char _paramsetA512_name[] = "1.2.643.7.1.2.1.2.1";

static const u32 _paramsetA512_l = 512;

static const octet _paramsetA512_p[] = {
	0xC7, 0xFD, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
};

static const octet _paramsetA512_a[] = {
	0xC4, 0xFD, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
};

static const octet _paramsetA512_b[] = {
	0x60, 0xC7, 0x71, 0x5A, 0x78, 0x90, 0x31, 0x50,
	0x61, 0x47, 0xEE, 0xEB, 0xD4, 0xF9, 0x2E, 0x86,
	0xDD, 0x90, 0xDA, 0x10, 0x40, 0x57, 0xB4, 0x4C,
	0x61, 0x27, 0x0D, 0xF3, 0x90, 0xB0, 0x3C, 0xEE,
	0x65, 0x62, 0x0B, 0xFD, 0x1C, 0x08, 0xBD, 0x79,
	0xE8, 0xB0, 0x1C, 0x76, 0x74, 0x25, 0xB8, 0x34,
	0xDA, 0xF1, 0x67, 0x66, 0x2B, 0x0B, 0xBD, 0xC1,
	0xDD, 0x86, 0xFC, 0xED, 0x5D, 0x50, 0xC2, 0xE8,
};

static const octet _paramsetA512_q[] = {
	0x75, 0xB2, 0x10, 0x1F, 0x41, 0xB1, 0xCD, 0xCA,
	0x5D, 0xB8, 0xD2, 0xFA, 0xAB, 0x38, 0x4B, 0x9B,
	0x60, 0x60, 0x05, 0x4E, 0x8D, 0x2B, 0xF2, 0x6F,
	0x11, 0x89, 0x8D, 0xF4, 0x32, 0x95, 0xE6, 0x27,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
};

static const octet _paramsetA512_n = 1;

static const octet _paramsetA512_xP[] = {
	0x03,
};

static const octet _paramsetA512_yP[] = {
	0xA4, 0xF2, 0x15, 0x52, 0xCB, 0x89, 0xA5, 0x89,
	0xB8, 0xF5, 0x35, 0xC2, 0x5F, 0xFE, 0x28, 0x80,
	0xE9, 0x41, 0x3A, 0x0E, 0xA5, 0xE6, 0x75, 0x3D,
	0xE9, 0x36, 0xD0, 0x4F, 0xBE, 0x26, 0x16, 0xDF,
	0x21, 0xA9, 0xEF, 0xCB, 0xFD, 0x64, 0x80, 0x77,
	0xC1, 0xAB, 0xF1, 0xAC, 0x93, 0x1C, 0x5E, 0xCE,
	0xE6, 0x50, 0x54, 0xE2, 0x16, 0x88, 0x1B, 0xA6,
	0xE3, 0x6A, 0x83, 0x7A, 0xE8, 0xCF, 0x03, 0x75,
};

/*
*******************************************************************************
Стандартные параметры: набор id-tC26-gost-3410-12-512-paramsetB 
из Методических рекомендаций
*******************************************************************************
*/

static const char _paramsetB512_name[] = "1.2.643.7.1.2.1.2.2";

static const u32 _paramsetB512_l = 512;

static const octet _paramsetB512_p[] = {
	0x6F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
};

static const octet _paramsetB512_a[] = {
	0x6C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
};

static const octet _paramsetB512_b[] = {
	0x16, 0x01, 0x14, 0xC5, 0xC7, 0xCB, 0x8C, 0xFB,
	0x6E, 0x10, 0xA3, 0x1F, 0xEE, 0x8B, 0xF7, 0x50,
	0x9C, 0xB6, 0x1A, 0xAD, 0x6F, 0x27, 0x8B, 0x7F,
	0x21, 0x6D, 0x41, 0xB1, 0x2D, 0x5D, 0x96, 0x3E,
	0x9F, 0x28, 0x4B, 0x6C, 0x80, 0xDC, 0x85, 0xBF,
	0xBC, 0x38, 0xF1, 0x4A, 0x61, 0x7D, 0x7C, 0xB9,
	0x17, 0x25, 0x5E, 0x6F, 0xCF, 0x06, 0x3E, 0x7E,
	0x45, 0x41, 0xC8, 0x9D, 0x45, 0x1B, 0x7D, 0x68,
};

static const octet _paramsetB512_q[] = {
	0xBD, 0x25, 0x4F, 0x37, 0x54, 0x6C, 0x34, 0xC6,
	0x0E, 0xEA, 0x1B, 0x10, 0x12, 0x67, 0x99, 0x8B,
	0xFA, 0x0C, 0xD4, 0xD9, 0x7B, 0xB7, 0xFD, 0xAC,
	0x45, 0xA5, 0x65, 0x25, 0x14, 0xEC, 0xA1, 0x49,
	0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
};

static const octet _paramsetB512_n = 1;

static const octet _paramsetB512_xP[] = {
	0x02, 
};

static const octet _paramsetB512_yP[] = {
	0xBD, 0x41, 0xFE, 0x80, 0x07, 0x34, 0x21, 0x7E,
	0xEC, 0xEE, 0x4C, 0xF9, 0x55, 0x10, 0x04, 0x28,
	0x88, 0x39, 0xC0, 0xF8, 0xAA, 0xBC, 0x2C, 0x15,
	0x39, 0x4A, 0xDF, 0x1E, 0xFD, 0x28, 0xB2, 0xDC,
	0x35, 0x73, 0xEC, 0xC8, 0xE6, 0xD9, 0x6D, 0xBE,
	0x13, 0xC2, 0x78, 0x75, 0x69, 0x3B, 0x12, 0x3C,
	0x0F, 0x94, 0xA8, 0x47, 0x36, 0x1E, 0x07, 0x2C,
	0x4C, 0x09, 0x9B, 0x38, 0xDA, 0x7E, 0x8F, 0x1A,
};

/*
*******************************************************************************
Загрузить стандартные параметры
*******************************************************************************
*/

#define _LOAD_NAMED_PARAMS(params, name)\
	(params)->l = _##name##_l;\
	memCopy((params)->p, _##name##_p, sizeof(_##name##_p));\
	memCopy((params)->a, _##name##_a, sizeof(_##name##_a));\
	memCopy((params)->b, _##name##_b, sizeof(_##name##_b));\
	memCopy((params)->q, _##name##_q, sizeof(_##name##_q));\
	(params)->n = _##name##_n;\
	memCopy((params)->xP, _##name##_xP, sizeof(_##name##_xP));\
	memCopy((params)->yP, _##name##_yP, sizeof(_##name##_yP))\

err_t g12sParamsStd(g12s_params* params, const char* name)
{
	if (!memIsValid(params, sizeof(g12s_params)))
		return ERR_BAD_INPUT;
	memSetZero(params, sizeof(g12s_params));
	if (strEq(name, _a1_name))
	{
		_LOAD_NAMED_PARAMS(params, a1);
		return ERR_OK;
	}
	if (strEq(name, _cryptoproA_name))
	{
		_LOAD_NAMED_PARAMS(params, cryptoproA);
		return ERR_OK;
	}
	if (strEq(name, _cryptoproB_name))
	{
		_LOAD_NAMED_PARAMS(params, cryptoproB);
		return ERR_OK;
	}
	if (strEq(name, _cryptoproC_name))
	{
		_LOAD_NAMED_PARAMS(params, cryptoproC);
		return ERR_OK;
	}
	if (strEq(name, _cryptocom_name))
	{
		_LOAD_NAMED_PARAMS(params, cryptocom);
		return ERR_OK;
	}
	if (strEq(name, _a2_name))
	{
		_LOAD_NAMED_PARAMS(params, a2);
		return ERR_OK;
	}
	if (strEq(name, _paramsetA512_name))
	{
		_LOAD_NAMED_PARAMS(params, paramsetA512);
		return ERR_OK;
	}
	if (strEq(name, _paramsetB512_name))
	{
		_LOAD_NAMED_PARAMS(params, paramsetB512);
		return ERR_OK;
	}
	return ERR_FILE_NOT_FOUND;
}

/*
*******************************************************************************
Создание описания эллиптической кривой

По долговременным параметрам params формируется описание pec эллиптической
кривой. Указатель *pec является одновременно началом фрагмента памяти, 
в котором размещается состояние и стек. Длина фрагмента определяется с учетом 
потребностей deep и размерностей dims.
\pre Указатель pec корректен.
\return ERR_OK, если описание успешно создано, и код ошибки в противном 
случае.
\remark Запрошенная память начинается по адресу objEnd(*pec, void).
\remark Проводится минимальная проверка параметров, обеспечивающая 
работоспособность высокоуровневых функций.
\remark Диапазоны для q:
-	2^254 \leq q \leq 2^256 (l == 256);
-	2^508 \leq q \leq 2^512 (l == 512).
.
\post Диапазоны для p:
	|nq - (p + 1)| \leq 2\sqrt{p} =>
		(\sqrt{nq} - 1)^2 \leq p \leq (\sqrt{nq} + 1)^2 =>
			p \geq (\sqrt{q} - 1)^2 =>
				p > 2^253 (l == 256)
				p > 2^507 (l == 512)
В частности, длина p (в октетах, в словах) не меньше длины q. 
*******************************************************************************
*/

static err_t g12sEcCreate(
	ec_o** pec,						/* [out] описание эллиптической кривой */
	const g12s_params* params,		/* [in] долговременные параметры */
	g12s_deep_i deep				/* [in] потребности в стековой памяти */
)
{
	// размерности
	size_t n, no, nb;
	size_t f_keep;
	size_t f_deep;
	size_t ec_d;
	size_t ec_keep;
	size_t ec_deep;
	// состояние
	void* state;	
	qr_o* f;			/* базовое поле */
	ec_o* ec;			/* кривая */
	void* stack;
	// pre
	ASSERT(memIsValid(pec, sizeof(*pec)));
	// минимальная проверка входных данных
	if (!memIsValid(params, sizeof(g12s_params)) ||
		params->l != 256 && params->l != 512)
		return ERR_BAD_PARAMS;
	// определить размерности
	no = memNonZeroSize(params->p, sizeof(params->p) * params->l / 512);
	n = W_OF_O(no);
	f_keep = gfpCreate_keep(no);
	f_deep = gfpCreate_deep(no);
	ec_d = 3;
	ec_keep = ecpCreateJ_keep(no);
	ec_deep = ecpCreateJ_deep(no, f_deep);
	// создать состояние
	state = blobCreate(
		f_keep + ec_keep +
		utilMax(3,
			ec_deep,
			ecCreateGroup_deep(f_deep),
			deep(n, f_deep, ec_d, ec_deep)));
	if (state == 0)
		return ERR_OUTOFMEMORY;
	// создать поле
	f = (qr_o*)((octet*)state + ec_keep);
	stack = (octet*)f + f_keep;
	if (!gfpCreate(f, params->p, no, stack))
	{
		blobClose(state);
		return ERR_BAD_PARAMS;
	}
	// проверить длину p
	nb = wwBitSize(f->mod, n);
	if (params->l == 256 && nb <= 253 ||
		params->l == 512 && nb <= 507)
	{
		blobClose(state);
		return ERR_BAD_PARAMS;
	}
	// создать кривую и группу
	ec = (ec_o*)state;
	if (!ecpCreateJ(ec, f, params->a, params->b, stack) ||
		!ecCreateGroup(ec, params->xP, params->yP, params->q, 
			params->l / 8, params->n, stack))
	{
		blobClose(state);
		return ERR_BAD_PARAMS;
	}
	// проверить q
	n = W_OF_B(params->l);
	nb = wwBitSize(ec->order, n);
	if (params->l == 256 && nb <= 254 ||
		params->l == 512 && nb <= 508 ||
		zzIsEven(ec->order, n))
	{
		blobClose(state);
		return ERR_BAD_PARAMS;
	}
	// присоединить f к ec
	objAppend(ec, f, 0);
	// все нормально
	*pec = ec;
	return ERR_OK;
}

/*
*******************************************************************************
Закрытие описания эллиптической кривой
*******************************************************************************
*/

void g12sEcClose(ec_o* ec)
{
	blobClose(ec);
}

/*
*******************************************************************************
Проверка параметров

-#	l \in {256, 512} (g12sEcCreate)
-#	2^254 < q < 2^256 или 2^508 < q < 2^512 (g12sEcCreate)
-#	p -- простое (ecpIsValid)
-#	q -- простое (ecpIsSafeGroup)
-#	q != p (ecpIsSafeGroup)
-#	p^m \not\equiv 1 (mod q), m = 1, 2,..., 31 или 131 (ecpIsSafeGroup)
-#	a, b < p (ecpCreateJ in g12sEcCreate)
-#	J(E) \notin {0, 1728} <=> a, b != 0 (g12sParamsVal)
-#	4a^3 + 27b^2 \not\equiv 0 (\mod p) (ecpIsValid)
-#	P \in E (ecpSeemsValidGroup)
-#	|n * q - (p + 1)| \leq 2\sqrt{p} (ecpSeemsValidGroup)
-#	qP = O (ecpHasOrder)

*******************************************************************************
*/

static size_t g12sParamsVal_deep(size_t n, size_t f_deep, size_t ec_d, 
	size_t ec_deep)
{
	return utilMax(4,
			ecpIsValid_deep(n, f_deep),
			ecpSeemsValidGroup_deep(n, f_deep),
			ecpIsSafeGroup_deep(n),
			ecHasOrderA_deep(n, ec_d, ec_deep, n));
}

err_t g12sParamsVal(const g12s_params* params)
{
	err_t code;
	// состояние
	ec_o* ec = 0;
	void* stack;
	// старт
	code = g12sEcCreate(&ec, params, g12sParamsVal_deep);
	ERR_CALL_CHECK(code);
	stack = objEnd(ec, void);
	// проверить кривую, проверить J(E)
	if (!ecpIsValid(ec, stack) ||
		!ecpSeemsValidGroup(ec, stack) ||
		!ecpIsSafeGroup(ec, params->l == 256 ? 31 : 131, stack) ||
		!ecHasOrderA(ec->base, ec, ec->order, ec->f->n, stack) ||
		qrIsZero(ec->A, ec->f) || 
		qrIsZero(ec->B, ec->f))
		code = ERR_BAD_PARAMS;
	// завершение
	g12sEcClose(ec);
	return code;
}

/*
*******************************************************************************
Управление ключами
*******************************************************************************
*/

static size_t g12sKeypairGen_deep(size_t n, size_t f_deep, size_t ec_d, 
	size_t ec_deep)
{
	const size_t m = n;
	return O_OF_W(m + 2 * n) + 
		ecMulA_deep(n, ec_d, ec_deep, n);
}

err_t g12sKeypairGen(octet privkey[], octet pubkey[],
	const g12s_params* params, gen_i rng, void* rng_stack)
{
	err_t code;
	size_t m, mo;
	// состояние
	ec_o* ec = 0;
	word* d;				/* [m] личный ключ */
	word* Q;				/* [2n] открытый ключ */	
	void* stack;
	// проверить rng
	if (rng == 0)
		return ERR_BAD_RNG;
	// старт
	code = g12sEcCreate(&ec, params, g12sKeypairGen_deep);
	ERR_CALL_CHECK(code);
	// размерности order
	m = W_OF_B(params->l);
	mo = O_OF_B(params->l);
	// проверить входные указатели
	if (!memIsValid(privkey, mo) || 
		!memIsValid(pubkey, 2 * ec->f->no))
	{
		g12sEcClose(ec);
		return ERR_BAD_INPUT;
	}
	// раскладка состояния
	d = objEnd(ec, word);
	Q = d + m;
	stack = Q + 2 * ec->f->n;
	// d <-R {1,2,..., q - 1}
	if (!zzRandNZMod(d, ec->order, m, rng, rng_stack))
	{
		g12sEcClose(ec);
		return ERR_BAD_RNG;
	}
	// Q <- d P
	if (!ecMulA(Q, ec->base, ec, d, m, stack))
	{
		g12sEcClose(ec);
		return ERR_BAD_PARAMS;
	}
	// выгрузить ключи
	wwTo(privkey, mo, d);
	qrTo(pubkey, ecX(Q), ec->f, stack);
	qrTo(pubkey + ec->f->no, ecY(Q, ec->f->n), ec->f, stack);
	// все нормально
	g12sEcClose(ec);
	return ERR_OK;
}

/*
*******************************************************************************
Выработка ЭЦП
*******************************************************************************
*/

static size_t g12sSign_deep(size_t n, size_t f_deep, size_t ec_d, 
	size_t ec_deep)
{
	const size_t m = n;
	return 	O_OF_W(3 * m + 2 * n) +
		utilMax(3,
			zzMod_deep(m, m),
			ecMulA_deep(n, ec_d, ec_deep, n),
			zzMulMod_deep(m));
}

err_t g12sSign(octet sig[], const g12s_params* params, const octet hash[],
	const octet privkey[], gen_i rng, void* rng_stack)
{
	err_t code;
	size_t m, mo;
	// состояние
	ec_o* ec = 0;
	word* d;		/* [m] личный ключ */
	word* e;		/* [m] обработанное хэш-значение */
	word* k;		/* [m] одноразовый ключ */
	word* C;		/* [2n] вспомогательная точка */
	word* r;		/* [m] первая (старшая) часть подписи */
	word* s;		/* [m] вторая часть подписи */
	void* stack;
	// проверить rng
	if (rng == 0)
		return ERR_BAD_RNG;
	// старт
	code = g12sEcCreate(&ec, params, g12sSign_deep);
	ERR_CALL_CHECK(code);
	// размерности order
	m = W_OF_B(params->l);
	mo = O_OF_B(params->l);
	// проверить входные указатели
	if (!memIsValid(hash, mo) ||
		!memIsValid(privkey, mo) ||
		!memIsValid(sig, 2 * mo))
	{
		g12sEcClose(ec);
		return ERR_BAD_INPUT;
	}
	// раскладка состояния
	d = objEnd(ec, word);
	e = d + m;
	k = e + m;
	C = k + m;
	r = C + 2 * ec->f->n;
	s = r + m;
	stack = s + m;
	// загрузить d
	wwFrom(d, privkey, mo);
	if (wwIsZero(d, m) || 
		wwCmp(d, ec->order, m) >= 0)
	{
		g12sEcClose(ec);
		return ERR_BAD_PRIVKEY;
	}
	// e <- hash \mod q
	memCopy(e, hash, mo);
	memRev(e, mo);
	wwFrom(e, e, mo);
	zzMod(e, e, m, ec->order, m, stack);
	// e == 0 => e <- 1
	if (wwIsZero(e, m))
		e[0] = 1;
	// k <-R {1,2,..., q - 1}
gen_k:
	if (!zzRandNZMod(k, ec->order, m, rng, rng_stack))
	{
		g12sEcClose(ec);
		return ERR_BAD_RNG;
	}
	// C <- k P
	if (!ecMulA(C, ec->base, ec, k, m, stack))
	{
		// если params корректны, то этого быть не должно
		g12sEcClose(ec);
		return ERR_BAD_INPUT;
	}
	// r <- x_C \mod q
	qrTo((octet*)C, ecX(C), ec->f, stack);
	wwFrom(r, C, ec->f->no);
	zzMod(r, r, ec->f->n, ec->order, m, stack);
	// r == 0 => повторить генерацию k
	if (wwIsZero(r, m))
		goto gen_k;
	// s <- (rd + ke) \mod q
	zzMulMod(k, k, e, ec->order, m, stack);
	zzMulMod(s, r, d, ec->order, m, stack);
	zzAddMod(s, s, k, ec->order, m);
	// выгрузить ЭЦП
	wwTo(sig, mo, s);
	wwTo(sig + mo, mo, r);
	memRev(sig, 2 * mo);
	// все нормально
	g12sEcClose(ec);
	return ERR_OK;
}

/*
*******************************************************************************
Проверка ЭЦП
*******************************************************************************
*/

static size_t g12sVerify_deep(size_t n, size_t f_deep, size_t ec_d, 
	size_t ec_deep)
{
	const size_t m = n;
	return O_OF_W(5 * m + 2 * n) +
		utilMax(4,
			zzMod_deep(m, m),
			zzMulMod_deep(m),
			zzInvMod_deep(m),
			ecAddMulA_deep(n, ec_d, ec_deep, 2, m, m));
}

err_t g12sVerify(const g12s_params* params, const octet hash[], 
	const octet sig[], const octet pubkey[])
{
	err_t code;
	size_t m, mo;
	// состояние
	ec_o* ec = 0;
	word* Q;		/* [2n] открытый ключ / точка R */
	word* r;		/* [m] первая (старшая) часть подписи */
	word* s;		/* [m] вторая часть подписи */
	word* e;		/* [m] обработанное хэш-значение, v */
	void* stack;
	// старт
	code = g12sEcCreate(&ec, params, g12sVerify_deep);
	ERR_CALL_CHECK(code);
	// размерности order
	m = W_OF_B(params->l);
	mo = O_OF_B(params->l);
	// проверить входные указатели
	if (!memIsValid(hash, mo) ||
		!memIsValid(sig, 2 * mo) ||
		!memIsValid(pubkey, 2 * ec->f->no))
	{
		g12sEcClose(ec);
		return ERR_BAD_INPUT;
	}
	// раскладка состояния
	Q = objEnd(ec, word);
	r = Q + 2 * ec->f->n;
	s = r + m;
	e = s + m;
	stack = e + m;
	// загрузить Q
	if (!qrFrom(ecX(Q), pubkey, ec->f, stack) ||
		!qrFrom(ecY(Q, ec->f->n), pubkey + ec->f->no, ec->f, stack))
	{
		g12sEcClose(ec);
		return ERR_BAD_PUBKEY;
	}
	// загрузить r и s
	memCopy(s, sig + mo, mo);
	memRev(s, mo);
	wwFrom(s, s, mo);
	memCopy(r, sig, mo);
	memRev(r, mo);
	wwFrom(r, r, mo);
	if (wwIsZero(s, m) || 
		wwIsZero(r, m) || 
		wwCmp(s, ec->order, m) >= 0 ||
		wwCmp(r, ec->order, m) >= 0)
	{
		g12sEcClose(ec);
		return ERR_BAD_SIG;
	}
	// e <- hash \mod q
	memCopy(e, hash, mo);
	memRev(e, mo);
	wwFrom(e, e, mo);
	zzMod(e, e, m, ec->order, m, stack);
	// e == 0 => e <- 1
	if (wwIsZero(e, m))
		e[0] = 1;
	// e <- e^{-1} \mod q [v]
	zzInvMod(e, e, ec->order, m, stack);
	// s <- s e \mod q [z1]
	zzMulMod(s, s, e, ec->order, m, stack);
	// e <- - e r \mod q [z2]
	zzMulMod(e, e, r, ec->order, m, stack);
	zzNegMod(e, e, ec->order, m);
	// Q <- s P + e Q [z1 P + z2 Q = R]
	if (!ecAddMulA(Q, ec, stack, 2, ec->base, s, m, Q, e, m))
	{
		g12sEcClose(ec);
		return ERR_BAD_PARAMS;
	}
	// s <- x_Q \mod q [x_R \mod q]
	qrTo((octet*)Q, ecX(Q), ec->f, stack);
	wwFrom(Q, Q, ec->f->no);
	zzMod(s, Q, ec->f->n, ec->order, m, stack);
	// s == r?
	code = wwEq(r, s, m) ? ERR_OK : ERR_BAD_SIG;
	// завершение
	g12sEcClose(ec);
	return code;
}
