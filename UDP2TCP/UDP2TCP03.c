#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#define BUFSIZE 1024
void error_handling(char* message);

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
    udp_serv_addr.sin_port = htons(5001);

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
    tcp_serv_addr.sin_addr.s_addr = inet_addr("192.168.1.12");
    tcp_serv_addr.sin_port = htons(5001);
    
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
            tcp_serv_addr.sin_addr.s_addr = inet_addr("192.168.1.12");
            tcp_serv_addr.sin_port = htons(5001);
            
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

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
