#include <stdio.h>
#include <stdlib.h>
#include <cups/cups.h>
#include <errno.h>

int main(int argc,char *argv[])
{ 		
	unsigned char	*buf;
	FILE		*fp;	

	int 	sockfd;
	char 	readbuf[1024];
	struct	sockaddr_in 	server_addr;
	struct	hostent		*host;
	int	portnumber,nbytes;

	clock_t start_16, end_16;

	/*
	if(argc==1)
	{
		printf("network pcl://127.0.0.1:10000 \"BayerPcl\" \"BayerPcl\"");
		return(0);
	}
	*/
	char    scheme[255],            /* Scheme in URI */  
                hostname[1024],         /* Hostname */
                username[255],          /* Username info (not used) */
                resource[1024],         /* Resource info (not used) */
		*deviceuri;
	int     port;                   /* Port number */

	if (freopen("/tmp/uri.txt", "wb", stderr)==NULL)
		fprintf(stderr, "error redirecting stderr\n");
	
	fp=fopen("/tmp/bke.txt","wb+");
	
	//ªÒ»°URI
	deviceuri=getenv("DEVICE_URI");
                 fprintf(stderr, "Printer URI: %d\n", deviceuri);
                 httpSeparateURI(HTTP_URI_CODING_ALL, deviceuri, scheme, sizeof(scheme),
                   username, sizeof(username), hostname, sizeof(hostname), &port,
                  resource, sizeof(resource));
      	fprintf(stderr, "URI RESULT:\n  scheme: %s\n    username: %s\n  hostname:%s\n   port:%d\n       resource:%s\n\n",
                                            scheme,         username,       hostname,       port,           resource );



	buf = (unsigned char*)malloc(1024*64);	
	
	if((sockfd=socket(AF_INET,SOCK_STREAM,0))== -1)
	{	
		fprintf(stderr,"Socket Error:%s\a\n",strerror(errno));
		exit(1);
	}

	start_16 = clock();

	bzero(&server_addr,sizeof(server_addr));
	server_addr.sin_family	=AF_INET;
	server_addr.sin_port	=htons(6000);
	server_addr.sin_addr.s_addr=inet_addr("192.168.0.1");

	if(connect(sockfd,(struct sockaddr*)(&server_addr),sizeof(struct sockaddr))==-1)
	{
		fprintf(stderr,"Connect Error:%s\a\n",strerror(errno));
		exit(1);
	}
	
	fprintf(stderr,"before buf\n");	
	fflush(stderr);
	
  	while(!(feof(stdin)))
 	{
		int n=fread(buf,1,1024*64,stdin);

		fwrite(buf,1,n,fp);		

		if((nbytes=send(sockfd,buf,n,0))==-1)
		{
			fprintf(stderr,"Write Error:%s\n",strerror(errno));
			exit(1);
		}
	}
	
	end_16	= clock();
	fprintf(stderr, "Sent to Windows time is:%f\n",(double)(end_16 - start_16) / CLOCKS_PER_SEC);
	
	close(sockfd);
	return(0);
}	
