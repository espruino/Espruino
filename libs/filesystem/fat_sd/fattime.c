/* Martin Thomas 4/2009 */

#include "integer.h"
#include "fattime.h"
//#include "rtc.h"

DWORD get_fattime (void)
{
	DWORD res;
	/*RTC_t rtc;

	rtc_gettime( &rtc );*/
	
	res =  (((DWORD)2012/*rtc.year*/ - 1980) << 25)
			| ((DWORD)1/*rtc.month*/ << 21)
			| ((DWORD)1/*rtc.mday*/ << 16)
			| (WORD)(0/*rtc.hour*/ << 11)
			| (WORD)(0/*rtc.min*/ << 5)
			| (WORD)(0/*rtc.sec*/ >> 1);

	return res;
}

