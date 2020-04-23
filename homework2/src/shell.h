#pragma once

#include <iostream>
#include <cstdlib>
#include <vector>
#include <map>
#include <unistd.h>
#include "client.h"
#include "user.h"

using namespace std;

extern map<int, string> clients;
extern int current_sockfd;


// remove space in the beginning of str
void remove_space(string& str) {
    while (str.length() > 0 && str.at(0) == ' ') str.erase(0, 1);
}

// split str using delim, return first half
string split(string& str, string delim) {
    int pos = str.find(delim);
    string res = str.substr(0, pos);
    str.erase(0, pos);
    return res;
}

vector<string> parse(string input) {
    vector<string> args;
    args.clear();
    while (input.length() > 0) {
        remove_space(input);
        if (input.length() == 0)    break;

        string arg = split(input, " ");
        args.push_back(arg);
    }
    return args;
}

void execute(string input) {
    // clear '\r' added by telnet
    if (input.back() == '\r')   input.pop_back();

    remove_space(input);
    string cmd_name = split(input, " ");
    vector<string> args = parse(input);

    if (cmd_name == "register") {
        execute_register(args);
    }
    if (cmd_name == "login") {
        execute_login(args);
    }
    if (cmd_name == "logout") {
        execute_logout();
    }
    if (cmd_name == "whoami") {
        execute_whoami();
    }
    if (cmd_name == "exit") {
        execute_exit();
    }
}