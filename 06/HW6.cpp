#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <stdexcept>

using namespace std;

template<class T>
string to_string(T&& arg) {
    stringstream sstr;
    sstr << arg;
    return sstr.str();
}

template <typename... Args>
string format(string s, Args&&... t) {
    vector<string> args = {to_string(std::forward<Args>(t))...};
    stringstream ostr;
    int braces_balance = 0;
    string current_number = "";
    string format_string = "";
    for (char c : s) {
        if (c == '{') {
            if (braces_balance > 0) {
                throw runtime_error("excess {}");
            }
            braces_balance += 1;
        } else if (c == '}') {
            if (braces_balance == 0) {
                throw runtime_error("excess {}");
            }
            braces_balance -= 1;
            if (current_number.size() == 0) {
                throw runtime_error("empty {}");
            }
            int idx = stoi(current_number);
            if (idx < 0 || idx >= args.size()) {
                throw runtime_error("error");
            } else
                format_string = format_string + args[idx];
            current_number = "";
        } else if (braces_balance == 0) {
            format_string.push_back(c);
        } else {
            if ((c < '0') || (c > '9')) {
                throw runtime_error("not number in {}");
            } else {
                current_number += c;
            }
        }
    }

    if (braces_balance > 0) {
        throw runtime_error("excess {}");
    }
    return format_string;
}
