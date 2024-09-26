#include "memory.h"
#include "exception.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*************************************************************************/
void* emalloc(size_t size) 
{
	void *res = malloc(size);

	if(!res)
		die(L"fatal error: could not malloc() %d bytes [ memory.emalloc() ]\n",(int) size);
	return res;
}
/*************************************************************************/
void* ecallocz(size_t size) 
{
	void *res = calloc(1, size);

	if(!res)
		die(L"fatal error: could not calloc() %d bytes [ memory.ecallocz() ]\n", (int) size);
	return res;
}
/*************************************************************************/
void* erealloc(void *ptr, size_t size) 
{
	void *res = realloc(ptr, size);

	if(!res)
		die(L"fatal error: could not realloc() %d bytes [ memory.erealloc() ]\n",(int) size);
	return res;
}
/*************************************************************************/
void* ereallocz(void* ptr, size_t size) 
{
	void* res = realloc(ptr, size);

	if (!res)
		die(L"fatal error: could not realloc() %d bytes [ memory.ereallocz() ]\n", (int)size);

	memset(res, 0, size);
	return res;
}

void efree(void* ptr)
{
	if (!ptr)
		die(L"fatal error: could not efree() [ memory.efree() ]\n");
	free(ptr);
}