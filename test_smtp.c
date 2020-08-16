#include "smtp.h"

int main(int args,  char *argv[])
{
	email_t  email;
	memset(&email, '\0', sizeof(email));

	#if 0
	printf("Input enter email add:  ");
	fgets(email.from_add, sizeof(email.from_add) -1,  stdin);
    printf("Input enter password:  ");
	fgets(email.password, sizeof(email.password) -1,  stdin);

	int i = 0;
	while(1)
	{
		printf("Input recver email add:  ");
		email.to_add[i] = (char *)malloc(sizeof(email.to_add) / sizeof(char));
		fgets(email.to_add[i++], sizeof(email.to_add) -1,  stdin);
		
		printf("Next recver email ? [NO] input 'N', ELSE input any key to continue:  ");
	    if(getchar() != 'N')
			continue;

		//清空标准输入缓存
		while ( getchar() != '\n' && getchar() != EOF );

		printf("Input send email subject:  ");
		fgets(email.title, sizeof(email.subject) -1,  stdin);
		printf("[----------[%s]\n", email.subject);
		
		printf("Input send email context:  ");
		fgets(email.context, sizeof(email.context) -1,  stdin);

		printf("send attachment ? [NO] input 'N' exit, ELSE input any key to continue:  ");
	    if(getchar() == 'N')
			break;

		while ( getchar() != '\n' && getchar() != EOF );
		
		printf("Input send email attachment:  ");
		fgets(email.Attachment, sizeof(email.Attachment) -1,  stdin);
		
		break;
	}
	#else
	
	 //char *f_d = "velen751@sina.com";
	 //char *p_w = "6a8e380f1e6bf24c";
     //char *t_a = "aken123de@163.com";

	char *f_d = "aken123de@163.com";
	char *p_w = "kvcs15dt"; //客户端授权第三方登录的密码.  
	char *t_a = "velen751@sina.com"; //554 DT:SPM 163


   //char *t_a = "2193924446@qq.com";
    //char *t_a = "1445536992@qq.com";
	char *subject = "love-1"; 
	char *context = "4565454 world !";
	char *attachment = "girl.jpg";
	
	memcpy(email.from_add, f_d, strlen(f_d));
	memcpy(email.password, p_w, strlen(p_w));
	//email.to_add[0] =(char *)malloc(sizeof(email.to_add[0]));
	memcpy(email.to_add[0], f_d, strlen(f_d));
	memcpy(email.to_add[1], t_a, strlen(t_a));
	memcpy(email.subject, subject, strlen(subject));
	email.is_context = 1;
	memcpy(email.context, context, strlen(context));
	email.is_atta = 0;
	memcpy(email.Attachment, attachment, strlen(attachment));
	#endif
	
	send_email(&email);

    #if 0
	i = 0;
	while(email.to_add[i])
	{
		free(email.to_add[i]);
	}
	#else
	//free(email.to_add[0]);
	#endif
	
	return 0;
}
