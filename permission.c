#include <stdlib.h>

int permission(char *key)
{
	// value for converted lock 
	int lock;

	// convert key to binary lock value
	lock = strtol(key, 0, 8);

	// return new lock value
	return lock;
}

