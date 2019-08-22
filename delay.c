#include <time.h>	// clock

void delay(int num_seconds)
{
	// convert time to milliseconds
	int ms = 1000 * num_seconds;

	// get current clock
	clock_t start = clock();

	// loop to desired time i.e. second
	while(clock() < start + ms);
}
