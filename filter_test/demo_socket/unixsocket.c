#include "unixsocket.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <stddef.h>
#include <string.h>

//for server
int unix_socket_listen(const  char *servername){
	int fd;
	struct sockaddr_un un;
	if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
		return (-1);
	}
	int rval;
	unlink(servername);
	memset(&un, 0, sizeof(un));
	un.sun_family = AF_UNIX;
	strcpy(un.sun_path, servername);
	if (bind(fd, (struct sockaddr *)&un, sizeof(struct sockaddr_un)) < 0) {
		rval = -2;
	} else {
		if (listen(fd, MAX_CONNECTION_NUMBER) < 0) {
			rval = -3;
		} else {
			return fd;
		}
	}
	int err;
	err = errno;
	close(fd);
	errno = err;
	return rval;
}

int unix_socket_accept(int listenfd, uid_t *uidptr) {
	int clifd, rval;
	struct sockaddr_un un;
	struct stat statbuf;
	int len;
	len = sizeof(un);
	if ((clifd = accept(listenfd, (struct sockaddr *)&un, &len)) < 0) {\
		return (-1);
	}
	if (stat(un.sun_path, &statbuf) < 0) {
		rval = -2;
	} else {
		if (S_ISSOCK(statbuf.st_mode)) {
			if (uidptr != NULL) *uidptr = statbuf.st_uid;
			unlink(un.sun_path);
			return clifd;
		} else {
			rval = -3;
		}
	}
	int err;
	err = errno;
	close(clifd);
	errno = err;
}

//for client
int unix_socket_conn(const char *servername) {
	int fd;
	if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
		return (-1);
	}
	int rval;
	struct sockaddr_un un;
	struct sockaddr_un un_server;
	memset(&un, 0, sizeof(un));
	un.sun_family = AF_UNIX;
	sprintf(un.sun_path, "/tmp/client%05d.sock", getpid());
	unlink(un.sun_path);
	if (bind(fd, (struct sockaddr *)&un, sizeof(struct sockaddr_un)) < 0) {
		rval = -2;
	} else {
		memset(&un_server, 0, sizeof(un_server));
		un_server.sun_family = AF_UNIX;
		strcpy(un_server.sun_path, servername);
		if (connect(fd, (struct sockaddr *)&un_server, sizeof(struct sockaddr_un)) < 0) {
			rval = -4;
		} else {
			return (fd);
		}
	}
	int err;
	err = errno;
	close(fd);
	errno = err;
	return (rval);
}

//common
static int unix_socket_recv(int fd, char *buf, int buf_len) {
    ssize_t recv_cnt = 0;
    while (recv_cnt < buf_len) {
        ssize_t cnt = recv(fd, buf + recv_cnt, buf_len - recv_cnt, 0);
        if (cnt == 0) { //connection closed by other side
            break;
        } else if (cnt > 0) {
            recv_cnt += cnt;
        } else {
            if (errno == EINTR || errno == EAGAIN)
                continue;
            else
                break;
        }
    }
    return !(recv_cnt == buf_len);
}

static int unix_socket_send(int fd, char *buf, int buf_len) {
    ssize_t send_cnt = 0;
    while (send_cnt < buf_len) {
        ssize_t cnt = send(fd, buf + send_cnt, buf_len - send_cnt, 0);
        if (cnt >= 0) {
            send_cnt += cnt;
        } else {
            if (errno == EINTR || errno == EAGAIN)
                continue;
            else
                break;
        }
    }
    return !(send_cnt == buf_len);
}

int send_msg(int fd, char *buf, int buf_len) {
    int res = unix_socket_send(fd, (char *)&buf_len, 4);  //use unix domain socket not care buf_len's endian
    if (res) return -1;
    return unix_socket_send(fd, buf, buf_len);
}

int recv_msg(int fd, char *buf) {
    char str_len[4];
    int res = unix_socket_recv(fd, str_len, 4);
    int len = *((int *)str_len);
    if (res || len > MAX_MSG_LEN || len < MIN_MSG_LEN) return -1;
    res = unix_socket_recv(fd, buf, len);
    return (res == 0) ? len: -1;
}

void unix_socket_close(int fd) {
	close(fd);
}

