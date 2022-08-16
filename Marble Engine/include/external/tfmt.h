/*
 * BSD 3-Clause License
 *
 * Copyright 2022 TophUwO <tophuwo01@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *     (1) Redistributions of source code must retain the above copyright notice, this
 *         list of conditions and the following disclaimer.
 *     (2) Redistributions in binary form must reproduce the above copyright notice, this
 *         list of conditions and the following disclaimer in the documentation and/or other
 *         materials provided with the distribution.
 *     (3) Neither the name of the copyright holder nor the names of its contributors may be
 *         used to endorse or promote products derived from this software without specific
 *         prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 */


#pragma once

#include <stdint.h> /* for standard integer types */
/*
 * Make "localtime_s()" available on platforms that
 * support it.
 */
#if (defined __STDC_LIB_EXT1__)
	#define __STDC_WANT_LIB_EXT1__ (1)
#endif
#include <time.h>   /* for struct tm */

/*
 * This is just here to avoid errors on platforms
 * that do not support SAL 2.0.
 * Other compiler may have different annotation languages
 * which are currently not supported.
 */
#if _SAL_VERSION < 20
	#define _In_
	#define _In_z_
	#define _In_opt_
	#define _Inout_
	#define _Inout_opt_z_
	#define _Success_(expr)
#endif 

/*
 * Formats a C-string according to the format
 * specified by **pz_fmt** and writes it into
 * **pz_buf**.
 * 
 * Behavior in case of special conditions:
 *  (1) If **size** is not large enough to hold the entire string,
 *      the function will write max. **size** bytes to the **pz_buf**,
 *      properly NUL-terminating it.
 *  (2) If **pz_fmt** is NULL or an empty string, the function returns 0
 *      and the contents of **pz_buf** remain unchanged.
 *  (3) If **pz_buf** is NULL, the function returns the minimum size that
 *      is needed to store the entire formatted string inside **pz_buf**,
 *      including the NUL-terminator.
 *  (4) If **ps_time** is NULL, the function will call "localtime()" to get
 *      the time adjusted for the current timezone, and proceed to format
 *      using the obtained data. If "localtime()" fails, the function returns 0
 *      and the contents of **pz_buf** remain unchanged.
 *  (5) If **ps_time** is not NULL but contains invalid data, the function
 *      returns 0 and the contents of **pz_buf** remain unchanged.
 *  (6) In case of an unexpected error with the expansion of a format specifier,
 *      the format specifier will just be ignored and NOT be replaced with
 *      any character.
 * 
 * Note that locales are not supported, meaning that calls to "setlocale()" will
 * have no effect on the output of this function.
 * 
 * The function is UTF-8-aware and will apply the same rules for both format
 * specifiers and individual UTF-8 characters if there is not enough space to
 * copy-over the character.
 * 
 * Supported format specifiers:
 * ---------------------------
 * a => abbreviated weekday name (e.g. "Thu")
 * A => full weekday name (e.g. "Thursday")
 * b => abbreviated month name (e.g. "Feb")
 * B => full month name (e.g. "February")
 * c => date and time representation (MM/DD/YY - HH:MM:SS)
 * C => century (first two digits of the current year)
 * d => day of the month, zero-padded (01 ... 31)
 * D => standard date format, equivalent to "%m/%d/%y"
 * e => day of the month, space-padded (_1 ... 31)
 * F => YYYY-MM-DD date notation, equivalent to "%Y-%m-%d"
 * g => week-based year, last to digits (YY)
 * G => week-based year (YYYY)
 * H => 24-hour hour (00 ... 23)
 * I => 12-hour hour (01 ... 12)
 * j => day of the year (001 ... 366)
 * m => month number [(00 ... 11) + 1]
 * M => minutes (00 ... 59)
 * p => AM/PM designation
 * r => 12-hour time in 'HH:MM:SS' format
 * R => 24-hour time in 'HH:MM' format
 * S => seconds (00 ... 61)
 * T => ISO 8601 standard time format (HH:MM:SS)
 * u => ISO 8601 week day ([1 ... 7], Monday = 1)
 * V => ISO 8601 week number (1 ... 53)
 * w => day of the week with Sunday being 0 (0 ... 6)
 * x => date representation ('YYYY-MM-DD')
 * X => time representation ('HH:MM:SS')
 * y => year, last two digits (00 ... 99)
 * Y => full year number since 1900 (0 ... 299)
 * Z => timezone name or abbreviation
 * % => a literal '%' sign
 * 
 * Returns the number of bytes written to **pz_buf**, including
 * NUL-terminator.
 */
extern _Success_(return > 0) size_t tfmt_strftime(
	_Inout_opt_z_ char *restrict pz_buf,            /* buffer that will receive the formatted string */
	_In_z_        char const *restrict pz_fmt,      /* format template **pz_buf** will be formatted after */
	_In_          size_t size,                      /* maximum size of **pz_buf**, in bytes. */
	_In_opt_      struct tm const *restrict ps_time /* time data that will be used for formatting */
);


