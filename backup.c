#include <string.h>	// strcat
#include <unistd.h>	// execlp
#include <sys/wait.h>	// wait
#include <sys/stat.h>	// chmod
#include <stdlib.h>	// exit_status
#include <syslog.h>	// syslog
#include <mqueue.h>	// msgQueue
#include "daemon.h"

//#include <stdio.h>	// printf


void backup()
{
	// open syslog and msgQ
	openlog("WEBSITE_ADMIN", LOG_PID/LOG_CONS, LOG_USER);
	mqd_t mq = mq_open("/website_admin", O_WRONLY);

	// create formatted timestamp
	char _timestamp[30];
	timestamp(_timestamp);

	// strings for filepaths 
	char src[50] = "/var/www/html/live";		// src - live file
	char dest[50] = "/var/www/backups/backup_"; 	// dest - a new backup file
	
	// concatenate the timestamp to the dest filename
	strcat(dest, _timestamp);

	// forced update message
	char msg[100] = "Forced Backup: \t\t";
	strcat(msg, _timestamp);
	strcat(msg, "\n");

	// generate file permission binary values
        int lock = permission("0000");		// locked for all users 
	int unlock = permission("0755");	// rwx-r_x-r_x

	// try to lock the live folder
        if (chmod (src,lock) < 0)
	{
		syslog(LOG_ERR, "BACKUP: Error LOCKING files for backup (chmod)");
		exit(EXIT_FAILURE);
	}
	else 
	{
		syslog(LOG_INFO, "BACKUP: Files LOCKED successfully for backup");

		// fork a child to perform the backup
		pid_t pid;
		if((pid = fork()) == -1)
		{
			syslog(LOG_ERR, "BACKUP: Error creating child process");
			exit(EXIT_FAILURE);
		}
		// child process
		else if(pid == 0)
		{
			syslog(LOG_INFO, "BACKUP: Child process created successfully");
			mq_send(mq, msg, 1024, 0);
		
			// copy the src file (live) as a new file in the backups folder
			execlp("cp", "cp", "-r", src, dest, NULL);
		
			// any code here will not run if the execlp call succeeds
			syslog(LOG_ERR, "BACKUP: Error backing up files: html/live -> backups");
			exit(EXIT_FAILURE);
			
		}
		// parent process
		else 
		{
			// parent waits for child process to finish
			int status=0;
			wait(&status);

			syslog(LOG_INFO, "BACKUP: Successful backup of files: html/live -> backups");

			// unlock files
			if((chmod(src, unlock)) > 0 || (chmod(dest, unlock)) > 0)
			{
				syslog(LOG_ERR, "BACKUP: Error UNLOCKING file permissions");
				exit(EXIT_FAILURE);
			}
			else
			{
				syslog(LOG_INFO, "BACKUP: File permissions UNLOCKED successfuly");
			}
			
		}
	}
	
	// close log
	closelog();
		
}
