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

int dummy_transform_password(char *pw, FILE *out);
int yubikey_transform_password(char *pw, FILE *out);

static char *socket_path = NULL;
int(*transformer)(char*, FILE*) = NULL;

int subcommand_broker(int argc, char *args[]) {
    
    int longopt_index = 0;
    static struct option long_options[] = {
        {"socketPath", required_argument, 0, 0},
        {"platform", required_argument, 0, 0}, // how to make this optional without coredumps?
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
            case 1:
                assert(strequal(long_options[longopt_index].name, "platform"));
                if (strequal(optarg, "ykpersonalize")) {
                    transformer = &yubikey_transform_password;
                } else if (strequal(optarg, "dummy")) {
                    transformer = &dummy_transform_password;
                }
                break;
            default:
              assert(false);
              break;  
        }

    }

    assert(transformer);
    assert(socket_path);

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

       int transform_result;
       if ((transform_result = (*transformer)(msg_buf, cfd)) != 0) {
            fprintf(stderr, "error transforming password\n");     
       }
       
       fclose(cfd);

    }

}

