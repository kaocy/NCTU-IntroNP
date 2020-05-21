#pragma once

#include <iostream>
#include <cstdlib>
#include <vector>
#include <map>
#include <unistd.h>
#include "client.h"
#include "user.h"
#include "bulletin.h"

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
        if (arg.substr(0, 2) == "##")   arg.erase(0, 2);
        args.push_back(arg);
    }
    return args;
}

vector<string> parse_post(string input) {
    vector<string> args;
    args.clear();
    while (input.length() > 0) {
        remove_space(input);
        if (input.length() == 0)    break;

        string arg = split(input, " ");
        if (arg == "--title" || arg == "--content") {
            remove_space(input);
            int pos = input.find("--");
            if (pos == -1) {
                arg = input;
                input = "";
            }
            else {
                arg = input.substr(0, pos);
                input.erase(0, pos);
            }
            if (arg.back() == ' ')  arg.pop_back();
        }
        args.push_back(arg);
    }
    return args;
}

vector<string> find_post_fields(string input) {
    vector<string> fields;
    int pos1 = input.find("--title");
    int pos2 = input.find("--content");
    if (pos1 != -1 && pos2 != -1) {
        if (pos1 < pos2) {
            fields.push_back("title");
            fields.push_back("content");
        }
        else {
            fields.push_back("content");
            fields.push_back("title");
        }
    }
    else if (pos1 != -1) {
        fields.push_back("title");
    }
    else if (pos2 != -1) {
        fields.push_back("content");
    }
    return fields;
}

vector<string> parse_comment(string input) {
    vector<string> args;
    args.clear();
    remove_space(input);
    string arg = split(input, " ");
    remove_space(input);
    if (arg != "")  args.push_back(arg);
    if (input != "")  args.push_back(input);
    return args;
}

void execute(string input) {
    // clear '\r' added by telnet
    if (input.back() == '\r')   input.pop_back();

    remove_space(input);
    string cmd_name = split(input, " ");
    vector<string> args;
    vector<string> fields;
    if (cmd_name == "create-post" || cmd_name == "update-post") {
        args = parse_post(input);
        fields = find_post_fields(input);
    }
    else if (cmd_name == "comment") {
        args = parse_comment(input);
    }
    else {
        args = parse(input);
    }

    if (cmd_name == "register")     execute_register(args);
    if (cmd_name == "login")        execute_login(args);
    if (cmd_name == "logout")       execute_logout();
    if (cmd_name == "whoami")       execute_whoami();
    if (cmd_name == "exit")         execute_exit();
    if (cmd_name == "create-board") create_board(args);
    if (cmd_name == "create-post")  create_post(args, fields);
    if (cmd_name == "list-board")   list_board(args);
    if (cmd_name == "list-post")    list_post(args);
    if (cmd_name == "read")         read_post(args);
    if (cmd_name == "delete-post")  delete_post(args);
    if (cmd_name == "update-post")  update_post(args, fields);
    if (cmd_name == "comment")      comment(args);
}