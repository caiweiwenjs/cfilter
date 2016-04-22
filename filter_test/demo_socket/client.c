#include <stdio.h>
#include <unistd.h>
#include <stddef.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <string.h>
#include "unixsocket.h"


int main() {
	int connfd;
	connfd = unix_socket_conn("/tmp/server.sock");
	if (connfd < 0) {
		printf("Error[%d] when connecting...\n", errno);
		return 1;
	}
	printf("Begin to recv/send...\n");
	int i, n, size;
	char buf[4096];
	size = recv_msg(connfd, buf);
	if (size <= 0) {
		printf("recv_msg error\n");
		printf("size = %d\n", size);
		for (int i = 0;i < size;i++)
			putchar(buf[i]);
		puts("");
	} else {
		printf("%s\n", buf);
	}
	/*
	for (i = 0;i < 10;i++) {
		memset(rvbuf, 'a', 2048);
		rvbuf[2047] = 'b';
		size = send(connfd, rvbuf, 2048, 0);
		if (size >= 0) {
			printf("Data[%d] Sent:%c.\n", size, rvbuf[0]);
		}
		if (size == -1) {
			printf("Error[%d] when Sending Data:%s.\n", errno, strerror(errno));
			break;
		}
		sleep(1);
	}*/
	unix_socket_close(connfd);
	printf("Client exited.\n");

	return 0;
}
