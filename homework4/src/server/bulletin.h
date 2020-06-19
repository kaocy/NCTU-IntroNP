#pragma once

#include <iostream>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <vector>
#include <map>
#include "client.h"
#include "user.h"

using namespace std;

extern map<int, string> clients;
extern int current_sockfd;

struct Comment {
    string content;
    string author;
    Comment(string content = "", string author = "") : content(content), author(author) {}
};

struct Post {
    int id;
    string title;
    string content;
    string author;
    string date;
    vector<Comment> comments;
    Post(int id = 0, string title = "", string content = "", string author = "", string date = "")
        : id(id), title(title), content(content), author(author), date(date) { comments.clear(); }
};

struct Board {
    string name;
    string moderator;
    vector<int> post_ids;
    Board(string name = "", string moderator = "") : name(name), moderator(moderator) { post_ids.clear(); }
};

vector<string> board_names;
map<string, Board> boards; // board name -> board
map<int, Post> posts; // post id -> post
int next_post_id;

void init_bulletin() {
    board_names.clear();
    boards.clear();
    posts.clear();
    next_post_id = 1;
}

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

void create_board(const vector<string> &args) {
    string current_username = clients[current_sockfd];
    if (current_username == "") {
        send_message("Please login first.\n");
        return ;
    }

    if (args.size() < 1) {
        send_message("Usage: create-board <name>\n");
        return ;
    }

    string board_name = args.at(0);
    if (boards.find(board_name) != boards.end()) {
        send_message("Board already exist.\n");
        return ;
    }

    board_names.push_back(board_name);
    boards[board_name] = Board(board_name, current_username);
    send_message("Create board successfully.\n");
}

void create_post(const vector<string> &args, const vector<string> &fields) {
    string current_username = clients[current_sockfd];
    if (current_username == "") {
        send_message("Please login first.\n");
        return ;
    }

    if (args.size() < 3) {
        send_message("Usage: create-post <board-name> --title <title> --content <content>\n");
        return ;
    }

    string board_name = args.at(0);
    if (boards.find(board_name) == boards.end()) {
        send_message("Board does not exist.\n");
        return ;
    }

    string title, content;
    if (fields.at(0) == "title") {
        title = args.at(1);
        content = args.at(2);
    }
    if (fields.at(0) == "content") {
        title = args.at(2);
        content = args.at(1);
    }
    string date = get_date();

    Post post(next_post_id, title, content, current_username, date);
    posts[next_post_id] = post;
    boards[board_name].post_ids.push_back(next_post_id++);
    send_message("Create post successfully.\n");
}

void list_board(const vector<string> &args) {
    int max_index_len = 8;
    int max_name_len = 6;
    int max_moderator_len = 11;
    for (string board_name : board_names) {
        const auto& board = boards[board_name];
        max_name_len = max(max_name_len, int(board.name.length()));
        max_moderator_len = max(max_moderator_len, int(board.moderator.length()));
    }
    max_name_len += 5;
    max_moderator_len += 5;

    stringstream title_stream;
    title_stream << "\t" << std::left << std::setfill(' ');
    title_stream << setw(max_index_len) << "Index";
    title_stream << setw(max_name_len) << "Name";
    title_stream << setw(max_moderator_len) << "Moderator" << setw(1) << "\n";
    string msg = "";
    msg += title_stream.str();

    if (args.size() > 0) {
        string key = args.at(0);
        int index = 1;

        for (string board_name : board_names) {
            const auto& board = boards[board_name];
            int pos = board.name.find(key);
            if (pos != -1) {
                stringstream line_stream;
                line_stream << "\t" << std::left << std::setfill(' ');
                line_stream << setw(max_index_len) << to_string(index++);
                line_stream << setw(max_name_len) << board.name;
                line_stream << setw(max_moderator_len) << board.moderator << setw(1) << "\n";
                msg += line_stream.str();
            }
        }
    }
    else {
        int index = 1;
        for (string board_name : board_names) {
            const auto& board = boards[board_name];
            stringstream line_stream;
            line_stream << "\t" << std::left << std::setfill(' ');
            line_stream << setw(max_index_len) << to_string(index++);
            line_stream << setw(max_name_len) << board.name;
            line_stream << setw(max_moderator_len) << board.moderator << setw(1) << "\n";
            msg += line_stream.str();
        }
    }
    send_message(msg);
}

void list_post(const vector<string> &args) {
    if (args.size() < 1) {
        send_message("Usage: list-post <board-name> ##<key>\n");
        return ;
    }

    string board_name = args.at(0);
    if (boards.find(board_name) == boards.end()) {
        send_message("Board does not exist.\n");
        return ;
    }

    int max_id_len = 5;
    int max_title_len = 6;
    int max_author_len = 11;
    int max_date_len = 12;
    for (int post_id : boards[board_name].post_ids) {
        // need to check existence since post_id in board will not be deleted when deleting post
        if (posts.find(post_id) == posts.end()) continue;

        const auto& post = posts[post_id];
        max_title_len = max(max_title_len, int(post.title.length()));
        max_author_len = max(max_author_len, int(post.author.length()));
    }
    max_title_len += 5;
    max_author_len += 5;

    stringstream title_stream;
    title_stream << "\t" << std::left << std::setfill(' ');
    title_stream << setw(max_id_len) << "ID";
    title_stream << setw(max_title_len) << "Title";
    title_stream << setw(max_author_len) << "Author";
    title_stream << setw(max_date_len) << "Date" << setw(1) << "\n";
    string msg = "";
    msg += title_stream.str();

    if (args.size() > 1) {
        string key = args.at(1);
        for (int post_id : boards[board_name].post_ids) {
            // need to check existence since post_id in board will not be deleted when deleting post
            if (posts.find(post_id) == posts.end()) continue;

            const auto& post = posts[post_id];
            int pos = post.title.find(key);
            if (pos != -1) {
                string date = transform_date(post.date);
                stringstream line_stream;
                line_stream << "\t" << std::left << std::setfill(' ');
                line_stream << setw(max_id_len) << to_string(post.id);
                line_stream << setw(max_title_len) << post.title;
                line_stream << setw(max_author_len) << post.author;
                line_stream << setw(max_date_len) << date << setw(1) << "\n";
                msg += line_stream.str();
            }
        }
    }
    else {
        for (int post_id : boards[board_name].post_ids) {
            // need to check existence since post_id in board will not be deleted when deleting post
            if (posts.find(post_id) == posts.end()) continue;

            const auto& post = posts[post_id];
            string date = transform_date(post.date);
            stringstream line_stream;
            line_stream << "\t" << std::left << std::setfill(' ');
            line_stream << setw(max_id_len) << to_string(post.id);
            line_stream << setw(max_title_len) << post.title;
            line_stream << setw(max_author_len) << post.author;
            line_stream << setw(max_date_len) << date << setw(1) << "\n";
            msg += line_stream.str();
        }
    }
    send_message(msg);
}

void read_post(const vector<string> &args) {
    if (args.size() < 1) {
        send_message("Usage: read <post-id>\n");
        return ;
    }

    int post_id = stoi(args.at(0));
    if (posts.find(post_id) == posts.end()) {
        send_message("Post does not exist.\n");
        return ;
    }

    string msg = "";
    msg += "\tAuthor\t:" + posts[post_id].author + "\n";
    msg += "\tTitle\t:" + posts[post_id].title + "\n";
    msg += "\tDate\t:" + posts[post_id].date + "\n";
    msg += "\t--\n";

    string content = posts[post_id].content;
    while (content != "") {
        int pos = content.find("<br>");
        if (pos == -1) {
            msg += "\t" + content + "\n";
            break;
        }
        msg += "\t" + content.substr(0, pos) + "\n";
        content.erase(0, pos + 4);
    }
    msg += "\t--\n";
    
    for (const auto& comment : posts[post_id].comments) {
        msg += "\t" + comment.author + ": " + comment.content + "\n";
    }
    send_message(msg);
}

void delete_post(const vector<string> &args) {
    string current_username = clients[current_sockfd];
    if (current_username == "") {
        send_message("Please login first.\n");
        return ;
    }

    if (args.size() < 1) {
        send_message("Usage: delete-post <post-id>\n");
        return ;
    }

    int post_id = stoi(args.at(0));
    if (posts.find(post_id) == posts.end()) {
        send_message("Post does not exist.\n");
        return ;
    }

    if (posts[post_id].author != current_username) {
        send_message("Not the post owner.\n");
        return ;
    }

    posts.erase(post_id);
    send_message("Delete successfully.\n");
    // for (auto& kv : boards) {
    //     for (auto post_iter = kv.second.posts.begin(); post_iter != kv.second.posts.end(); post_iter++) {
    //         if (*post_iter == post_id) {
    //             kv.second.posts.erase(post_iter);
    //             return ;
    //         }
    //     }
    // }
}

void update_post(const vector<string> &args, const vector<string> &fields) {
    string current_username = clients[current_sockfd];
    if (current_username == "") {
        send_message("Please login first.\n");
        return ;
    }

    if (args.size() < 2) {
        send_message("Usage: update-post <post-id> --title/content <new>\n");
        return ;
    }

    int post_id = stoi(args.at(0));
    if (posts.find(post_id) == posts.end()) {
        send_message("Post does not exist.\n");
        return ;
    }

    if (posts[post_id].author != current_username) {
        send_message("Not the post owner.\n");
        return ;
    }

    if (fields.at(0) == "title") {
        posts[post_id].title = args.at(1);
    }
    if (fields.at(0) == "content") {
        posts[post_id].content = args.at(1);
    }
    if (fields.size() == 2 && fields.at(1) == "title") {
        posts[post_id].title = args.at(2);
    }
    if (fields.size() == 2 && fields.at(1) == "content") {
        posts[post_id].content = args.at(2);
    }
    send_message("Update successfully.\n");
}

void comment(const vector<string> &args) {
    string current_username = clients[current_sockfd];
    if (current_username == "") {
        send_message("Please login first.\n");
        return ;
    }

    if (args.size() < 2) {
        send_message("Usage: comment <post-id> <comment>\n");
        return ;
    }

    int post_id = stoi(args.at(0));
    if (posts.find(post_id) == posts.end()) {
        send_message("Post does not exist.\n");
        return ;
    }

    string content = args.at(1);
    posts[post_id].comments.emplace_back(content, current_username);
    send_message("Comment successfully.\n");
}