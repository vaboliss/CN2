#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>
#include <sys/socket.h>
#include <resolv.h>
#include <netdb.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <stdbool.h>
#define bzero(b,len) (memset((b), '\0', (len)), (void) 0)
#define FAIL    -1
#define h_addr h_addr_list[0]
#define Help " WELCOME TO EMAIL SERVICE PROGRAM\n \
                [1] Show all email headers\n \
                [2] Show how many emails ?\n \
                [3] Show specific email\n  \
              [4] Delete specific email\n \
                [5] Reset marked emails\n \
                [6] Quit\n"


int TakeInputNumber();
int OpenConnection(const char *hostname, int port);
int STAT(SSL *ssl);

SSL_CTX* InitCTX(void);

void Start();
void TOP(SSL *ssl,int msg);
void RETR(SSL *ssl,int msg);
void DELE(SSL *ssl,int msg);
void Quit(SSL *ssl);
void RSET(SSL *ssl);
void AddToArray(int option2,int *deleted);

bool Compare(int option2,int *deleted);

int main()
{

    int choice;
    do
    {
         printf("\n\nMenu\n\n");
         printf("1. LOGIN\n");
         printf("2. Exit\n");
         choice = TakeInputNumber();
        

         switch (choice)
        {
            case 1: Start();
                break;
            case 2: printf("Exiting....\n");
                break;
            default: printf("Wrong Choice. Enter again\n");
            break;
        }    
    } while (choice != 2);

    return 0;
}

void Start()
{
    SSL_CTX *ctx;
    SSL *ssl;

    int server;
    int bytes;

    char buf[1024];
    char acClientRequest[1024] ={0};
    char hostname[100]; 
    char portnum[100];
    char acUsername[16] ={0};
    char acPassword[16] ={0};
    const char *cpRequestMessage = "USER %s\n";

    printf("Provide POP3 hostname: \n");
    scanf("%s", hostname);
    printf("Provide POP3 port: \n");
    scanf("%s", portnum);

   

    printf("hostname : %s\n", hostname);
    printf("port : %s\n", portnum);
    fflush(stdin);
    SSL_library_init();
    ctx = InitCTX();
    server = OpenConnection(hostname, atoi(portnum));
    ssl = SSL_new(ctx);      /* create new SSL connection state */
    SSL_set_fd(ssl, server);    /* attach the socket descriptor */

    if ( SSL_connect(ssl) == FAIL ) {
        ERR_print_errors_fp(stderr);}
    else
    {  
        printf("\n\nEmail Reader\n\n");
        

        printf("Enter the User Name : ");
        scanf("%s",acUsername);

        sprintf(acClientRequest, cpRequestMessage,acUsername); 
  
        SSL_write(ssl,acClientRequest, strlen(acClientRequest));  
        bytes = SSL_read(ssl, buf, sizeof(buf));
        buf[bytes] = 0;

        bytes = SSL_read(ssl, buf, sizeof(buf)); 
        buf[bytes] = 0;
        if(buf[0] == '+')
        {
            printf("Provide password: \n");
        }

        scanf("%s",acPassword);

        const char *cmd = "PASS %s\n";

        sprintf(acClientRequest,cmd,acPassword);

        SSL_write(ssl,acClientRequest, strlen(acClientRequest));
        bytes = SSL_read(ssl, buf, sizeof(buf)); 
        buf[bytes] = 0;
        if(buf[0] == '+')
        {
            printf("Successfully Loged In\n");
        }
        else
        {
            printf("Wrong username\n");
            return;
        }

       int option1;
       int option2;
       int deleted[1000];
        int msgQ;
        for(;;) 
        {
            printf(Help);
            int option = TakeInputNumber();
            switch (option)
            {
                case 1:
                    msgQ = STAT(ssl);
                    for(int i = 1; i<=msgQ;i++)
                    {
                    if(Compare(i,deleted))
                    {
                        i++;
                        TOP(ssl,i);
                    
                    }else{
                        TOP(ssl,i);
                    }
                    
                    }
                    break;
                case 2:
                    msgQ = STAT(ssl);
                    printf("\nEmails : %d\n\n",msgQ);
                    break;
                case 3:
                    msgQ= STAT(ssl);
                    if (msgQ != 0){
                    printf("\nChoose your option (from 1 to %d)\n ",msgQ);
                    for(;;)
                        {
                            option1 = TakeInputNumber();
                            if(option1 > msgQ || option1 < 1)
                            {
                                printf("Wrong email number\n");
                            }
                            else break;
                        }
                    RETR(ssl,option1);
                    break;
                    }
                    else
                    {
                        printf("There are no emails\n");
                        break;
                    }
                case 4:
                    msgQ= STAT(ssl);
                    if (msgQ != 0)
                    {
                        printf("\nChoose your option (from 1 to %d)\n ",msgQ);
                        for(;;)
                        {
                            option2 = TakeInputNumber();
                            if(option2 > msgQ || option2 < 1)
                                    {
                                        printf("Wrong email number\n");
                                    }
                                else 
                                    { 
                                        if(Compare(option2,deleted))
                                        {
                                            printf("The email is already marked %d\n",option2);
                                        }else
                                        {
                                            AddToArray(option2,deleted);
                                            break;
                                        }
                                    }
                        }
                        DELE(ssl,option2);
                    }
                      else
                    {
                        printf("There are no emails\n");
                        break;
                    }
                    break;
                case 6:
                    Quit(ssl);
                    CleanArray(deleted);
                    goto END;
                case 5:
                    CleanArray(deleted);
                    RSET(ssl);
                default:
                break;
            }   
            

        }   
    }
    END:
    SSL_free(ssl);
    SSL_CTX_free(ctx); 
    close(server);           
}
void CleanArray(int*deleted)
{
    for(int i = 0 ; i<sizeof(deleted);i++)
    {
        deleted[i] = NULL;
    }

}
void AddToArray(int option2,int *deleted)
{

    deleted[strlen(deleted)] =option2; 

}
bool Compare(int option2,int *deleted)
{
    for(int i = 0; i < sizeof(deleted); i++)
    {
        if(deleted[i] == option2)
        {
            return true;
        }
        else return false;
    }
}
void Quit(SSL *ssl)
    {
        int bytes;
        char acClientRequest[1024]; 
        char buf[1024];
        char *cmd = "QUIT\n";
        sprintf(acClientRequest,cmd);
        SSL_write(ssl,acClientRequest, strlen(acClientRequest));
        bzero(buf,sizeof(buf));
        bytes = SSL_read(ssl, buf, sizeof(buf)); 
        buf[bytes] = 0;
        printf("%s\n",buf);
      }
void RSET(SSL *ssl)
    {
        int bytes;
        char acClientRequest[1024] = {0}; 
        char buf[1024] = {0};
        char *cmd = "RSET\n";
        sprintf(acClientRequest,cmd);
        SSL_write(ssl,acClientRequest, strlen(acClientRequest));
        bzero(buf,sizeof(buf));
        bytes = SSL_read(ssl, buf, sizeof(buf)); 
        buf[bytes] = 0;
        printf("%s\n",buf);
    }
void DELE(SSL *ssl,int msg)
{

    int bytes;
    char acClientRequest[1024]; 
    char buf[1024];
    char *cmd = "DELE %d\n";
    sprintf(acClientRequest,cmd,msg);
    SSL_write(ssl,acClientRequest, strlen(acClientRequest));
    bzero(buf,sizeof(buf));
    bytes = SSL_read(ssl, buf, sizeof(buf)); 
    buf[bytes] = 0;
    if (buf[0] == '+')
    {
        printf("Email is marked to be deleted. It will be deleted after [5]Quit\n");
    }

}
void RETR(SSL *ssl,int msg)
{
    printf("\n%d\n",msg);
    int bytes;
    char acClientRequest[1024]; 
    char buf[1024];
    char *cmd = "RETR %d\n";

    int i = 0;

    sprintf(acClientRequest,cmd,msg);
    SSL_write(ssl,acClientRequest, strlen(acClientRequest));
    for(;;)
    {
        bzero(buf,sizeof(buf));
        bytes = SSL_read(ssl, buf, sizeof(buf)); 
        buf[bytes] = 0;

        char *boundpositon = strstr(buf,"boundary=");
        if(boundpositon != NULL)
            { 
            for(i = 11; i<strlen(boundpositon);i++)
            {
                if(boundpositon[i] == '"') break;
            }
            char *com = strndup(boundpositon+10,i-10);
            for(int j = strlen(com); j > 0;j--)
            {
                com[j+2] = com[j];
            }
            com[0] = '-';
            com[1] = '-';
            
            char *start = strstr(buf,com);
            
            
            com[strlen(com)]='-';
            com[strlen(com)+1]='-';
        
            if(start != NULL)
                {
                printf("%s", start);
                for(;;)
                {
                char * end = strstr(buf,com);            
                if(end != NULL)
                {;break;}
                else
                {
                    bzero(buf,sizeof(buf));
                    SSL_read(ssl,buf,sizeof(buf));
                    buf[bytes]= 0;
                    printf("%s\n",buf);
                }
            
                }
                break;
                }
            }
                
    }
}   
void TOP(SSL *ssl,int msg)
{

    char *cmd3 = "Top %d 1\n";
    char buf[1024];
    char acClientRequest[1024];
    int bytes;
    char Subject[1024];
    char From[1024];
    char Date[1024];
    int j =0;
    int i =0;
    int z = 0;
    sprintf(acClientRequest,cmd3,msg,1);
    SSL_write(ssl,acClientRequest, strlen(acClientRequest));
    
    for(;;)
    {
         
    bzero(buf,sizeof(buf));
    bytes = SSL_read(ssl, buf, sizeof(buf)); 
    buf[bytes] = 0;

    char search[] = "Subject:";
    char *Sposition = strstr(buf,search);
     if (Sposition == NULL)
     {}
     else
     {
        
        for(;j<strlen(buf);j++)
        {
            if(Sposition[j] == '\n')break;
        }
        
         char *s1 = strndup(Sposition,j);
         strcpy(Subject,s1);
        j = 1;
     }
       
    char *Fposition = strstr(buf,"From:"); 
    if (Fposition == NULL)
     {}
    else
     {
        for(;i<strlen(buf);i++)
        {
            if(Fposition[i] == '>')break;
        }
         char *s = strndup(Fposition,i);
         strcpy(From,s);
        i = 1;
     }

    


   char *Dposition = strstr(buf,"Date:");
      if (Dposition == NULL)
     {}
    else
     {
        
        for(;z<strlen(buf);z++)
        {
            if(Dposition[z] == '\n')break;
        }
        char *s2 = strndup(Dposition,z);
        strcpy(Date,s2); 
        z = 1;
     }
    if (i == 1 && z == 1 && j == 1 ){
            printf(" %d. ________________________________\n",msg);
            printf("\n %s\n",Subject);
            printf("\n %s\n",From); 
            printf("\n %s\n",Date); 
             printf("________________________________\n");

        break;
    }
    }
}

int STAT(SSL *ssl)
{
    int bytes;
    char acClientRequest[1024]; 
    char buf[1024];
    char *cmd = "STAT\n";
    sprintf(acClientRequest,cmd);
    SSL_write(ssl,acClientRequest, strlen(acClientRequest));
    bzero(buf,sizeof(buf));
    bytes = SSL_read(ssl, buf, sizeof(buf)); 
    buf[bytes] = 0;
    int i = 1;
    char *space = strchr(buf,' ');
    for(;i<strlen(space);i++)
    {
        if(space[i] == ' '){
            break;
        }
    } 
    char *number = strndup(space,i);
    int num = atoi(number);
    return num;
}
int TakeInputNumber()
{
    int z = 0;
    int chosen;
    char choice[100] = {0};
    printf("Choose your option: \n");
    scanf("%s",choice);
    
    for(int i = 0;i<strlen(choice);i++)
    {
        for(int j = 48; j<58; j++)
        {
        if( choice[i] == j )
            {
                z++;
            } 
        }
        if (z == strlen(choice))
        {
            chosen = atoi(choice);
            return chosen;
        }
    }
    printf("Wrong input\n");
    TakeInputNumber();
    return -1;

}
int OpenConnection(const char *hostname, int port)
{ 
    int sd;
    struct hostent *host;
    struct sockaddr_in addr;
    if ( (host = gethostbyname(hostname)) == NULL )
        {
            printf("Wrong hostname or port\n");
            Start();
            
        }
    sd = socket(PF_INET, SOCK_STREAM,0);
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = *(long*)(host->h_addr);
    if ( connect(sd, (struct sockaddr*)&addr, sizeof(addr)) != 0 )
        {
            close(sd);
            perror("Wrong hostname or port\n");
            Start();
        }
    return sd;
}
SSL_CTX* InitCTX(void)
{ 
    SSL_METHOD *method;
    SSL_CTX *ctx;
    OpenSSL_add_all_algorithms();  /* Load cryptos, et.al. */
    SSL_load_error_strings();   /* Bring in and register error messages */
    method = TLSv1_2_client_method();  /* Create new client-method instance */
    ctx = SSL_CTX_new(method);   /* Create new context */
    if ( ctx == NULL )
        {
            printf("Error\n");
            abort();
        }
    return ctx;
}
