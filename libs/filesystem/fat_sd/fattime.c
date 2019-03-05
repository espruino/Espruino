/* Martin Thomas 4/2009
 * Gordon Williams 2019 */

#include "integer.h"
#include "fattime.h"
#include "jswrap_date.h"

DWORD get_fattime (void)
{
  JsVarFloat time = jswrap_date_now();
  TimeInDay tid = getTimeFromMilliSeconds(time, false);
  CalendarDate date = getCalendarDate(tid.daysSinceEpoch);

	DWORD res;
	res =  (((DWORD)date.year - 1980) << 25)
			| ((DWORD)(date.month+1) << 21)
			| ((DWORD)date.day << 16)
			| (WORD)(tid.hour << 11)
			| (WORD)(tid.min << 5)
			| (WORD)(tid.sec >> 1);
	return res;
}

