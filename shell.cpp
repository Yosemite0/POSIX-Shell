#include "./env/ShellEnv.h"

#include "input.cpp"
#include "parse.cpp"
#include "inbuilt.cpp"
#include "execute.cpp"
#include "signal.cpp"


using namespace std;


ShellEnv shellEnv;

pid_t foregroundPid;
map<pid_t, int> backgroundProcesses;

//input
extern string input();

// parse
extern vector<vector<char*>> parse(char* lines_read);

//execution
extern void processCommand(vector<char*>& command_parts);

//Signal
extern void signalHandler();

//helper 
string trim(const string& str);



int main() {
    shellEnv.initialize();

    foregroundPid = -1;
    signalHandler();

    while (true) {
        cout<<shellEnv.getFormattedPrompt();
        string line_read = input();
        string trimStr = trim(line_read);
        if(trimStr == "")continue;
        shellEnv.addHistory(trimStr);

        vector<vector<char*>> parsedList = parse(strdup(line_read.data()));

        for (auto& command_parts : parsedList) {
            processCommand(command_parts);
        }
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
