#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

void server(int port, int type)
{
	char buf[1024], message[500];
	int errsv, server_sock, client_sock;
	if (type == 0)
		server_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (type == 1)
		server_sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (server_sock == -1)
		printf("Socket initialize fail!\n");
	struct sockaddr_in server, client;
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = inet_addr("192.168.0.11");
	int len = sizeof(server);
	if (bind(server_sock, (struct sockaddr *) &server, len) == -1)
	{
		errsv = errno;
		if (errsv) printf(strerror(errsv));
		printf(" bind\n");
	}
	strcpy (message, "Hello");
	while (1)
	{
		if (type == 0)
		{
			listen(server_sock,5);
			client_sock = accept(server_sock, (struct sockaddr *) &client, &len);
			if (client_sock == -1)
			{
				errsv = errno;
				if (errsv) printf(strerror(errsv));
				printf(" accept\n");
			}
			if (recv(client_sock, buf,1024,0) == -1)
			{
				errsv = errno;
				if (errsv) printf("%d", errsv, strerror(errsv));
				printf(" recv\n");
			}
			if (!strcmp(buf, "Hi"))
			{
				printf("%s\n", buf);
				memcpy((void *)buf, (void *)message, sizeof(message));
				if (send(client_sock,(void*)buf,1024,0) == -1)
				{
					errsv = errno;
					if (errsv) printf(strerror(errsv));
					printf(" send\n");
				}
			}
		}
		if (type == 1)
		{
			if (recvfrom(server_sock, buf, 1024, 0, (struct sockaddr *) &client, &len) == -1)
			{
				errsv = errno;
				if (errsv) printf(strerror(errsv));
				printf(" recvfrom\n");
			}
			if (!strcmp(buf, "Hi"))
			{
				printf("%s\n", buf);
				memcpy((void *)buf, (void *)message, sizeof(message));
				if (sendto(server_sock, buf, 1024, 0, (struct sockaddr *) &client, len) == -1)
				{
					errsv = errno;
					if (errsv) printf(strerror(errsv));
					printf(" sendto\n");
				}
			}
		}
	}
	close(client_sock);
	close(server_sock);
	exit(EXIT_SUCCESS);
}

void client(uint32_t ip, int port, int type)
{
	char buf[1024], message[500];
	int errsv, connect_status, recv_check, client_sock;
	if (type == 0)
		client_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (type == 1)
		client_sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (type == 2)
		client_sock = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
	if (client_sock == -1)
		printf("Socket");
	struct sockaddr_in server;
	struct udphdr udph;
	struct iphdr iph;
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = ip;
	int len = sizeof(server);
	int on = 1;
	if (type == 2)
	{
		if (setsockopt(client_sock, SOL_IP, IP_HDRINCL,  &on, sizeof(int)) == -1)
		{
			errsv = errno;
			if (errsv) printf(strerror(errsv));
			printf(" HDRINCL\n");
		}
		else
		{
			printf("Succes\n");
		}
		udph.source = htons(port + 1);
		udph.dest = htons(port);
		udph.len = htons(sizeof(udph)+sizeof(message));
		udph.check = 0;
		iph.version = 4;
		iph.ihl = 5;
		iph.tos = 0;
		iph.id = 0;
		iph.frag_off = 0;
		iph.ttl = htons(64);
		iph.protocol = IPPROTO_UDP;
		iph.saddr = 0;
		iph.daddr = ip;
	}
	strcpy (message, "Hi");
	if (type == 0)
	{
		memcpy((void *)buf, (void *)message, sizeof(message));
		connect_status = connect(client_sock,(struct sockaddr *) &server,len);
		if (connect_status == -1)
		{
			errsv = errno;
			if (errsv) printf(strerror(errsv));
			printf(" connect");
			
		}
		if (send(client_sock, buf, 1024, 0) == -1)
		{
			errsv = errno;
			if (errsv) printf(strerror(errsv));
			printf(" send\n");
		}
		if (recv(client_sock, buf, 1024, 0) == -1)
		{
			errsv = errno;
			if (errsv) printf(strerror(errsv));
			printf(" recv\n");
		}
	}
	if (type == 1)
	{
		memcpy((void *)buf, (void *)message, sizeof(message));
		if (sendto(client_sock, buf, 1024, 0, (struct sockaddr *) &server, len) == -1)
		{
			errsv = errno;
			if (errsv) printf(strerror(errsv));
			printf(" sendto\n");
		}
		if (recvfrom(client_sock, buf, 1024, 0, (struct sockaddr *) &server, &len) == -1)
		{
			errsv = errno;
			if (errsv) printf(strerror(errsv));
			printf(" recvfrom\n");
		}
	}
	if (type == 2)
	{
		memcpy((void *)buf, (void *)&iph, sizeof(iph));
		memcpy((void *)(buf + sizeof(iph)), (void *)&udph, sizeof(udph));
    	memcpy((void *)(buf + sizeof(iph) + sizeof(udph)), (void *)message, sizeof(message));
		if (sendto(client_sock, buf, 1024, 0, (struct sockaddr *) &server, len) == -1)
		{
			errsv = errno;
			if (errsv) printf(strerror(errsv));
			printf(" sendto\n");
		}
		do
		{
			if (recvfrom(client_sock, buf, 1024, 0, (struct sockaddr *) &server, &len) == -1)
			{
				errsv = errno;
				if (errsv) printf(strerror(errsv));
				printf(" recvfrom\n");
			}
			memcpy((void*)buf, (void*)(buf+28), sizeof(message));
		}while (strcmp(buf, "Hello"));
	}
	printf("msg %s\n", &buf);	
	close(client_sock);
	exit(EXIT_SUCCESS);
}


void main(int argc, char* argv[])
{
	int opt;
	int port = 0;
	int type = 0;
	uint32_t ip = 0;
	while ((opt = getopt(argc, argv, "sc:p:t:")) != -1)
	{
		switch (opt)
		{
			case 't':
			{
				if (!strcmp(optarg,"tcp"))
					type = 0;
				if (!strcmp(optarg,"udp"))
					type = 1;
				if (!strcmp(optarg,"raw"))
					type = 2;
				break;
			}
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
				//ip = inet_addr(optarg);
				ip = inet_addr("192.168.0.11");
				client(ip, port, type);
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