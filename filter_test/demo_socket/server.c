#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <stddef.h>
#include <string.h>

/*
int main() {
	int listenfd, connfd;
	listenfd = unix_socket_listen("/tmp/server.sock");
	if (listenfd < 0) {
		printf("Error[%d] when listening.. \n", errno);
		return 1;
	}
	printf("Finished listening..\n");
	uid_t uid;
	connfd = unix_socket_accept(listenfd, &uid);
	unix_socket_close(listenfd);
	if (connfd < 0) {
		printf("Error[%d] when accepting...\n", errno);
		return 1;
	}
	printf("Begin to recv/send...\n");
	int i, n, size;
	char rvbuf[2048];
	for (i = 0;i < 2;i ++) {
		size = recv(connfd, rvbuf, 804, 0);
		if (size >= 0) {
			printf("Recieved Data[%d]:%c...%c\n", size, rvbuf[0], rvbuf[size - 1]);
		}
		if (size == -1) {
			printf("Error[%d] when recieving Data:%s.\n", errno, strerror(errno));
			break;
		}
		sleep(30);
	}
	unix_socket_close(connfd);
	printf("Server exited.\n");
	return 0;
}
*/
