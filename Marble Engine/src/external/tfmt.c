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


/* References used:
 * --------------------------------------------------------------
 *
 * https://cplusplus.com/reference/ctime/strftime/
 * https://en.wikipedia.org/wiki/ISO_week_date
 * https://webspace.science.uu.nl/~gent0113/calendar/isocalendar.htm
 */


#include <external/tfmt.h>


#define TFMT_ESCCHAR ('%')

/* Just serves as a shortcut; makes the code a little bit cleaner, too. */
#define TFMT_CURWD               (glas_weekdaydata[ps_time->tm_wday])
/* Just serves as a shortcut; makes the code a little bit cleaner, too. */
#define TFMT_CURMON              (glas_monthnamedata[ps_time->tm_mon])
/* 
 * Checks whether a numeric value 
 * **x** lies within **lo** and **hi**
 * (bounds are inclusive). 
 */
#define TFMT_INRANGE(x, lo, hi)  (x >= lo && x <= hi)
/*
 * Converts the year number found in "struct tm" (years since 1900)
 * to the actual, ordinal year.
 */
#define TFMT_ORDYEAR(year)       (year + 1900)
/*
 * Converts the day-of-the-year number found in "struct tm" (days since
 * January 1st) to the actual, ordinal DOY number (1 ... 366)
 */
#define TFMT_ORDDOY(doy)         (doy + 1)
/*
 * Converts regular weekday number ([0 ... 6] first day = Sunday)
 * to ISO 8601 weekday number ([1 ... 7] first day = Monday).
 */
#define TFMT_ISO8601WD(wd)       (wd == 0 ? 7 : wd)
/*
 * Calculates the ISO 8601 week number (1 - 53), based on the current
 * day of the year and day of the week.
 * 
 * **doy** e { 1; 366 }
 * **dow** e { 1; 7 }
 */
#define TFMT_ISO8601WN(doy, dow) ((doy - dow + 10) / 7)
/*
 * Calculates the ISO 8601 weekday of December 31st of **year**.
 * ISO 8601 weeks start on Monday and end on Saturday.
 */
#define TFMT_ISO8601LWD(year)    ((year + year / 4 - year / 100 + year / 400) % 7)
/*
 * Converts current hour (00 ... 23) from 24h to 12h
 * format.
 */
#define TFMT_24HTO12H(hour)      (hour > 12 ? hour - 12 : (hour ? hour : 12))
/*
 * Evaluates to 1 (true) if **hour** is PM, 0 (false)
 * if **hour** is AM.
 */
#define TFMT_ISPM(hour)          (hour > 11)
/*
 * Evaluates 1 (true) if **year** is a leap year, 0 (false)
 * **year** is not a leap year.
 */
#define TFMT_ISLEAPYEAR(year)    ((year) % 4 == 0 && ((year) % 100 != 0 || (year) % 400 == 0))
/*
 * Evaluates to the last day of February of **year**. This can be
 * either 28 or 29, depending on whether **year** is a leap
 * year or not.
 */
#define TFMT_LASTMDAYFEB(year)   (28 + TFMT_ISLEAPYEAR(year))  
/*
 * Evaluates to the last day of the year of **year**. Note that this
 * does not have to coincide with the ISO 8601 last day of the year.
 */
#define TFMT_LASTDOY(year)       (364 + TFMT_ISLEAPYEAR(year))
/*
 * Converts **dst** (daylight saving time) flag to dst index.
 * This may be needed as **ps_time->tm-isdst** can also be -1
 * (meaning no information), which we want to specifically
 * interpret as 0 (standard time).
 */
#define TFMT_ISDST(dst)          (dst == 1)
/*
 * Calculates length of a string literal, excluding
 * NUL-terminator.
 */
#define TFMT_SZLIT(str)          (sizeof str - 1)


#pragma region STATIC-DATA
/*
 * Predefined arrays of strings that we can access
 * and copy from very quickly.
 * 
 * Note that invalid data provided by **ps_time** can
 * result in undefined behavior, as these arrays do not
 * predict error cases (i.e. numbers too large to validly
 * be used as an index to one of these arrays).
 */

/*
 * Look-up table used to get the size in bytes of a UTF-8
 * character. This value will then be used to copy the required
 * number of bytes. It does not account for invalid encoding, as
 * the only thing one needs to know the size of the encoded
 * character is its first byte. In case the input string is badly
 * encoded, the behavior is generally undefined.
 */
static size_t const gla_utf8charsz[256] = {
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,   2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,   4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 6
};

/*
 * Used by some format specifiers to copy-over predefined number strings.
 * There is no real need to do any "itoa()" etc.
 */
static char const *const glaz_num00thru99[] = {
	"00", "01", "02", "03", "04", "05", "06",
	"07", "08", "09", "10", "11", "12", "13",
	"14", "15", "16", "17", "18", "19", "20",
	"21", "22", "23", "24", "25", "26", "27",
	"28", "29", "30", "31", "32", "33", "34",
	"35", "36", "37", "38", "39", "40", "41",
	"42", "43", "44", "45", "46", "47", "48",
	"49", "50", "51", "52", "53", "54", "55",
	"56", "57", "58", "59", "60", "61", "62",
	"63", "64", "65", "66", "67", "68", "69",
	"70", "71", "72", "73", "74", "75", "76",
	"77", "78", "79", "80", "81", "82", "83",
	"84", "85", "86", "87", "88", "89", "90",
	"91", "92", "93", "94", "95", "96", "97",
	"98", "99"
};

/* 
 * Look-up table defining default English weekday
 * names, in their long and short form, with Sunday
 * being the first day of the week, according to
 * the C standard.
 */
static struct {
	/* full weekday name */
	char const *mpz_wdlong;
	/* abbreviated weekday name */
	char const *mpz_wdshort;
	/* 
	 * Length of full weekday name, in bytes (excl. NUL).
	 * The length of the short weekday name does not need
	 * to be stored, as it will always be 3 in this
	 * particular implementation.
	 */
	size_t      m_wdlongsz;
} const glas_weekdaydata[7] = {
	{ "Sunday",    "Sun", 6 },
	{ "Monday",    "Mon", 6 },
	{ "Tuesday",   "Tue", 7 },
	{ "Wednesday", "Wed", 9 },
	{ "Thursday",  "Thu", 8 },
	{ "Friday",    "Fri", 6 },
	{ "Saturday",  "Sat", 8 }
};

/* 
* Look-up table defining default English month
* names, in their long and short form, with January
* being the first month, according to the C standard.
*/
static struct {
	/* full month name */
	char const *mpz_monlong;
	/* abbreviated month name */
	char const *mpz_monshort;
	/*
	 * Length of full month name, in bytes (excl. NUL).
	 * The length of the short month name does not need
	 * to be stored, as it will always be 3 in this
	 * particular implementation.
	 */
	size_t      m_monlongsz;
} const glas_monthnamedata[12] = {
	{ "January",   "Jan", 7 },
	{ "February",  "Feb", 8 },
	{ "March",     "Mar", 5 },
	{ "April",     "Apr", 5 },
	{ "May",       "May", 3 },
	{ "June",      "Jun", 3 },
	{ "July",      "Jul", 3 },
	{ "August",    "Aug", 6 },
	{ "September", "Sep", 9 },
	{ "October",   "Oct", 7 },
	{ "November",  "Nov", 8 },
	{ "December",  "Dec", 8 }
};

/* AM/PM designation */
static char const *const glaz_designampm[] = { "AM", "PM" };
#pragma endregion


/*
 * Small and naive implementation of "memcpy()". It's not really
 * faster than actual memcpy in my tests, but it removes the
 * dependency.
 * This function is, admittedly, a bit obsolete, as we could just 
 * use "strcpy()" to do the same, but that would also add
 * a dependency.
 * 
 * Returns nothing.
 */
static void tfmt_internal_copy(
	_Inout_ char *restrict pz_dest,      /* destination buffer */
	_In_z_  char const *restrict pz_src, /* source buffer */
	_In_    size_t size                  /* number of bytes to copy */
) {
	while (size--)
		*pz_dest++ = *pz_src++;
}

/*
 * Convert a number into its **ndigits**-digit decimal
 * string representation, inserting zeroes where needed.
 * 
 * Returns nothing.
 */
static void tfmt_internal_toint(
	_Inout_ char *const pz_buf, /* output buffer */
	        int number          /* year number to convert */,
	        size_t ndigits      /* number of digits */
) {
	do {
		*(pz_buf + --ndigits) = '0' + number % 10;

		number /= 10;
	} while (ndigits);
}

/*
 * Validate **ps_time** parameter.
 * 
 * If the parameter and its members are valid,
 * the function returns non-zero. If not, the 
 * return value is 0.
 */
static _Bool tfmt_internal_isparamsvalid(
	_In_ struct tm const *const ps_time
) {
	/*
	* Do basic validation first, checking 
	* the values of each member of **ps_time**
	* for invalid ranges.
	*/
	if (   TFMT_INRANGE(ps_time->tm_year, -1900, 8099) == 0
	    || TFMT_INRANGE(ps_time->tm_mon, 0, 11)        == 0
	    || TFMT_INRANGE(ps_time->tm_mday, 1, 31)       == 0
	    || TFMT_INRANGE(ps_time->tm_wday, 0, 6)        == 0
	    || TFMT_INRANGE(ps_time->tm_yday, 0, 365)      == 0
	    || TFMT_INRANGE(ps_time->tm_hour, 0, 23)       == 0
	    || TFMT_INRANGE(ps_time->tm_min, 0, 59)        == 0
	    || TFMT_INRANGE(ps_time->tm_sec, 0, 60)        == 0
	) return 0;

	/*
	 * Check if mday and yday members make sense considering
	 * leap years.
	 */
	if (   TFMT_LASTMDAYFEB(TFMT_ORDYEAR(ps_time->tm_year)) < ps_time->tm_mday
		|| TFMT_LASTDOY(TFMT_ORDYEAR(ps_time->tm_year))     < ps_time->tm_yday
	) return 0;

	/* If everything is alright, return 1 (true). */
	return 1;
}

/*
 * Returns the required size based on the current
 * format specifier and supplementary data that is 
 * provided by **ps_time**.
 * 
 * Return value is always in bytes.
 */
static size_t tfmt_internal_getreqsize(
	/* current format specifier */
	_In_ char const fmt,
	/*
	 * Supplementary data; used by some format specifiers
	 * -- specifically those that result in varying sizes, e.g.
	 * weekdays or month names. The data provided is assumed
	 * to be valid.
	 */
	_In_ struct tm const *const ps_time
) {
	size_t tmp;

	switch (fmt) {
		/*
		 * The length of short weekday and month names typically
		 * used by "strftime()" implementations using an English locale 
		 * is always 3 bytes (excl. NUL-terminator).
		 */
		case 'a':
		case 'b':
		case 'j': return TFMT_SZLIT("xxx");
		case 'A': return TFMT_CURWD.m_wdlongsz;
		case 'B': return TFMT_CURMON.m_monlongsz;
		case 'c': return TFMT_SZLIT("MM/DD/YY - HH:MM:SS");
		case 'T':
		case 'X':
		case 'D': return TFMT_SZLIT("XX/XX/XX");
		case 'C':
		case 'd':
		case 'e':
		case 'g':
		case 'H':
		case 'I':
		case 'm':
		case 'M':
		case 'p':
		case 'S':
		case 'V':
		case 'y': return TFMT_SZLIT("Xx");
		case 'R': return TFMT_SZLIT("HH:MM");
		case 'G':
		case 'Y': return TFMT_SZLIT("YYYY");
		case 'F':
		case 'x': return TFMT_SZLIT("YYYY-MM-DD");
		case 'r': return TFMT_SZLIT("HH:MM:SS PP");
#if (defined _MSC_VER)
		case 'Z':
			/*
			 * Get the size in bytes of the timezone string.
			 * This function includes the NUL-terminator which has
			 * to be subtracted before the function returns the
			 * value.
			 */
			_get_tzname(&tmp, NULL, 0, TFMT_ISDST(ps_time->tm_isdst));

			/* Return final value. */
			return tmp - 1;
#endif
		case 'u':
		case 'w':
		case TFMT_ESCCHAR: return 1;
	}

	/*
	 * As unknown format specifiers are simply
	 * passed through without throwing an error, we
	 * have to reserve 2 bytes ('<TFMT_ESCCHAR>' + '<fmt-specifier>').
	 * 
	 * In case of an error regarding the specifier (i.e. NUL after a single
	 * '<TFMT_ESCCHAR>'), we return the largest number we can to ensure
	 * that the loop WILL break. This number is not (size_t)(-1), as this would
	 * cause an overflow inside "tfmt_strftime()" while trying to check whether we
	 * have enough space for the current specifier. The overflow would cause
	 * a wrap-around to 0, resulting in an infinite loop.
	 * In case of such an error, the '<TFMT_ESCCHAR>' sign will
	 * not be written to **pz_buf**.
	 */
	return fmt ? 2 : (size_t)(-2);
}

/*
 * Calculates the number of weeks of a given year.
 * 
 * Returns the number of weeks; either 52 (i.e. "short year",
 * or 53 (i.e. "long year").
 */
static uint32_t tfmt_internal_iso8601wc(
	int year /* ordinal year number */
) {
	/*
	 * A year has 53 weeks, i.e. it ends Thursday, or the last year ended
	 * Wednesday. If it does not, it has 52 weeks.
	 */
	return 52 + (
			  TFMT_ISO8601LWD(year)     == 4 /* wdn(4) == Thursday */
		   || TFMT_ISO8601LWD(year - 1) == 3 /* wdn(3) == Wednesday */
	);
}

/*
 * Calculates the actual ISO 8601 week number, accounting
 * for long and short years alike.
 * 
 * Returns ISO 8601 week number (1 ... 53).
 */
static uint32_t tfmt_internal_iso8601wn(
	_In_ struct tm const *const ps_time /* time data */
) {
	uint32_t wn = TFMT_ISO8601WN(
		TFMT_ORDDOY(ps_time->tm_yday),
		TFMT_ISO8601WD(ps_time->tm_wday)
	);

	int const year = TFMT_ORDYEAR(ps_time->tm_year);
	if (wn < 1) {
		/* 
		 * If the week number is 0, the week
		 * is the last week of the last year.
		 * 
		 * Calculate the week count of the last year.
		 */
		return tfmt_internal_iso8601wc(year - 1);
	} else if (wn == 53) {
		/*
		 * If the week number is 53, it can either be the
		 * last week of the current year, or the first week
		 * of the next year.
		 */
		return wn > tfmt_internal_iso8601wc(year) ? 1 : wn;
	}

	return wn;
}

/*
 * Calculates the week-based year according to ISO 8601.
 * 
 * Returns the ordinal year number of the week-based
 * year.
 */
static uint32_t tfmt_internal_iso8601wbyear(
	_In_ struct tm const *const ps_time /* time data */
) {
	/* Convert struct tm year number to ordinal year n*/
	uint32_t const year = TFMT_ORDYEAR(ps_time->tm_year);
	/* Get current ISO 8601 week number. */
	uint32_t const wn = TFMT_ISO8601WN(
		TFMT_ORDDOY(ps_time->tm_yday),
		TFMT_ISO8601WD(ps_time->tm_wday)
	);

	/*
	 * If a value of 0 is calculated, the week is the last
	 * week of the last year.
	 */
	if (wn < 1)
		return year - 1;
	/* Special rules come into play when a week number of 53 is obtained. */
	if (wn == 53) {
		/*
		 * If the current week number is 53, and we are in a short year
		 * (i.e. a year with 52 weeks), the current week is
		 * actually week 1 of the next year.
		 */
		if (wn > tfmt_internal_iso8601wc(year))
			return year + 1;
	}

	/*
	 * If the week number is neither < 0 nor 53, or **year**
	 * is a "long year", just return the **year**.
	 */
	return year;
}


#pragma region COMBINED-SPECIFIERS
/*
 * +++ NOTE REGARDING COMBINED SPECIFIERS +++
 * 
 * For performance reasons, we do not recursively call
 * "tfmt_strftime()"; we just write the data directly
 * because we know what a specifier evaluates to.
 * Of course, this comes at the cost of reduced flexibility,
 * as altering to what combined specifiers expand is now
 * a rather tedious task. This is only exacerbated by the
 * fact that the implementation used somewhat "magic" numbers
 * to offset the writing position.
 * However, this is a sacrifice I am willing to make as the
 * user can always choose to compose their strings using the
 * "naked" specifiers, forming whatever combination they so
 * desire.
 */

/*
 * Handles specifier 'Y'. This specifier expands to the full year number
 * with the century.
 * 
 * Returns nothing.
 */
static void tfmt_internal_handlespec_Y(
	_Inout_ char *restrict pz_buf,                  /* destination buffer */
	_In_    struct tm const *restrict const ps_time /* time data */
) {
	uint32_t const oyear = TFMT_ORDYEAR(ps_time->tm_year);

	/* Write the two century-related digits of the year number. */
	*(uint16_t *)pz_buf       = *(uint16_t *)glaz_num00thru99[oyear / 100];
	/* Write ordinal year number within century. */
	*(uint16_t *)(pz_buf + 2) = *(uint16_t *)glaz_num00thru99[oyear % 100];
}

/*
 * Handles combined specifier 'D', evaluating to a MM/DD/YY standard
 * date notation. It is equivalent to '%m/%d/%y'.
 * 
 * Returns nothing.
 */
static void tfmt_internal_handlespec_D(
	_Inout_ char *restrict pz_buf,                  /* destination buffer */
	_In_    struct tm const *restrict const ps_time /* time data */
) {
	/* MM/DD/YY
	 * ^-       */
	*(uint16_t *)pz_buf       = *(uint16_t *)glaz_num00thru99[ps_time->tm_mon + 1];
	/* MM/DD/YY
	 *    ^-    */
	*(uint16_t *)(pz_buf + 3) = *(uint16_t *)glaz_num00thru99[ps_time->tm_mday];
	/* MM/DD/YY
	 *       ^- */
	*(uint16_t *)(pz_buf + 6) = *(uint16_t *)glaz_num00thru99[TFMT_ORDYEAR(ps_time->tm_year) % 100];

	/*
	 * Insert delimiters.
	 * 
	 * %m/%d/%y
	 *   ^  ^
	 */
	*(pz_buf + 2) = *(pz_buf + 5) = '/';
}

/*
 * Handles combined character 'F', evaluating to a 'YYYY-MM-DD'
 * standard date notation (equivalent to '%Y-%m-%d').
 * 
 * Returns nothing.
 */
static void tfmt_internal_handlespec_F(
	_Inout_ char *restrict pz_buf,                  /* destination buffer */
	_In_    struct tm const *restrict const ps_time /* time data */
) {
	/* YYYY-MM-DD
	 * ^---       */
	tfmt_internal_handlespec_Y(pz_buf, ps_time);
	/* YYYY-MM-DD
	 *      ^-    */
	*(uint16_t *)(pz_buf + 5) = *(uint16_t *)glaz_num00thru99[ps_time->tm_mon + 1];
	/* YYYY-MM-DD
	 *         ^- */
	*(uint16_t *)(pz_buf + 8) = *(uint16_t *)glaz_num00thru99[ps_time->tm_mday];

	/*
	 * Insert delimiters.
	 *
	 * YYYY-MM-DD
	 *     ^  ^
	 */
	*(pz_buf + 4) = *(pz_buf + 7) = '-';
}

/*
 * Handles combined specifier 'T', evaluating to the 'HH:MM:SS'
 * ISO 8601 standard time format (24h). Equivalent to
 * '%H:%M:%S'.
 * 
 * Returns nothing.
 */
static void tfmt_internal_handlespec_T(
	_Inout_ char *restrict pz_buf,                  /* destination buffer */
	_In_    struct tm const *restrict const ps_time /* time data */
) {

	/* HH:MM:SS
	 * ^-       */
	*(uint16_t *)pz_buf       = *(uint16_t *)glaz_num00thru99[ps_time->tm_hour];
	/* HH:MM:SS
	 *    ^-    */
	*(uint16_t *)(pz_buf + 3) = *(uint16_t *)glaz_num00thru99[ps_time->tm_min];
	/* HH:MM:SS
	 *       ^- */
	*(uint16_t *)(pz_buf + 6) = *(uint16_t *)glaz_num00thru99[ps_time->tm_sec];

	/*
	 * Insert delimiters.
	 * 
	 * HH:MM:SS
	 *   ^  ^
	 */
	*(pz_buf + 2) = *(pz_buf + 5) = ':';
}

/*
 * Handles combined specifier 'c'. Being the most complex specifier that
 * "strftime()" supports, it is normally up to the locale to choose
 * an appropriate format.
 * This version uses the 'MM/DD/YY - HH:MM:SS' notation, evaluating
 * to, for instance, '08/06/22 - 00:50:02'. In this case, it
 * is equivalent to '%D - %T'.
 * 
 * Returns nothing.
 */
static void tfmt_internal_handlespec_c(
	_Inout_ char *restrict pz_buf,                  /* destination buffer */
	_In_    struct tm const *restrict const ps_time /* time data */
) {
	/*
	 * Insert delimiter. 
	 *
	 * In this function, we insert the delimiter first, as it will otherwise
	 * overwrite the first character of the ISO 8601 time notation with a NUL
	 * character (i.e. the fourth character of the string " - ").
	 * 
	 * MM/DD/YY - HH:MM:SS
	 *         ^--
	 */
	*(uint32_t *)(pz_buf + 8) = *(uint32_t *)" - ";

	/*
	 * 'MM/DD/YY' date notation.
	 *
	 * MM/DD/YY - HH:MM:SS
	 * ^-------
	 */
	tfmt_internal_handlespec_D(pz_buf, ps_time);
	/*
	 * ISO 8601 time notation.
	 *
	 * MM/DD/YY - HH:MM:SS
	 *            ^-------
	 */
	tfmt_internal_handlespec_T(pz_buf + 11, ps_time);
}

/*
 * Handles specifiers 'g' and 'G'. These, amongst others, represent
 * the ISO 8601 week-based calender.
 * 
 * Returns nothing.
 */
static void tfmt_internal_handlspec_gG(
	_Inout_ char *restrict pz_buf,                  /* destination buffer */
	_In_    char const fmt,                         /* fmt character ('g' or 'G') */
	_In_    struct tm const *restrict const ps_time /* time data */
) {
	uint32_t const year = tfmt_internal_iso8601wbyear(ps_time);

	if (fmt == 'g')
		*(uint16_t *)pz_buf = *(uint16_t *)glaz_num00thru99[(year - 1900) % 100];
	else
		tfmt_internal_handlespec_Y(pz_buf, ps_time);
}

/*
 * Handles specifiers 'r' and 'R'. 'r' designates 12h clock time (HH:MM:SS AA/PP) which is
 * equivalent to '%H:%M:%S %p'. 'R' expands to 12h HH:MM time, being exactly the same
 * as '%H:%M'.
 * 
 * Returns nothing.
 */
static void tfmt_internal_handlespec_rR(
	_Inout_ char *restrict pz_buf,                  /* destination buffer */
	_In_    char const fmt,                         /* fmt character ('g' or 'G') */
	_In_    struct tm const *restrict const ps_time /* time data */
) {
	if (fmt == 'r') {
		/* 
		 * 'HH:MM:SS' time notation.
		 * 
		 * HH:MM:SS XX
		 * ^-------
		 */
		tfmt_internal_handlespec_T(pz_buf, ps_time);

		/*
		 * Because the 'T' specifier expands to 'HH:MM:SS' time in
		 * 24h format, we need to replace only the 'HH' part with the
		 * 12h format of **ps_time->tm_hour**.
		 * 
		 * HH:MM:SS XX
		 * ^-
		 */
		*(uint16_t *)pz_buf       = *(uint16_t *)glaz_num00thru99[TFMT_24HTO12H(ps_time->tm_hour)];
		/*
		 * AM/PM designation.
		 * 
		 * HH:MM:SS XX
		 *          ^-
		 */
		*(uint16_t *)(pz_buf + 9) = *(uint16_t *)glaz_designampm[TFMT_ISPM(ps_time->tm_hour)];
		
		/* Insert delimiter.
		 *
		 * HH:MM:SS XX
		 *         ^
		 */
		*(pz_buf + 8) = ' ';
		return;
	}

	/* 'HH:MM' time in 24h format. */

	/*
	 * HH:MM
	 * ^-    */
	*(uint16_t *)pz_buf       = *(uint16_t *)glaz_num00thru99[ps_time->tm_hour];
	/*
	 * HH:MM
	 *    ^- */
	*(uint16_t *)(pz_buf + 3) = *(uint16_t *)glaz_num00thru99[ps_time->tm_min];

	/* Insert delimiter.
	 *
	 * HH:MM
	 *   ^
	 */
	*(pz_buf + 2) = ':';
}

/*
 * Handles specifier 'Z', evaluating to the (abbreviated) time zone name.
 * Because some systems use non-standard "struct tm" members, and some systems
 * use different means of obtaining the timezone, this function is to be
 * implemented per platform.
 * 
 * Returns nothing.
 */
static void tfmt_internal_handlespec_Z(
	_Inout_ char *restrict pz_buf,                  /* destination buffer */
	_In_    struct tm const *restrict const ps_time /* time data */
) {
#if (defined _MSC_VER)
	size_t tmp;

	_get_tzname(
		&tmp,
		pz_buf,
		/*
		 * It does not matter what we pass here as we've
		 * already verified that the function has enough
		 * space to write the required number of bytes to
		 * **pz_buf**.
		 */
		(size_t)(-1),
		TFMT_ISDST(ps_time->tm_isdst)
	);
#endif
}

/*
 * Handles 'e' specifier.
 * 'e' expands to to the day of the month, but space-padded as opposed to
 * 'd', which pads single-digit values by a zero.
 * 
 * Returns nothing.
 */
static void tfmt_internal_handlespec_e(
	_Inout_ char *restrict pz_buf,                  /* destination buffer */
	_In_    struct tm const *restrict const ps_time /* time data */
) {
	/* First, "blit" a zero-padded number. */
	*(uint16_t *)pz_buf = *(uint16_t *)glaz_num00thru99[ps_time->tm_mday];

	/* Then, replace the zero with a space. */
	if (ps_time->tm_mday < 10)
		*pz_buf = ' ';
}
#pragma endregion


size_t _Success_(return > 0) tfmt_internal_strftime_IMPL(
	_Inout_opt_z_ char *restrict pz_buf,
	_In_z_        char const *restrict pz_fmt,
	_In_          size_t size,
	_In_          struct tm const *restrict const ps_time
) {
	/* Starting size of buffer; will be used to calculate total bytes written. */
	size_t const startsz = size;

	/*
	 * Temporary variable, representing the required
	 * size in bytes for the current format specifier.
	 */
	size_t tmp;

	/*
	 * If **pz_buf** is NULL, the function will enter
	 * a "only-calculate-required-size mode" which will do
	 * precisely what its name suggests: Do not write to **pz_buf**
	 * but return the required size (incl. NUL-terminator) that is 
	 * required to store the formatted string, as specified by
	 * **pz_fmt**, inside **pz_buf**.
	 */
	if (pz_buf == NULL) {
		/*
		 * Start with 1; that removes the need to add 1 for the
		 * compulsory NUL-terminator later.
		 */
		tmp = 1;

		while (*pz_fmt)
			if (*pz_fmt++ == TFMT_ESCCHAR)
				tmp += tfmt_internal_getreqsize(*pz_fmt, ps_time);
			else 
				++tmp;

		return tmp;
	}

	/* Run regular time string formatting. */
	while (*pz_fmt) {
		if (*pz_fmt == TFMT_ESCCHAR) {
			/*
			 * Get the size required for the current specifier, in bytes.
			 * If there is not enough space to hold the entirety of the
			 * evaluated format specifier AND a terminating NUL, the function
			 * will finalize.
			 */
			if ((tmp = tfmt_internal_getreqsize(*++pz_fmt, ps_time)) + 1 > size)
				goto lbl_FINALIZE;

			/*
			 * Main dispatcher to handle supported format specifiers.
			 * 
			 * Regarding 'a' and 'b':
			 *  It is totally fine to copy-over four bytes as the source buffer
			 *  for the short weekday and month names is always (at least) four
			 *  bytes long. Replacing the otherwise needed "strcpy()" or "memcpy()"
			 *  call by a simple four-bytes-at-once copy considerably speeds up the
			 *  formatting in case these specifiers are needed.
			 */
			switch (*pz_fmt++) {
				/*
				 * Treat unknown specifiers like regular characters, copying both the
				 * escape character and the character that follows it immediately.
				 */
				default:
					*pz_buf++ = TFMT_ESCCHAR;

					/*
					 * Decrement **pz_fmt** to allow jumping to 'lbl_REGCHAR', which
					 * assumes that **pz_fmt**'s current first character is the same as
					 * the one that was checked against NUL in the "while" loop head.
					 */
					--pz_fmt;
					--size;
					goto lbl_REGCHAR;

				case 'a': *(uint32_t *)pz_buf = *(uint32_t *)TFMT_CURWD.mpz_wdshort;                                 break;
				case 'b': *(uint32_t *)pz_buf = *(uint32_t *)TFMT_CURMON.mpz_monshort;                               break;
				case 'A': tfmt_internal_copy(pz_buf, TFMT_CURWD.mpz_wdlong, TFMT_CURWD.m_wdlongsz);                  break;
				case 'B': tfmt_internal_copy(pz_buf, TFMT_CURMON.mpz_monlong, TFMT_CURMON.m_monlongsz);              break;
				case 'c': tfmt_internal_handlespec_c(pz_buf, ps_time);                                               break;
				case 'C': *(uint16_t *)pz_buf = *(uint16_t *)glaz_num00thru99[TFMT_ORDYEAR(ps_time->tm_year) / 100]; break;
				case 'd': *(uint16_t *)pz_buf = *(uint16_t *)glaz_num00thru99[ps_time->tm_mday];                     break;
				case 'D': tfmt_internal_handlespec_D(pz_buf, ps_time);                                               break;
				case 'e': tfmt_internal_handlespec_e(pz_buf, ps_time);                                               break;
				case 'F':																						     
				case 'x': tfmt_internal_handlespec_F(pz_buf, ps_time);                                               break;
				case 'g':																						     
				case 'G': tfmt_internal_handlspec_gG(pz_buf, *(pz_fmt - 1), ps_time);                                break;
				case 'H': *(uint16_t *)pz_buf = *(uint16_t *)glaz_num00thru99[ps_time->tm_hour];                     break;
				case 'I': *(uint16_t *)pz_buf = *(uint16_t *)glaz_num00thru99[TFMT_24HTO12H(ps_time->tm_hour)];      break;
				case 'j': tfmt_internal_toint(pz_buf, ps_time->tm_yday + 1, 3);                                      break;
				case 'm': *(uint16_t *)pz_buf = *(uint16_t *)glaz_num00thru99[ps_time->tm_mon + 1];                  break;
				case 'M': *(uint16_t *)pz_buf = *(uint16_t *)glaz_num00thru99[ps_time->tm_min];                      break;
				case 'p': *(uint16_t *)pz_buf = *(uint16_t *)glaz_designampm[TFMT_ISPM(ps_time->tm_hour)];           break;
				case 'r':																						     
				case 'R': tfmt_internal_handlespec_rR(pz_buf, *(pz_fmt - 1), ps_time);                               break;
				case 'S': *(uint16_t *)pz_buf = *(uint16_t *)glaz_num00thru99[ps_time->tm_sec];                      break;
				case 'T':																						     
				case 'X': tfmt_internal_handlespec_T(pz_buf, ps_time);                                               break;
				case 'u': *pz_buf = *(glaz_num00thru99[TFMT_ISO8601WD(ps_time->tm_wday)] + 1);                       break;
				case 'V': *(uint16_t *)pz_buf = *(uint16_t *)glaz_num00thru99[tfmt_internal_iso8601wn(ps_time)];     break;
				case 'w': *pz_buf = *(glaz_num00thru99[ps_time->tm_wday] + 1);                                       break;
				case 'y': *(uint16_t *)pz_buf = *(uint16_t *)glaz_num00thru99[TFMT_ORDYEAR(ps_time->tm_year) % 100]; break;
				case 'Y': tfmt_internal_handlespec_Y(pz_buf, ps_time);                                               break;
#if (defined _MSC_VER)
				case 'Z': tfmt_internal_handlespec_Z(pz_buf, ps_time);                                               break;
#endif
																												 
				case TFMT_ESCCHAR: *pz_buf = TFMT_ESCCHAR;                                                           break;
			}
		} else {
		lbl_REGCHAR:
			/*
			 * Optimization for ASCII-characters; keeping them out
			 * of the switch statement yields some performance improvements.
			 */
			if (*(uint8_t *)pz_fmt < 0x80 && size > 1) {
				*pz_buf++ = *pz_fmt++;

				--size;
				continue;
			}
			
			/*
			 * Get the size of the current UTF-8 byte sequence.
			 * If there is not enough space to hold both the entire
			 * sequence AND the terminating NUL, finalize.
			*/
			if ((tmp = gla_utf8charsz[*(uint8_t *)pz_fmt]) + 1 > size)
				goto lbl_FINALIZE;

			/* Copy the entire UTF-8 sequence at once. */
			switch (tmp) {
				case 2: *(uint16_t *)pz_buf = *(uint16_t *)pz_fmt; pz_fmt += 2; break;
				case 3:
					    /*
					     * When the function encounters a three-byte sequence, it is
					     * also totally fine to copy-over four bytes in one go, for
					     * even if the 3-byte sequence is the last UTF-8 character
					     * in the string, there is still a NUL behind it, meaning
					     * we can safely copy four bytes without reading memory we
					     * are not supposed to read.
					     * Of course, this is only safe if **pz_fmt** is properly
					     * NUL-terminated and contains no invalid (i.e. too short)
					     * UTF-8 sequences.
					     */
					    *(uint32_t *)pz_buf = *(uint32_t *)pz_fmt; pz_fmt += 3; break;
				case 4: *(uint32_t *)pz_buf = *(uint32_t *)pz_fmt; pz_fmt += 4; break;
			}
		}

		/*
		 * Advance writing position and update
		 * free space in buffer.
		 */
		pz_buf += tmp;
		size   -= tmp;
	}

	/*
	 * At last, append a terminating NUL
	 * and return the number of bytes written
	 * to **pz_buf** (incl. NUL), in bytes.
	 */
lbl_FINALIZE:
	*pz_buf = '\0';
	return startsz - size + 1;
}


/*
 * See inline documentation inside "tfmt.h" for detailed information
 * regarding this function's parameters and behavior.
 */
_Success_(return > 0) size_t tfmt_strftime(
	_Inout_opt_z_ char *restrict pz_buf,
	_In_z_        char const *restrict pz_fmt,
	_In_          size_t size,
	_In_opt_      struct tm const *restrict ps_time
) {
	/*
	 * Carry-out parameter validation.
	 * If anything's wrong, return 0 and leave **pz_buf** unchanged.
	 */
	if (pz_fmt == NULL || *pz_fmt == 0 || (ps_time != NULL && tfmt_internal_isparamsvalid(ps_time) == 0))
		return 0;

	/*
	 * If **ps_time** is NULL, we get the
	 * current time and use this to format
	 * the time string.
	 */
	struct tm s_time;
	if (ps_time == NULL) {
		time_t const currtime = time(NULL);
		/*
		 * Use "localtime_s()" on platforms that have
		 * it in their standard library.
		 */
#if (defined _MSC_VER || __STDC_WANT_LIB_EXT1__ == 1)
		if (localtime_s(&s_time, &currtime) != 0)
			return 0;
#else
		/*
		 * Work around "localtime_s()" not being available
		 * by calling "localtime()" and copying the result
		 * to a temporary buffer whose address is then assigned
		 * to **ps_time**. This avoids data races in case
		 * the internal buffer shared by "localtime()", "gmtime()",
		 * "ctime()", etc. is modified by a different thread while
		 * "tfmt_strftime()" does its work.
		 */
		ps_time = localtime(&currtime);
		if (ps_time == NULL)
			return 0;

		s_time  = *ps_time;
#endif

		ps_time = &s_time;
	}

	/* If no error could be found, proceed with regular formatting. */
	return tfmt_internal_strftime_IMPL(pz_buf, pz_fmt, size, ps_time);
}	


