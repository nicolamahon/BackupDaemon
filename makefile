CC = gcc
objects = main.o transfer.o backup.o delay.o audit.o logging.o timestamp.o permission.o stop.o
headers = daemon.h	

prog			:	$(objects) 	
				$(CC) -o website_admin $(objects) -lrt

main.o			:	main.c $(headers)
				$(CC) -c main.c

transfer.o		:	transfer.c
				$(CC) -c transfer.c

backup.o		: 	backup.c
				$(CC) -c backup.c 

delay.o			:	delay.c
				$(CC) -c delay.c

audit.o			:	audit.c
				$(CC) -c audit.c -lrt

logging.o		:	logging.c
				$(CC) -c logging.c

timestamp.o		:	timestamp.c
				$(CC) -c timestamp.c

permission.o		:	permission.c
				$(CC) -c permission.c

stop.o			:	stop.c
				$(CC) -c stop.c

clean			:	
				rm website_admin $(objects)
