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

    string modifiableString = preParsedString;
    char* modified_lines = new char[modifiableString.size() + 1];
    strcpy(modified_lines, modifiableString.c_str());

    char* tok;
    vector<char*> pre_commands;

    char* saveptr;
    tok = strtok_r(modified_lines, ";", &saveptr);
    while (tok != NULL) {
        pre_commands.push_back(strdup(tok));
        tok = strtok_r(NULL, ";", &saveptr);
    }
    
    vector<char*> processed_commands;
    string cmd;
    for (size_t i = 0; i < pre_commands.size(); ++i) {
        cmd = (pre_commands[i]);i++;
        while(count(cmd.begin(),cmd.end(),'\"') % 2!=0 && pre_commands[i]!=NULL){
            cmd+=(";"+(string)pre_commands[i++]);
        }
        processed_commands.push_back(strdup(cmd.c_str()));
        cmd.clear();
    }
    if(!cmd.empty())processed_commands.push_back(strdup(cmd.c_str()));
    for(int i = 0 ; i < (int)pre_commands.size(); ++i) free(pre_commands[i]);

    vector<vector<char*>> intermediate;
    for (auto cmd : processed_commands) {
        vector<char*> tokens;
        char* sub_tok;
        char* sub_saveptr;
        sub_tok = strtok_r(cmd, "\"", &sub_saveptr);
        while (sub_tok != NULL) {
            string arg = (sub_tok);
            if(arg[0] == '~')arg.replace(0,1,shellEnv.home_dir);
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
    delete[] modified_lines;

    return parsedList;
}


