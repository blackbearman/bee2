/*
*******************************************************************************
\file botp.h
\brief STB 34.101.47/botp: OTP algorithms
\project bee2 [cryptographic library]
\created 2015.11.02
\version 2023.02.02
\license This program is released under the GNU General Public License 
version 3. See Copyright Notices in bee2/info.h.
*******************************************************************************
*/

/*!
*******************************************************************************
\file botp.h
\brief Алгоритмы управления одноразовыми паролями
*******************************************************************************
*/

#ifndef __BEE2_BOTP_H
#define __BEE2_BOTP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bee2/defs.h"
#include "bee2/core/tm.h"

/*!
*******************************************************************************
\file botp.h

\section botp-common Общие положения

Реализованы алгоритмы управления одноразовыми паролями. Алгоритмы соответствуют
стандартам RFC 4226, RFC 6238, RFC 6287 и базируются на механизме имитозащиты 
HMAC[belt-hash], определенному в СТБ 34.101.47 и реализованному в модуле belt.

Пароль представляет собой строку из digit символов алфавита {'0' - '9'}, 
4 <= digit <= 9. 

Алгоритмы объединяются в группы, которые определяют следующие криптографические
механизмы (режимы):
-	HOTP ---  пароли на основе событий (RFC 4226);
-	TOTP ---  пароли на основе времени (RFC 6238);
-	OCRA ---  пароли на основе запросов (RFC 6287).

Каждый механизм реализуется связкой из нескольких функций. Эти функции 
используют общее состояние, указатель на которое передается в функции 
как дополнительный параметр. Имеются функция определения объема памяти 
для состояния (_keep). Память для состояния готовит вызывающая программа 
и она же отвечает за очистку памяти. Состояние можно копировать как фрагмент 
памяти.

В связке обязательно имеется функция инициализации режима (Start)
и одна или несколько функций обработки фрагментов данных и получения
результатов обработки (StepX).

Логика суффиксов функций StepX:
-	S -- set (установить данные сеанса);
-	R -- rand (сгенерировать пароль);
-	V -- verify (проверить пароль, восстановить синхронизацию);
-	G -- get (получить обновленные данные сеанса).

Функции связки спроектированы как максимально простые и эффективные.
В частности, в этих функциях не проверяются входные данные.

Каждая связка покрывается высокоуровневой функцией, которая
обрабатывает все данные целиком. В высокоуровневых функциях есть
проверка входных данных.

В функции botpDT() реализован вспомогательный алгоритм "динамической обрезки"
(dynamic truncation). Функцию можно использовать с произвольными имитовставками
(а не только с имитовставками hmac-hbelt).

В режимах HOTP и OCRA используется счетчик -- строка из 8 октетов. Эта строка 
интерпретируются как число по правилам big-endian ("от старших к младшим"), 
принятым в RFC 4226. Пароли генерируются на последовательных \mod 2^64 
значениях счетчика. Инкремент счетчика реализован в функции botpCtrNext().

\expect Общее состояние связки функций не изменяется вне этих функций.

\expect{ERR_BAD_INPUT} Все входные указатели высокоуровневых функций 
действительны.

\pre Все входные указатели низкоуровневых функций действительны.
Размер буфера строки на 1 октет больше длины строки (с учетом
завершающего нулевого октета).

\pre Если не оговорено противное, то входные буферы функций связки 
не пересекаются.
*******************************************************************************
*/

/*!	\brief Построение пароля по имитовставке

	По имитовставке [mac_len]mac строится одноразовый пароль otp из digit 
	десятичных цифр.
	\pre 4 <= digit && digit <= 10.
	\pre mac_len >= 20.
*/
void botpDT(
	char* otp,			/*< одноразовый пароль */
	size_t digit,		/*< число цифр в пароле */
	const octet mac[],	/*< имитовставка */
	size_t mac_len		/*< длина имитовставки в октетах */
);

/*!	\brief Следующий счетчик

	Счетчик ctr режимов HOTP и OCRA инкрементируется \mod 2^64.
*/
void botpCtrNext(
   octet ctr[8]		/*< счетчик */
);

/*!
*******************************************************************************
\file botp.h

\section botp-hotp Режим HOTP

При выработке, а также при успешной проверке пароля в функциях botpHOTPStepR(),
botpHOTPStepV() счетчик, размещенный в состоянии, инкрементируется. 
Обновленный счетчик можно использовать для генерации или проверки нового 
пароля. Выгрузить счетчик из состояния можно с помощью функции botpHOTPStepG().
*******************************************************************************
*/

/*!	\brief Длина состояния функций HOTP

	Возвращается длина состояния (в октетах) функций механизма HOTP.
	\return Длина состояния.
*/
size_t botpHOTP_keep();

/*!	\brief Инициализация режима HOTP

	По числу digit ключу [key_len]key в state формируются структуры данных, 
	необходимые для управления паролями длины digit в режиме HOTP.
	\pre 6 <= digit && digit <= 8.
	\pre По адресу state зарезервировано botpHOTP_keep() октетов.
	\remark Рекомендуется использовать ключ из 32 октетов.
*/
void botpHOTPStart(
	void* state,			/*!< [out] состояние */
	size_t digit,			/*!< [in] число цифр в пароле */
	const octet key[],		/*!< [in] ключ */
	size_t key_len			/*!< [in] длина ключа в октетах */
);

/*!	\brief Установка счетчика режима HOTP

	В state устанавливается счетчик ctr.
	\expect botpHOTPStart() < botpHOTPStepS().
*/
void botpHOTPStepS(
	void* state,			/*!< [out] состояние */
	const octet ctr[8]		/*!< [in] счетчик */
);

/*!	\brief Генерация очередного пароля в режиме HOTP

	По числу digit, ключу и счетчику, размещенным в state, генерируется 
	одноразовый пароль из digit десятичных символов. После генерации пароля 
	счетчик инкрементируется.
	\expect botpHOTPStepS() < botpHOTPStepR()*.
*/
void botpHOTPStepR(
	char* otp,			/*!< [out] одноразовый пароль */
	void* state			/*!< [in,out] состояние */
);

/*!	\brief Проверка очередного пароля в режиме HOTP

	По числу digit, ключу и счетчику, размещенным в state, строится 
	одноразовый пароль из digit десятичных цифр. Построенный пароль 
	сравнивается с otp. Если пароли совпадают, то счетчик инкрементируется.
	\expect botpHOTPStepS() < botpHOTPStepV()*.
	\return Признак совпадения паролей.
	\remark Функция регулярна: полностью проверяется даже заведомо 
	неверный пароль.
*/
bool_t botpHOTPStepV(
	const char* otp,		/*!< [in] контрольный пароль */
	void* state				/*!< [in,out] состояние */
);

/*!	\brief Возврат счетчика

	В ctr возвращается текущий счетчик, размещенный в state.
	\expect botpHOTPStepS() < botpHOTPStepG().
*/
void botpHOTPStepG(
	octet ctr[8],		/*!< [out] счетчик */
	const void* state	/*!< [in] состояние */
);

/*!	\brief Генерация пароля в режиме HOTP

	По ключу [key_len]key и счетчику ctr генерируется одноразовый пароль 
	otp из digit десятичных символов.
	\expect{ERR_BAD_PARAMS} 6 <= digit && digit <= 8.
	\return ERR_OK, если пароль успешно сгенерирован, и код ошибки
	в противном случае.
*/
err_t botpHOTPRand(
	char* otp,			/*!< [in] одноразовый пароль */
	size_t digit,		/*!< [in] длина пароля */
	const octet key[],	/*!< [in] ключ */
	size_t key_len,		/*!< [in] длина ключа в октетах */
	const octet ctr[8]	/*!< [in] счетчик */
);

/*!	\brief Проверка пароля в режиме HOTP

	По ключу [key_len]key и счетчику ctr строится одноразовый пароль 
	из digit = strLen(otp) символов. Построенный пароль сравнивается с otp. 
	\expect{ERR_BAD_PWD} 6 <= digit && digit <= 8.
	\expect{ERR_BAD_PWD} Пароль otp совпадает с построенным.
	\return ERR_OK в случае успеха или код ошибки.
*/
err_t botpHOTPVerify(
	const char* otp,		/*!< [in] контрольный пароль */
	const octet key[],		/*!< [in] ключ */
	size_t key_len,			/*!< [in] длина ключа в октетах */
	const octet ctr[8]		/*!< [in] счетчик */
);

/*!
*******************************************************************************
\file botp.h

\section botp-totp Режим TOTP

Текущее время --- это, так называемое, UNIX-время --- число секунд, прошедших 
с момента 1970-01-01T00:00:00Z. Текущее время t округляется. Округление 
выполняется с параметрами t0 -- базовая отметка времени и ts -- шаг времени.
Округление состоит в замене t на (t - t0) / ts. Если t < t0, то округление 
завершится с ошибкой. Округление можно выполнить с помощью функции 
tmTimeRound(). Эта функция возвращает TIME_ERR при ошибках получения отметки
текущего времени или ее округления.

При округлении рекомендуется использовать t0 = 0 и ts = 30 или 60.

Отметка времени представляется типом tm_time_t. Отметка преобразуется в счетчик
режима HOTP, т.е. в 64-разрядное беззнаковое число. 
*******************************************************************************
*/

/*!	\brief Длина состояния функций TOTP

	Возвращается длина состояния (в октетах) функций механизма TOTP.
	\return Длина состояния.
*/
size_t botpTOTP_keep();

/*!	\brief Инициализация режима TOTP

	По числу digit и ключу [key_len]key в state формируются структуры данных, 
	необходимые для управления паролями длины digit в режиме TOTP.
	\pre 6 <= digit && digit <= 8.
	\pre По адресу state зарезервировано botpTOTP_keep() октетов.
	\remark Рекомендуется использовать ключ из 32 октетов.
*/
void botpTOTPStart(
	void* state,			/*!< [out] состояние */
	size_t digit,			/*!< [in] число цифр в пароле */
	const octet key[],		/*!< [in] ключ */
	size_t key_len			/*!< [in] длина ключа в октетах */
);

/*!	\brief Генерация очередного пароля в режиме TOTP

	По округленной отметке t текущего времени, по числу digit и ключу, 
	размещенным в state, генерируется одноразовый пароль otp из digit 
	десятичных символов.
	\pre t != TIME_ERR.
	\expect botpTOTPStart() < botpTOTPStepR()*.
*/
void botpTOTPStepR(
	char* otp,			/*!< [out] одноразовый пароль */
	tm_time_t t,		/*!< [in] округленная отметка времени */
	void* state			/*!< [in,out] состояние */
);

/*!	\brief Проверка очередного пароля в режиме TOTP

	По округленной отметке t текущего времени, по числу digit и ключу, 
	размещенным в state, строится одноразовый пароль из digit символов.	
	Полученный пароль сравнивается с otp.
	\pre t != TIME_ERR.
	\expect botpTOTPStart() < botpTOTPStepV()*.
	\return TRUE, если пароль подошел, и FALSE в противном случае.
*/
bool_t botpTOTPStepV(
	const char* otp,		/*!< [in] контрольный пароль */
	tm_time_t t,			/*!< [in] округленная отметка времени */
	void* state				/*!< [in,out] состояние */
);

/*!	\brief Генерация пароля в режиме TOTP

	По числу digit, ключу [key_len]key и округленной отметке t текущего 
	времени генерируется одноразовый пароль otp из digit цифр.
	\expect{ERR_BAD_PARAMS} 6 <= digit && digit <= 8.
	\expect{ERR_BAD_TIME} t != TIME_ERR.
	\return ERR_OK, если пароль успешно сгенерирован, и код ошибки
	в противном случае.
*/
err_t botpTOTPRand(
	char* otp,			/*!< [out] одноразовый пароль */
	size_t digit,		/*!< [in] длина пароля */
	const octet key[],	/*!< [in] ключ */
	size_t key_len,		/*!< [in] длина ключа в октетах */
	tm_time_t t			/*!< [in] округленная отметка времени */
);

/*!	\brief Проверка пароля в режиме TOTP

	По ключу [key_len]key и округленной отметке t текущего времени
	строится одноразовый пароль из strLen(otp) символов. Построенный пароль 
	сравнивается с otp.
	\expect{ERR_BAD_PWD} 6 <= digit && digit <= 8.
	\expect{ERR_BAD_TIME} t != TIME_ERR.
	\expect{ERR_BAD_PWD} Пароль otp подошел.
	\return ERR_OK в случае успеха или код ошибки.
*/
err_t botpTOTPVerify(
	const char* otp,		/*!< [in] контрольный пароль */
	const octet key[],		/*!< [in] ключ */
	size_t key_len,			/*!< [in] длина ключа в октетах */
	tm_time_t t				/*!< [in] округленная отметка времени */
);

/*!
*******************************************************************************
\file botp.h

\section botp-ocra Режим OCRA

В режиме OCRA используются следующие параметры:
-	q -- запрос (клиента, сервера или составной);
-	ctr -- счетчик;
-	p -- хэш-значение статического пароля клиента и сервера;
-	t -- округленная отметка времени;
-	s -- идентификатор сеанса между клиентом и сервером.

Параметры ctr, p, s описывают сеанс между клиентом и сервером.
Параметры ctr, p, t, s являются необязательными. 

Необходимость использования того или иного параметра, а также формат 
параметра, его длина, параметры округления времени определяются строковым 
описателем suite.

Запрос q может быть 3 типов: A (буквенно-цифровой), N (цифровой) и H 
(шестнадцатеричный). Формат запроса в функциях OCRA не проверяется. 
Для предварительной проверки запроса можно использовать функции 
strIsAlphanumeric(), decIsValid(), hexIsValid().

Запрос q может быть одиночным или двойным. Если q -- одиночный, то его 
длина лежит в пределах от 4 до q_max, где q_max -- максимальная длина, 
указанная в suite. Если q -- двойной, то его длина лежит в пределах от 8 до
2 * q_max. За подготовку составного запроса отвечает вызывающая программа.
*******************************************************************************
*/

/*!	\brief Длина состояния функций OCRA

	Возвращается длина состояния (в октетах) функций механизма OCRA.
	\return Длина состояния.
*/
size_t botpOCRA_keep();

/*!	\brief Инициализация режима OCRA

	По описателю suite и ключу [key_len]key в state формируются структуры данных, 
	необходимые для управления паролями в режиме OCRA.
	\pre По адресу state зарезервировано botpOCRA_keep() октетов.
	\return TRUE, если описатель корректен, и FALSE в противном случае.
	\remark Рекомендуется использовать ключ из 32 октетов.
*/
bool_t botpOCRAStart(
	void* state,			/*!< [out] состояние */
	const char* suite,		/*!< [in] описатель */
	const octet key[],		/*!< [in] ключ */
	size_t key_len			/*!< [in] длина ключа в октетах */
);

/*!	\brief Установка данных сеанса режима OCRA

	В state устанавливаются счетчик ctr, хэш-значение p статического пароля,
	идентификатор сеанса s.
	\expect botpOCRAStart() < botpOCRAStepS().
	\remark Каждый из параметров ctr, p, s является необязательным. Указатель 
	на необязательный параметр не контролируется.
	\remark Функцию можно не вызывать, если все параметры необязательны.
*/
void botpOCRAStepS(
	void* state,			/*!< [out] состояние */
	const octet ctr[8],		/*!< [in] счетчик */
	const octet p[],		/*!< [in] хэш-значение статического пароля */
	const octet s[]			/*!< [in] идентификатор сеанса */
);

/*!	\brief Генерация очередного пароля в режиме OCRA

	По запросу [q_len]q и округленной отметке t текущего времени, 
	по описателю suite, ключу и данным сеанса, размещенным в state, 
	генерируется одноразовый пароль otp. Длина otp определяется описателем 
	suite. Если suite задает использование счетчика, то после генерации пароля
	он инкрементируется.
	\pre 4 <= q_len && q_len < = 2 * q_max, где q_max -- максимальная длина 
	одиночного запроса, указанная в suite.
	\pre Если suite задает использование t, то t != TIME_ERR.
	\expect Формат q соответствует типу, указанному в suite.
	\expect botpOCRAStart() < [botpOCRAStepS()] < botpOCRAStepR()*.
*/
void botpOCRAStepR(
	char* otp,			/*!< [out] одноразовый пароль */
	const octet q[],	/*!< [in] запрос */
	size_t q_len,		/*!< [in] длина запроса в октетах */
	tm_time_t t,		/*!< [in] округленная отметка времени */
	void* state			/*!< [in,out] состояние */
);

/*!	\brief Проверка очередного пароля в режиме OCRA

	По запросу [q_len]q и округленной отметке t текущего времени, 
	по описателю suite, ключу и данным сеанса, размещенным в state, 
	строится одноразовый пароль. Длина пароля определяется описателем
	suite. Построенный пароль сравнивается c otp. Если suite задает 
	использование счетчика, то после успешной проверки пароля он 
	инкрементируется.
	\pre 4 <= q_len && q_len < = 2 * q_max, где q_max -- максимальная длина 
	одиночного запроса, указанная в suite.
	\pre Если suite задает использование t, то t != TIME_ERR.
	\expect Формат q соответствует типу, указанному в suite.
	\expect botpOCRAStart() < [botpOCRAStepS()] < botpOCRAStepV()*.
	\return TRUE, если пароль подошел, и FALSE в противном случае.
*/
bool_t botpOCRAStepV(
	const char* otp,	/*!< [in] контрольный пароль */
	const octet q[],	/*!< [in] запрос */
	size_t q_len,		/*!< [in] длина запроса в октетах */
	tm_time_t t,		/*!< [in] округленная отметка времени */
	void* state			/*!< [in,out] состояние */
);

/*!	\brief Возврат счетчика

	В ctr возвращается текущий счетчик, размещенный в state.
	\expect botpOCRAStepS() < botpHOTPStepG().
*/
void botpOCRAStepG(
	octet ctr[8],		/*!< [out] счетчик */
	const void* state	/*!< [in] состояние */
);

/*!	\brief Генерация пароля в режиме OCRA

	По описателю suite, ключу [key_len]key, запросу q, счетчику ctr, 
	хэш-значению p статического пароля, идентификатору сеанса s и округленной 
	отметке t текущего времени генерируется одноразовый пароль otp.
	\expect{ERR_BAD_FORMAT} Формат suite корректен.
	\expect{ERR_BAD_PARAMS} 4 <= q_len && q_len < = 2 * q_max, 
	где q_max -- максимальная длина одиночного запроса, указанная в suite.
	\expect{ERR_BAD_TIME} Если suite задает использование t, то t != TIME_ERR.
	\expect Формат q соответствует типу, указанному в suite.
	\return ERR_OK, если пароль успешно сгенерирован, и код ошибки
	в противном случае.
*/
err_t botpOCRARand(
	char* otp,			/*!< [out] одноразовый пароль */
	const char* suite,	/*!< [in] описатель */
	const octet key[],	/*!< [in] ключ */
	size_t key_len,		/*!< [in] длина ключа в октетах */
	const octet q[],	/*!< [in] запрос */
	size_t q_len,		/*!< [in] длина запроса в октетах */
	const octet ctr[8],	/*!< [in] счетчик */
	const octet p[],	/*!< [in] хэш-значение статического пароля */
	const octet s[],	/*!< [in] идентификатор сеанса */
	tm_time_t t			/*!< [in] округленная отметка времени */
);

/*!	\brief Проверка пароля в режиме TOTP

	По описателю suite, ключу [key_len]key, запросам [q_len]q, счетчику ctr, 
	хэш-значению p статического пароля, идентификатору сеанса s и
	округленной отметке t текущего времени строится одноразовый пароль.
	Построенный пароль сравнивается с otp.
	\expect{ERR_BAD_FORMAT} Формат suite корректен.
	\expect{ERR_BAD_PARAMS} 4 <= q_len && q_len < = 2 * q_max, 
	где q_max -- максимальная длина одиночного запроса, указанная в suite.
	\expect{ERR_BAD_TIME} Если suite задает использование t, то t != TIME_ERR.
	\expect{ERR_BAD_PWD} Пароль otp подошел.
	\expect Формат q соответствует типу, указанному в suite.
	\return ERR_OK в случае успеха или код ошибки.
*/
err_t botpOCRAVerify(
	const char* otp,	/*!< [in] контрольный пароль */
	const char* suite,	/*!< [in] описатель */
	const octet key[],	/*!< [in] ключ */
	size_t key_len,		/*!< [in] длина ключа в октетах */
	const octet q[],	/*!< [in] запрос */
	size_t q_len,		/*!< [in] длина запроса в октетах */
	const octet ctr[8],	/*!< [in] счетчик */
	const octet p[],	/*!< [in] хэш-значение статического пароля */
	const octet s[],	/*!< [in] идентификатор сеанса */
	tm_time_t t			/*!< [in] округленная отметка времени */
);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __BEE2_BOTP_H */
