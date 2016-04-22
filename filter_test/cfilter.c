#include <cups/cups.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "unixsocket.h"
#include "psutil.h"


void error_msg(char *str) {
	fprintf(stderr, "ERROR: %s\n", str);
}

void debug_msg(char *str) {
	fprintf(stderr, "DEBUG: %s\n", str);
}

int main(int argc, char *argv[]) {	
	/*
	int fd = open("/tmp/abc", O_RDWR);
	if (fd < 0) {
		char str[100];
		sprintf(str, "%d %d", getuid(), geteuid());
		error_msg(strerror(errno));
		error_msg(str);
		return 1;
	}
	write(fd, "123", 10);
	close(fd);
	*/
	int connfd;
	connfd = unix_socket_conn("/tmp/server.sock");
	if (connfd < 0) {
		char err_info[100];
		sprintf(err_info, "[%d]", connfd);
		error_msg(err_info);
		error_msg("Error when connecting...");
		error_msg(strerror(errno));
		return 1;
	}
	int size;
	char prt_name[128];
	size = recv_msg(connfd, prt_name);
	if (size <= 0) {
		error_msg("recv_msg error");
		return 1;
	}
	unix_socket_close(connfd);
	char str_tm[256];
	time_t tim = time(NULL);
	strftime(str_tm, sizeof(str_tm), "%Y-%m-%d %H:%M:%S", localtime(&tim));
	char buf[4096];
	sprintf(buf, "job id: %s;job title: %s;submit time: %s;print user: %s;copies: %s;output printer:%s;", 
				argv[1], argv[3], str_tm,argv[2], argv[4], prt_name);

	char str[4096];
	while (fgets(str, sizeof(str), stdin) != NULL) {
		char *find_pos;
		if ((find_pos = strstr(str, PS_SHOWPAGE)) != NULL && 
					(find_pos == str || *(find_pos - 1) != '/')) {
			if (find_pos != str) {
				char chr = *find_pos;
				*find_pos = '\0';
				fputs_flush(str, stdout);
				*find_pos = chr;
			}
			inject(buf);
			fputs_flush(find_pos, stdout);
		} else if ((find_pos = strstr(str, PS_SHOWPAGE)) != NULL &&
					(find_pos != str && *(find_pos - 1) == '/')) {
			inject(buf);
			fputs_flush(str, stdout);
		} else { 
			fputs_flush(str, stdout);
		}
	}
	/*
	cups_dest_t *dests;

	int num_dests = cupsGetDests(&dests);

	int i;
	for (i = 0;i < num_dests;i++) {
		cups_dest_t *t = dests + i;
		if (t != NULL) {
			printf("#%d:name = %s, instance=%s\n", i, t->name, t->instance);
		}
	}

	printf("num_dests = %d\n", num_dests);
	cups_dest_t *dest = cupsGetDest(NULL, NULL, num_dests, dests);

	if (dest != NULL) {
		printf("name = %s, instance = %s\n", dest->name, dest->instance);
	}

	cupsFreeDests(num_dests, dests);
*/
	return 0;
}
