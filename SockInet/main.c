#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
void server(int port, int type)
{
	int server_sock = socket(AF_INET, SOCK_STREAM, 0);
	char buf[500];
	int errsv;
	if (server_sock == -1)
		printf("Socket initialize fail!\n");
	struct sockaddr_in server, client;
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = inet_addr("192.168.0.11");
	int len = sizeof(server);
	if (bind(server_sock,(struct sockaddr *) &server,len) == -1)
	{
		errsv = errno;
		if (errsv) printf(strerror(errsv));
		printf(" bind\n");
	}
	listen(server_sock,5);
	int client_sock = accept(server_sock, (struct sockaddr *) &client, &len);
	if (client_sock == -1)
	{
		errsv = errno;
		if (errsv) printf(strerror(errsv));
		printf(" accept\n");
	}
	strcpy (buf, "Hi");
	printf("%s\n",buf );
	if (send(client_sock,buf,500,0) == -1)
	{
		errsv = errno;
		if (errsv) printf(strerror(errsv));
		printf(" send\n");
	}
	int recv_check = recv(client_sock,buf,500,0);
	if (recv_check == -1)
	{
		errsv = errno;
		if (errsv) printf(strerror(errsv));
		printf(" %d\n", recv_check);
		printf("recv\n");
	}
	printf("%s\n", &buf);
	close(client_sock);
	close(server_sock);
	exit(EXIT_SUCCESS);
}

void client(uint32_t ip, int port,int type)
{
	int client_sock = socket(AF_INET, SOCK_STREAM, 0);
	char buf[500];
	int errsv, connect_status, recv_check;
	if (client_sock == -1)
		printf("Socket");
	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = ip;
	int len = sizeof(server);
	connect_status = connect(client_sock,(struct sockaddr *) &server,len);
	if (connect_status == -1)
		{
			errsv = errno;
			if (errsv) printf(strerror(errsv));
			printf(" connect");
			
		}
	recv_check = recv(client_sock, buf,500,0);
	if (recv_check == -1)
	{
		errsv = errno;
		if (errsv) printf("%d", errsv, strerror(errsv));
		printf(" %d\n", recv_check);
		printf("recv\n");
	}
	if (!strcmp(buf, "Hi"))
	{
		printf("%s\n", buf);
		strcpy (buf, "Hello");
		printf("%s\n", buf);
		send(client_sock,(void*)buf,500,0);	
	}
	close(client_sock);
	exit(EXIT_SUCCESS);
}


void main(int argc, char* argv[])
{
	int opt;
	int port = 0;
	int type = 0;
	while ((opt = getopt(argc, argv, "sc:p:")) != -1)
	{
		switch (opt)
		{
			case 'p':
			{
				int tmp = atoi(optarg);
				if (tmp > 1000 && tmp <= 65535)
				{
					port = tmp;
				}
				break;
			}
			case 's':
			{
				printf("Server\n");
				if (port == 0)
				{
					port = 2222;
				}
				server(port, type);
				break;
			}
			case 'c':
			{
				printf("Client\n");
				if (port == 0)
				{
					port = 2222;
				}
				client(inet_addr("192.168.0.11"), port, type);
				break;
			}
			case ':':
			{
				printf("Missing argument\n", optopt );
				break;
			}
			case '?':
			{
				printf("Unrecognized option\n", optopt );
				break;
			}
			default: printf("Unexpected case in switch\n");
		}
	}
	
}