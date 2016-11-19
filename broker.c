#include "common.h"
#include <getopt.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/un.h>

int broker_server(char *socket_path);
int transform_password(char *pw, FILE *out);

int subcommand_broker(int argc, char *args[]) {
    
    char *socket_path = NULL;
    
    int longopt_index = 0;
    static struct option long_options[] = {
        {"socketPath", required_argument, 0, 0},
        {0, 0, 0, 0}
    }; 

    
    while (true) {
        int c = getopt_long(argc, args, "", long_options, &longopt_index);
        if (c == -1) {
            break;
        }
        assert (c == 0);

        switch(longopt_index) {
            case 0:
                assert(strequal(long_options[longopt_index].name, "socketPath"));
                socket_path = optarg;
                break;
            default:
              assert(false);
              break;  
        }

    } 

    return broker_server(socket_path);;

}


int broker_server(char *socket_path) {

    int ret;

    unlink(socket_path);

    int lsock = socket(AF_UNIX, SOCK_SEQPACKET, 0);
    if (lsock == -1) {
        perror("cannot create socket");
        return 1;
    }

    struct sockaddr_un lsockaddr;
    memset(&lsockaddr, 0, sizeof(struct sockaddr_un));
    lsockaddr.sun_family = AF_UNIX;
    strncpy(lsockaddr.sun_path, socket_path, sizeof(lsockaddr.sun_path) - 1);

    ret = bind(lsock, (struct sockaddr*)&lsockaddr, sizeof(struct sockaddr_un)); 
    if (ret == -1) {
        perror("cannot bind socket");
        return 1;
    }

    ret = listen(lsock, SOMAXCONN);

    void *msg_buf __attribute__((__cleanup__(free_ptr)));
    msg_buf = malloc(MAX_MSGLEN);

    while (1) {

        memset(msg_buf, 0, MAX_MSGLEN);

        int conn;

        struct sockaddr_un peer;
        socklen_t peer_len = sizeof(peer);
        if ((conn = accept(lsock, (struct sockaddr*)&peer, &peer_len)) == -1) {
            perror("cannot accept");
            return 1;
        }
        
        struct iovec iovecs[1] = { {.iov_base = msg_buf, .iov_len = MAX_MSGLEN-1} };
        struct msghdr hdr = { .msg_iov = iovecs, .msg_iovlen = 1 };
        ssize_t nbytes;

        nbytes = recvmsg(conn, &hdr, 0);
        if (nbytes == -1) {
            perror("could not receive");
            close(conn);
            continue;
        }
        if (hdr.msg_flags && MSG_TRUNC) {
            fprintf(stderr, "received truncated message, probably too long");
            close(conn);
            continue;
        }
        
        FILE *cfd;
        if ((cfd = fdopen(conn, "w+")) == NULL) {
            perror("cannot fdopen");
            close(conn);
            continue;
        }
       
       assert(((char*)msg_buf)[MAX_MSGLEN-1] == '\0');

       transform_password(msg_buf, cfd);
       
       fclose(cfd);

    }

}

int transform_password(char *pw, FILE *out) {
    assert(pw);
    assert(out);
    for (int c= strlen(pw)-1; c >= 0; c--) {
        fprintf(out, "%c", pw[c]);
    }
    return 0;
}

