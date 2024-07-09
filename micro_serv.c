#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct s_client {
    int     id;
    char    msg[1024];
} t_client;

t_client   clients[1024];
fd_set      readfds, writefds, nowfds;
int         maxfd = 0, g_id = 0;
char        readbuf[120000], writebuf[120000];

void    ft_err(const char *s) {
    write(2, s, strlen(s));
    write(2, "\n", 1);
    exit(1);
}

void    ft_send(int not) {
    for(int i = 3; i <= maxfd; i += 1)
        if(FD_ISSET(i, &writefds) && i != not)
            send(i, writebuf, strlen(writebuf), 0);
}

int main(int argc, const char **argv) {
    if (argc != 2)
        ft_err("Wrong number of arguments");

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        ft_err("Fatal error");

    FD_ZERO(&nowfds);
    bzero(&clients, sizeof(clients));
    maxfd = sockfd;
    FD_SET(sockfd, &nowfds);

    struct sockaddr_in  servaddr;
    socklen_t           len;
   	bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(2130706433); //127.0.0.1
	servaddr.sin_port = htons(atoi(argv[1]));

    if ((bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr))) < 0)
        ft_err("Fatal error");
    if (listen(sockfd, 10) < 0)
        ft_err("Fatal error");

    while(1) {
        readfds = writefds = nowfds;
        if (select(maxfd + 1, &readfds, &writefds, NULL, NULL) < 0)
            continue;
        for(int fd = 3; fd <= maxfd; fd += 1) {
            if (FD_ISSET(fd, &readfds)) {
                if (fd == sockfd) {
                    int connfd = accept(sockfd, (struct sockaddr *)&servaddr, &len);
                    if (connfd < 0)
                        continue;
                    maxfd = connfd > maxfd ? connfd : maxfd;
                    clients[connfd].id = g_id++;
                    FD_SET(connfd, &nowfds);
                    sprintf(writebuf, "server: client %d just arrived\n", clients[connfd].id);
                    ft_send(connfd);
                } else {
                    int res = recv(fd, readbuf, sizeof(readbuf), 0);
                    if (res <= 0) {
                        sprintf(writebuf, "server: client %d just left\n", clients[fd].id);
                        ft_send(fd);
                        FD_CLR(fd, &nowfds);
                        close(fd);
                    } else {
                        for (int i = 0, j = strlen(clients[fd].msg); i < res; i++, j++) {
                            clients[fd].msg[j] = readbuf[i];
                            if (clients[fd].msg[j] == '\n') {
                                clients[fd].msg[j] = '\0';
                                sprintf(writebuf, "client %d: %s\n", clients[fd].id, clients[fd].msg);
                                ft_send(fd);
                                bzero(&clients[fd].msg, strlen(clients[fd].msg));
                                j = -1;
                            }
                        }
                    }
                }
				break;
            }
        }
    }
    return 0;
}
