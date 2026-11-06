#include <iostream>
#include <fstream>
#include <map>
#include <ranges>
#include <vector>
using namespace std;

struct user
{
    string username;
    bool isAdmin = false;
    bool isBanned = false;
};

// split function found here: https://stackoverflow.com/questions/14265581/parse-split-a-string-in-c-using-string-delimiter-standard-c
std::vector<std::string> split(std::string s, const std::string& delimiter) {
    std::vector<std::string> tokens;
    size_t pos = 0;
    std::string token;
    while ((pos = s.find(delimiter)) != std::string::npos) {
        token = s.substr(0, pos);
        tokens.push_back(token);
        s.erase(0, pos + delimiter.length());
    }
    tokens.push_back(s);

    return tokens;
}

int main() {
    ifstream fin("filetesting.txt");
    map<long, user> users = map<long, user>();

    string line;
    while (getline(fin, line))
    {
        bool b = true;

        vector<string> results = split(line, ", ");
        long userid = stoi(results[0]);
        users.insert({userid, user()});
        users[userid].username = results[1];
        users[userid].isAdmin = stoi(results[2]);
        users[userid].isBanned = stoi(results[3]);
    }

    for (const auto& kv : users)
    {
        cout << "UserID: " << kv.first
        << " Username: " << kv.second.username
        << " IsAdmin: " << kv.second.isAdmin
        << " IsBanned: " << kv.second.isBanned
        << endl;
    }
    fin.close();
    return 0;
}


