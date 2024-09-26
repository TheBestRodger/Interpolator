#include "exception.h"
#include <stdarg.h>
#include <stdlib.h>
void die(const wchar* errstr, ...)
{
	va_list ap;
	wchar msg[4096];

	va_start(ap, errstr);
	vswprintf_s(msg, 4096,errstr, ap);
	_wperror(msg);
	va_end(ap);
	abort();
}
void print_error(const wchar* errstr, ...)
{
	va_list ap;
	wchar msg[4096];

	va_start(ap, errstr);
	vswprintf_s(msg, 4096, errstr, ap);
	_wperror(msg);
	va_end(ap);
}