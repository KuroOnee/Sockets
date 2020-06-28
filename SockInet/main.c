#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>



uint16_t checksumm (struct iphdr ip_h)
{
	char buf[sizeof(ip_h)+1];
	memcpy((void *)buf, (void *)&ip_h, sizeof(ip_h));
	int csum = 0;
	short * ptr;
	ptr = (short*)buf;
	for(int i = 0; i < 10; i++)
	{
		csum = csum + *ptr;
		ptr++;
	}
	uint tmp = 0;
	do
	{
		tmp = csum >> 16;
		csum = (uint16_t)csum + tmp;
	}while (tmp != 0);
	csum = ~csum;
	return (uint16_t)csum;
}

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
	char buf[1024], message[100];
	int errsv, connect_status, recv_check, client_sock;
	if (type == 0)
		client_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (type == 1)
		client_sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (type == 2)
		client_sock = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
	if (type == 3)
		client_sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (client_sock == -1)
		printf("Socket\n");
	struct sockaddr_in server;
	struct sockaddr_ll server_p = {0};
	struct udphdr udp_h;
	struct iphdr ip_h;
	struct ether_header eth_h;
	if (type != 3)
	{
		server.sin_family = AF_INET;
		server.sin_port = htons(port);
		server.sin_addr.s_addr = ip;
	}
	else
	{
		server_p.sll_family = AF_PACKET;
		server_p.sll_protocol = htons(ETH_P_ALL);
		server_p.sll_ifindex = if_nametoindex("enp0s3");
		server_p.sll_halen = ETHER_ADDR_LEN;
		memset(server_p.sll_addr, 0xff, ETHER_ADDR_LEN);
	}
	int len = sizeof(server);
	int len_p = sizeof(server_p);
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
	}
	if (type == 2 || type == 3)
	{
		udp_h.source = htons(port + 1);
		udp_h.dest = htons(port);
		udp_h.len = htons(sizeof(udp_h)+sizeof(message));
		udp_h.check = 0;
		ip_h.version = 4;
		ip_h.ihl = 5;
		ip_h.tot_len = htons(sizeof(ip_h)+sizeof(udp_h)+sizeof(message));
		ip_h.tos = 0;
		ip_h.id = 0;
		ip_h.frag_off = 0;
		ip_h.ttl = htons(64);
		ip_h.protocol = IPPROTO_UDP;
		ip_h.check = 0;
		ip_h.saddr = 0;
		ip_h.daddr = ip;
		ip_h.check = checksumm(ip_h);
		if (type == 3)
		{
			char dest[ETHER_ADDR_LEN] = {0xff,0xff,0xff,0xff,0xff,0xff};
			char src[ETHER_ADDR_LEN] = {0x08,0x00,0x27,0xd0,0x1b,0x4c};
			memcpy(eth_h.ether_dhost, dest, ETHER_ADDR_LEN);
			memcpy(eth_h.ether_shost, src, ETHER_ADDR_LEN);
			eth_h.ether_type = htons(ETHERTYPE_IP);
		}

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
			printf(" connect\n");
			
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
		memcpy((void *)buf, (void *)&ip_h, sizeof(ip_h));
		memcpy((void *)(buf + sizeof(ip_h)), (void *)&udp_h, sizeof(udp_h));
    	memcpy((void *)(buf + sizeof(ip_h) + sizeof(udp_h)), (void *)message, sizeof(message));
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
	if (type == 3)
	{
		memcpy((void *)buf, (void *)&eth_h, sizeof(eth_h));
		memcpy((void *)(buf + sizeof(eth_h)), (void *)&ip_h, sizeof(ip_h));
		memcpy((void *)(buf + sizeof(eth_h) + sizeof(ip_h)), (void *)&udp_h, sizeof(udp_h));
    	memcpy((void *)(buf + sizeof(eth_h) + sizeof(ip_h) + sizeof(udp_h)), (void *)message, sizeof(message));
		if (sendto(client_sock, buf, sizeof(buf), 0, (struct sockaddr *) &server_p, len_p) == -1)
		{
			errsv = errno;
			if (errsv) printf(strerror(errsv));
			printf(" sendto\n");
		}
		do
		{
			if (recvfrom(client_sock, buf, 1024, 0, (struct sockaddr *) &server_p, &len_p) == -1)
			{
				errsv = errno;
				if (errsv) printf(strerror(errsv));
				printf(" recvfrom\n");
			}
			memcpy((void*)buf, (void*)(buf+28), sizeof(message));
		}while (!strcmp(buf, "Hello"));
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
				if (!strcmp(optarg,"packet"))
					type = 3;
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
