#pragma once
#ifndef __LIBSCIENCE_CORE_EXTYPE_HPP_INCLUDED__
#define __LIBSCIENCE_CORE_EXTYPE_HPP_INCLUDED__

#include <wchar.h>

typedef wchar_t 			wchar;
/** \brief Перечисление форматов файлов */
typedef enum {
	  TEXT = 0x01		/**< тектовый формат */
	, BINARY = 0x02		/**< бинарный формат */
} FormatFile;
#endif  // __LIBSCIENCE_CORE_EXTYPE_HPP_INCLUDED__