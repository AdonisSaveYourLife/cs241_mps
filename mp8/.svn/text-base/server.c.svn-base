/** @file server.c */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <queue.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>

#include "queue.h"
#include "libdictionary.h"

//#define DEBUG
#ifdef DEBUG
	#define D(x) x
	#define printinfo(x) _print_(info)
	char info[200];
#else
	#define D(x)
	#define printinfo(x)
#endif

const char *HTTP_404_CONTENT = "<html><head><title>404 Not Found</title></head><body><h1>404 Not Found</h1>The requested resource could not be found but may be available again in the future.<div style=\"color: #eeeeee; font-size: 8pt;\">Actually, it probably won't ever be available unless this is showing up because of a bug in your program. :(</div></html>";
const char *HTTP_501_CONTENT = "<html><head><title>501 Not Implemented</title></head><body><h1>501 Not Implemented</h1>The server either does not recognise the request method, or it lacks the ability to fulfill the request.</body></html>";

const char *HTTP_200_STRING = "OK";
const char *HTTP_404_STRING = "Not Found";
const char *HTTP_501_STRING = "Not Implemented";

#define PROCESS_END 1
#define PART_OF_HEAD 1
#define BACKLOG 10
#define MAXBUF	200
#define error_exit(e) 			_error_(e,__FUNCTION__,__LINE__,0)
#define error_noexit(e) 		_error_(e,__FUNCTION__,__LINE__,1)
#define response_501(fd) 		_response_file_(fd,NULL,501,&http_pram)
#define response_404(fd) 		_response_file_(fd,NULL,404,&http_pram)
#define response_200(fd,fn) 	_response_file_(fd,fn,200,&http_pram)
#define MAX(a,b)				((a)>(b) ? (a) : (b))

char* process_http_header_request(const char *request);

void _print_(const char* info)
{
	printf("%s\n",info);
}

void _error_(const char* err, const char* func, int line, int exitflag)
{
	char buf_error[100];
	sprintf(buf_error,"Exception: %s in Function %s at line %d. ",err,func,line);
	perror(buf_error);
	if(exitflag == 0) exit(1);
	else return;
}

typedef struct _connect_node_
{
	int sockfd;
	pthread_t thread_id;
} node_t;

// queue_t node_li; // the linked list of the connected host
// pthread_mutex_t mutex_nodeli;

int kill_signal = 0; // the signal that the process is to be killed.
pthread_mutex_t mutex_signal = PTHREAD_MUTEX_INITIALIZER;
void set_kill(int v)
{
	pthread_mutex_lock(&mutex_signal);
	kill_signal = v;
	pthread_mutex_unlock(&mutex_signal);
}
int get_kill()
{
	int v;
	pthread_mutex_lock(&mutex_signal);
	v = kill_signal;
	pthread_mutex_unlock(&mutex_signal);
	return v;
}

int close_print(int sockfd)
{
	D(printf("close sock: %d\n",sockfd));
	return close(sockfd);
}


int recv_head(int sockfd,char* headbuf,char* buf)
{
	// after the whole head is received, buf is going to be cleared.
	// if the fisrt line is read, headbuf is the copy of buf.
	// only runs once. no block.
	D(printf("line %d, begin to recv head\n",__LINE__));
	int numread=0;
	char* data = buf + strlen(buf);
	char* end = NULL;

	if(data-buf > MAXBUF) error_exit("not enough room for buf");

	if ( (numread = recv(sockfd,data,MAXBUF-(data-buf),0)) == -1)
			error_exit("recv error");
	data[numread] = '\0';

	D(printf("buf: \n%s\n", buf);)
	if( (end=strstr(buf,"\r\n\r\n")) != NULL ) {
		end += strlen("\r\n\r\n");
		memcpy(headbuf,buf,sizeof(char)*(end-buf));
		headbuf[(end-buf)] = '\0';
		memmove(buf,end,strlen(end)+1);
		D(printf("got the end!\n"));
		return 0; // got the end
	}

	if( strstr(buf,"\r\n") != NULL)
	{
		strcpy(headbuf,buf);
		D(printf("Got the head\n");)
		return PART_OF_HEAD; // only gets part of the head.
	}
		
	return -1;

}

char* parse_http(char* headbuf, dictionary_t* d)
{
	char* filename = NULL;
	char *line=NULL, *nextline=NULL;
	int ret=0;

	line = headbuf;
	nextline = strstr(line,"\r\n");
	if(nextline == NULL) {
		error_exit("the first line has not yet been read");
	}
	*nextline = '\0';
	nextline += strlen("\r\n");
	filename = process_http_header_request(line);

	/* store the lines into the dictionary */
	while( strcmp(nextline,"\r\n")!=0 && strstr(nextline,"\r\n")!=NULL )
	{
		line 	  = nextline;
		nextline  = strstr(line,"\r\n");
		*nextline = '\0';
		nextline += strlen("\r\n");
		ret = dictionary_parse(d,line);
		if(ret==KEY_EXISTS)
		{
			error_exit("key already exits");
		}else if (ret == ILLEGAL_FORMAT) error_exit("illegal format");

	}

	return filename;

}

int is_string_end(const char* string, const char* end)
{
	unsigned int length = strlen(string);
	if(length >= strlen(end) && strcmp(string+length-strlen(end),end)==0)
		return 1;

	return 0;
}

// return keepalive
int _response_file_(int sockfd, const char* filename, int response_code,dictionary_t* d)
{
	int keepalive = 0;

	char contenttype[20];
	contenttype[0] = '\0';

	char line[100]; line[0]= '\0';

	const char* response_code_string=NULL;
	switch(response_code){
		case 200:
			response_code_string = HTTP_200_STRING;
			break;
		case 501:
			response_code_string = HTTP_501_STRING;
			break;
		case 404:
			response_code_string = HTTP_404_STRING;
			break;
		default:
			error_exit("unknown response type");
			break;
	}

	if(response_code == 501 || response_code == 404)
	{
		strcpy(contenttype,"text/html");
	}else if(response_code==200)
	{

		if( is_string_end(filename,".html") )
		{
			strcpy(contenttype,"text/html");
		}
		else if( is_string_end(filename,".css") )
		{
			strcpy(contenttype,"text/css");
		}
		else if( is_string_end(filename,".jpg") )
		{
			strcpy(contenttype,"image/jpeg");
		}
		else if( is_string_end(filename,".png") )
		{
			strcpy(contenttype,"image/png");
		}
		else strcpy(contenttype,"text/plain");
	}
	else error_exit("unknown response type");

	// content length
	int contentLength = 0;
	struct stat st;
	switch(response_code){
		case 200:
   	 		stat(filename, &st);
    		contentLength = st.st_size;
			break;
		case 501:
			contentLength = strlen(HTTP_501_STRING);
			break;
		case 404:
			contentLength = strlen(HTTP_404_STRING);
			break;
		default:
			error_exit("unknown response type");
			break;
	}

	char *responsebuf = malloc(contentLength + 200); responsebuf[0]='\0';


	sprintf(line, "HTTP/1.1 %d %s\r\n", response_code, response_code_string);
	strcpy(responsebuf,line);

	sprintf(line, "Content-Type: %s\r\n",contenttype);
	strcat(responsebuf,line);

	sprintf(line, "Content-Length: %d\r\n",contentLength );
	strcat(responsebuf,line);

	if(
		dictionary_get(d,"Connection") != NULL
		&& strcasecmp( dictionary_get(d,"Connection"),"Keep-Alive")==0 )
	{
		strcat(responsebuf,"Connection: Keep-Alive\r\n");
		keepalive = 1;
	}
	else{
		strcat(responsebuf,"Connection: close\r\n");
		keepalive = 0;
	}
	strcat(responsebuf,"\r\n");
	/* end of the file head */

	/* contents */
	int responsebufLength = 0, readLen=0;
	FILE *pfile;
	switch(response_code){
		case 200:
			pfile = fopen(filename,"rb");
			readLen = 0;
			responsebufLength = strlen(responsebuf);
			while( (readLen = fread(responsebuf+responsebufLength, 
				1, contentLength, pfile)) > 0 )
			{
				responsebufLength += readLen;
			}
			responsebuf[responsebufLength] = '\0';
			break;
		case 501:
			strcat(responsebuf,HTTP_501_STRING);
			responsebufLength = strlen(responsebuf);
			break;
		case 404:
			strcat(responsebuf,HTTP_404_STRING);
			responsebufLength = strlen(responsebuf);
			break;
		default:
			error_exit("unknown response type");
			break;
	}

	// send to sockfd.
	int bytes_sent = send(sockfd, responsebuf, responsebufLength, 0);
	if(bytes_sent < responsebufLength) error_exit("send error");

	free(responsebuf);
	return keepalive;

}

void* worker_thread(void* atg)
{
	node_t * node_ptr = (node_t*) atg;
	int sockfd = node_ptr -> sockfd;

	D(printf("new thread build: thread_id:%u sockfd: %d\n",node_ptr->thread_id,sockfd));

	char newfilename[MAXBUF +10]; newfilename[0] = '\0';
	char buf[MAXBUF+10]; buf[0] = '\0';
	char headbuf[MAXBUF+10]; headbuf[0] = '\0';

	dictionary_t http_pram;

	int keepalive = 0;

	// select no wait
	int retval;
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(sockfd,&rfds);
    int nfds = MAX(0,sockfd) +1;

    D(printf("line %d, begin to select in thread %u\n",__LINE__,node_ptr->thread_id));
    while( get_kill()!=PROCESS_END
    	&& (retval = select(nfds, &rfds, NULL, NULL, &tv))!= -1 )
    {
    	D(printf("line %d, retval: %d, I am reading, buf: %s\n",__LINE__,retval,buf));

    	if(retval >0 && FD_ISSET(sockfd,&rfds) )
    	{
    		int ret = recv_head(sockfd,headbuf,buf);
    		if(ret==0 || (ret==PART_OF_HEAD) )
    		{
	    		dictionary_init(&http_pram);
				char* filename = parse_http(headbuf,&http_pram);
				D(printf("line %d, begin to send, filename is %s\n",__LINE__,filename));
				if(get_kill() != PROCESS_END)
				{
					if(filename == NULL){
						D(printf("line %d, send 501\n",__LINE__));
						keepalive = response_501(sockfd);
					}
					else
					{
						if(strcmp(filename,"/")==0)
							sprintf(newfilename,"web/index.html");
						else{
							sprintf(newfilename,"web%s",filename);
						}
						if(access(newfilename,R_OK)!=0){
							D(printf("line %d, send 404\n",__LINE__));
							keepalive = response_404(sockfd);
						}
						else {
							D(printf("line %d, send 200, filename: %s\n",__LINE__,newfilename));
							keepalive = response_200(sockfd,newfilename);
						}
					}
				}
				if(filename != NULL) free(filename);
				dictionary_destroy(&http_pram);
			}
    	}
    	FD_ZERO(&rfds);
    	FD_SET(sockfd,&rfds);
    	nfds = MAX(0,sockfd) +1;
    	tv.tv_sec = 1;
    	tv.tv_usec = 0;
    	if(!keepalive) break;
    }
    if(retval == -1 && get_kill()!=PROCESS_END) error_exit("select");

	close_print(sockfd);
	// remove the node from list, or not. it seems to be does not matter that much.
	return NULL;
}


/* open and bind and listen to a tcp socket */
int openTCPListener(const char* port)
{
	int status;
	int sockfd;
	struct addrinfo hints;
	struct addrinfo *servinfo, *p; // point to the results
	memset(&hints, 0, sizeof hints); // empty struct
	hints.ai_family = AF_UNSPEC; // IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
	hints.ai_flags = AI_PASSIVE; // fill in my IP for me
	if ((status = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
		exit(1);
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) 
		{
			perror("listener: socket");
			continue;
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close_print(sockfd);
			perror("listener: bind");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "listener: failed to bind socket at line %d.\n",__LINE__);
		exit(1);
	}

	// servinfo now points to a linked list of 1 or more struct addrinfos
	// ... do everything until you don't need servinfo anymore ....
	freeaddrinfo(servinfo); // free the linked-list

	if (listen(sockfd, BACKLOG) == -1) {
		error_exit("listener error");
	}

	printf("listener in port: %s with sockfd: %d, waiting to recvfrom...\n", port,sockfd);
	return sockfd;
	
}


int acceptConnect(int sockfd)
{
	struct sockaddr_in their_addr; /* connector addr */
	socklen_t sin_size;
	int new_fd;
	queue_t node_list;
	queue_init(&node_list);

	// select no wait
	int retval;
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(sockfd,&rfds);
    int nfds = MAX(0,sockfd)+1;

    while( get_kill()!=PROCESS_END
    	&& (retval = select(nfds, &rfds, NULL, NULL, &tv))!= -1 )
    {
    	if(retval > 0 && FD_ISSET(sockfd,&rfds))
    	{
    		if(get_kill()==PROCESS_END) break;
    		sin_size = sizeof(struct sockaddr_in);
			/* block wait*/
			if ( (new_fd = accept(sockfd, (struct sockaddr*)&their_addr, &sin_size)) == -1 )
			{
				error_noexit("accept error");
				continue;
			}
			D(printf("line %d, new request accepted!\n",__LINE__));
			node_t * new_node = (node_t*)malloc(sizeof(node_t));
			new_node -> sockfd  = new_fd;
			if ( pthread_create(&new_node->thread_id,NULL,
					&worker_thread,new_node) != 0 )
				error_exit("pthread_create error");
			queue_enqueue(&node_list,new_node);
    	}

    	FD_ZERO(&rfds);
    	FD_SET(sockfd,&rfds);
    	tv.tv_sec = 1;
    	tv.tv_usec = 0;
    }

    if(retval == -1 && get_kill()!=PROCESS_END) error_exit("select");


	void * item;
	unsigned int i;
	for(i=0;i<queue_size(&node_list);i++)
	{
		item = queue_at(&node_list,i);
		if( pthread_join(((node_t*)item)->thread_id,NULL) !=0 )
			error_exit("pthread_join");
	}
	while( (item=queue_dequeue(&node_list))!=NULL ) free(item);
	queue_destroy(&node_list);
	return 0;
}


/**
 * Processes the request line of the HTTP header.
 * 
 * @param request The request line of the HTTP header.  This should be
 *                the first line of an HTTP request header and must
 *                NOT include the HTTP line terminator ("\r\n").
 *
 * @return The filename of the requested document or NULL if the
 *         request is not supported by the server.  If a filename
 *         is returned, the string must be free'd by a call to free().
 */
char* process_http_header_request(const char *request)
{
	// Ensure our request type is correct...
	if (strncmp(request, "GET ", 4) != 0)
		return NULL;

	// Ensure the function was called properly...
	assert( strstr(request, "\r") == NULL );
	assert( strstr(request, "\n") == NULL );

	// Find the length, minus "GET "(4) and " HTTP/1.1"(9)...
	int len = strlen(request) - 4 - 9;

	// Copy the filename portion to our new string...
	char *filename = malloc(len + 1);
	strncpy(filename, request + 4, len);
	filename[len] = '\0';

	// Prevent a directory attack...
	//  (You don't want someone to go to http://server:1234/../server.c to view your source code.)
	if (strstr(filename, ".."))
	{
		free(filename);
		return NULL;
	}

	return filename;
}

void handle(int sig) 
{
	set_kill(PROCESS_END);
}


int main(int argc, char **argv)
{
	struct sigaction sa;
	sa.sa_handler = handle; /* the handler function!! */
	sa.sa_flags = 0;
	if( sigemptyset(&sa.sa_mask) !=0 ) /* block all signals during handler */
		error_exit("sigemptyset");
	if( sigaction(SIGINT, &sa, NULL) != 0)
		error_exit("sigaction");

	if(get_kill()==PROCESS_END) return 1;

	if(argc != 2){
		printf("Usage: %s <port#>\n",argv[0]);
		exit(1);
	}

	char * port = argv[1];
	int sockfd = openTCPListener(port);
	if(sockfd < 0 ) error_exit("openTCPListener");

	acceptConnect(sockfd);

	if(close_print(sockfd) == -1) error_exit("close sockfd");

	return 0;
}
