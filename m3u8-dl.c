#include<pthread.h>
#include<stdio.h>
#include<stdlib.h>
#include "m3u8-dl.h"

int  Download( char *url,char *outfile){
	char *arg[1024];
	char *cmd="wget -c --no-check-certificate -T 5 -t 0 -o /dev/null -O '%s' '%s'";
	//return execlp(cmd,cmd,"-c","--no-check-certificate","-T","5","-t","0",/*"-o","/dev/null",*/"-O",outfile,url,NULL); 
	sprintf(arg,cmd,outfile,url);
	fprintf(stderr,"%s\n",arg);
	return system(arg);

}
int download_ts(struct ts_w *ts){
	char *kz=0;
	kz=strchr(ts->url,'\n');
	if(kz)
		*kz=0;
	while(Download(ts->url,ts->file)!=0){
	 sleep(1);
	}
	return 0;
}
struct ts_w *index_m3u8(struct m3u8_fb *m3u8,char *buf,struct ts_w *ts){
	char *url=m3u8->url;
	int w=0;
	char *ho=0;
	char *kz=0;
	kz=strchr(buf,'\n');
	if(kz)
		*kz=0;
	kz=0;
	if(*buf==0)
		return 0;
	char tsurl[1024],filets[1024];
	strncpy(tsurl,buf,10);
	if(*buf!='#'){
		ho=strstr(tsurl,"http://");
		if(ho!=0){
			strcpy(tsurl,buf);
			w=1;
		}
		ho=strstr(tsurl,"https://");
		if(ho!=0){
			strcpy(tsurl,buf);
			w=1;
		}
		if(*buf=='/'){
			strcpy(tsurl,url);
			ho=strchr(tsurl+9,'/');
			if(ho==0){
				fprintf(stderr,"致命错误\n");
				exit(-1);
			}
			strcpy(ho,buf);
			w=1;
		}
		if(w!=1){
			strcpy(tsurl,url);
			ho=strrchr(tsurl,'/');
			if(ho==0){
				fprintf(stderr,"致命错误\n");
				exit(-1);
			}
			strcpy(ho+1,buf);
			w=1;
		}

		if(w==1){
			/*
			ts=malloc(sizeof(struct ts_w));
			memset(ts,0,sizeof(struct ts_w));
			*/
			ho=strrchr(buf,'/');
			if(ho==0){
			ho=buf;
			}
			else
			ho+=1;
			kz=strrchr(ho,'?');
			if(kz==0)
				kz=strrchr(ho,'&');
			if( kz) 
				strncpy(ts->file,ho,kz-ho);
			else
				strcpy(ts->file,ho);
			strcpy(ts->url,tsurl);
			return ts;
		}
		return 0;
	}
	else
	{
		if(strstr(buf,"KEY")){
			printf("KEY=%s",buf);
		}
		return 0;
	}
}
struct ts_w  *get_ts(struct m3u8_fb *m3u8,char *buf,struct ts_w *ts){
	struct ts_w *ts2=0;
	if(m3u8->fp==0)
		exit (-1);;
	while(feof(m3u8->fp)==0){
		int cao=strlen(buf);                                                          
		for(int i=0; i<cao; i++)                                                           
			if(buf[i]=='\r'||buf[i]=='\n')
                                buf[i]=0;
		fgets(buf, 1024, m3u8->fp);
		ts2=index_m3u8(m3u8,buf,ts);
		if(ts2)
			 return ts;
	}
	return 0;
}
FILE *open_m3u8(struct m3u8_fb *m3u8){
	FILE *fp=0;
	if(m3u8->url==0)
		exit(-1);
	if(strlen(m3u8->url)<1)
		exit(-1);
	fp=fopen(m3u8->file,"rb");
	if(fp==NULL){
		fprintf(stderr,"打开文件: '%s' 失败",m3u8->file);
		exit(-1);
	}
	m3u8->fp=fp;
	return fp;
}
void err(char *str){
	fprintf(stderr,"%s\n",str);
}
struct m3u8_fb *download_m3u8(char *url){
	
	struct m3u8_fb *m3u8=0;
	char m3u8file[1024];
	int ext=0;
	char *kz=0;
	char *file=0;
	kz=strstr(url,".m3u8");
	if(kz==0){
		sprintf(m3u8file,"index-%d.m3u8");
	}
	else
	{
		file=strrchr(kz,'/');
		if(file==0)
			sprintf(m3u8file,"index-%d.m3u8");
		else
		{
			file+=1;
			strncpy(m3u8file,file,kz-file+5);
			kz=strstr(m3u8file,".m3u8");
			if(kz!=0){
				sprintf(kz,"-%d.m3u8");
			}
		}
	}
	
	ext=Download(url,m3u8file);
	if(ext!=0){
		fprintf(stderr,"下载%s错误\n",m3u8file);
		exit(ext);
	}
	m3u8=malloc(sizeof(struct m3u8_fb));
	memset((void *)m3u8,0,sizeof(struct m3u8_fb));
	strcpy(m3u8->url,url);
	strcpy(m3u8->file,m3u8file);
	open_m3u8(m3u8);
	if(m3u8->fp==0)
		return 0;
	thread_create(m3u8);
	thread_wait(m3u8);
	free(m3u8);
	return m3u8;
}
 void *threadnew(struct m3u8_fb *m3u8){
	 struct ts_w *ts=0;
	 struct ts_w *ts2=0;
	 char buf[1024];
	 ts=malloc(sizeof(struct ts_w));
	 while(1){
		 memset(ts,0,sizeof(struct ts_w));
		 ts2=get_ts(m3u8,buf,ts);
		 if(ts2)
			 download_ts(ts);
		 else
			 break;
	 }
	 fprintf(stderr,"警告⚠: 线程已经结束:\n");
	 free(ts);
	 pthread_exit(NULL);
 }
 void thread_wait(struct m3u8_fb *m3u8){
	 for(int i=1; i<=pth;i++){
		 if(thread[i] !=0) {
			 pthread_join(thread[i],NULL);
			 fprintf(stderr,"线程%d已经结束: %ld\n",i,thread[i]);
		 }
	 }
	 fclose(m3u8->fp);
	 }
 void thread_create(struct m3u8_fb *m3u8){
	 int temp;
	 thread=(pthread_t *)malloc(sizeof(pthread_t)*pth);
	 memset(thread,0,sizeof(pthread_t)*pth);
	 for(int i=1; i<=pth;i++){ 
	  	 if((temp = pthread_create(&thread[i], NULL, threadnew, m3u8)) != 0){
		  	 printf("线程%d创建失败!\n",i);
	  	} 
	 }

 }
void phelp(){
	printf("用法: m3u8 -t 线程数 url url \n");
	exit(1);
}
int main( int argc, char **argv){
	if(argc==1){
		phelp();
	}
	for( int i=1; i<argc;i++){
		if(strcmp("-t",argv[i])==0){
				pth=atoi(argv[i+1]);
				if(pth==0){
					fprintf(stderr,"输入线程错误: %s\n",argv[i]);
					exit(-1);
				}
				i++;
				continue;
				}
		if(*argv[i]=='-'){
			fprintf(stderr,"输入错误: %s\n",argv[i]);
			phelp();
		}
		download_m3u8(argv[i]);
	}
	return 0;
}
