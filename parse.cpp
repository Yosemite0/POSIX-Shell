#include "env/ShellEnv.h"


extern ShellEnv shellEnv;

vector<vector<char*>> parse(char* lines_read);


vector<vector<char*>> parse(char* lines_read) {
    string preParsedString = lines_read;
    string target = "\"\"";
    size_t pos = 0;
    while ((pos = preParsedString.find(target, pos)) != string::npos) {
        preParsedString.replace(pos, target.length(), "");
    }

    vector<char*> pre_commands;
    stringstream ss(preParsedString);
    string token;

    while (getline(ss, token, ';')) {
        pre_commands.push_back(strdup(token.c_str()));
    }
    
    vector<char*> processed_commands;
    for (size_t i = 0; i < pre_commands.size(); ++i) {
        string cmd(pre_commands[i]);
        free(pre_commands[i]);

        // Handle commands with unmatched quotes
        while (count(cmd.begin(), cmd.end(), '\"') % 2 != 0 && i + 1 < pre_commands.size()) {
            cmd += ";" + string(pre_commands[++i]);
        }
        
        processed_commands.push_back(strdup(cmd.c_str()));
    }

    vector<vector<char*>> intermediate;
    for (auto cmd : processed_commands) {
        vector<char*> tokens;
        char* sub_tok;
        char* sub_saveptr;
        
        sub_tok = strtok_r(cmd, "\"", &sub_saveptr);
        while (sub_tok != NULL) {
            string arg(sub_tok);
            if (arg[0] == '~') {
                arg.replace(0, 1, shellEnv.home_dir);
            }
            tokens.push_back(strdup(arg.c_str()));
            sub_tok = strtok_r(NULL, "\"", &sub_saveptr);
        }
        intermediate.push_back(tokens);
        free(cmd);
    }

    vector<vector<char*>> parsedList;
    for (auto vec : intermediate) {
        vector<char*> tokens;
        for (size_t j = 0; j < vec.size(); ++j) {
            if (j % 2) {
                tokens.push_back(vec[j]);
            } else {
                char* sub_tok;
                char* sub_saveptr;
                sub_tok = strtok_r(vec[j], " ", &sub_saveptr);
                while (sub_tok != NULL) {
                    tokens.push_back(strdup(sub_tok));
                    sub_tok = strtok_r(NULL, " ", &sub_saveptr);
                }
                free(vec[j]);
            }
        }
        parsedList.push_back(tokens);
    }

    return parsedList;
}


