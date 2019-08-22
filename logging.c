#include <stdio.h>	// popen **
#include <time.h>	// time
#include <string.h>	// strcat
#include <syslog.h>	// syslog
#include <mqueue.h>	// msgQ
#include "daemon.h"

void logging()
{
	// open syslog and msgQ
	openlog("WEBSITE_ADMIN", LOG_PID/LOG_CONS, LOG_USER);	
	mqd_t mq = mq_open("/website_admin", O_WRONLY);

	// output fileapth
	char logfile[50] = "/var/www/logs/log_"; 	// a new log file 
	
	// auditsearch command string
	char *search = "ausearch -ts today -f /var/www/html \
			-te now | aureport -f -i";	

	// create formatted timestamp
	char _timestamp[30];
	timestamp(_timestamp);

	// concatenate the timestamp to the dest filename
	strcat(logfile, _timestamp);
	strcat(logfile, ".txt");

	// forced update message
	char msg[100] = "Forced Log: \t\t";
	strcat(msg, _timestamp);
	strcat(msg, "\n");

	// file descriptors
	FILE *searchFP;
	FILE *logFP;

	// for reading return from popen
	char buffer[1024];

	// popen() -> pipe, fork and run command to read
	searchFP = popen(search, "r");

	// open new file for logging
	logFP = fopen(logfile, "w");

	// if file descriptors fail to create
	if(searchFP == NULL || logFP == NULL )
	{
		syslog(LOG_ERR, "LOGGING: Failed to create pipe and/or new log file");
	}
	else
	{		
		syslog(LOG_INFO, "LOGGING: Pipe and new log file created successfully");
		mq_send(mq, msg, 1024, 0);
		
		// read result of the returned command
		while(fgets(buffer, 1024, searchFP))
		{
			// write the contents of the buffer to the file
			fputs(buffer, logFP);
		}

		// close the file descriptors
		fclose(logFP);
		pclose(searchFP);	
	
	}
	
	// close the log
	closelog();

}
