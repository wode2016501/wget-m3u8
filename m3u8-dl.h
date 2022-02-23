#include<stdio.h>
#include<stdlib.h>
struct ts_w{
	char url[1024];
	char file[1024];
};
struct m3u8_fb{
	char url[1024];
	char file[1024];
	FILE *fp;
};
//char m3u8file[1024];
//char buf[1024];
void thread_create(struct m3u8_fb *m3u8);
void thread_wait(struct m3u8_fb *m3u8);
int pth=10;
pthread_mutex_t mut;
pthread_t  *thread=0;

