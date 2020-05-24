#include <iostream>
#include <fstream>
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
string split(string&, string);

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
        response = response.substr(6);

        Aws::SDKOptions options;
        Aws::InitAPI(options);

        if (input.find("register") == 0) {
            string s3_bucket_name = split(response, " ");
            create_bucket(Aws::String(s3_bucket_name.c_str(), s3_bucket_name.size()));
        }
        if (input.find("create-post") == 0) {
            string s3_bucket_name = split(response, " ");
            string s3_object_name = split(response, " ");
            string content = split(response, " Create post successfully.");

            string local_file_name = "../../local_files/" + s3_object_name + ".txt";
            fstream file;
            file.open(local_file_name, ios::out | ios::trunc);
            file << content;
            file.close();
            
            upload_object(Aws::String(s3_bucket_name.c_str(), s3_bucket_name.size()),
                        Aws::String(s3_object_name.c_str(), s3_object_name.size()),
                        local_file_name);
        }
        if (input.find("update-post") == 0) {
            string s3_bucket_name = split(response, " ");
            string s3_object_name = split(response, " ");
            string content = split(response, " Update successfully.");

            string local_file_name = "../../local_files/" + s3_object_name + ".txt";
            fstream file;
            file.open(local_file_name, ios::out | ios::trunc);
            file << content;
            file.close();
            
            upload_object(Aws::String(s3_bucket_name.c_str(), s3_bucket_name.size()),
                        Aws::String(s3_object_name.c_str(), s3_object_name.size()),
                        local_file_name);
        }
        if (input.find("read") == 0) {
            string s3_bucket_name = split(response, " ");
            string s3_object_name = split(response, " ");
            
            string content = \
            get_object(Aws::String(s3_bucket_name.c_str(), s3_bucket_name.size()),
                       Aws::String(s3_object_name.c_str(), s3_object_name.size()));

            response.pop_back();
            response.pop_back();
            response += "\t--\n";
            while (content != "") {
                int pos = content.find("<br>");
                if (pos == -1) {
                    response += "\t" + content;
                    break;
                }
                response += "\t" + content.substr(0, pos) + "\n";
                content.erase(0, pos + 4);
            }
            response += "\n";
            response += "\t--\n";

            if (response.find("\tAuthor\t:") > 0) {
                string s = split(response, " \tAuthor\t:");
                stringstream ss(s);
                string comment_s3_object_name;
                while (ss >> comment_s3_object_name) {
                    response += \
                    get_object(Aws::String(s3_bucket_name.c_str(), s3_bucket_name.size()),
                            Aws::String(comment_s3_object_name.c_str(), comment_s3_object_name.size()));
                }
            }
            response += "% ";
        }
        if (input.find("delete-post") == 0) {
            string s3_bucket_name = split(response, " ");
            string s3_object_name = split(response, " ");

            delete_object(Aws::String(s3_bucket_name.c_str(), s3_bucket_name.size()),
                        Aws::String(s3_object_name.c_str(), s3_object_name.size()));
        }
        if (input.find("comment") == 0) {
            string s3_bucket_name = split(response, " ");
            string s3_object_name = split(response, " ");
            string content = split(response, " Comment successfully.");

            string local_file_name = "../../local_files/" + s3_object_name + ".txt";
            fstream file;
            file.open(local_file_name, ios::out | ios::trunc);
            file << content;
            file.close();
            
            upload_object(Aws::String(s3_bucket_name.c_str(), s3_bucket_name.size()),
                        Aws::String(s3_object_name.c_str(), s3_object_name.size()),
                        local_file_name);
        }
        if (input.find("mail-to") == 0) {
            string s3_bucket_name = split(response, " ");
            string s3_object_name = split(response, " ");
            string content = split(response, " Sent successfully.");

            string local_file_name = "../../local_files/" + s3_object_name + ".txt";
            fstream file;
            file.open(local_file_name, ios::out | ios::trunc);
            file << content;
            file.close();

            upload_object(Aws::String(s3_bucket_name.c_str(), s3_bucket_name.size()),
                        Aws::String(s3_object_name.c_str(), s3_object_name.size()),
                        local_file_name);
        }
        if (input.find("retr-mail") == 0) {
            string s3_bucket_name = split(response, " ");
            string s3_object_name = split(response, " ");
            
            string content = \
            get_object(Aws::String(s3_bucket_name.c_str(), s3_bucket_name.size()),
                       Aws::String(s3_object_name.c_str(), s3_object_name.size()));

            response.pop_back();
            response.pop_back();
            response += "\t--\n";
            while (content != "") {
                int pos = content.find("<br>");
                if (pos == -1) {
                    response += "\t" + content;
                    break;
                }
                response += "\t" + content.substr(0, pos) + "\n";
                content.erase(0, pos + 4);
            }
            response += "\n";
            response += "\t--\n";
            response += "% ";
        }
        if (input.find("delete-mail") == 0) {
            string s3_bucket_name = split(response, " ");
            string s3_object_name = split(response, " ");

            delete_object(Aws::String(s3_bucket_name.c_str(), s3_bucket_name.size()),
                        Aws::String(s3_object_name.c_str(), s3_object_name.size()));
        }

        Aws::ShutdownAPI(options);
    }
    cout << response << flush;
}

string split(string& str, string delim) {
    int pos = str.find(delim);
    string res = str.substr(0, pos);
    str = str.substr(pos + 1);
    return res;
}
