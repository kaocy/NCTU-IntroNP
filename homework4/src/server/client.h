#pragma once

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <sys/types.h>
#include <unistd.h>

using namespace std;

map<int, string> clients; // sockfd -> username
int current_sockfd;
int server_sockfd;
fd_set allset, rset;


void init_clients() {
    clients.clear();
}

void add_client(int sockfd) { // when a client connect to server
    clients[sockfd] = "";
}

void remove_client() { // when a client quit the connection
    dup2(server_sockfd, fileno(stdin));
    clients.erase(current_sockfd);
    FD_CLR(current_sockfd, &allset);
    close(current_sockfd);
}

void send_message(string message) {
    char msg[message.length() + 5];
    message += "% ";
    sprintf(msg, "%s", message.c_str());
    write(current_sockfd, msg, strlen(msg));
}