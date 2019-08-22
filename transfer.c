#include <unistd.h>	// execlp
#include <sys/wait.h>	// wait
#include <sys/stat.h>	// chmod
#include <syslog.h>	// syslog
#include <stdlib.h>	// exit_status
#include <mqueue.h>	// msgQ
#include <string.h>	// strcat
#include "daemon.h"

//#include <stdio.h>	// printf

void transfer()
{
	// open log and msgQ
	openlog("WEBSITE_ADMIN", LOG_PID/LOG_CONS, LOG_USER);	
	mqd_t mq = mq_open("/website_admin", O_WRONLY);

	// create formatted timestamp
	char _timestamp[30];
	timestamp(_timestamp);
	
	// forced update message
	char msg[100] = "Forced Transfer: \t";
	strcat(msg, _timestamp);
	strcat(msg, "\n");

	// strings for filepaths 
	char src[50] = "/var/www/html/intranet/."; 	// src - intranet file
	char dest[50] = "/var/www/html/live/."; 	// dest - live file

	// generate file permission binary values
	int lock = permission("0000");			// locked for all users
	int unlock_copy = permission("0755");		// rwx-r_x-r_x
	int unlock_original = permission("0777");	// rwx-rwx-rwx

	// try to lock the intranet folder
        if (chmod (src,lock) < 0 || chmod(dest, lock) < 0)
	{
		syslog(LOG_ERR, "TRANSFER: Error LOCKING files to transfer");
		exit(EXIT_FAILURE);
	}
	else 
	{
		syslog(LOG_INFO, "TRANSFER: Files LOCKED successfully");

		// fork a child to perform the transfer
		pid_t pid;
		
		if((pid = fork()) == -1)
		{
			syslog(LOG_ERR, "TRANSFER: Error creating child process");
			exit(EXIT_FAILURE);
		}
		// child process
		else if(pid == 0)
		{
			syslog(LOG_INFO, "TRANSFER: Child process created successfully");
			mq_send(mq, msg, 1024, 0);
		
			// copy the contents of the src file (intranet) to the dest file (live)
			execlp("cp", "cp", "-ru", src, dest, NULL);
		
			// any code here will not run if the execlp call succeeds
			syslog(LOG_ERR, "TRANSFER: Error transferring files: intranet -> live");
			exit(EXIT_FAILURE);
			
		}
		// parent process
		else 
		{
			// parent waits for child process to finish
			int status=0;
			wait(&status);
			syslog(LOG_INFO, "TRANSFER: Successful transfer of files: intranet -> live");

			// unlock files
			if((chmod(src, unlock_original)) > 0 || (chmod(dest, unlock_copy)) > 0)
			{
				syslog(LOG_ERR, "TRANSFER: Error UNLOCKING file permissions");
				exit(EXIT_FAILURE);
			}
			else
			{
				syslog(LOG_INFO, "TRANSFER: File permissions UNLOCKED successfuly");
			}
		}
	}

	// close the log
	closelog();

}
