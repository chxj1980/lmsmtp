#include "smtp.h"

const char *smtp_add[24] = { "smtp.126.com", "smtp.sina.com", "smtp.163.com", "smtp.qq.com"};

char *mixed_boundary = "001###########";
char *alternative_boundary = "002###########";

const char base64Char[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
char* EncodeBase64(char *outbuff, char const* origSigned, unsigned origLength) 
{
	unsigned char const* orig = (unsigned char const*)origSigned; // in case any input bytes have the MSB set
	if (orig == NULL) return NULL;

	unsigned const numOrig24BitValues = origLength/3;
	char havePadding = origLength > numOrig24BitValues*3;
	char havePadding2 = origLength == numOrig24BitValues*3 + 2;
	unsigned const numResultBytes = 4*(numOrig24BitValues + havePadding);
	char* result =  outbuff;

	// Map each full group of 3 input bytes into 4 output base-64 characters:
	unsigned i;
	for (i = 0; i < numOrig24BitValues; ++i) 
	{
		result[4*i+0] = base64Char[(orig[3*i]>>2)&0x3F];
		result[4*i+1] = base64Char[(((orig[3*i]&0x3)<<4) | (orig[3*i+1]>>4))&0x3F];
		result[4*i+2] = base64Char[((orig[3*i+1]<<2) | (orig[3*i+2]>>6))&0x3F];
		result[4*i+3] = base64Char[orig[3*i+2]&0x3F];
	}

	// Now, take padding into account.	(Note: i == numOrig24BitValues)
	if (havePadding) 
	{
		result[4*i+0] = base64Char[(orig[3*i]>>2)&0x3F];
		if (havePadding2) 
		{
			result[4*i+1] = base64Char[(((orig[3*i]&0x3)<<4) | (orig[3*i+1]>>4))&0x3F];
			result[4*i+2] = base64Char[(orig[3*i+1]<<2)&0x3F];
		} 
		else 
		{
			result[4*i+1] = base64Char[((orig[3*i]&0x3)<<4)&0x3F];
			result[4*i+2] = '=';
		}
		result[4*i+3] = '=';
	}

  result[numResultBytes] = '\0';
  return result;
}

//velen751@sina.com
int getserveradd(const char *emailadd, char *serveradd)
{
	if(memchr(emailadd, '@', strlen(emailadd)))
	{
		int i = 0; 
		while(smtp_add[i])
		{
			if(strstr(emailadd, &smtp_add[i][5]))
			{
				 memcpy(serveradd, smtp_add[i], strlen(smtp_add[i]));
				 return 1;
			}
			i++;
		}
	}
	
	return -1;
}

void* getip(const char *emailadd )
{
	if(emailadd == NULL ){
		printf("Input email add  NULL, exit !\n");
		return NULL;
	}
	
	char serveradd[32] = {'\0'};
  	if(getserveradd(emailadd, serveradd) < 0)
  	{
  			printf("Get smtp server add  failed, exit !\n");
		return NULL;
	}

	struct hostent *host = gethostbyname(serveradd);
	if(host == NULL){
		printf("gethostbyname parse server add  failed, exit !\n");
		return NULL;
	}
	
	printf("SMTP server name is         :[%s]\n", serveradd);
	printf("SMTP server ip   is         :[%s]\n", inet_ntoa(*(struct in_addr*)host->h_addr_list[0]));
	return (host->h_addr_list[0]); //write such as, error: host->h_addr_list
}

int send_data(int sockfd, const char *date,  int size,  char * respondid)
{
	if(size <= 0){
		    printf("Send date size <= 0, exit !\n");
		return -1;	
	}
	
	if(date == NULL){
     		printf("Send date failed, exit !\n");
		return -1;
	}

	if(size < 1100)
		printf("%s", date);
	
	if(send(sockfd, date, size, 0) < 0 ){
		printf("Send data failed, exit !\n");
		return -1;
	}

	usleep(1000 * 10);

	if(respondid)
	{ 
		usleep(1000 * 150);
		char buff[128] = {'\0'};
		if(recv(sockfd, buff, sizeof(buff), 0)< 0 )
		{
			perror("===>ERROR:");
			printf("Recv data failed, exit !\n");
			return -1;
		}

		printf("%s", buff);
		if(strstr(buff, respondid))
		{
			
			return 0;
		}
		else
		{
			printf("----132324------>\n");
			return -1;
		}
		
	}
	
	return 0;
}

int create_net(const char *emailadd)
{
	if(emailadd == NULL){
		printf("Input email add  is  NULL, exit !\n");
		return -1;
	}  
    
	struct in_addr*  ip = NULL;
	if((ip = (struct in_addr*)getip(emailadd)) == NULL)
	{
		perror("get server ip failed, exit\n");
		return -1;
	}

	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if(fd < 0){
		perror("socket failed.\n");
		return -1;
	}

	struct sockaddr_in  add;
	bzero(&add, sizeof(add));

	add.sin_family = AF_INET;
	add.sin_port = htons(SMTP_PORT);
	add.sin_addr = *ip;
	
	if(connect(fd,  (struct sockaddr *)&add, sizeof(struct sockaddr_in)) < 0)
	{
		perror(" connect server failed.\n");
		return -1;
	}
	
	char buff[128] = {'\0'};
	if(recv(fd, buff, sizeof(buff), 0)< 0 )
	{
		printf("Recv data failed, exit !\n");
		return -1;
	}
	printf("--%s", buff);

	//send HELO command.
	char ehlo[40] = {'\0'};
	char hostname[32] = {'\0'};
	gethostname(hostname, sizeof(hostname));
	sprintf(ehlo, "EHLO %s\r\n", hostname);
	if(send_data(fd,ehlo, strlen(ehlo), "250") < 0){
   			printf("Send HELO command failed, exit !\n");
			return -1;
	}

	return fd;
}

int get_name(char * email, char *account)
{
	char *p = NULL;
	if( (p = memchr(email, '@', strlen(email))) )
	{
		memcpy(account, email, p - email);
		return 1;
	}
	
	return -1;
}

int login(int sockfd, const char *emailadd, const char *password)
{	
    //send LOGIN command.
	if(send_data(sockfd, "AUTH LOGIN\r\n", strlen("AUTH LOGIN\r\n"), "334") < 0){
   			printf("Send AUTH LOGIN command failed, exit !\n");
			return -1;
	}

	char buff[72] = {'\0'};
	memset(buff, '\0', sizeof(buff));
	//send email add.
	char buff64[64] = {'\0'};
	char account[24] = {'\0'};
		
	get_name((char *)emailadd, account);
	EncodeBase64(buff64, (char *)account, strlen(account));
	sprintf(buff, "%s\r\n", buff64);
	if(send_data(sockfd, buff, strlen(buff), "334") < 0){
   			printf("Send account failed, exit !\n");
			return -1;
	}

	memset(buff, '\0', sizeof(buff));
	//send email password.
	memset(buff64,'\0', sizeof(buff64));
	EncodeBase64(buff64, (char *)password, strlen(password));
	sprintf(buff, "%s\r\n", buff64);
	if(send_data(sockfd, buff, strlen(buff), "235") < 0)
	{
   			printf("Send password failed, exit !\n");
			return -1;
	}
	
	return 1;
}

int login_smtp(const char *emailadd,  const char *password)
{
	int sockfd = create_net(emailadd);
	if(sockfd < 0)
	{
		printf(" create net failed , exit !\n");
		return -1;
	}

	if(login(sockfd, emailadd, password) < 0)
	{
		printf(" Login smtp server failed , exit !\n");
		return -1;
	}

	return sockfd;
}

int send_meail_from(int sockfd,  char * fromadd)
{ 
	char buff[64] = {'\0'};
	sprintf(buff, "MAIL FROM: <%s>\r\n", fromadd);
	if(send_data(sockfd, buff, strlen(buff), "250") < 0 )
	{
		printf("Send MAIL FROM command failed, exit !\n");
		return -1;
	}

	return 0;
}

int send_recv_to(int sockfd, const char *recvadd)
{
	if(recvadd == NULL){
		printf("Input recver email add  is  NULL, exit !\n");
		return -1;
	}  
	
	printf("SMTP recever is       :[%s]\n",  recvadd);
	
	char buff[64] = {'\0'};
	sprintf(buff, "RCPT TO: <%s>\r\n", recvadd);

	if(send_data(sockfd, buff, strlen(buff), "250") < 0 )
	{
		printf("Send RCPT TO command failed, exit !\n");
		return -1;
	}

	return 1;
}


int send_data_comd(int sockfd)
{
	char buff[16] = {'\0'};
	sprintf(buff, "DATA\r\n");
	if(send_data(sockfd, buff, strlen(buff), "354") < 0 )
	{
		printf("Send DATA command failed, exit !\n");
		return -1;
	}

	return 1;
}

int get_email_info(int sockfd, email_t *email, int recverid, char *outbuff)
{
	int sz = 0;
	char *buff = outbuff;
	char recvername[32] = {'\0'};
	memset(buff, '\0', sizeof(buff));
	
	//eg:Date: Sun, 22 Dec 2019 18:10:15 +0800
	time_t st;
    struct tm *info;
	char tm[36] = {'\0'};
    time(&st);
    info = localtime(&st);
    strftime(tm, sizeof(tm), "%a, %d %b %Y %H:%M:%S %Z", info);
	sz += sprintf(buff , "Date: %s\n", tm);
	sz += sprintf(buff + sz, "From: \"%s\" <%s>\r\n", email->from_add, email->from_add);
	get_name((char *)email->to_add[recverid], recvername);
	sz += sprintf(buff + sz, "To: %s <%s>\r\n", recvername, email->to_add[recverid]);
	sz += sprintf(buff + sz, "Subject: %s\r\n", email->subject);//邮件主题
	sz += sprintf(buff + sz, "X-Priority: %d\r\n", 3);
	sz += sprintf(buff + sz, "X-Has-Attach: %s\r\n", email->is_atta ? "yes" : "no");//附件
	sz += sprintf(buff + sz, "X-Mailer: %s\r\n", "TNC-TN.0.1.0");//第三方代理客户端名字
	sz += sprintf(buff + sz, "Mime-Version: %s\r\n", "1.0");
	sz += sprintf(buff + sz, "Message-ID: <%ld@sina.com>\r\n", time(NULL));

	return sz;
}

int send_attachment(int sockfd, char *attachment)
{
	char src_buff[MAX_SIZE] = {'\0'};
	char des_buff[MAX_SIZE] = {'\0'};

	FILE *fd;
	int sz = 0;
	unsigned int fz = 0;
	unsigned int len = 0;
	struct stat buf;
	lstat(attachment, &buf);
	fz = buf.st_size;
	
	if(S_ISREG(buf.st_mode))
	{
		if( (fd = fopen(attachment, "r")) > 0)
		{	
			while(!feof(fd))
			{
				sz = fread(src_buff, 1, MAX_SIZE, fd);
				if(sz > 0)
				{
					EncodeBase64(des_buff, src_buff, sz);
					len  += sz;
					if(len >= fz)
					{
						sz += sprintf(des_buff + sz - 1, "%s", "\r\n");
					}else
					{
						sz += sprintf(des_buff + sz, "%s", "\r\n");
					}
					
					printf("===[%d]===[%d]=====\n", len, fz);
					if(send_data(sockfd, des_buff, sz, NULL) < 0)
					{
			   			printf("Send attachment data failed, exit !\n");
						return -1;
					}
				}
				memset(src_buff, '\0', MAX_SIZE);
				memset(des_buff, '\0', MAX_SIZE);
				usleep(1000 * 500);
			}
			
			return 0;
		}
	}
	 printf("==[%s]=33333333==\n", attachment);
  return -1;
}


//发送正文内容
int send_email_text(int sockfd, email_t *email, char * data,  int len)
{
	if(email->context == NULL){
		printf("Input recver email add  is  NULL, exit !\n");
		return -1;
	}

	int sz = 0;
	char buff[512] = {'\0'};
	char buff64[64] = {'\0'};

	//-------multi-part set-----------
	sz += sprintf(buff, "Content-Type: %s;\r\nboundary=\"%s\"\r\n\r\n", "multipart/alternative", alternative_boundary);
	sz += sprintf(buff + sz, "---%s\r\n",  alternative_boundary);
	sz += sprintf(buff + sz, "Content-Type: %s\r\n",  "text/plain;");
	sz += sprintf(buff + sz, "charset=%s\r\n",  "\"us-ascii\"");
	sz += sprintf(buff + sz, "Content-Transfer-Encoding: %s\r\n\r\n",  "base64");

	//------text date----------------
	EncodeBase64(buff64, (char *)email->context, strlen(email->context));
	sz += sprintf(buff + sz, "%s\n", buff64);
	memset(buff64, '\0', sizeof(buff64));
	EncodeBase64(buff64, (char *)email->from_add, strlen(email->from_add));
	sz += sprintf(buff + sz, "%s\r\n\r\n", buff64);
	
	sprintf(buff + sz, "%s---\r\n.\r\n",  alternative_boundary); 

    //--------send-------------------
	sprintf(data + len, "%s",  buff); 
	if(send_data(sockfd, data, strlen(data), "250") < 0 )
	{
		printf("Send email text failed, exit !\n");
		return -1;
	}
	
	return 0;
}

//发送附件
int send_email_attach(int sockfd, email_t *email, char * data,  int len)
{
	int sz = 0;
	char buff[256] = {'\0'};
	
	sz += sprintf(buff, "Content-Type:%s\n;boundary=\"%s\"\n\n", "multipart/mixed", mixed_boundary);
	
	//------attach date-------
	if(send_data(sockfd, buff, strlen(buff), NULL) < 0 )
	{
		printf("Send email attach failed, exit !\n");
		return -1;
	}
	
	return 0;
}

int send_text_attach(int sockfd, email_t *email, char * data,  int len)
{
	int sz = 0;
	char buff[800] = {'\0'};
	char buff64[64] = {'\0'};

	//--------------------
	sz += sprintf(buff, "Content-Type: %s;\r\nboundary=\"%s\"\r\n\r\n", "multipart/mixed", mixed_boundary);
	sz += sprintf(buff + sz, "---%s\r\n",  mixed_boundary);

	//------text date-------
	sz += sprintf(buff + sz, "Content-Type: %s;\r\nboundary=\"%s\"\r\n\r\n", "multipart/alternative", alternative_boundary);
	sz += sprintf(buff + sz, "---%s\r\n",  alternative_boundary);
	
	sz += sprintf(buff + sz, "Content-Type: %s\r\n",  "text/plain;");
	sz += sprintf(buff + sz, " charset=%s\r\n",  "\"us-ascii\"");
	sz += sprintf(buff + sz, "Content-Transfer-Encoding: %s\r\n\r\n",  "base64");

	EncodeBase64(buff64, (char *)email->context, strlen(email->context));
	sz += sprintf(buff + sz, "%s\r\n\r\n", buff64);
	sz += sprintf(buff + sz, "%s---\r\n",  alternative_boundary); //这里多写了2个'\r\n',解析出来多一个个文件.

	//------attach info-------
	sz += sprintf(buff + sz, "---%s\r\n",  mixed_boundary);
	sz += sprintf(buff + sz, "Content-Type: %s\r\n",  "application/octet-stream;");
	sz += sprintf(buff + sz, " name=\"%s\"\r\n",  email->Attachment); //必须有空格,不然解析不出名字.
	sz += sprintf(buff + sz, "Content-Transfer-Encoding: %s\r\n",  "base64");
	sz += sprintf(buff + sz, "Content-Disposition:%s;\r\n",  "attachment");
	sz += sprintf(buff + sz, "filename=\"%s\"\r\n\r\n",  email->Attachment);

	sz+= sprintf(data + len, "%s", buff);
	if(send_data(sockfd, data, sz, NULL) < 0 )
	{
		printf("Send text and attach failed, exit !\n");
		return -1;
	}

	//------send attach data-------
	if( send_attachment(sockfd, email->Attachment) < 0)
	{
		printf("Send email attachment failed, exit !\n");
		return -1;
	}

	memset(buff, '\0', sizeof(buff));
	sprintf(buff, "\n%s---\r\n\r\n.\r\n",  mixed_boundary); 
	if(send_data(sockfd, buff, strlen(buff), "250") < 0 )
	{
		printf("Send text and attach failed, exit !\n");
		return -1;
	}
	
	return 0;
}

int send_context(int sockfd, email_t *email, int recverid)
{
	char sendbuff[1100] = {'\0'};
	int len  = 0;
	send_data_comd(sockfd);
  	len = get_email_info(sockfd, email, recverid, sendbuff);
	
	if(email->is_atta && email->is_context)
	{
		send_text_attach(sockfd, email, sendbuff, len);
	}
	else if(email->is_atta)
	{ //只附件.
		send_email_attach(sockfd, email, sendbuff, len);
	}
	else if(email->is_context)
	{ //只有超文本和纯文本.
		send_email_text(sockfd, email, sendbuff, len);
	}
	
	return 0;
}

int send_quit(int sockfd)
{ 
	if(send_data(sockfd, "QUIT\r\n", strlen("QUIT\r\n"), "221") < 0 )
	{
		printf("Send QUIT command failed, exit !\n");
		return -1;
	}
	
	return 0;
}

int send_email(email_t *email)
{
	int sockfd = 0;
	int i = 0;
	sockfd = login_smtp(email->from_add, email->password);
	
	while(email->to_add[i] )
	{
		send_meail_from(sockfd, email->from_add);
		send_recv_to(sockfd, email->to_add[i]);
		send_context(sockfd, email, i);
		i++;
		printf("\n@@@@@@@@@@@@@@@[%d]@@@@@@@@@@@@@@@@@@@@@@@\n\n", i);
	}

	send_quit(sockfd);
	return 0;
}

