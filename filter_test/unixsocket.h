#ifndef __UNIXSOCKET_H__
#define __UNIXSOCKET_H__

#include <sys/types.h>

#define MAX_CONNECTION_NUMBER 100
#define MIN_MSG_LEN 1
#define MAX_MSG_LEN 64
//for server
int unix_socket_listen(const  char *servername);
int unix_socket_accept(int listenfd, uid_t *uidptr);
//for client
int unix_socket_conn(const char *servername);
//common
int send_msg(int fd, char *buf, int buf_len);//if success return 0 else return < 0
int recv_msg(int fd, char *buf);//if success return length of buf(may not '\0') else return < 0
void unix_socket_close(int fd);


#endif
