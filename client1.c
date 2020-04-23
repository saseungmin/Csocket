#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include <unistd.h>

#include <arpa/inet.h>

#include <sys/socket.h>

#define BUF_SIZE 1024


 

void error_handling(char*message);

void read_routine(int sock, char*buf);

void write_routine(int sock, char*buf);


 

int main(int argc, char *argv[])

{

        int sock;

        pid_t pid;

        char buf[BUF_SIZE];

        struct sockaddr_in serv_adr;

        if(argc!=3){

               printf("Usage : %s <IP> <port>\n", argv[0]); //IP, PORT번호 입력받음.

               exit(1);

        }

        //클라이언트1 소켓 생성.

        sock=socket(PF_INET, SOCK_STREAM, 0); //TCP 소켓 생성.


 

        //구조체 변수 serv1_adr의 모든 멤버 0으로 초기화.

        memset(&serv_adr,0,sizeof(serv_adr));

        serv_adr.sin_family=AF_INET;  //주소체계 지정.

        serv_adr.sin_addr.s_addr=inet_addr(argv[1]); //IP주소 초기화.

        serv_adr.sin_port=htons(atoi(argv[2])); //PORT번호 초기화.


 

        //서버에 연결 요청

        if(connect(sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr))==-1)

               error_handling("connect() error!");


 

        pid=fork(); //클라이언트1 fork.(입출력 루틴 분할)

        if(pid==0) //자식 프로세스 생성.

                write_routine(sock, buf); //데이터 출력.

        else //부모 프로세스 생성.

               read_routine(sock, buf); //데이터 입력.


 

        close(sock); //소켓 close.

        return 0;

}


 

void read_routine(int sock, char*buf) //데이터 입력에 관련된 코드만 존재.

{

        while(1)

        {

               int str_len=read(sock, buf, BUF_SIZE); //서버에서 데이터 수신.

               if(str_len==0)

                       return;


 

               buf[str_len]=0;

               printf("Message from client#2: %s",buf);

        }

}


 

void write_routine(int sock, char*buf) //데이터 출력에 관련된 코드만 존재.

{

        while(1)

        {

               fgets(buf, BUF_SIZE, stdin); //표준 입력.

               if(!strcmp(buf,"q\n")||!strcmp(buf,"Q\n"))

               {

                       shutdown(sock, SHUT_WR); //EOF 전달을 위해서 shutdown함수 호출.(출력 스트림 종료)

                       return;

               }

               write(sock, buf,strlen(buf)); //서버에 데이터 송신.

        }

}

void error_handling(char*message)

{

        fputs(message, stderr);

        fputc('\n', stderr);

        exit(1);

}
