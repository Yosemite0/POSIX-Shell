#ifndef SHELL_ENV_H
#define SHELL_ENV_H

// Standard C++ Libraries
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <queue>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <limits.h>

// POSIX Libraries
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <signal.h>

#include <readline/readline.h>

using namespace std;

class ShellEnv {
public:
    // Shell Info
    string user_name;
    string system_name;
    string shell_dir;
    string prev_dir;

    // History
    string history_path = "/.history/history.txt";
    vector<string> history;
    int hist_fd;

    // Directory
    string curr_dir;
    string home_dir;
    string formatted_dir; // For display purposes

    //alias
    unordered_map<string,string> alias;

    // Methods
    void initialize();
    void getShellName();
    void getUserName();
    void getCurrentDirectory();
    void getShellDirectory();
    string getFormattedPrompt();
    int setDirectory(const string& new_dir);
    
    void addHistory(string &line_read);
    void getHistoryFile();
    void writeHistory();

};

#endif 
