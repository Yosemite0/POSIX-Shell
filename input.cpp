#include "env/ShellEnv.h"

extern ShellEnv shellEnv;

void disableCanonicalMode(struct termios& oldt) {
    struct termios newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
}

void restoreTerminalMode(struct termios& oldt) {
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
}

vector<string> getMatchingFiles(const string& prefix) {
    vector<string> matches;
    DIR* dir = opendir(".");
    if (dir == NULL) {
        perror("opendir");
        return matches;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        string name = entry->d_name;
        if(name=="." || name=="..")continue;
        if (name.find(prefix) == 0) {
            matches.push_back(name+"/");
        }
    }

    closedir(dir);
    return matches;
}

string autoComplete(const string& input) {
    size_t lastSpace = input.find_last_of(" ");
    string prefix = (lastSpace == string::npos) ? input : input.substr(lastSpace + 1);
    
    vector<string> matches = getMatchingFiles(prefix);

    if (matches.empty()) {
        return input;
    } else if (matches.size() == 1) {
        return input.substr(0, lastSpace + 1) + matches[0]; 
    } else {
        cout << endl;
        for (const auto& match : matches) {
            cout << match << "\t";
        }
        cout << endl << input;
        return input;
    }
}

string input() {
    string command;
    vector<string>& history = shellEnv.history;
    int historyIndex = history.size();
    char ch;
    struct termios oldt;
    
    // Get current terminal settings
    tcgetattr(STDIN_FILENO, &oldt);
    disableCanonicalMode(oldt);

    while (true) {
        ch = getchar();
        if (ch == 4) { // Ctrl+D
            cout << endl;
            restoreTerminalMode(oldt);
            cout<<"Exiting Terminal.. on Ctrl+D"<<endl;
            exit(0); 
        } else if (ch == '\n') { // Enter
            cout << endl;
            break;
        } else if (ch == '\t') {
            command = autoComplete(command);
            cout << "\r" << shellEnv.getFormattedPrompt()<<command;
            continue;
        } else if (ch == 127 || ch == 8) { // Backspace
            if (!command.empty()) {
                command.pop_back();
                cout << "\b \b";
            }
        } else if (ch == '\033') { 
            getchar();
            switch (getchar()) {
                case 'A': // Up arrow
                    if (historyIndex > 0) {
                        historyIndex--;
                        command = history[historyIndex];
                        cout << "\r\33[K" << shellEnv.getFormattedPrompt()<<command; // Clear line and print command
                    }
                    break;
                case 'B': // Down arrow
                    if (historyIndex < (int) history.size() - 1) {
                        historyIndex++;
                        command = history[historyIndex];
                        cout << "\r\33[K" << shellEnv.getFormattedPrompt()<<command;; // Clear line and print command
                    } else if (historyIndex == (int) history.size() - 1) {
                        historyIndex++;
                        command.clear();
                        cout << "\r\33[K" << shellEnv.getFormattedPrompt(); 
                    }
                    break;
            }
        } else {
            command.push_back(ch);
            cout << ch;
        }
    }

    restoreTerminalMode(oldt);
    return command;
}