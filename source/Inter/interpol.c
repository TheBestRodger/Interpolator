#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "exception.h"
#include "memory.h"
#include "wutil.h"
#ifndef SECURE_BIN
#define SECURE_BIN 0x0f0ff0f0		// Маска, по которой определяется принадлежность бинарника
#endif
static unsigned ninterpol;	/* Количество открытых БД для ЛИ */

static Interpol *interpol, *curr;	/* Массив баз данных для ЛИ */
static bool find_quest; // признак нахождения символа '?'
static bool find_attent;// признак нахождения сивола '!'
static bool desc_values;// признак что идут описания значений
static bool ini_arg_mem;// признак первичной инициализации данных для дополнительного формата данных
static bool ini_desc_arg;// признак первичной инициализации описания данных.

static void mkinterval(unsigned const pos);
/*************************************************************************/
static int sign(sreal_t arg)
{
    if (arg > (sreal_t)(0))
        return 1;
    else if (arg < (sreal_t)(0))
        return -1;
    else
        return 0;
}
static bool is_monotonic(sreal_t const* arr, unsigned const arraySize)
{
    if(arraySize < 0)
        return false;
    if (arraySize < 2)
        return true;

    int const derivativeSign = sign(arr[1] - arr[0]);
    for (unsigned i = 1; i < arraySize; ++i)
        if (sign(arr[i] - arr[i - 1]) != derivativeSign)
            return false;
    return true;
}
static int Cod(const wchar c)
{
	if (c == '\t')
		return -3;
	if (c == '\n')
		return -2;
	if (c == ' ')
		return -1;

	int cod = (int) c;

	if (cod)
		return cod;

	die(L"[ %S ]: fatal: Ошибка в кодировании символа [interpol->Cod()]\n", curr->path);
	return 0;
}
static const wchar EnCod(int c)
{
	if (c == -3)
		return '\t';
	if (c == -2)
		return '\n';
	if (c == -1)
		return ' ';
	const wchar EnCod = (wchar) c;
	if(!EnCod)
	    die(L"[ %S ]: fatal: Ошибка в декодировании символа [interpol->EnCod()]\n", curr->path);
	return EnCod;
}
static void check()
{
	unsigned sum = 1;

	for (unsigned i = 0; i < curr->nscales; i++) sum *= curr->len[i];
    
	if (curr->data.amount != sum){
        die(L"[ %S ]: fatal: Размер данных не совпадает.\nПроизведение аргументов = %d  \
                неравно(!=) Числу точек = %d [interpol.check()]\n", curr->path, sum, curr->data.amount);
        mkfree(ninterpol);
        return;
    }
}
static bool ismonotonik(wchar* tok[256], int k)
{
	if(k <= 1)
		return true;
	bool in = true;
	bool de = true;
	sreal_t* numbers = (sreal_t*)emalloc(sizeof(sreal_t) * (k + 1));

	for(int i = 0; i < k; ++i)
		numbers[i] = wcstod(tok[i], NULL);
	for(int i = 1; i < k; ++i){
		if(numbers[i - 1] > numbers[i])
			in = false;
		if(numbers[i - 1] < numbers[i])
			de = false;
	}
	efree(numbers);
	if(in == de)
		return false;
	return in || de;
}
/* Чтение и загрузка данных по строчно */
static void getwline(int line, const wchar * buf)
{
	wchar* tok[256];
    wchar tokbuf[1024];
    wcsncpy_s(tokbuf, 1024, buf, 1024);
	int k = wtokenize(tok, tokbuf, L" \t");

	if (k == 0) return;
    if(buf[0] == '?'){
        find_quest  = true;
        desc_values = false;
        return;
    }
    if(buf[0] == '!')
    {
        find_quest = false;
        find_attent = true;
        return;
    }
    if (iswdigit(tok[0][0]) && !find_quest && !find_attent)
       curr->format = false;
    if (!curr->format)
    {
        /*
        * Второй формат данных
        * Есть описание данных
        * Один аргумент и значения
        *  #dsfvmkl
        *  #vko
        *  #A1 
        *  1 0.9
        *  2 0.6
        *  3 0.43
        *  4 0.12
        */
        if (ini_arg_mem) {
            ini_arg_mem = false;
            curr->scales = (Scale**)ecallocz(sizeof(Scale*));
            curr->len = (int*)ecallocz(sizeof(int));
            curr->nscales = 1;
            curr->data.amount = 0;
        }
        curr->scales[0] = (Scale*)erealloc(curr->scales[0],
            sizeof(Scale) * (curr->data.amount + 1));

        curr->scales[0][curr->data.amount].arg = wcstod(tok[0], NULL);

        curr->data.points = (sreal_t*)erealloc(curr->data.points,
            sizeof(sreal_t) * (curr->data.amount + 1));
        curr->data.points[curr->data.amount] = wcstod(tok[1], NULL);
        curr->data.amount++;
        curr->len[0] = curr->data.amount;
    }
    else
    {
        curr->format = true;
        // Основной формат данных
        // У него может быть или не быть описание данных
        // Может присутствовать или отсутствовать имя аргумента и его описание
        // А также описание табличных данных
        // Аргументов от 1 до n
        if(buf[0] == '#' && desc_values){
            //Описание файла
            int size_v = wcslen(buf);
            curr->filedesc.description_file = (int*)erealloc(curr->filedesc.description_file, 
            sizeof(int) * (curr->filedesc.length_description_file + size_v + 1));

            for (int i = 0; i < size_v; ++i, curr->filedesc.length_description_file++)
                curr->filedesc.description_file[curr->filedesc.length_description_file] = Cod(buf[i]);

            curr->filedesc.description_file[curr->filedesc.length_description_file] = -2;

            curr->filedesc.length_description_file++;

            return;
        }
        else if(find_quest)
        {
            if(ini_arg_mem)
            {
                // Memory allocate
                // Оно нужно чтоб инициализировать память, при отсутствии описания или имени у аргумента
                // пример:
                // # a1
                // 1 2 3
                // 
                // 4 5 6
                // #aa
                // #a3
                // 2 2 2
                ini_arg_mem = false;
                curr->argdesc = (ArgumentDesc*)erealloc(curr->argdesc, sizeof(ArgumentDesc) * (curr->nscales + 1));
                curr->argname = (ArgumentName*)erealloc(curr->argname, sizeof(ArgumentName) * (curr->nscales + 1));
                //curr->argdesc[curr->nscales].description_argument = (int*)emalloc(sizeof(int) * (1));
                //curr->argname[curr->nscales].name_argument = (int*)emalloc(sizeof(int) * (1));
                curr->argdesc[curr->nscales].length_description_argument = 0;
                curr->argname[curr->nscales].length_name_argument = 0;
            }
            if (buf[0] == '#'){
                int size_b = wcslen(buf);
                ini_desc_arg = true;
                
                curr->argdesc[curr->nscales].description_argument = (int*)erealloc(curr->argdesc[curr->nscales].description_argument,
                    sizeof(int) * (curr->argdesc[curr->nscales].length_description_argument + size_b + 1));

                for (int i = 0; i < size_b; ++i, curr->argdesc[curr->nscales].length_description_argument++)
                    curr->argdesc[curr->nscales].description_argument
                    [curr->argdesc[curr->nscales].length_description_argument]= Cod(buf[i]);

                curr->argdesc[curr->nscales].description_argument
                    [curr->argdesc[curr->nscales].length_description_argument] = -2;

                curr->argdesc[curr->nscales].length_description_argument++;
                // Имя аргумента 
                // Утечка памяти при инициализации имени
                curr->argname[curr->nscales].name_argument = (int*)erealloc(curr->argname[curr->nscales].name_argument, sizeof(int) * (size_b));

                for (int i = 0; i < size_b; ++i)
                    curr->argname[curr->nscales].name_argument[i] = Cod(buf[i]);

                curr->argname[curr->nscales].length_name_argument = size_b;
                return;
            }        
        }
        else if(buf[0] == '#' && !find_quest)
        {
            //Описание таблицы
            int size_b = wcslen(buf);

            curr->datadesc.description_table = (int*)erealloc(curr->datadesc.description_table, 
                (curr->datadesc.length_description_table + size_b + 1) * sizeof(int));

            for (int i = 0; i < size_b; ++i, curr->datadesc.length_description_table++)
                curr->datadesc.description_table[curr->datadesc.length_description_table] = Cod(buf[i]);

            curr->datadesc.description_table[curr->datadesc.length_description_table] = -2;

            curr->datadesc.length_description_table++;

            return;
        }
        if(find_quest)
        {
            if(!ini_desc_arg){
                ////Удаление ласт строчки из описания так как туда попадает имя аргумента
                ///* Удаление имени аргумента из описания */
                //int size1 = curr->argdesc[curr->nscales].length_description_argument;
                //int size2 = curr->argname[curr->nscales].length_name_argument;
                //if (size1 != size2)
                //{
                //    int s12 = size1 - size2;
                //    curr->argdesc[curr->nscales].description_argument = (int*)erealloc(curr->argdesc[curr->nscales].description_argument,
                //        sizeof(int) * s12);
                //    curr->argdesc[curr->nscales].length_description_argument = s12;
                //}
            }
            ////////////////////////////////////
            if(!ismonotonik(tok, k))
                die(L"%s:%d: fatal error: Последовательность не монотонна [interpol.getline()]\n", curr->path, line);
            curr->scales = (Scale**)erealloc(curr->scales, sizeof(Scale*) * (curr->nscales + 1));
            curr->len    = (int*)erealloc(curr->len, sizeof(int) * (curr->nscales + 1));
            curr->scales[curr->nscales] = (Scale*)ecallocz(sizeof(Scale) * k);
            ini_arg_mem = ini_desc_arg = true;
            for (int i = 0; i < k; i++){
                sreal_t a = (sreal_t)wcstold(tok[i], NULL);
                //wprintf(L"%.10Lf | ", a);
                curr->scales[curr->nscales][i].arg = a;
            }
            //printf("\n");
            curr->len[curr->nscales] = k;
            curr->nscales++;
        }
        else{
            /* Блок добавления табличных значений */
            curr->data.points = (sreal_t*)erealloc(curr->data.points, /* Расширение данных массива при получении новый значений */
            sizeof(sreal_t) * (curr->data.amount + k));

            for (int i = 0; i < k; i++)
                curr->data.points[curr->data.amount + i] = wcstod(tok[i], NULL);

            curr->data.amount += k;
            /* Конец блока */
        }
    }
}
/* Полное чтение бинарного файла */
void readerb(FILE* stream) {
    /* Определение размера файла в байтах */
    fseek(stream, 0L, SEEK_END);
    size_t const s = ftell(stream) - sizeof(long);
    fseek(stream, 0L, SEEK_SET);

    /* Проверка формата файла по маске */
    long offset = 0;
    fread(&offset, sizeof(long), 1, stream);
    if (offset != (SECURE_BIN & s)) {
        die(L"Wrong binary file's format!");
        return;
    }
    fread(&curr->nscales, sizeof(unsigned int), 1, stream);
    fread(&curr->data.amount, sizeof(unsigned int), 1, stream);
    curr->data.points = (sreal_t*)emalloc(curr->data.amount * sizeof(sreal_t));
    curr->len = (int*)emalloc(curr->nscales * sizeof(int));
    curr->scales = (Scale**)emalloc(curr->nscales * sizeof(Scale*));
    curr->argdesc = (ArgumentDesc*)emalloc(curr->nscales * sizeof(ArgumentDesc));
    curr->argname = (ArgumentName*)emalloc(curr->nscales * sizeof(ArgumentName));
    fread(curr->data.points, sizeof(sreal_t), (unsigned)curr->data.amount, stream);
    fread(&offset, sizeof(long), 1, stream);
    fread(curr->len, sizeof(int), (unsigned)curr->nscales, stream);
    for (int i = 0; i < curr->nscales; i++) {
        curr->scales[i] = (Scale*)emalloc(curr->len[i] * sizeof(Scale));
        fread((void*)(curr->scales[i]), sizeof(Scale), curr->len[i], stream);
    }

    fread(&curr->filedesc.length_description_file, sizeof(int), 1, stream);
    curr->filedesc.description_file = (int*)emalloc(curr->filedesc.length_description_file * sizeof(int));
    fread(curr->filedesc.description_file, sizeof(int), (int)curr->filedesc.length_description_file, stream);

    fread(&curr->datadesc.length_description_table, sizeof(int), 1, stream);
    curr->datadesc.description_table = (int*)emalloc(curr->datadesc.length_description_table * sizeof(int));
    fread(curr->datadesc.description_table, sizeof(int), (int)curr->datadesc.length_description_table, stream);
    
    //curr->argdesc[i].length_description_argument = (int*)emalloc(curr->nscales * sizeof(int));
    for(int i = 0; i < curr->nscales; ++i){
        fread(&(curr->argdesc[i].length_description_argument), sizeof(int), 1, stream);
    }

    for (int i = 0; i < curr->nscales; i++)
    {
        if (curr->argdesc[i].length_description_argument > 0){
            curr->argdesc[i].description_argument = (int*)emalloc(curr->argdesc[i].length_description_argument * sizeof(int));
            fread((void*)(curr->argdesc[i].description_argument), sizeof(int), curr->argdesc[i].length_description_argument, stream);
        }
    }

    //curr->length_name_argument = (int*)emalloc(curr->nscales * sizeof(int));
    for(int i = 0; i < curr->nscales; ++i)
        fread(&(curr->argname[i].length_name_argument), sizeof(int), 1, stream);

    for (int i = 0; i < curr->nscales; i++)
    {
        if(curr->argname[i].length_name_argument > 0){
            curr->argname[i].name_argument = (int*)emalloc(curr->argname[i].length_name_argument * sizeof(int));
            fread((void*)(curr->argname[i].name_argument), sizeof(int), curr->argname[i].length_name_argument, stream);
        }
    }
}
static int initialize_Scale(int const amount_args)
{
    interpol = (Interpol*)erealloc(interpol, sizeof(Interpol) * (ninterpol + 1));
    curr = interpol + ninterpol;
    memset(curr, 0, sizeof(Interpol));
    curr->format = true;
    curr->scales						= (Scale**) emalloc(sizeof(Scale*)	    * (amount_args));
    curr->argdesc			            = (ArgumentDesc*)   emalloc(sizeof(ArgumentDesc)		* (amount_args));
    curr->argname					    = (ArgumentName*)   emalloc(sizeof(ArgumentName)		* (amount_args));
    curr->len							= (int*)    emalloc(sizeof(int)		    * (amount_args));
    curr->filedesc.description_file		= (int*)    emalloc(sizeof(int)		    * (amount_args));
    curr->datadesc.description_table	= (int*)    emalloc(sizeof(int)		    * (amount_args));
    curr->path							= (wchar*)  ecallocz(sizeof(wchar)      * wcslen(L""));
    curr->users							= 1;
    curr->filedesc.length_description_file  = 0;
    curr->datadesc.length_description_table = 0;
    /* Инициализация шкал для того чтобы устнавливать в них случайым образом */
    /* pos в моем случае это i : 0 1 2 */
    /* Будем считать начало с 0 */
    for (unsigned i = 0; i < amount_args; ++i)
    {
        curr->argdesc[i].length_description_argument    = 0;
        curr->argname[i].length_name_argument           = 0;
        curr->scales[i]					                = (Scale*)emalloc(sizeof(Scale));
        curr->argdesc[i].description_argument	        = (int*)emalloc(sizeof(int));
        curr->argname[i].name_argument			        = (int*)emalloc(sizeof(int));
    }

    curr->nscales = amount_args;
    return ninterpol++;
}
const int set_amount_args(const int  am)
{
    if (am <= 0)
        die(L"Amount must be [%d] > 0 ", am);

    return initialize_Scale(am);
}
void set_scale(int const pos, sreal_t const* arg_arr, unsigned const size)
{
    if (pos < 0)
        die(L"Position must be positive: [%d]", pos);
    else if (pos >= curr->nscales)
        die(L"Position over then set: [%d]", pos);

    unsigned int i;
    if (!is_monotonic(arg_arr, size))
        die(L"%s:%d: fatal: scale is not monotonic [interpol.getline()]\n");

    curr->scales[pos] = (Scale*)erealloc(curr->scales[pos], sizeof(Scale) * size);

    for (i = 0; i < size; i++)
        curr->scales[pos][i].arg = arg_arr[i];

    curr->len[pos] = size;

    mkinterval(pos);
}
void set_data(sreal_t const* data, unsigned const size_k)
{
    /* Замена данных */
    /* Блок добавления табличных значений */
    curr->data.points = (sreal_t*)erealloc(curr->data.points, sizeof(sreal_t) * (size_k)); /* Изменение массива при получении новый значений */

    for (unsigned i = 0; i < size_k; i++)
        curr->data.points[i] = data[i];
    curr->data.amount = size_k;

    check();
}
void add_data(sreal_t const* data, unsigned const size_k)
{
    /* Расширение данных */
    /* Блок добавления табличных значений */
    curr->data.points = (sreal_t*)erealloc(curr->data.points, sizeof(sreal_t) * (curr->data.amount + size_k)); /* Изменение массива при получении новый значений */

    for (unsigned i = 0; i < size_k; i++)
        curr->data.points[curr->data.amount + i] = data[i];

    curr->data.amount += size_k;
    check();
}
void set_wstr_data(wchar const* str)
{
    wchar tokbuf[1024];
    wchar* tok[256];
    unsigned i, k;

    wcsncpy_s(tokbuf, 1024,str, 1024);
    k = wntokenize(tok, 255, tokbuf, L" ");// Возвращает кол-во строк разделенных символом
    /* Замена данных */
    /* Блок добавления табличных значений */
    curr->data.points = (sreal_t*)erealloc(curr->data.points, sizeof(sreal_t) * (k)); /* Изменение массива при получении новый значений */

    for (i = 0; i < k; i++)
        curr->data.points[i] = wcstold(tok[i], NULL);
    curr->data.amount = k;

    /* Конец блока */
    check();
}
void add_wstr_data(wchar const* str)
{
    wchar tokbuf[1024];
    wchar* tok[256];
    unsigned i, k;

    wcsncpy_s(tokbuf, 1024,str, 1024);
    k = wntokenize(tok, 255, tokbuf, L" ");// Возвращает кол-во строк разделенных символом
    /* Расширение данных */
    /* Блок добавления табличных значений */
    curr->data.points = (sreal_t*)erealloc(curr->data.points, sizeof(sreal_t) * (curr->data.amount + k)); /* Изменение массива при получении новый значений */

    for (i = 0; i < k; i++)
        curr->data.points[curr->data.amount + i] = wcstold(tok[i], NULL);

    curr->data.amount += k;
}
void set_desc_file(const wchar* opis)
{
    int size_b = wcslen(opis);
    curr->filedesc.length_description_file = size_b;
    curr->filedesc.description_file = (int*)erealloc(curr->filedesc.description_file,
        sizeof(int) * (curr->filedesc.length_description_file));

    for (int i = 0; i < size_b; ++i)
        curr->filedesc.description_file[i] = Cod(opis[i]);

}
void set_name_arg(int key, const wchar* name)
{
    int size_b = wcslen(name);

    curr->argname[key].name_argument = (int*)emalloc(sizeof(int) * (size_b));

    for (int i = 0; i < size_b; ++i)
        curr->argname[key].name_argument[i] = Cod(name[i]);

    curr->argname[key].length_name_argument = size_b;
}
void set_desc_arg(int key, const wchar* name)
{
    int size_b = wcslen(name);

    curr->argdesc[key].description_argument = (int*)emalloc(sizeof(int) * (size_b));

    for (int i = 0; i < size_b; ++i)
        curr->argdesc[key].description_argument[i] = Cod(name[i]);

    curr->argdesc[key].length_description_argument = size_b;
}
void set_desc_table(const wchar* opis)
{
    int size_b = wcslen(opis);
    curr->datadesc.length_description_table = size_b;
    curr->datadesc.description_table = (int*)erealloc(curr->datadesc.description_table,
        sizeof(int) * (curr->datadesc.length_description_table));

    for (int i = 0; i < size_b; ++i)
        curr->datadesc.description_table[i] = Cod(opis[i]);
}
Scale* get_scales(int key)
{
    return curr->scales[key];
}
int get_amount_of_args()
{
    return curr->nscales;
}
Data get_data()
{
    return curr->data;
}
int get_len_of_arg(int key)
{
    return curr->len[key];
}
const wchar*  get_descriptionFile()
{
    if(curr->filedesc.length_description_file < 0)
        return L"";
    wchar* str = (wchar*)ecallocz(curr->filedesc.length_description_file + 1);

	for (int i = 0; i < curr->filedesc.length_description_file; ++i)
		str[i] = EnCod(curr->filedesc.description_file[i]);
	return str;
}
const wchar*  get_descriptionArgument(int key)
{
	wchar* str = (wchar*)ecallocz(curr->argdesc[key].length_description_argument + 1);

	for (int i = 0; i < curr->argdesc[key].length_description_argument; ++i)
		str[i] = EnCod(curr->argdesc[key].description_argument[i]);
	return str;
}
const wchar*  get_nameArgument(int key)
{
	wchar* str = (wchar*)ecallocz(curr->argname[key].length_name_argument + 1);

	for (int i = 0; i < curr->argname[key].length_name_argument; ++i)
		str[i] = EnCod(curr->argname[key].name_argument[i]);
	return str;
}
const wchar*  get_descriptionTable()
{
    if(curr->datadesc.length_description_table < 0)
        return L"";
    wchar* str = (wchar*)ecallocz(curr->datadesc.length_description_table + 1);

	for (int i = 0; i < curr->datadesc.length_description_table; ++i)
		str[i] = EnCod(curr->datadesc.description_table[i]);
	return str;
}
/* Interpolations */
static void mkinterval(unsigned const pos)
{
    unsigned n = curr->len[pos];
    Scale* s = curr->scales[pos];
    if (n > 0) {
        s[0].min = s[0].arg;
        s[n - 1].max = s[n - 1].arg;
    }
    if (n > 1) {
        s[0].max = (s[0].arg + s[1].arg) / 2;
        s[n - 1].min = (s[n - 1].arg + s[n - 2].arg) / 2;
    }
    for (unsigned j = 1; j < n - 1; j++) {
        s[j].min = (s[j].arg + s[j - 1].arg) / 2;
        s[j].max = (s[j].arg + s[j + 1].arg) / 2;
    }
}
static void mkintervals()
{
	unsigned i, j;
	for (i = 0; i < curr->nscales; i++)
	{
    unsigned n = curr->len[i];
    Scale* s = curr->scales[i];

        if (n > 0) {
            s[0].min = s[0].arg;
            s[n - 1].max = s[n - 1].arg;
        }
        if (n > 1) {
            s[0].max = (s[0].arg + s[1].arg) / 2;
            s[n - 1].min = (s[n - 1].arg + s[n - 2].arg) / 2;
        }
        for (j = 1; j < n - 1; j++) {
            s[j].min = (s[j].arg + s[j - 1].arg) / 2;
            s[j].max = (s[j].arg + s[j + 1].arg) / 2;
        }
    }
}
static int compinterval(const void* p1, const void* p2)
{
    sreal_t arg = *(const sreal_t*)p1;
    const Scale* s = (const Scale*)p2;

    if (arg < s->min)
        return -1;
    else if (s->max <= arg)
        return 1;
    else
        return 0;
}
static int icompinterval(const void* p1, const void* p2)
{
    sreal_t arg = *(const sreal_t*)p1;
    const Scale* s = (const Scale*)p2;
    if (arg < 0 && s->min < 0 && s->max < 0)
    {
        if (fabs(arg) < fabs(s->min))/* Если числа  -10 -20 -30 и ищем интервал -15 -20 */
            return -1;
        else if (fabs(s->max) < fabs(arg))
            return 1;
        else
            return 0;
    }
    else
    {
        if (arg > s->min) /* Для положительных, но убывающих */
            return -1;
        else if (s->max > arg)
            return 1;
        else
            return 0;
    }

}
static bool is_ScaleMonotonic(Scale* s, int const size)
{
    for (int i = 1; i < size; ++i)
        if (s[i].arg > s[i - 1].arg)
            return false;
    return true;
}
static sreal_t f(int n, int offset, int* a, sreal_t* k)
{
    sreal_t f1, f2;
    int i, off = 1;

    for (i = 0; i < n - 1; i++)
        off *= curr->len[i];

    if (n == 1) {
        if (curr->len[n - 1] == 1)
            return curr->data.points[offset];
        else {
            f1 = curr->data.points[offset + off * a[n - 1]];
            f2 = curr->data.points[offset + off * (a[n - 1] + 1)];
            return INTK(k[n - 1], f1, f2);
        }
    }
    else {
        if (curr->len[n - 1] == 1)
            return f(n - 1, offset + off * a[n - 1], a, k);
        else {
            f1 = f(n - 1, offset + off * a[n - 1], a, k);
            f2 = f(n - 1, offset + off * (a[n - 1] + 1), a, k);
            return INTK(k[n - 1], f1, f2);
        }
    }
}
sreal_t interpolate(int i, int n, sreal_t const* arg) {
    int j;
	Scale* found;// находит интервал содержащией или приблезительно содержащий инт.аргумент
	sreal_t rez; // результат интерполяции
	int* a; // Содержит номера интервалов из found в Scale*
	sreal_t* k; //разница в дястичной записи на сколько отличаеться от аргумента (100 - арг, дали 120 k[j] = 0.2)
	curr = interpol + i;
	if (n != curr->nscales)
		die(L"%s: fatal: wrong arg [interpolate()]\n", curr->path);

	a = (int*)ecallocz(sizeof(int) * n);
	k = (sreal_t*)ecallocz(sizeof(sreal_t) * n);

	for (j = 0; j < n; j++)
	{
		//sreal_t VIEWARG = arg[j];
		//Scale VS[10];
		//for (int ik = 0; ik < curr->len[j]; ++ik)
		//{
		//	VS[ik].arg = curr->scales[j][ik].arg;
		//	VS[ik].min = curr->scales[j][ik].min;
		//	VS[ik].max = curr->scales[j][ik].max;
		//}
		if (is_ScaleMonotonic(curr->scales[j], curr->len[j]))
		{	/* Убывающая последовательность */
			if (curr->len[j] > 1)
			{

				if (arg[j] >= curr->scales[j][0].arg)
				{
					a[j] = 0;
					k[j] = 0;
					continue;
				}
				if (arg[j] <= curr->scales[j][curr->len[j] - 1].arg)
				{
					a[j] = curr->len[j] - 2;
					k[j] = 1;
					continue;
				}
			}
			else
				continue;

			found = (Scale*)bsearch(&arg[j], curr->scales[j],
				curr->len[j], sizeof(Scale), icompinterval);
			if (!found)
				die(L"%s: fatal: Интервал не найден [interpolate()]\n", curr->path);

			if (found->arg > arg[j]) {
				a[j] = found - curr->scales[j];
				k[j] = (arg[j] - curr->scales[j][a[j]].arg) /
					(curr->scales[j][a[j] + 1].arg - curr->scales[j][a[j]].arg);
			}
			else if (arg[j] > found->arg) {
				a[j] = found - curr->scales[j] - 1;
				k[j] = (arg[j] - curr->scales[j][a[j]].arg) /
					(curr->scales[j][a[j] + 1].arg - curr->scales[j][a[j]].arg);
			}
			else {
				a[j] = found - curr->scales[j];// запись для нахождения номера
				k[j] = 0;
			}
		}
		else
		{
			if (curr->len[j] > 1)
			{
				/* Возраст.последовательность */
				if (arg[j] <= curr->scales[j][0].arg)
				{
					a[j] = 0;
					k[j] = 0;
					continue;
				}
				if (arg[j] >= curr->scales[j][curr->len[j] - 1].arg)
				{
					a[j] = curr->len[j] - 2;
					k[j] = 1;
					continue;
				}
			}
			else
				continue;


			found = (Scale*)bsearch(&arg[j], curr->scales[j],
				curr->len[j], sizeof(Scale), compinterval);
			if (!found)
				die(L"%s: fatal: Интервал не найден [interpolate()]\n", curr->path);

			if (found->arg < arg[j]) {
				a[j] = found - curr->scales[j];
				k[j] = (arg[j] - curr->scales[j][a[j]].arg) /
					(curr->scales[j][a[j] + 1].arg - curr->scales[j][a[j]].arg);
			}
			else if (arg[j] < found->arg) {
				a[j] = found - curr->scales[j] - 1;
				k[j] = (arg[j] - curr->scales[j][a[j]].arg) /
					(curr->scales[j][a[j] + 1].arg - curr->scales[j][a[j]].arg);
			}
			else {
				a[j] = found - curr->scales[j];
				k[j] = 0;
			}
		}
	}

	rez = f(n, 0, a, k);

	efree(k);
	efree(a);

	return rez;;
}
/* Поиск свободной ячейки, если какой-либо из промежуточных номеров был освобожден. */
static int find_free_cell()
{
    for (unsigned i = 0; i < (unsigned)ninterpol; ++i)
        if (interpol[i].is_free)
            return i;

    return -1;
}
/* Проверка, что массив с данным файлом уже инициализирован. */
static int find_path(wchar const* path)
{
    for (int i = 0; i < ninterpol; i++)
        if (wcscmp(interpol[i].path, path) == 0)
            return i;

    return -1;
}
/*Main enter point*/
int mkinterpol(const wchar* path, FormatFile const fileT)
{
    FILE* fstream;
    int const free_cell = find_free_cell();
    int const same_path_number = find_path(path);
    if (same_path_number >= 0) {
        interpol[same_path_number].users++;
        return same_path_number;
    }

    if (free_cell < 0) {
        interpol = (Interpol*)erealloc(interpol, sizeof(Interpol) * (ninterpol + 1));
        curr = interpol + ninterpol;
    }
    else
        curr = interpol + free_cell;
    
    memset(curr, 0, sizeof(Interpol));
    
    curr->path = (wchar*)emalloc(sizeof(wchar) * (wcslen(path) + 1));
    wcscpy_s(curr->path, (wcslen(path) + 1), path);
    /*ini*/
	curr->users = 1;    
    curr->nscales = 0;
    find_quest      = false;
    find_attent     = false;
    desc_values     = true;
    ini_arg_mem     = true;
    ini_desc_arg    = true;
    curr->format          = true;

    switch (fileT)
    {
    case TEXT:
        _wfopen_s(&fstream, path, L"r, ccs=UTF-8");
        if(fstream == NULL){
            die(L"[ %s ]: fatal [_wfopen_s]: Невозможно открыть файл [mkinterpol()]\n", path);
            efree(interpol);
            return -1;
        }
        wreader(fstream, getwline);
        break;
    case BINARY:
        _wfopen_s(&fstream, path, L"r+b");
        if(fstream == NULL){
            die(L"[ %s ]: fatal [_wfopen_s]: Невозможно открыть бинарный файл [mkinterpol()]\n", path);
            efree(interpol);
            return -1;
        }
        readerb(fstream);
    default:
        break;
    }
    if (fstream)
		fclose(fstream);
    check();
    mkintervals();
    return ninterpol++;
}
/*************************************************************************/
static unsigned const get_free_interpol_number()
{
    unsigned number = 0;

    for (unsigned i = 0; i < ninterpol; ++i) {
        if (interpol[i].is_free)
            number++;
    }

    return number;
}
static void free_interpol(Interpol* itp)
{
    if (itp->users)
        itp->users--;

    if(itp->users > 0)
        return;

    if (itp->filedesc.length_description_file > 0)
        efree(itp->filedesc.description_file);
    if (itp->datadesc.length_description_table > 0)
        efree(itp->datadesc.description_table);
    
    for (int i = 0; i < (unsigned)itp->nscales; ++i)
    {
        efree(itp->scales[i]);
        if (itp->format) {
            //if(itp->argdesc[i].length_description_argument > 0)
            efree(itp->argdesc[i].description_argument);
            //if (itp->argname[i].length_name_argument > 0)
            efree(itp->argname[i].name_argument);
        }
    }
    if (itp->format) {
        efree(itp->argdesc);
        efree(itp->argname);
    }
    efree(itp->scales);
    efree(itp->len);
    efree(itp->data.points);
    efree(itp->path);
    itp->data.amount    = 0;
    itp->nscales  = 0;
    itp->is_free  = 1;
}
void mkfree(unsigned const i)
{
	free_interpol(interpol + i);
	if (get_free_interpol_number() == ninterpol) {
		free(interpol);
		interpol  = 0;
		ninterpol--;
	}
}

size_t size(Interpol* ii)
{
    if (ii == 0)
        return 0;

    size_t sb = 0;
    sb += sizeof(unsigned int);	// nscales
    sb += sizeof(unsigned int);	// amount
    sb += sizeof(sreal_t) * ii->data.amount;	// data
    sb += sizeof(int) * ii->nscales;	// len
    sb += sizeof(int);	// length_description_file
    sb += sizeof(int);	// length_description_table
    sb += sizeof(int) * ii->filedesc.length_description_file;// description_file
    sb += sizeof(int) * ii->datadesc.length_description_table;// description_table
    sb += sizeof(int) * ii->nscales;	// length_description_argument
    sb += sizeof(int) * ii->nscales;	// length_name_argument
    sb += sizeof(wchar); // path

    /* Scales */
    for (int i = 0; i < ii->nscales; i++)
    {
        sb += sizeof(Scale) * ii->len[i];
        sb += sizeof(int) * ii->argdesc[i].length_description_argument;//description_argument
        sb += sizeof(int) * ii->argname[i].length_name_argument;//name_argument
    }
    /* After-data offset */
    sb += sizeof(long);

    return sb;
}
size_t dumpb(unsigned const i, wchar const* fname)
{
    Interpol* ii = interpol + i;
    if (ii == 0)
        return 0;

    /* Проверка, если расширение заданного файла не равно .lid, то делаем его .lid */
    wchar fnameModf[1024];

    if (!wfindext(fname, L"lid"))//проверка есть ли .lid расширение
        wsetext(fnameModf, fname, L".lid");// нет копирует путь и добавляем расширение
    else
        wcsncpy_s(fnameModf, 1024, fname, 1024);// да просто копируем путь

    // Суммарный размер записанных данных в файл
    size_t sb = 0;

    FILE* file;
    _wfopen_s(&file, fnameModf, L"w+b");

    long offset = SECURE_BIN & size(ii);
    sb += fwrite((void*)(&offset), sizeof(long), 1, file);	// Смещение
    sb += fwrite((void*)(&ii->nscales), sizeof(unsigned int), 1, file);	// Число шкал
    sb += fwrite((void*)(&ii->data.amount), sizeof(unsigned int), 1, file);	// Число данных
    sb += fwrite((void*)(ii->data.points), sizeof(sreal_t), ii->data.amount, file);	// Данные
    long afterData = 0xbaba33;
    sb += fwrite((void*)(&afterData), sizeof(long), 1, file);	// Смещение
    sb += fwrite((void*)(ii->len), sizeof(int), ii->nscales, file);	// Длины шкал
    for (int i = 0; i < ii->nscales; i++)
        sb += fwrite((void*)(ii->scales[i]), sizeof(Scale), ii->len[i], file);	// Шкалы
    
    sb += fwrite((void*)(&ii->filedesc.length_description_file), sizeof(int), 1, file); // Длинна описание файла
    sb += fwrite((void*)(ii->filedesc.description_file), sizeof(int), ii->filedesc.length_description_file, file);	// Данные

    sb += fwrite((void*)(&ii->datadesc.length_description_table), sizeof(int), 1, file); // Длинна описание таблицы
    sb += fwrite((void*)(ii->datadesc.description_table), sizeof(int), ii->datadesc.length_description_table, file);	// Данные

    for (int i = 0; i < ii->nscales; i++)
        sb += fwrite(&(ii->argdesc[i].length_description_argument), sizeof(int), 1, file);	// Длины описаний аргументов
    
    for (int i = 0; i < ii->nscales; i++)
    {
        if(ii->argdesc[i].length_description_argument > 0)
            sb += fwrite((void*)(ii->argdesc[i].description_argument), sizeof(int), ii->argdesc[i].length_description_argument, file);	// Описание аргументов
    }

    for (int i = 0; i < ii->nscales; i++)
        sb += fwrite(&(ii->argname[i].length_name_argument), sizeof(int), 1, file);	// Длины описаний аргументов
    
    for (int i = 0; i < ii->nscales; i++)
    {
        if (ii->argname[i].length_name_argument > 0)
            sb += fwrite((void*)(ii->argname[i].name_argument), sizeof(int), ii->argname[i].length_name_argument, file);// Имена аргументов
    }
    fclose(file);

    return sb;
}
size_t dumpt(unsigned const i, wchar const* fname)
{
    Interpol* ii = interpol + i;
    if (ii == NULL)
        return 0;

    wchar fnameModf[1024];

    wsetext(fnameModf, fname, L".dat");
    FILE* file;

    _wfopen_s(&file, fnameModf, L"w, ccs=UTF-8");

    for (int i = 0; i < ii->filedesc.length_description_file; ++i)
    {
        fwprintf_s(file, L"%c", EnCod(ii->filedesc.description_file[i]));

    }
    /* Вывод списка аргументов */
    fwprintf_s(file, L"\n?\n");
    for (int i = 0; i < ii->nscales; i++) 
    {
        if (curr->format) {
            if (ii->argdesc[i].length_description_argument != 0)
                for (int j = 0; j < ii->argdesc[i].length_description_argument; ++j)
                {
                    fwprintf_s(file, L"%c", EnCod(ii->argdesc[i].description_argument[j]));
                }
            fwprintf_s(file, L"\n");
            if (ii->argname[i].length_name_argument != 0)
                for (int j = 0; j < ii->argname[i].length_name_argument; ++j)
                {
                    fwprintf_s(file, L"%c", EnCod(ii->argname[i].name_argument[j]));
                }
        }
        fwprintf_s(file, L"\n");
        for (int j = 0; j < ii->len[i]; j++)
            fwprintf_s(file, L"%lf\t", ii->scales[i][j].arg);

        fwprintf_s(file, L"\n");
    }
    fwprintf_s(file, L"\n!\n\n");
    /* Вывод описание таблицы */
    for (int i = 0; i < curr->datadesc.length_description_table; ++i)
    {
        fwprintf(file, L"%c", EnCod(curr->datadesc.description_table[i]));
    }

    /* Вывод табличных данных */
    fwprintf_s(file, L"\n");
    for (int i = 0; i < ii->data.amount; i++) {
        fwprintf_s(file, L"%lf\t", ii->data.points[i]);
        if (!((i + 1) % ii->len[0]))
            fwprintf_s(file, L"\n");
    }
    fclose(file);

    return 1;
}

#ifdef DEBUG
/***********DEBUG****************/
void printInter()
{
    printf_s("\n");
    for(sreal_t a = 1.5; a <= 4.5; a+= 1.5){
        for(sreal_t b = 4; b <= 6; b+= 0.5)
        {
            sreal_t const arg[] = {a , b};
            printf_s(" %f |", interpolate(0, 2, arg));
        }
        printf_s("\n");
    }
}
void displayDisriptionOfData()
{
    for(int i = 0 ; i < curr->datadesc.length_description_table;++i)
        wprintf_s(L"%c",EnCod(curr->datadesc.description_table[i]));
}
void displayDisriptionOfTable()
{
    for(int i = 0; i < curr->filedesc.length_description_file;++i)
        wprintf_s(L"%c",EnCod(curr->filedesc.description_file[i]));
}
void displayScales()
{
    if(curr->format)
    for(int i = 0 ; i < curr->nscales; ++i)
    {
        if(curr->argdesc[i].length_description_argument)
        for(int j = 0; j < curr->argdesc[i].length_description_argument; ++j)
            wprintf_s(L"%c", EnCod(curr->argdesc[i].description_argument[j]));
        printf_s("\n");
        if(curr->argname[i].length_name_argument)
        for(int j = 0; j < curr->argname[i].length_name_argument; ++j)
            wprintf_s(L"%c", EnCod(curr->argname[i].name_argument[j]));
        printf_s("\n");
        for(int j = 0; j < curr->len[i]; ++j)
        {
            printf_s("Arg = %.8Lf | Min = %.2Lf | Max = %.2Lf | \n", curr->scales[i][j].arg, curr->scales[i][j].min, curr->scales[i][j].max);
        }
        printf_s("\n");
    }
}
void displayData()
{
    int k = 0;
    for(int i = 0; i < curr->data.amount; ++i, ++k)
    {   
        if(k == curr->len[0]){
            printf_s("\n");
            k = 0;
        }
        printf_s("%.5Lf | ",curr->data.points[i]);
    }
}
#endif