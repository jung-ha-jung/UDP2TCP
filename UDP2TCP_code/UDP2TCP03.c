#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#define BUFSIZE 1024
void error_handling(char* message);
void get_network_info(char* cpRemoteTcpIp, uint16_t *uspPort);
int Find(char txt[], char cFind);
int ReverseFind(char txt[], char cFind);


/* --------------------------------------------------------
인자 :
리턴 :

-------------------------------------------------------- */
int main(int argc, char **argv)
{
	// 
	// UDP initialization part
	//
    int udp_serv_sock;
    char message[BUFSIZE];
    struct sockaddr_in udp_serv_addr;
    struct sockaddr_in udp_clnt_addr;
    unsigned int udp_clnt_addr_size;
    char caRemoteTcpIp[16] = "192.168.1.12";
    // char caLocalIp[16] = "192.168.1.11";
    uint16_t  usPort = 5001;
    int iRet;

    // system("sudo setcap 'cap_net_bind_service=+ep' UDP2TCP03"); // 1024 미만의 포트 사용

    get_network_info(caRemoteTcpIp, &usPort);
    printf("%s, %d\n", caRemoteTcpIp, usPort);
   
    udp_serv_sock = socket(PF_INET, SOCK_DGRAM, 0);
    if(udp_serv_sock == -1)
    {
        error_handling("socket() error");
    }
    else
    {
        printf("udp_serv_sock : %d\n", udp_serv_sock);
    }

    memset(&udp_serv_addr, 0, sizeof(udp_serv_addr));
    udp_serv_addr.sin_family = AF_INET;
    udp_serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);  // Set as local IP address
    udp_serv_addr.sin_port = htons(usPort);

    if(bind(udp_serv_sock, (struct sockaddr*) &udp_serv_addr, sizeof(udp_serv_addr)) == -1 )
    {
        error_handling("bind() error");
    }
    
    
    udp_clnt_addr_size = sizeof(udp_clnt_addr);
        
    
    //
	// TCP initialization part
	//
	int tcp_clnt_sock;
    struct sockaddr_in tcp_serv_addr;

    tcp_clnt_sock = socket(PF_INET, SOCK_STREAM, 0);
    if(tcp_clnt_sock == -1)
    {
        error_handling("TCP socket() error");
    }
    else
    {
        printf("tcp_clnt_sock : %d\n", tcp_clnt_sock);
    }

    memset(&tcp_serv_addr, 0, sizeof(tcp_serv_addr));
    tcp_serv_addr.sin_family = AF_INET;
    tcp_serv_addr.sin_addr.s_addr = inet_addr(caRemoteTcpIp);
    tcp_serv_addr.sin_port = htons(usPort);
    
    while(connect(tcp_clnt_sock, (struct sockaddr*)&tcp_serv_addr, sizeof(tcp_serv_addr)) == -1)
    {
        usleep(500000);     // Sleep for 0.5 sec to save CPU resource
    }
    printf("TCP Socket is    Connected\n");



	//
	// Trans receiver converter
	//
    while(1)
    {
        // 
        // UDP Recevie, TCP Send
        //
        memset(message, 0, BUFSIZE);    // Clear buffer before receiving from UDP client.
    
        // Receive from UDP
        int recv_udp_len = recvfrom(udp_serv_sock, message, BUFSIZE, MSG_DONTWAIT, (struct sockaddr*) &udp_clnt_addr, &udp_clnt_addr_size);
        if(recv_udp_len > 0)
        {
             // Send to TCP
             send(tcp_clnt_sock, message, recv_udp_len, MSG_NOSIGNAL);
             printf("UDP to TCP Message Length : %d\n", (int)(recv_udp_len));
        }
        
        
        // 
        // TCP Receive, UDP Send
        //
        memset(message, 0, BUFSIZE);    // Clear buffer before receiving from TCP server.
        
        // Receive from TCP
        int recv_tcp_len = recv(tcp_clnt_sock, message, BUFSIZE, MSG_DONTWAIT); // MSG_DONTWAIT
        if(recv_tcp_len > 0)
        {
            // Send to UDP
            sendto(udp_serv_sock, message, recv_tcp_len, 0, (struct sockaddr*) &udp_clnt_addr, sizeof(udp_clnt_addr));
            printf("TCP to UDP Message Length : %d\n", (int)(recv_tcp_len));
        }
        else if(recv_tcp_len == 0)      // If TCP server socket is closed and disconnected, try to connect again.
        {
            close(tcp_clnt_sock);
            printf("TCP socket is disconnected\n");
              
            // Connect again            
            tcp_clnt_sock = socket(PF_INET, SOCK_STREAM, 0);
            
            memset(&tcp_serv_addr, 0, sizeof(tcp_serv_addr));
            tcp_serv_addr.sin_family = AF_INET;
            tcp_serv_addr.sin_addr.s_addr = inet_addr(caRemoteTcpIp);
            tcp_serv_addr.sin_port = htons(usPort);
            
            while(connect(tcp_clnt_sock, (struct sockaddr*)&tcp_serv_addr, sizeof(tcp_serv_addr)) == -1)
            {
                usleep(500000);     // Sleep for 0.5 sec to save CPU resource
            }
            
            printf("TCP socket is Re-connected\n");
        }
       
        usleep(500);      // Sleep for 0.0005 sec (0.5 msce) to save CPU resource (2000Hz Send and Receive Rate)
    }

    close(tcp_clnt_sock);
    close(udp_serv_sock);


	return 0;
}


/* --------------------------------------------------------
인자 :
리턴 :

-------------------------------------------------------- */
void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}


/* --------------------------------------------------------
인자 :
리턴 :

-------------------------------------------------------- */
void get_network_info(char* cpRemoteTcpIp, uint16_t *uspPort)
{
    char caFName[40] = "/home/pi/UDP2TCP/setting.txt";
    // char caFName[20] = "setting.txt";
    char caStr[128], caTmp[16];
    int iLeft, iRight;

	FILE* pFile = fopen(caFName, "r");
	if(pFile == NULL) return;

    while ( fgets(caStr, sizeof(caStr), pFile) != NULL ) {
        memset(caTmp, 0, sizeof(caTmp)); // memory clear
        if( strncmp(caStr, "remote ip = ", (sizeof("remote ip = ")-1)) == 0 ) {
            iLeft = Find(caStr, '"') + 1;
            iRight = ReverseFind(caStr, '"');
            strncpy(caTmp, &caStr[iLeft], (iRight-iLeft));
            sprintf(cpRemoteTcpIp, "%s", caTmp);
        }
       else if( strncmp(caStr, "port = ", (sizeof("port = ")-1)) == 0 ) {
            iLeft = Find(caStr, '[') + 1;
            iRight = ReverseFind(caStr, ']');
            strncpy(caTmp, &caStr[iLeft], (iRight-iLeft));
            *uspPort = atoi(caTmp);
        }
    }

    fclose( pFile );
}


/* --------------------------------------------------------
인자 :
리턴 :

-------------------------------------------------------- */
int Find(char txt[], char cFind)
{
	int i, len = strlen(txt);

	for(i=0; i<len; i++)
	{
		if(txt[i] == cFind)
			return i;
	}

	return -1;
}


/* --------------------------------------------------------
인자 :
리턴 :

-------------------------------------------------------- */
int ReverseFind(char txt[], char cFind)
{
	int i, len = strlen(txt);

	for(i=(len-1); i>=0; i--)
	{
		if(txt[i] == cFind)
			return i;
	}

	return -1;
}
