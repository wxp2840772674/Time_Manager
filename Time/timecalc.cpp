#include "timecalc.h"
#include <time.h>
struct tm timeadddays(struct tm time1,int days)
{
	struct tm result;
	time_t add1, sum;
	add1 = mktime(&time1);
	sum = add1 + (double)days*86400;
	localtime_s(&result,&sum);
	return result;
}