#pragma once

#include <iostream>
#include <map>
#include "client.h"
using namespace std;

extern map<int, string> clients;
extern int current_sockfd;

struct Subscription {
    string board;
    string author;
    vector<string> keywords;

    Subscription(string board = "", string author = "") : board(board), author(author) { keywords.clear(); }
};

struct User {
    string email;
    string password;
    vector<Subscription> subs;
    
    User(string email = "", string password = "") : email(email), password(password) { subs.clear(); }
};

map<string, User> users; // username -> User

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
    users[username] = User(email, password);
    send_message("Register successfully.\n");
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

    // clear subscription
    for (auto &sub : users[username].subs) {
        sub.keywords.clear();
    }
    users[username].subs.clear();
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