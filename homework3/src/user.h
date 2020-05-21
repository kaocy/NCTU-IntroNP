#pragma once

#include <iostream>
#include <sstream>
#include <map>
#include <cstdlib>
#include <ctime>
#include "client.h"
using namespace std;

extern map<int, string> clients;
extern int current_sockfd;

struct User {
    string email;
    string password;
    string bucket_name;
    
    User(string email = "", string password = "", string bucket_name = "")
        : email(email), password(password), bucket_name(bucket_name) {}
};

map<string, User> users; // username -> User

string get_date() {
    time_t now = time(0);
    tm *localtm = localtime(&now);
    string year = to_string(localtm->tm_year + 1900);
    string month = to_string(localtm->tm_mon + 1);
    string day = to_string(localtm->tm_mday);

    if (month.length() == 1)    month = "0" + month;
    if (day.length() == 1)      day = "0" + day;
    return year + "-" + month + "-" + day;
}

string transform_date(string date) {
    return date.substr(5, 2) + "/" + date.substr(8, 2);
}

string get_timestamp() {
    time_t now = time(0);
    stringstream ss;
    ss << now;
    return ss.str();
}

void execute_register(const vector<string> &args) {
    if (args.size() < 3) {
        send_message("Usage: register <username> <email> <password>\n");
        return ;
    }
    string username = args.at(0);
    string email = args.at(1);
    string password = args.at(2);
    if (users.find(username) != users.end()) {
        send_message("Username is already used.\n");
        return ;
    }
    string bucket_name = "0516007-" + username + "-" + get_timestamp();
    users[username] = User(email, password, bucket_name);

    string msg = "";
    msg += "--arg " + bucket_name + " ";
    msg += "Register successfully.\n";
    send_message(msg);
}

void execute_login(const vector<string> &args) {
    if (args.size() < 2) {
        send_message("Usage: login <username> <password>\n");
        return ;
    }
    string current_username = clients[current_sockfd];
    string username = args.at(0);
    string password = args.at(1);
    if (current_username != "") {
        send_message("Please logout first.\n");
        return ;
    }
    if (users.find(username) == users.end()) {
        send_message("Login failed.\n");
        return ;
    }
    if (users[username].password != password) {
        send_message("Login failed.\n");
        return ;
    }
    clients[current_sockfd] = username;
    send_message("Welcome, " + username + ".\n");
}

void execute_logout() {
    string current_username = clients[current_sockfd];
    if (current_username == "") {
        send_message("Please login first.\n");
        return ;
    }
    clients[current_sockfd] = "";
    send_message("Bye, " + current_username + ".\n");
}

void execute_whoami() {
    string current_username = clients[current_sockfd];
    if (current_username == "") {
        send_message("Please login first.\n");
        return ;
    }
    send_message(current_username + "\n");
}

void execute_exit() {
    remove_client();
}