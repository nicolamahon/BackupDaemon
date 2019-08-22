#include <unistd.h>	// execlp
#include <sys/wait.h>	// wait
#include <unistd.h> 	// fork
#include <syslog.h>	// syslog	
#include <stdlib.h>	// exit_failure
#include <mqueue.h>	// msgQ

#include <stdio.h>

// TODO
// error checks / messages

void audit()
{
	// open syslog
	openlog("WEBSITE_ADMIN", LOG_PID/LOG_CONS, LOG_USER);

	// strings for filepath
	char target[50] = "/var/www/html"; 	

	// child pid
	pid_t pid;

	// attempt fork
	if((pid = fork()) == -1)
	{
		syslog(LOG_ERR, "AUDIT: Error creating child process");
	}
	// child process
	else if(pid == 0)
	{
		syslog(LOG_INFO, "AUDIT: Child process created to initialise audit");
		
		// start audit on the website folders
		if(execlp("auditctl", "auditctl", "-w", target, "-p", "rwxa", NULL) < 0)
		{
			// any code here will not run if the execlp call succeeds
			syslog(LOG_ERR, "AUDIT: Error setting audit on file: /var/www/html");
			exit(EXIT_FAILURE);
		}
		else 
		{
			syslog(LOG_INFO, "AUDIT: Audit set successfully on file: /var/www/html");
		}
	}
	// parent process
	else 
	{
		// parent waits for child process to finish
		int status=0;
		wait(&status);
	}	

	// close the log
	closelog();	
}
