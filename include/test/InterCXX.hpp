#pragma once
/*!*****************************************************************************
	@file interpolator.hpp				               
	@author Kuslya A.M.
	@date 2023/05/25
	@copyright Copyright (c) 2023 by Alexander Kuslya <alexander.kuslya@gmail.com>
	@brief Модуль интерполяции
	
	Данный модуль содержит определение класса многомерной линейной интерполяции
    Interpolator, предназначенного для работы со специализированными входными
    файлами в текстовом и бинарном форматах.
*******************************************************************************/


#ifndef __LIBSCIENCE_INTERPOLATOR_HPP_INCLUDED__
#define __LIBSCIENCE_INTERPOLATOR_HPP_INCLUDED__

extern "C"{
#include "Inter/extra_type.h"
#include "Inter/type.h"
}

#include <string>
#include <vector>

/** \brief science - Common utilities library */
namespace science {

/*!*****************************************************************************
	@brief Модуль многомерной линейной интерполяции
	@author Kuslya A.M.
	@version 1.0
	@date 2023/03/01
	
	Класс-обертка для функционала многоменрной линейной интерполяции, работающей
    как с текстовым, так и с бинарным входным потоком данных
*******************************************************************************/
class Interpolator
{
    /** \brief Уникальный идентификатор на блок данных для интерполяции */
    int identifier_;

    /** \brief Счетчик ссылок на один и тотже блок данных */
	int* ref_counter;

    /** \brief Имя загруженного файла */
    std::wstring fname_;
    /* \brief Режим работы интерполятора false - атоматический | true - ручной*/
    bool mode;
public:
    /** \brief Конструктор по-умолчанию */
    Interpolator();

    /*!***************************************************************************** 
    	@brief Конструктор с аргументами
		@param fname - строка с именем файла
		@param f - формат входного файла: бинарный или текстовый
    	
    	Конструктор, инициализирующий модуль интерполяции файлом с именем fname 
        заданного формата f.
    *******************************************************************************/
    Interpolator(wchar const* fname, FormatFile const f = TEXT);


    /*!*****************************************************************************
        @brief Конструктор с аргументами.
        @param amount - количество аргументов, которое будет добавленно.

        Конструктор, инициализирующий модуль для создание данных для интерполяции.
    *******************************************************************************/
    Interpolator(unsigned const amount);


    /** \brief Конструктор копирования */
	Interpolator(Interpolator const& other);


    /** \brief Стандартный деструктор */
    ~Interpolator();

    void set_file(wchar const* fname, FormatFile const f = TEXT);


    void set_file(const std::wstring & fname, FormatFile const f = TEXT);


    void make_data(int const  amount_of_args);


    sreal_t const operator() (sreal_t const arg1) const;

    sreal_t const operator() (
          sreal_t const arg1
        , sreal_t const arg2
        ) const;

    sreal_t const operator() (
          sreal_t const arg1
        , sreal_t const arg2
        , sreal_t const arg3
        ) const;

    sreal_t const operator() (
          sreal_t const arg1
        , sreal_t const arg2
        , sreal_t const arg3
        , sreal_t const arg4
        ) const;

    sreal_t const operator() (
          sreal_t const arg1
        , sreal_t const arg2
        , sreal_t const arg3
        , sreal_t const arg4
        , sreal_t const arg5
        ) const;

    sreal_t const operator() (
          sreal_t const arg1
        , sreal_t const arg2
        , sreal_t const arg3
        , sreal_t const arg4
        , sreal_t const arg5
        , sreal_t const arg6
        ) const;


    sreal_t const operator() (
          sreal_t const* arg_arr
        , unsigned const size
        ) const;


    /** \brief Оператор копирования */
	Interpolator& operator = (Interpolator const& r);

	std::wstring const& fname() const;

	void dump(wchar const* fname, FormatFile const f = TEXT);

    void dump(const std::wstring & fname, FormatFile const f = TEXT);

    void set_argument(int const pos, sreal_t const* arg_arr, unsigned const size);

    void set_argument(int const pos, const std::vector<sreal_t> &vector_arg_arr);

    void set_values(wchar const* arg_ndat);

    void set_values(sreal_t const* arg_ndat, unsigned size_k);

    void set_values(const std::vector<sreal_t> &vector_data);
    /******************************************************************************/
    void set_description_file(const wchar * description_file);
    void set_description_file(const std::wstring description_file);

    void set_description_argument(const int pos, const wchar* description_arg);
    void set_description_argument(const int pos, const std::wstring description_arg);

    void set_name_argument(const int pos, const wchar* description_arg);
    void set_name_argument(const int pos, const std::wstring description_arg);

    void set_description_table(const wchar* description_file);
    void set_description_table(const std::wstring description_file);
    /******************************************************************************/

    const std::vector<sreal_t> get_argument(int key);

    const std::vector<sreal_t> get_values();

    const int get_arguments_count() const;

    const std::wstring get_description_file();
    const std::wstring get_description_argument(int pos);

    const std::wstring get_name_argument(int pos);
    const std::wstring get_description_table();

private:
    /** \brief Уменьшение счетчика ссылок на одиницу */
	void countdown();

    /** \brief Выгрузка данных из памяти с последующей ее очисткой */
	void free();
};

} /* namespace science */


#endif  // __LIBSCIENCE_INTERPOLATOR_HPP_INCLUDED__