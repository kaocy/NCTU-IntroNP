#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include "client.h"
#include "shell.h"
#include "bulletin.h"

using namespace std;

extern map<int, string> clients;
extern int current_sockfd;
extern int server_sockfd;
extern fd_set allset, rset;

int create_socket(int, int);
void handle_new_connection(int);
void handle_old_connection(int);

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: ./server [port]\n");
        exit(EXIT_FAILURE);
    }

    server_sockfd = create_socket(atoi(argv[1]), 30);
    int max_fd = server_sockfd;
    
    FD_ZERO(&allset);
    FD_SET(server_sockfd, &allset);

    init_clients();
    init_bulletin();

    while (true) {
        rset = allset;
        while (select(max_fd + 1, &rset, NULL, NULL, NULL) < 0);

        // new client
        if (FD_ISSET(server_sockfd, &rset)) {
            struct sockaddr_in client_addr;
            socklen_t client_addr_len = sizeof(client_addr);
            int client_sockfd;
            while ((client_sockfd = accept(server_sockfd, (struct sockaddr*) &client_addr, &client_addr_len)) < 0);

            FD_SET(client_sockfd, &allset);
            max_fd = max(max_fd, client_sockfd);
            handle_new_connection(client_sockfd);
        }

        // old client
        map<int, string>::iterator it;
        for (it = clients.begin(); it != clients.end(); it++) {
            int sockfd = it->first;
            if (!FD_ISSET(sockfd, &rset))   continue;
            handle_old_connection(sockfd);
        }
    }
    return 0;
}

int create_socket(int port, int listenQ) {
    struct sockaddr_in server_addr;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&server_addr, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    int optval = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void *) &optval, sizeof(int)) == -1) {
        printf("Error: setsockopt() failed\n");
        exit(EXIT_FAILURE);
    }

    if (bind(sockfd, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0) {
        printf("Error: bind() failed\n");
        exit(EXIT_FAILURE);
    }

    if (listen(sockfd, listenQ) < 0) {
        printf("Error: listen() failed\n");
        exit(EXIT_FAILURE);
    }
    return sockfd;
}

void handle_new_connection(int sockfd) {
    printf("New connection.\n");

    char msg[] = "********************************\n"
                 "** Welcome to the BBS server. **\n"
                 "********************************\n";
    write(sockfd, msg, strlen(msg));

    add_client(sockfd);
    write(sockfd, "% ", strlen("% "));
}

void handle_old_connection(int sockfd) {
    dup2(sockfd, fileno(stdin));
    current_sockfd = sockfd;

    string input;
    getline(cin, input);
    execute(input);
    write(sockfd, "% ", strlen("% "));
}