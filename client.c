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

#include <readline/readline.h>
#include <readline/history.h>

int subcommand_client(int argc, char *args[]) {
    int exit_code = 0;
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
        assert(c == 0);

        switch(longopt_index) {
            case 0:
                assert(strequal(long_options[longopt_index].name, "socketPath"));
                socket_path = optarg;
                break;
            default:
                assert(false);
        }
    }

    int csock, err;
    if ((csock = socket(AF_UNIX, SOCK_SEQPACKET, 0)) == -1) {
        perror("could not get socket");
        exit_code = 1;
        goto ereturn;    
    }

    struct sockaddr_un conn = { .sun_family = AF_UNIX };
    strncpy(conn.sun_path, socket_path, sizeof(conn.sun_path) - 1);

    if ((err = connect(csock, (struct sockaddr*)&conn, sizeof(conn))) == -1) {
        perror("could not connect to socket");
        exit_code = 1;
        goto eclose;
    }

    ssize_t sent;

    char *input = readline("Enter message: ");

    // sendto destination address is NULL because we are using SEQPACKET
    sent = send(csock, input, strlen(input), 0);
    if (sent == -1) {
        perror("could not send");
        exit_code = 1;
        goto eclose;
    }

    void *msg_buf = malloc(MAX_MSGLEN);
    memset(msg_buf, 0, MAX_MSGLEN);

    struct iovec iovecs[1] = { {.iov_base = msg_buf, .iov_len = MAX_MSGLEN-1} };
    struct msghdr hdr = { .msg_iov = iovecs, .msg_iovlen = 1 };
    ssize_t nbytes;

    if ((nbytes = recvmsg(csock, &hdr, 0)) == -1) {
        perror("could not receive response");
        exit_code = 1;
        goto efreebuf;
    }

    if (hdr.msg_flags && MSG_TRUNC) {
        fprintf(stderr, "received truncated message, probably too long");
        exit_code = 1;
        goto efreebuf;
    }

    assert(((char*)msg_buf)[MAX_MSGLEN-1] == '\0');

    if (nbytes == 0) {
        fprintf(stderr, "broker closed the connection, assuming error\n");
    } else {
        printf("%s\n", msg_buf);
    }

    exit_code = 0;

efreebuf:
    free(msg_buf);
eclose:
    close(csock);
ereturn:
    return exit_code;
}

