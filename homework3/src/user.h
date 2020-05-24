#pragma once

#include <iostream>
#include <iomanip>
#include <sstream>
#include <map>
#include <cstdlib>
#include <ctime>
#include "client.h"
using namespace std;

extern map<int, string> clients;
extern int current_sockfd;

struct Mail {
    string subject;
    string from;
    string date;
    string s3_object_name;

    Mail(string subject = "", string from = "", string date = "", string s3_object_name = "")
        : subject(subject), from(from), date(date), s3_object_name(s3_object_name) {}
};

struct User {
    string email;
    string password;
    string s3_bucket_name;

    vector<Mail> mails;
    
    User(string email = "", string password = "", string s3_bucket_name = "")
        : email(email), password(password), s3_bucket_name(s3_bucket_name) { mails.clear(); }
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
    string s3_bucket_name = "0516007-" + username + "-" + get_timestamp();
    users[username] = User(email, password, s3_bucket_name);

    string msg = "";
    msg += "--arg " + s3_bucket_name + " ";
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

void mail_to(const vector<string> &args) {
    string current_username = clients[current_sockfd];
    if (current_username == "") {
        send_message("Please login first.\n");
        return ;
    }

    if (args.size() < 3) {
        send_message("Usage: mail-to <username> --subject <subject> --content <content>\n");
        return ;
    }

    string target_username = args.at(0);
    if (users.find(target_username) == users.end()) {
        send_message(target_username + " does not exist.\n");
        return ;
    }

    string from = current_username;
    string subject = args.at(1);
    string content = args.at(2);
    string date = get_date();
    string s3_bucket_name = users[target_username].s3_bucket_name;
    string s3_object_name = "0516007-mail-" + from + "-" + get_timestamp();
    users[target_username].mails.emplace_back(subject, from, date, s3_object_name);

    string msg = "";
    msg += "--arg " + s3_bucket_name + " " + s3_object_name + " " + content + " ";
    msg += "Sent successfully.\n";
    send_message(msg);
}

void list_mail() {
    string current_username = clients[current_sockfd];
    if (current_username == "") {
        send_message("Please login first.\n");
        return ;
    }

    int max_id_len = 8;
    int max_subject_len = 9;
    int max_from_len = 6;
    int max_date_len = 12;
    for (const auto &mail : users[current_username].mails) {
        max_subject_len = max(max_subject_len, int(mail.subject.length()));
        max_from_len = max(max_from_len, int(mail.from.length()));
    }
    max_subject_len += 5;
    max_from_len += 5;

    stringstream title_stream;
    title_stream << "\t" << std::left << std::setfill(' ');
    title_stream << setw(max_id_len) << "ID";
    title_stream << setw(max_subject_len) << "Subject";
    title_stream << setw(max_from_len) << "From";
    title_stream << setw(max_date_len) << "Date" << setw(1) << "\n";
    string msg = "";
    msg += title_stream.str();

    int id = 1;
    for (const auto &mail : users[current_username].mails) {
        string date = transform_date(mail.date);
        stringstream line_stream;
        line_stream << "\t" << std::left << std::setfill(' ');
        line_stream << setw(max_id_len) << to_string(id++);
        line_stream << setw(max_subject_len) << mail.subject;
        line_stream << setw(max_from_len) << mail.from;
        line_stream << setw(max_date_len) << date << setw(1) << "\n";
        msg += line_stream.str();
    }

    send_message(msg);
}

void retr_mail(const vector<string> &args) {
    string current_username = clients[current_sockfd];
    if (current_username == "") {
        send_message("Please login first.\n");
        return ;
    }

    if (args.size() < 1) {
        send_message("Usage: retr-mail <mail#>\n");
        return ;
    }

    int mail_id = stoi(args.at(0));
    if (mail_id < 1 || mail_id > int(users[current_username].mails.size())) {
        send_message("No such mail\n");
        return ;
    }

    const auto &mail = users[current_username].mails.at(mail_id - 1);
    string s3_bucket_name = users[current_username].s3_bucket_name;
    string s3_object_name = mail.s3_object_name;

    string msg = "";
    msg += "--arg " + s3_bucket_name + " " + s3_object_name + " ";
    msg += "\tSubject\t:" + mail.subject + "\n";
    msg += "\tFrom\t:" + mail.from + "\n";
    msg += "\tDate\t:" + mail.date + "\n";
    send_message(msg);
}

void delete_mail(const vector<string> &args) {
    string current_username = clients[current_sockfd];
    if (current_username == "") {
        send_message("Please login first.\n");
        return ;
    }

    if (args.size() < 1) {
        send_message("Usage: delete-mail <mail#>\n");
        return ;
    }

    int mail_id = stoi(args.at(0));
    if (mail_id < 1 || mail_id > int(users[current_username].mails.size())) {
        send_message("No such mail\n");
        return ;
    }

    const auto &mail = users[current_username].mails.at(mail_id - 1);
    string s3_bucket_name = users[current_username].s3_bucket_name;
    string s3_object_name = mail.s3_object_name;
    users[current_username].mails.erase(users[current_username].mails.begin() + (mail_id - 1));

    string msg = "";
    msg += "--arg " + s3_bucket_name + " " + s3_object_name + " ";
    msg += "Mail deleted.\n";
    send_message(msg);
}
