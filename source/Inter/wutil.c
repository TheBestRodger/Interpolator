/*!*****************************************************************************
	@file wutil.c			               
	@date 2024/05/03
	@brief Реализация вспомагательных функций для реализации модуля многомерной
	линейной интерполяции.
*******************************************************************************/

#include "wutil.h"
#include <assert.h>

int wtokenize(wchar* res[], wchar * str, const wchar* delim)
{
	wchar* s;
	unsigned int i = 0;
	s = str;
	int len_s = wcslen(s);
	while (i < len_s && *s) {
		while (wcsrchr(delim, *s) && *s != L'\0')
			*(s++) = L'\0';
		if (*s)
			res[i++] = s;
		while (*s && wcsrchr(delim, *s) == NULL)
			s++;
	}
	return i;
}
int wntokenize(wchar* res[], size_t reslen, wchar* str, wchar* delim)
{
	wchar* s;
	unsigned int i;

	i = 0;
	s = str;

	while (i < reslen && *s) {
		while (wcschr(delim, *s) && *s != '\0')
			*(s++) = '\0';
		if (*s)
			res[i++] = s;
		while (*s && wcschr(delim, *s) == NULL)
			s++;
	}
	return i;
}
void wreader(FILE* fstream, void(*getwline)(int, const wchar *)) 
{
    wchar buf[1024];
	int line = 0;
    while (fgetws(buf, 1024, fstream)!= NULL)
    {
		int l = wcslen(buf);
		if (buf[l - 1] == '\r' || buf[l - 1] == '\n')
			buf[l - 1] = '\0';
		if (buf[l - 2] == '\r' || buf[l - 2] == '\n')
			buf[l - 2] = '\0';

		line++;
		getwline(line, buf);
    }
}
size_t wextlen(wchar const* fname)
{
	int i = 0;
	if (fname == 0)
		return 0;

	size_t const bSize = wcslen(fname);	// Длина имени файла

	/* Поиск точки */
	for (i = bSize - 1; i >= 0; i--)
		if (fname[i] == '.')
			break;
		else if (fname[i] == '\\' || fname[i] == '/')
			return 0;


	if (i < 0)
		return 0;

	return bSize - i;
}
void wsetext(wchar* dest, wchar const* src, wchar const* ext)
{
	assert(dest != src);

	/* Найдено некорректное расширение файла. Нужно заменить на корректное */
	size_t wslen = wcslen(src);
	if (wextlen(src) > 0 && wcscmp(ext, src + wslen - wextlen(src)) != 0) {
		wcsncpy_s(dest, wcslen(dest), src, wslen - wextlen(src));
		*(dest + wslen - wextlen(src)) = 0;
	}

	/* Расширение отсутствует. Добавляем */
	else if (wextlen(src) < 1)
		wcscpy_s(dest, 1024, src);
	else
	{
		wcscpy_s(dest, 1024, src);
		return;
	}
	
	wcscat_s(dest, 1024, ext);
}
bool wfindext(const wchar* dest, const wchar * srs)
{
	wchar tokbuf[1024];
	wchar* tok[256];

	wcsncpy_s(tokbuf, 1024, dest, 1024);
	
	int k = wntokenize(tok, 255, tokbuf, L".");
	for (int i = 0; i < k; ++i) {
		const wchar* ss = tok[i];
		if (!wcscmp(ss, srs))
			return 1;
			
	}
	return 0;
}