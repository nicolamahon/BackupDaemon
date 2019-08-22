#include <stdio.h>		// printf
#include <string.h>		// strcmp
#include <time.h>		// time
#include <syslog.h>		// syslog
#include <mqueue.h>		// message queue
#include <stdlib.h>		// exit
#include <unistd.h>		// fork
#include <sys/types.h>		// umask
#include <sys/stat.h>
#include "daemon.h"



int main(int argc, char* argv[])
{
	// start the daemon
	if(strcmp(argv[1], "start") == 0)
	{
		pid_t pid = fork();

		if(pid > 0)
		{
			sleep(10);
			exit(EXIT_SUCCESS);
		}
		else if (pid == 0) 
		{
     			// Elevate the orphan process to session leader, to loose controlling TTY
       			// This command runs the process in a new session
       			if (setsid() < 0) { exit(EXIT_FAILURE); }

		       	// fork here again, just to guarantee that the process is not a session leader
       			int pid = fork();
       			if (pid > 0) 
			{
				exit(EXIT_SUCCESS);
			} 
			else 
			{
				// call umask() to set the file mode creation mask to 0
				// This will allow the daemon to read and write files 
				// with the permissions/access required 
				umask(0);

				// Change the current working dir to root.
				// This will eliminate any issues of running on a mounted drive, 
				// that potentially could be removed etc..
				if (chdir("/") < 0 ) { exit(EXIT_FAILURE); }

				// Step 5: Close all open file descriptors
				/* Close all open file descriptors */
				int x;
				for (x = sysconf(_SC_OPEN_MAX); x>=0; x--)
				{
					close (x);
				} 


				// open syslog
				openlog("WEBSITE_ADMIN", LOG_PID/LOG_CONS, LOG_USER);

				// start the audit on the file
				audit();

				// get current time; same as: now = time(NULL)  
				time_t now;
				time(&now); 

				// structure for storing exact time of auto backup/transfer
				struct tm midnight;

				// assign time to the auto backup/transfer
				midnight = *localtime(&now);
				midnight.tm_hour = 00; 
				midnight.tm_min = 35; 
				midnight.tm_sec = 0;

				// store time difference between NOW and MIDNIGHT
				double seconds;

				// flag to check backup/transfer does not reoccur same night
				int complete = 0;

				// counting down to midnight
				while(1)
				{
					// custom wait to handle milliseconds 
					delay(1);

					// get time now
					time(&now);

					// check difference between now and midnight
					seconds = difftime(now, mktime(&midnight));

					// it must be midnight
					if(seconds == 0)
					{
						// check that updates have not been done yet
						if(complete == 0)
						{
							// perform backup, transfer and logging
							backup();
							transfer();
							logging();

							// set flag to ensure no additional backup tonight
							complete = 1;
						}			
					}
					else
					{
						// reset flag to ensure backup at next midnight
						complete = 0;
					}
				} // while(1)			
			} // else orphan
		}
	}
	else	// other available commands
	{	
		// create msg queue
		mqd_t mq;
		struct mq_attr queue_attributes;

		// set queue attrs
		queue_attributes.mq_flags = 0;
		queue_attributes.mq_maxmsg = 10;
		queue_attributes.mq_msgsize = 1024;
		queue_attributes.mq_curmsgs = 0;

		// open msg queue
		mq = mq_open("/website_admin", O_CREAT | O_RDONLY, 0644, &queue_attributes);

		// create buffers to read from msgQ
		char q_buff[1024 + 1];
		ssize_t read_msg;


		/* BACKUP, TRANSFER, LOG, STOP - forced events */

		if (strcmp(argv[1], "backup") == 0)
		{
			backup();
		}
		else if (strcmp(argv[1], "transfer") == 0)
		{
			transfer();
		}
		else if (strcmp(argv[1], "log") == 0)
		{
			logging();

		}
		else if (strcmp(argv[1], "stop") == 0)
		{
			stop();
		}
		else
		{
			syslog(LOG_WARNING, "Attempted website update with invalid command\n");
			exit(1);
		}
		
		// read from msg queue
		read_msg = mq_receive(mq, q_buff, 1024, NULL);
		q_buff[read_msg] = '\0';

		// open file for msgQ messages
		FILE *msgFP;
		msgFP = fopen("/var/www/updates.txt", "a");
		
		// write the contents of the msgQ to the updates file
		fputs(q_buff, msgFP);
		fclose(msgFP);
	
	}

	// close WEBSITE_ADMIN log
	closelog();
	return 0;
}
