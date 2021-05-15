/*
*******************************************************************************
\file mt_test.c
\brief Tests for multithreading
\project bee2/test
\author (C) Sergey Agievich [agievich@{bsu.by|gmail.com}]
\created 2021.05.15
\version 2021.05.15
\license This program is released under the GNU General Public License 
version 3. See Copyright Notices in bee2/info.h.
*******************************************************************************
*/

#include <stdio.h>
#include <bee2/core/mt.h>

/*
*******************************************************************************
Тестирование
*******************************************************************************
*/

static size_t _init = 0;
void init()
{
	_init++;
}

bool_t mtTest()
{
	mt_mtx_t mtx[1];
	size_t ctr[1] = { SIZE_0 };
	// мьютексы
	if (!mtMtxCreate(mtx))
		return FALSE;
	mtMtxLock(mtx);
	mtMtxUnlock(mtx);
	mtMtxClose(mtx);
	// атомарные операции
	mtAtomicIncr(ctr);
	mtAtomicIncr(ctr);
	mtAtomicDecr(ctr);
	if (mtAtomicCmpSwap(ctr, 1, 0) != 1 || *ctr != SIZE_0)
		return FALSE;
	// однократный вызов
	if (!mtCallOnce(ctr, init) || _init != 1)
		return FALSE;
	if (!mtCallOnce(ctr, init) || _init != 1)
		return FALSE;
	// все нормально
	return TRUE;
}
