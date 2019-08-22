#include <time.h>

void timestamp(char *time_stamp)
{
	// variable for storing time	
	time_t now;

	// get current time
	time(&now);
	
	// convert now (time_t) to struct tm (localtime) and format the time
	strftime(time_stamp, 30, "%Y-%m-%d_%H:%M:%S", localtime(&now));

}
