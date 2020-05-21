#include <iostream>
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

#include <aws/core/Aws.h>

#include "s3.h"

using namespace std;

const int MAX_LEN = 2048;
char buf[MAX_LEN];
int sockfd;

int create_socket(string, int);
void send_to_server(string);
void read_from_server(string);

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: ./client [ip] [port]\n");
        exit(EXIT_FAILURE);
    }

    sockfd = create_socket(string(argv[1]), atoi(argv[2]));
    read_from_server("");
    while (true) {
        string input;
        getline(cin, input);
        send_to_server(input);
        read_from_server(input);
    }

    return 0;
}

int create_socket(string ip, int port) {
    struct sockaddr_in server_addr;
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&server_addr, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip.c_str());
    server_addr.sin_port = htons(port);

    int optval = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const void *) &optval, sizeof(int)) == -1) {
        printf("Error: setsockopt() failed\n");
        exit(EXIT_FAILURE);
    }

    if (connect(fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)  {
        printf("Error: connect() failed\n");
        exit(EXIT_FAILURE);
    }
    return fd;
}

void send_to_server(string message) {
    message += "\n";
    char msg[message.length() + 5];
    sprintf(msg, "%s", message.c_str());
    int n = write(sockfd, msg, strlen(msg));
}

void read_from_server(string input) {
    memset(buf, 0, sizeof(buf));
    int n = read(sockfd, buf, MAX_LEN);
    if (n == 0) {
        printf("Connection closed.\n");
        exit(EXIT_FAILURE);
    }

    string response = string(buf);
    if (response.find("--arg ") == 0) {
        stringstream ss(response);
        string tmp, arg1, arg2;
        ss >> tmp;

        Aws::SDKOptions options;
        Aws::InitAPI(options);

        if (input.find("register") == 0) {
            ss >> arg1;
            create_bucket(Aws::String(arg1.c_str(), arg1.size()));
        }

        Aws::ShutdownAPI(options);

        ss >> tmp;
        cout << tmp << flush;
    }
    else {
        cout << response << flush;
    }
}
