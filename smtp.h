#ifndef __SMTP_H__
#define __SMTP_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>   
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/stat.h>
#include <time.h>

#define  SMTP_PORT (25)

#define MAX_SIZE   (1024*1024 * 1)

typedef struct
{
	char is_atta;
	char is_context;
	char from_add[28];
	char password[24];
	char to_add[1][28];
	char subject[20];
	char context[256];
	char Attachment[36];
}email_t;

int send_email(email_t *email);

#endif
