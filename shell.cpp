#include "./env/ShellEnv.h"


#include "execute.cpp"
#include "inbuilt.cpp"
#include "parse.cpp"

using namespace std;

// search Left


ShellEnv shellEnv;

pid_t foregroundPid;
map<pid_t, int> backgroundProcesses;

// parse
vector<vector<char*>> parse(char* lines_read);

//execution
extern void processCommand(vector<char*>& command_parts);

//helper 
string trim(const string& str);

void handleSigTSTP(int sig) {
    if (foregroundPid > 0) {
        cout<<"SIGSTP"<<endl;
        kill(-foregroundPid, SIGTSTP);
        backgroundProcesses[foregroundPid] = 1;
        foregroundPid = -1;
    }
}

void handleSigINT(int sig) {
    if (foregroundPid > 0) {
        cout<<"SIGINT"<<endl;
        // Send SIGINT to the foreground process group
        kill(-foregroundPid, SIGINT);
        foregroundPid = -1;
    }
}

int main() {
    shellEnv.initialize();

    foregroundPid = -1;
    // Set up signal handlers
    signal(SIGTSTP, handleSigTSTP); // Handle CTRL-Z
    signal(SIGINT, handleSigINT);   // Handle CTRL-C
    // signal(SIGQUIT, handleSigQUIT);     // Handle Ctrl- D
    // signal(SIGQUIT, SIG_IGN);       // Ignore CTRL-\ (core dump signal)

    while (true) {
        char* lines_read = readline(shellEnv.getFormattedPrompt().c_str());
        if (!lines_read) {
            cout<<"Exiting from Terminal : EXIT_SUCCESS\n";
            shellEnv.writeHistory();
            exit(EXIT_SUCCESS);
        }
        string str = lines_read;
        string trimStr = trim(str);
        if(trimStr == "")continue;
        shellEnv.addHistory(trimStr);

        vector<vector<char*>> parsedList = parse(lines_read);

        for (auto& command_parts : parsedList) {
            processCommand(command_parts);
        }

        free(lines_read); // Free the memory allocated by readline
    }

    return 0;
}

string trim(const string& str) {
    auto start = find_if_not(str.begin(), str.end(), ::isspace);

    if (start == str.end()) {
        return "";
    }
    auto end = find_if_not(str.rbegin(), reverse_iterator(start), ::isspace).base();

    return str.substr(start-str.begin(), end-start);
}
