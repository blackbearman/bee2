%module bee2
%include "carrays.i"
%include "cdata.i"
%include "cpointer.i"
#ifdef SWIGPYTHON
%include "cstring.i"
#endif
%include "typemaps.i"

%{
#include "bee2/core/safe.h"
#include "bee2/core/mem.h"
#include "bee2/core/hex.h"

#include "bee2/crypto/belt.h"
#include "bee2/crypto/bign.h"

void* ptradd(void* ptr, int offset) {
	return ptr + offset;
}
%}

typedef unsigned int u32;
typedef u32 err_t;
#define ERR_OK	((err_t)0)

%array_functions(u32, u32arr)
%apply SWIGTYPE* { size_t* count };
%apply SWIGTYPE* { size_t *ary };
%array_functions(size_t, sizeTarr)

%pointer_cast(octet*, void*, op2vp)
%pointer_cast(void*, octet*, vp2op)
%pointer_cast(unsigned char*, void*, bp2vp)
%pointer_cast(void*, unsigned char*, vp2bp)

void* ptradd(void* ptr, int offset);

%include "../include/bee2/core/safe.h"
%include "../include/bee2/core/mem.h"

%include "../include/bee2/crypto/belt.h"

%include "../include/bee2/crypto/bign.h"

/*!	\brief Декодирование буфера памяти

	Шестнадцатеричная строка src преобразуется в строку октетов 
	[strLen(src) / 2]dest. По первой паре символов src определяется первый 
	октет dest, по второй паре -- второй октет и т.д.
	\pre hexIsValid(hex) == TRUE.
*/
void hexTo(
	void* dest,			/*!< [out] память-приемник */
	const char* src		/*!< [in] строка-источник */
);

/*!	\brief Кодирование буфера памяти

	Буфер [count]src кодируется шестнадцатеричной строкой
	{2 * count + 1}dest. Первому октету src соответствует первая пара 
	символов dest, второму октету -- вторая пара и т.д.
	\pre Буферы dest и src не пересекаются.
*/
#ifdef SWIGPYTHON
%cstring_mutable(char* dest)
void hexFrom(
	char* dest,			/*!< [out] строка-приемник */
	const void* src,	/*!< [in] память-источник */
	size_t count		/*!< [in] число октетов */
);
#else
%typemap(freearg) char * {  }
%apply SWIGTYPE* { char *dest };
void hexFrom(
	char* dest,			/*!< [out] строка-приемник */
	const void* src,	/*!< [in] память-источник */
	size_t count		/*!< [in] число октетов */
);
#endif


