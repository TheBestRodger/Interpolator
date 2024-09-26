
//////////////////////////////////////////////////////////////////////////////
// interpolator.cpp
// Copyright (c) 2014 by Alexander Kuslya

extern "C"{
#include "Inter/common.h"
#include "Inter/exception.h"
}

#include "test/InterCXX.hpp"
#include "locale.h"
#include <algorithm>
#include <iomanip>

//////////////////////////////////////////////////////////////////////////////
namespace science{
/** \brief Перевод структуры типа Scale* в вектор */
static const std::vector<sreal_t> Scale_to_stdVector(const Scale* array, int size);

/* P U B L I C */
/*************************************************************************/
Interpolator::Interpolator()
    : identifier_(-1)
	, ref_counter(0)
    , mode(false)
{
	ref_counter = new int(0);
}


/*************************************************************************/
Interpolator::Interpolator(wchar const* fname, FormatFile const f /*= FormatT*/)
    : identifier_(mkinterpol(fname, f))
	, ref_counter(0)
    , fname_(fname)
    , mode(false)
{
	ref_counter = new int(0);
}


/*************************************************************************/
Interpolator::Interpolator(unsigned const amount_of_args)
    : identifier_(-1)
    , ref_counter(0)
    , mode(true)
{
    ref_counter = new int(0);
    make_data(amount_of_args);
}


/*************************************************************************/
Interpolator::Interpolator(Interpolator const& r)
	: identifier_(-1)
	, ref_counter(0)
    , mode(false)
{
	*this = r;
}


/*************************************************************************/
Interpolator::~Interpolator()
{
	countdown();
	free();
}


/*************************************************************************/
void Interpolator::set_file(wchar const* fname, FormatFile const f /*= FormatT*/)
{
    identifier_ = mkinterpol(fname, f);
    fname_ = fname;
}


/*************************************************************************/
void Interpolator::set_file(const std::wstring & fname, FormatFile const f /*= FormatT*/)
{

    identifier_ = mkinterpol(fname.c_str(), f);
    fname_ = fname;
    
}


/*************************************************************************/
void Interpolator::make_data(int const _amount_of_args)
{
    fname_ = L"";
    mode = true;
    identifier_ = set_amount_args(_amount_of_args);
    
}


/*************************************************************************/
sreal_t const Interpolator::operator() (sreal_t const arg1) const
{
    static unsigned const number_of_arguments = 1;
    sreal_t const arg[] = {arg1};

    return operator() (arg, number_of_arguments);
}

/*************************************************************************/
sreal_t const Interpolator::operator() (
      sreal_t const arg1
    , sreal_t const arg2
    ) const
{
    static unsigned const number_of_arguments = 2;
    sreal_t const arg[] = {arg1, arg2};

    return operator() (arg, number_of_arguments);
}

/*************************************************************************/
sreal_t const Interpolator::operator() (
      sreal_t const arg1
    , sreal_t const arg2
    , sreal_t const arg3
    ) const
{
    static unsigned const number_of_arguments = 3;
    sreal_t const arg[] = {arg1, arg2, arg3};

    return operator() (arg, number_of_arguments);
}

/*************************************************************************/
sreal_t const Interpolator::operator() (
      sreal_t const arg1
    , sreal_t const arg2
    , sreal_t const arg3
    , sreal_t const arg4
    ) const
{
    static unsigned const number_of_arguments = 4;
    sreal_t const arg[] = {arg1, arg2, arg3, arg4};

    return operator() (arg, number_of_arguments);
}

/*************************************************************************/
sreal_t const Interpolator::operator() (
      sreal_t const arg1
    , sreal_t const arg2
    , sreal_t const arg3
    , sreal_t const arg4
    , sreal_t const arg5
    ) const
{
    static unsigned const number_of_arguments = 5;
    sreal_t const arg[] = {arg1, arg2, arg3, arg4, arg5};

    return operator() (arg, number_of_arguments);
}

/*************************************************************************/
sreal_t const Interpolator::operator() (
      sreal_t const arg1
    , sreal_t const arg2
    , sreal_t const arg3
    , sreal_t const arg4
    , sreal_t const arg5
    , sreal_t const arg6
    ) const
{
    static unsigned const number_of_arguments = 6;
    sreal_t const arg[] = {arg1, arg2, arg3, arg4, arg5, arg6};

    return operator() (arg, number_of_arguments);
}

/*************************************************************************/
sreal_t const Interpolator::operator() (
      sreal_t const* arg_arr
    , unsigned const size
    ) const
{
    return interpolate(identifier_, size, arg_arr);
}


/*************************************************************************/
Interpolator& Interpolator::operator= (Interpolator const& r)
{
	countdown();

	ref_counter = r.ref_counter;
	(*ref_counter)++;

	fname_ = r.fname_;
	identifier_ = r.identifier_;

	return *this;
}


/*************************************************************************/
std::wstring const& Interpolator::fname() const
{
    return fname_;
}


/*************************************************************************/
void Interpolator::dump(wchar const* fname, FormatFile const f /* = FormatT */)
{
    switch (f)
    {
    case TEXT:
        dumpt(identifier_, fname);
        break;
    case BINARY:
		dumpb(identifier_, fname);
		break;
    default:
        break;
    }
}



/*************************************************************************/
void Interpolator::dump(const std::wstring& fname, FormatFile const f /* = FormatT */)
{
    switch (f)
    {
    case TEXT:
        dumpt(identifier_, fname.c_str());
        break;
    case BINARY:
        dumpb(identifier_, fname.c_str());
        break;
    default:
        break;
    }
}


/*************************************************************************/
void Interpolator::set_argument(int const pos, sreal_t const* arg_arr, unsigned const size)
{
    if (mode)
        set_scale(pos, arg_arr, size);
    else
        die(L"ERROR mode, use make data before set argument");
}


/*************************************************************************/
void Interpolator::set_argument(int const pos, const std::vector<sreal_t> & vector_arg_arr)
{
    if (mode)
        set_scale(pos, vector_arg_arr.data(), vector_arg_arr.size());
    else
        die(L"ERROR mode, use make data before set argument");
}


/*************************************************************************/
void Interpolator::set_values(wchar const* arg_ndat)
{
    if (mode)
        set_wstr_data(arg_ndat);
    else
        die(L"Error mode use make data before set values");
}


/*************************************************************************/
void Interpolator::set_values(sreal_t const* arg_ndat, unsigned size_k)
{
    if (mode)
        set_data(arg_ndat, size_k);
    else
        die(L"ERROR mode, use make data before set values");
}


/*************************************************************************/
void Interpolator::set_values(const std::vector<sreal_t> & vector_data)
{
    if (mode)
        set_data(vector_data.data(), vector_data.size());
    else
        die(L"ERROR mode, use make data before set values");
}


/*************************************************************************/
void Interpolator::set_description_file(const wchar* description_file)
{
    if (mode)
        set_desc_file(description_file);
    else
        die(L"ERROR mode, use make data before set_description_file");
}


/*************************************************************************/
void Interpolator::set_description_file(const std::wstring description_file)
{
    if (mode)
        set_desc_file(description_file.c_str());
    else
        die(L"ERROR mode, use make data before set_description_file");
}


/*************************************************************************/
void Interpolator::set_description_argument(const int pos, const wchar* description_arg)
{
    if (mode)
        set_desc_arg(pos, description_arg);
    else
        die(L"ERROR mode, use make data before set_description_argument");
}


/*************************************************************************/
void Interpolator::set_description_argument(const int pos, const std::wstring description_arg)
{
    if (mode)
        set_desc_arg(pos, description_arg.c_str());
    else
        die(L"ERROR mode, use make data before set_description_argument");
}


/*************************************************************************/
void Interpolator::set_name_argument(const int pos, const wchar* description_arg)
{
    if (mode)
        set_name_arg(pos, description_arg);
    else
        die(L"ERROR mode, use make data before set_name_argument");
}


/*************************************************************************/
void Interpolator::set_name_argument(const int pos, const std::wstring description_arg)
{
    if (mode)
        set_name_arg(pos, description_arg.c_str());
    else
        die(L"ERROR mode, use make data before set_name_argument");
}


/*************************************************************************/
void Interpolator::set_description_table(const wchar* description_file)
{
    if (mode)
        set_desc_table(description_file);
    else
        die(L"ERROR mode, use make data before set_description_table");
}


/*************************************************************************/
void Interpolator::set_description_table(const std::wstring description_file)
{
    if (mode)
        set_desc_table(description_file.c_str());
    else
        die(L"ERROR mode, use make data before set_description_table");
}


/*************************************************************************/
const std::vector<sreal_t> Interpolator::get_argument(int key)
{
    if (identifier_ >= 0 && key >= 0)
        return Scale_to_stdVector(get_scales(key), get_len_of_arg(key));
    else
        die(L"ERROR identifier_ [%d], key [%d]", identifier_, key);

    return Scale_to_stdVector(0, 0);
}


/*************************************************************************/
const std::vector<sreal_t> Interpolator::get_values()
{
    std::vector<sreal_t> a(get_data().points, get_data().points + get_data().amount);
    if (identifier_ >= 0)
        return a;
    else
        die(L"ERROR identifier_ [%d]", identifier_);
    a = { 0 };
    return a;
}


/*************************************************************************/
const int Interpolator::get_arguments_count() const
{
    if (identifier_ >= 0)
        return get_amount_of_args();
    else
        die(L"ERROR identifier_ [%d]", identifier_);
    return NULL;
}


/*************************************************************************/
const std::wstring Interpolator::get_description_file()
{
    return get_descriptionFile();
}


/*************************************************************************/
const std::wstring Interpolator::get_description_argument(int pos)
{
    if (pos < 0)
    {
        die(L"ERROR position of argument: %d", pos);
        return NULL;
    }
    return get_descriptionArgument(pos);
}


/*************************************************************************/
const std::wstring Interpolator::get_name_argument(int pos)
{
    if (pos < 0)
    {
        die(L"ERROR position of argument: %d", pos);
        return NULL;
    }
    return get_nameArgument(pos);
}


/*************************************************************************/
const std::wstring Interpolator::get_description_table()
{
    return get_descriptionTable();
}


/* P R I V A T E */
/*************************************************************************/
void Interpolator::countdown()
{
	if (ref_counter) {
		(*ref_counter)--;
		free();
	}
}


/*************************************************************************/
void Interpolator::free()
{
	if (identifier_ != unsigned(-1) && ref_counter && *ref_counter < 0) {
		mkfree(identifier_);
		identifier_ = -1;
		fname_.clear();
	}

	if (ref_counter && *ref_counter < 0) {
		delete ref_counter;
		ref_counter = 0;
	}
}


/*************************************************************************/
static const std::vector<sreal_t> Scale_to_stdVector(const Scale* arr, int size)
{
    std::vector<sreal_t> a;
    for (unsigned i = 0; i < size; ++i)
        a.push_back(arr[i].arg);
    return a;
}

} // namespace science