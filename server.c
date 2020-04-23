#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include <unistd.h>

#include <signal.h>

#include <sys/wait.h>

#include <arpa/inet.h>

#include <sys/socket.h>


 

#define BUF_SIZE 1024   //버프사이즈 지정

void error_handling(char *message);

void read_childproc(int sig);


 


 

int main(int argc, char *argv[])

{

         int serv_sock1, clnt_sock1, serv_sock2, clnt_sock2;

         struct sockaddr_in serv_adr1, clnt_adr1, serv_adr2, clnt_adr2;


 

         int fds1[2],fds2[2];

         pid_t pid1;

         pid_t pid2;

         struct sigaction act1;

         struct sigaction act2;

         socklen_t adr_sz1, adr_sz2;

         int str_len,state1,state2;

         char buf[BUF_SIZE];

         if(argc!=3){

                  printf("Usage : %s <port> <port>\n",argv[0]); // 클라이트언트 두개 PORT번호 입력받음.

                  exit(1);

         }

         //sigaction 구조체.

         act1.sa_handler=read_childproc; //함수 포인터 값을 저장.

         sigemptyset(&act1.sa_mask); //모든 비트를 0으로.

         act1.sa_flags=0; //0으로 초기화.

         state1=sigaction(SIGCHLD, &act1, 0); //자식 프로세스가 종료된 상황이면 시그널 핸들러의 정보 전달.


 

         serv_sock1=socket(PF_INET, SOCK_STREAM, 0); //TCP 소켓1 생성.

         if(serv_sock1==-1) //소켓1 생성 실패.

                  error_handling("sock() error");

        

         //구조체 변수 serv_adr1의 모든 멤버 0으로 초기화.

         memset(&serv_adr1, 0, sizeof(serv_adr1));

         serv_adr1.sin_family=AF_INET; // 주소체계 지정. 

         serv_adr1.sin_addr.s_addr=htonl(INADDR_ANY);  // IP주소 초기화.

         serv_adr1.sin_port=htons(atoi(argv[1])); // PORT번호 초기화.


 

         if(bind(serv_sock1, (struct sockaddr*)&serv_adr1, sizeof(serv_adr1))==-1) //첫 번째 소켓의 주소정보 할당.

                  error_handling("bind() error");

         if(listen(serv_sock1, 5)==-1) //연결 요청 대기상태 (큐 5개).

                  error_handling("listen() error");


 

         //sigaction 구조체.

         act2.sa_handler=read_childproc; //함수 포인터 값을 저장.

         sigemptyset(&act2.sa_mask); //모든 비트를 0으로.

         act2.sa_flags=0; //0으로 초기화.

         state2=sigaction(SIGCHLD, &act2, 0); //자식 프로세스가 종료된 상황이면 시그널 핸들러의 정보 전달.


 

         serv_sock2=socket(PF_INET, SOCK_STREAM, 0); //TCP 소켓2 생성.

         if(serv_sock2==-1) //소켓2 생성 실패.

                  error_handling("sock() error");

        

         //구조체 변수 serv_adr2의 모든 멤버 0으로 초기화.

         memset(&serv_adr2, 0, sizeof(serv_adr2)); 

         serv_adr2.sin_family=AF_INET; // 주소체계 지정.

         serv_adr2.sin_addr.s_addr=htonl(INADDR_ANY); // IP주소 초기화.

         serv_adr2.sin_port=htons(atoi(argv[2])); // PORT번호 초기화.


 

         if(bind(serv_sock2, (struct sockaddr*)&serv_adr2, sizeof(serv_adr2))==-1) //두 번째 소켓의 주소정보 할당.

                  error_handling("bind() error");

         if(listen(serv_sock2, 5)==-1) //연결 요청 대기상태 (큐 5개).

                  error_handling("listen() error");


 

                  adr_sz1=sizeof(clnt_adr1);

                  adr_sz2=sizeof(clnt_adr2);


 


 

                  clnt_sock1=accept(serv_sock1, (struct sockaddr*)&clnt_adr1, &adr_sz1); //연결 허용, 첫 번째 데이터 소켓 생성.

                  if(clnt_sock1==-1) //연결 실패.

                           error_handling("accept() error");

                  else //연결 성공시.

                           puts("client#1 connected...");

                 

                  clnt_sock2=accept(serv_sock2, (struct sockaddr*)&clnt_adr2, &adr_sz2); //연결 허용, 두 번째 데이터 소켓 생성.

                  if(clnt_sock2==-1) //연결 실패.

                           error_handling("accept() error");

                  else //연결 성공시.

                           puts("client#2 connected...");


 

         while(1){

                 


 

                  pipe(fds1),pipe(fds2); //두개의 파이프를 사용해 양방향 통신.

                  pid1=fork();  //첫번째 클라이언트 fork.

                  if(pid1==-1) //fork 실패시 -1.

                  {

                           close(clnt_sock1); //데이터 소켓 close.

                           continue;

                  }                

                  if(pid1==0) //클라이언트1 첫번째 자식 프로세스 생성.

                  {


 

                           close(serv_sock1); //자식1 프로세스 리스닝 소켓 필요없으니 close.


 

                           while(1){


 

                                    str_len=read(fds1[0],buf,BUF_SIZE); //파이프1을 통해 자식2에서 자식1로 전달받음.

                                    write(clnt_sock2,buf,str_len); //클라이언트1에서 서버로 입력받은 것을 파이프를 통해 전달받고 클라이언트2로 출력.


 

                           }


 

                           close(clnt_sock1); //자식1 프로세스 데이터 소켓 close.

                           return 0;

                  }

                  else

                  {

                           pid1=fork();

                           if(pid1==0) //클라이언트1의 또 다른 자식2 프로세스.

                           {

                                    close(serv_sock1); //자식2 프로세스 리스닝 소켓 close.


 

                                    while(1){

                                             //나중에 생긴 자식2부터 실행.

                                             str_len=read(clnt_sock1,buf,BUF_SIZE); //클라이언트1에서 서버로 입력을 받음.

                                             write(fds1[1], buf, str_len); //파이프1을 통해 자식2에서 자식1로 전달.(fork시 메모리공간을 공유하지 않기 때문 파이프사용)


 

                                             if(str_len==0) //클라이언트1이 EOF보낼시 break.

                                                     break;

                                    }

                                    close(clnt_sock1); //자식2 프로세스 데이터 소켓 close.

                                    puts("client#1 disconnected....");

                                    return 0;

                           }


 

                  }

                  pid2=fork(); //두번째 클라이언트 fork.

                  if(pid2==-1) //fork 실패시.

                  {

                           close(clnt_sock2); //데이터 소켓 close.

                           continue;

                  }                

                  if(pid2==0) //클라이언트2의 첫번째 자식 프로세스 생성.

                  {


 

                           close(serv_sock2); //첫번째 자식 프로세스 리스닝 소켓 필요없으니 close.


 

                           while(1){


 

                                    str_len=read(fds2[0],buf,BUF_SIZE); //파이프2을 통해 자식2에서 자식1로 전달받음.

                                    write(clnt_sock1,buf,str_len); //클라이언트2에서 서버로 입력받은것을 파이프를 통해 전달받고 클라이언트1로 출력.

                           }

                           close(clnt_sock2); //첫번째 자식 프로세스 데이터 소켓 close.

                           return 0;

                  }

                  else

                  {

                           pid2=fork();

                           if(pid2==0) //클라이언트2의 또 다른 두번째 자식 프로세스 생성.

                           {


 

                                    close(serv_sock2); //두번째 자식 프로세스 리스닝 소켓 필요없으니 close.


 

                                    while(1){

                                   

                                             str_len=read(clnt_sock2,buf,BUF_SIZE); //클라이언트2에서 서버로 입력 받음.

                                             write(fds2[1], buf, str_len); //파이프2을 통해 자식2에서 자식1로 전달.(fork시 메모리공간을 공유하지 않기 때문 파이프사용)

                                            

                                             if(str_len==0) //클라이언트2이 EOF보낼시 break.

                                                     break;

                                    }

                                    close(clnt_sock2); //두번째 자식 프로세스 데이터 소켓 close.

                                    puts("client#2 disconnected....");

                                    return 0;


 

                           }

                  }

                  close(clnt_sock1); //클라이언트1 데이터소켓 close.

                  close(clnt_sock2); //클라이언트2 데이터소켓 close.

         }

         close(serv_sock1); //클라이언트1 리스닝소켓 close.

         close(serv_sock2); //클라이언트2 리스닝소켓 close.

         return 0;

}


 


 


 

void read_childproc(int sig)

{

         pid_t pid;

         int status;

         // -1 : 임의의 자식 프로세스가 종료되길 기달린다. WNOHANG : 종료된 자식 프로세스가 존재하지 않아도 블로킹 상태에 있지않고 0을 반환하며 함수을 빠져나온다.

         pid=waitpid(-1, &status, WNOHANG);

         printf("removed proc id: %d \n",pid); //자식 프로세스 ID 출력.

}

void error_handling(char*message)

{

         fputs(message, stderr);

         fputc('\n',stderr);

         exit(1);

}
