#ifndef daemon_H_
#define daemon_H_

void transfer();
void backup();
void delay(int num_seconds);
void audit();
void logging();
void timestamp(char *timestamp);
int permission(char *key);
void stop();

#endif // daemon_H_
