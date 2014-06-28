/*
 ============================================================================
 Name        : http.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description :
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <stdio.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>


#include <string.h>
#include <time.h>

#include "errlib.h"
#include "sockwrap.h"


#define LISTENQ 15
#define MAXTEMPL 16384
#define MAXBUFL 500
#define MAXHEADERL 1024
#define MAXHEADERA 256

char *prog_name;


int body_length(char** header)
{
	int result = -1;
	int i = 0;

	while (header[i] != NULL)
	{
		if (strcmp(header[i],"Content-Length") == 0)
		{
			result = atoi(header[i+1]);
			break;
		}
		else
		{
			i++;
			i++;
		}

	}
	return result;
}


int connection_close(char** header)
{
	int result = 0;
	int i = 0;

	while (header[i] != NULL)
	{
		if (strcmp(header[i],"Connection") == 0)
		{
			if (strcmp (header[i+1], "close") == 0)
			{
				result = 1;
				break;
			}
		}
		else
		{
			i++;
			i++;
		}

	}
	return result;
}


char** parse_header(char* header)
{

	char** args;

	int i = 0, j = 0, k = 0;
	int header_len;
	int arg_num;

	char* first;
	char* second;

	while (header[i++]!='\0');
	header_len = i;

	arg_num = 0;
	i = 0;
	while (i < header_len)
	{
		if (header[i] == '\n')
		{
			arg_num++;
		}
		i++;
	}


	args = (char**) malloc(sizeof(char*) * arg_num * 2 + 1);



	i = 0;
	j = 0;
	k = 0;
	first = (char*)malloc(MAXHEADERA);
	second = (char*)malloc(MAXHEADERA);

	while (i<header_len)
	{
		j = 0;

		while (header[i] != ':' && i < header_len)
		{
			first[j] = header[i];
			i++;
			j++;
		}
		i++; // pass the  :
		first[j] = '\0';

		j = 0;

		while(header[i] != '\r' && header[i] != '\n' && i < header_len)
		{
			second[j] = header[i];
			i++;
			j++;
		}
		second[j] = '\0';
		i++;
		i++;


		args[k] = (char*) malloc(strlen(first));
		strcpy(args[k],first);
		k++;
		args[k] = (char*) malloc(strlen(second));
		strcpy(args[k],second);
		k++;
	}

	args[k] = '\0';

	/*
	int f;

	for (f = 0; f < arg_num*2; f+=2)
		printf("%s is %s\n" , args[f] , args[f+1]);
*/

//	free(first);
//	free(second);

return args;


}



int start (int connfd)
{


	struct timeval tv;

	tv.tv_sec = 15;  /* 15 Secs Timeout */
	tv.tv_usec = 0;

	setsockopt(connfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval));


	char buf[MAXBUFL+1];
	int nread;

	do
	{
		if ((nread = read(connfd, buf, MAXBUFL)) <= 0 )
					return 0;

		buf[nread] = '\0';

		//printf(buf);

		int i = 0, j = 0, k = 0;
		char temp[MAXTEMPL+1];

		// split the start line here.



		// discard \r\n begining
		while (i < nread && buf[i] != '\0')
		{
			if (buf[i] == '\r' || buf[i] == '\n') i++;
			else break;
		}
		// take out the start line
		while (i < nread && buf[i] != '\0' && buf[i] != '\r' && buf[i] != '\n')
		{
			temp[j++] = buf[i++];
		}
		temp[j] = '\0';

		char* start_line = (char*) malloc(j+1);
		k = 0;

		while (k < j+1)
		{
			start_line[k] = temp[k];
			k++;
		}

		//printf("here is the start line: %s\n", start_line);

		// discard \r\n beginning
		while (i < nread && buf[i] != '\0')
		{
			if (buf[i] == '\r' || buf[i] == '\n') i++;
			else break;
		}

		j = 0; k = 0;

		// cut out the header text
		while (i<nread && !(buf[i-3] == '\r' && buf[i-2] == '\n' && buf[i-1] == '\r' && buf[i] == '\n'))
		{
			temp[j++] = buf[i++];
		}
		temp[j] = '\0';


		char* header_text = (char*) malloc(j+1);

		while (k < j+1)
		{
			header_text[k] = temp[k];
			k++;
		}

	//	printf("here is the header text: %s\n", header_text);

		char** header_args = parse_header(header_text);
		char* body;
		int body_len = body_length(header_args);

		if (body_len == -1)
		{
			// no body
			body = "\0";
		}
		else
		{
			//hala dige...

			k = 0;
			do
			{
				while (i < nread && k < body_len)
				{
					temp[k++] = buf[i++];
				}

				if (k >= body_len)
				{
					temp[k] = '\0';
					break;
				}
				else
				{
					i = 0;
					if ((nread = read(connfd, buf, MAXBUFL)) <= 0 )
						break;

				}
			}while(1);

			body = (char*) malloc (k + 1);
			j = 0;
			while (j<=body_len)
			{
				body[j] = temp[j];
				j++;
			}

			printf(body);
		}



		char* result = doRequest(start_line,header_args,body);
		//printf(result);
		int len = 0;
		while(result[len] != '\0') len++;

		write(connfd,result,len);

		if (body_len != -1 && !connection_close(header_args))
		{
			free(result);

			continue;
		}
		else
		{
			free(result);
			break;
		}

	}while(1);
	return 0;
}





int main(int argc, char* argv[])
{
	int listenfd, connfd, err=0;
	short port;
	struct sockaddr_in servaddr, cliaddr;
	socklen_t cliaddrlen = sizeof(cliaddr);



	prog_name = argv[0];

	if(argc < 2)
	{
		printf("Usage: ./prog_name [portnumber]");
		return 0;
	}

	port=atoi(argv[1]);



	listenfd = Socket(AF_INET, SOCK_STREAM, 0);


	/* specify address to bind to */
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);
	servaddr.sin_addr.s_addr = htonl (INADDR_ANY);

	Bind(listenfd, (SA*) &servaddr, sizeof(servaddr));

	Listen(listenfd, LISTENQ);


	int pid;
	signal(SIGCHLD,SIG_IGN);

	while (1) {

		connfd = Accept (listenfd, (SA*) &cliaddr, &cliaddrlen);

		if ((pid = fork()) == -1)
		{
			close(connfd);
			continue;
		}
		else if(pid > 0)
		{
			close(connfd);
			continue;
		}
		else if(pid == 0)
		{
			err = start(connfd);
			close(connfd);
		}
		close(connfd);
	}

	return EXIT_SUCCESS;
}
