#pragma once
/*!*****************************************************************************
	@file config.hpp				               
	@author Kuslya A.M.
	@date 2013/12/10
	@copyright Copyright (c) 2022 by Alexander Kuslya <alexander.kuslya@gmail.com>
	@brief Конфигурационный файл библиотеки Science
	
	Конфигурационный файл библиотеки Science, который содержит в себе определение
	основных типов данных, используемых в библиотеке
*******************************************************************************/


#ifndef __LIBSCIENCE_CORE_TYPE_HPP_INCLUDED__
#define __LIBSCIENCE_CORE_TYPE_HPP_INCLUDED__

/** \brief Определение основной переменной с плавающей запятой, 
		   использующейся в библиотеке science.
*/
typedef double 		sreal_t;
/////////////////////////////////////////////////////////////


/** \brief Максимально возможная положительная величина для типа sreal_t */
static sreal_t const MAX_SREAL_T = 1.7976931348623158e+308;

/** \brief Минимально возможная положительная величина для типа sreal_t */
static sreal_t const MIN_SREAL_T = 2.2250738585072014e-308;


#endif  // __LIBSCIENCE_CORE_TYPE_HPP_INCLUDED__