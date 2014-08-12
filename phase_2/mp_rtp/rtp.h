/*
 * rtp.h
 *
 */

#ifndef RTP_H_
#define RTP_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <sys/sem.h>
#include <arpa/inet.h>

#include <ifaddrs.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <unistd.h>
#include <getopt.h>
#include <netdb.h>
#include <pthread.h>

#include "Error.h"

#define SPS	(1)
#define PPS	(2)
#define RTP_VERSION_OFFSET	(0)
#define RTP_MARK_OFFSET		(1)
#define RTP_SEQ_OFFSET		(2)
#define RTP_TIMETAMP_OFFSET	(4)
#define RTP_SSRC_OFFSET		(8)

#define MAX_PACK_LEN (1448)
#define RTP_HEAD_LEN (12)


#define H264_TYPE			(0x60)
#define SET_RTP_VERSION()	{*(rtp_buff+RTP_VERSION_OFFSET)=0x80;}
#define SET_RTP_MARK()		{*(rtp_buff+RTP_MARK_OFFSET)=(H264_TYPE&0x7f);}
#define SET_RTP_SEQ()		{*(rtp_buff+RTP_SEQ_OFFSET+1)=(head->sernum>>0)&0xff;\
				 *(rtp_buff+RTP_SEQ_OFFSET)=(head->sernum>>8)&0xff;\
				}

#define SET_RTP_TIMETAMP()	{*(rtp_buff+RTP_TIMETAMP_OFFSET+3)=(head->timtamp>>0)&0xff;\
				 *(rtp_buff+RTP_TIMETAMP_OFFSET+2)=(head->timtamp>>8)&0xff;\
				 *(rtp_buff+RTP_TIMETAMP_OFFSET+1)=(head->timtamp>>16)&0xff;\
				 *(rtp_buff+RTP_TIMETAMP_OFFSET+0)=(head->timtamp>>24)&0xff;\
				}

//synchronization source identifier, 32 bits
#define SET_RTP_SSRC()	{*(rtp_buff+RTP_SSRC_OFFSET+3)=(head->ssrc>>0)&0xff;\
			 *(rtp_buff+RTP_SSRC_OFFSET+2)=(head->ssrc>>8)&0xff;\
			 *(rtp_buff+RTP_SSRC_OFFSET+1)=(head->ssrc>>16)&0xff;\
			 *(rtp_buff+RTP_SSRC_OFFSET+0)=(head->ssrc>>24)&0xff;\
			}

#define SET_RTP_FIXHEAD()	{SET_RTP_VERSION();\
				 SET_RTP_MARK();\
				 SET_RTP_SEQ();\
				 SET_RTP_TIMETAMP();\
				 SET_RTP_SSRC();\
				}

//rtp packet header
struct rtp_pack_head
{
	unsigned short sernum;
	unsigned int timtamp;
	unsigned int ssrc;
};

//data in the packet
struct rtp_data
{
	void *buff;
	char rtp_fui;
	char rtp_fuh;
	unsigned int offset;
	unsigned int datalen;
	unsigned int bufrelen;
	unsigned int rtpdatakcount;
};


struct rtp_pack
{
	void *databuff;
	unsigned int packlen;
	unsigned int packtype;
};



struct sockets
{
	int local;
	int reclen;
	FILE *in,*out;
	struct sockaddr_in *cliaddr;
};


typedef struct config {
	char* ip;
	char time;
	int port;
	int type;
	int protocol;
}SOCKCONFIG;



//*****************************  RTP varialbes

struct rtp_pack *rtp;
struct rtp_data *rtpdata;

struct rtp_pack_head head;

struct sockets *sock;


FILE *temp;

void *creat_rtp_pack(struct rtp_data *data)
{
	unsigned char *buf;

	if(data->datalen>MAX_PACK_LEN&&data->datalen>(data->bufrelen+data->rtpdatakcount))
	{
		unsigned int templen=(data->datalen-data->bufrelen-1);
		data->rtpdatakcount+=1;
		if(templen>MAX_PACK_LEN-1)
		{
			buf=(unsigned char *)malloc(MAX_PACK_LEN);
			memset(buf,0,MAX_PACK_LEN);
			if(data->bufrelen==0)
			{
				memcpy((buf+RTP_HEAD_LEN+1),(data->buff+data->offset),(MAX_PACK_LEN-RTP_HEAD_LEN-1));
				data->rtp_fui=((*(buf+RTP_HEAD_LEN+1)&0xE0));
				data->rtp_fuh=(*(buf+RTP_HEAD_LEN+1)&0x1f);
				*(buf+RTP_HEAD_LEN+1)=(data->rtp_fuh|(0x80));
				data->bufrelen+=MAX_PACK_LEN;
				data->offset+=MAX_PACK_LEN-RTP_HEAD_LEN-1;
			}else{
				 memcpy((buf+RTP_HEAD_LEN+2),(data->buff+data->offset),(MAX_PACK_LEN-RTP_HEAD_LEN-2));
				*(buf+RTP_HEAD_LEN+1)=(data->rtp_fuh|(0x00));
				data->bufrelen+=MAX_PACK_LEN;
				data->offset+=MAX_PACK_LEN-RTP_HEAD_LEN-2;
			}

			*(buf+RTP_HEAD_LEN)=(data->rtp_fui|(0x1c));

		}else{

			templen=data->datalen-data->offset;
			buf=(unsigned char *)malloc(templen+RTP_HEAD_LEN+2);
			memset(buf,0,templen+RTP_HEAD_LEN+2);
			memcpy((buf+RTP_HEAD_LEN+2),(data->buff+data->offset),templen);
			*(buf+RTP_HEAD_LEN)=(0x1c|data->rtp_fui);
			*(buf+RTP_HEAD_LEN+1)=(data->rtp_fuh|(0x40));
			data->bufrelen+=templen+RTP_HEAD_LEN+2;
			data->offset+=templen-1;
		}
	}else if(data->datalen>data->bufrelen){
		buf=(unsigned char *)malloc(data->datalen+RTP_HEAD_LEN);
		memset(buf,0,data->datalen+RTP_HEAD_LEN);

		memcpy((buf+RTP_HEAD_LEN),data->buff,data->datalen);
		data->bufrelen+=data->datalen+RTP_HEAD_LEN;
	}else
		return NULL;

	return buf;
}

struct rtp_pack *rtp_pack(struct rtp_data *pdata,struct rtp_pack_head *head)
{
	char *rtp_buff;
	int len=pdata->bufrelen;

	rtp_buff=creat_rtp_pack(pdata);
	
	struct rtp_pack *pack;
	if(rtp_buff!=NULL)
	{
		SET_RTP_FIXHEAD();

		pack=(struct rtp_pack *)malloc(sizeof(struct rtp_pack));
		pack->databuff=rtp_buff;
		pack->packlen=pdata->bufrelen-len;
	}else{
		free(pdata->buff);
		pdata->buff=NULL;
		return NULL;
	}
	return pack;
}


void *getdata(uint8_t *data, uint32_t length)
{


	uint8_t *databuf=(uint8_t *)malloc(length);
	memcpy(databuf,data,length);
	
	//rtp_dataΩ·ππ¥¥Ω®£¨ÃÓ≥‰
	struct rtp_data *rtp_data=(struct rtp_data *)malloc(sizeof(struct rtp_data));
	memset(rtp_data,0,sizeof(struct rtp_data));

	rtp_data->buff=databuf;
	rtp_data->datalen=length;
	rtp_data->bufrelen=0;
	return rtp_data;
}


char rtp_send(struct rtp_pack *rtp)
{
	int num=0;

fprintf(stderr, "called!!!!!!!!!!!rtp->packlen: %d, sock->local:%d \n", rtp->packlen, sock->local);
//for_test();
	 if((num=sendto(sock->local,rtp->databuff,rtp->packlen,0,sock->cliaddr,sizeof(struct sockaddr_in)))==-1)
	 {
fprintf(stderr, "WTF failed!!!!!!!!!!!rtp->packlen: %d\n", rtp->packlen);
		 fail("socket: %s\n", strerror(errno));
	 }
fprintf(stderr, "WTF stopped!!!!!!!!!!!rtp->packlen: %d\n", rtp->packlen);
	 debug("number is : %d\n", num);
	 free(rtp->databuff);
	 rtp->databuff=NULL;
	 return 0;
}



int socket_dgram_setup(SOCKCONFIG *socketconfig)
{
	int s;

	if ((s=socket(AF_INET, (socketconfig->type), (socketconfig->protocol)))== -1)
	{
		fail("socket: %s\n", strerror(errno));
	}

	return s;
}


struct sockets *get_dgram_sockets(int sock)
{

	struct sockets *soc;
	soc = malloc(sizeof(struct sockets));
	if (soc == NULL)
	{
		warn("malloc failed.\n");
		return NULL;
	}

	soc->local = sock;
	soc->in = fdopen(sock, "r");
	soc->out = fdopen(sock, "w");

	return soc;
}

#endif /* RTP_H_ */
