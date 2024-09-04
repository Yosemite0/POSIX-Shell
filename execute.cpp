#include "./env/ShellEnv.h"


extern void handleRedirection(vector<char*>& arguments, int& input_fd, int& output_fd);

//built-in
extern bool isBuiltInCommand(string& command);
extern bool isNotChildable(string& command);
extern int executeBuiltInCommand(string& command, vector<char*>& arguments);

extern map<pid_t, int> backgroundProcesses;
extern pid_t foregroundPid;

int jobCounter = 1;

void processCommand(vector<char*>& command_parts);
int executeSingleCommand(vector<char*>& command_parts, int input_fd, int output_fd, bool isFirstCommand, bool isBackground);
void executeCommands(vector<vector<char*>>& commands, bool isBackground) ;



void setupRedirection(int& input_fd, int& output_fd);
void setupPipe(int pipefd[2], int& output_fd);
//handle i/o
void handleRedirection(vector<char*>& arguments, int& input_fd, int& output_fd);

void setupRedirection(int& input_fd, int& output_fd) {
    if (input_fd != STDIN_FILENO) {
        dup2(input_fd, STDIN_FILENO);
        close(input_fd);
    }
    if (output_fd != STDOUT_FILENO) {
        dup2(output_fd, STDOUT_FILENO);
        close(output_fd);
    }
}

void setupPipe(int pipefd[2], int& output_fd) {
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(1);
    }
    output_fd = pipefd[1];
}


int executeSingleCommand(vector<char*>& command_parts, int input_fd, int output_fd, bool isFirstCommand, bool isBackground) {
    string command = command_parts[0];
    vector<char*> arguments(command_parts.begin() + 1, command_parts.end());
    handleRedirection(arguments, input_fd, output_fd);

    if (isBuiltInCommand(command) && isNotChildable(command) && isFirstCommand) {
        int status = executeBuiltInCommand(command, arguments);
    return status;
    } else {
        pid_t pid = fork();
        if (pid == 0) { // Child process
            if (input_fd != STDIN_FILENO) {
                dup2(input_fd, STDIN_FILENO);
                close(input_fd);
            }
            if (output_fd != STDOUT_FILENO) {
                dup2(output_fd, STDOUT_FILENO);
                close(output_fd);
            }
            if(isBuiltInCommand(command) && !isNotChildable(command)){
                int status = executeBuiltInCommand(command, arguments);
                exit(status==-1);
            }
            else{
                arguments.push_back(nullptr);
                arguments.insert(arguments.begin(), const_cast<char*>(command.c_str()));
                execvp(command.c_str(), arguments.data());
                perror("execvp failed");
                exit(1);
            }
        } else if (pid > 0) { // Parent process
            if (!isBackground) {
                foregroundPid = pid;
                int status;
                waitpid(pid, &status, WUNTRACED);

                foregroundPid = -1;

                if (WIFSTOPPED(status)) {
                    backgroundProcesses[pid] = jobCounter++;
                    cout << "Process stopped with PID: " << pid << endl;
                }
                return status;
            } else {
                setpgid(pid, pid);
                backgroundProcesses[pid] = jobCounter++;
                cout << "Process running in background with PID: " << pid << endl;
                return 0;
            }
        } else {
            perror("fork failed");
            exit(1);
        }
    }
}

void executeCommands(vector<vector<char*>>& commands, bool isBackground) {
    int input_fd = STDIN_FILENO;
    int pipefd[2];

    for (size_t i = 0; i < commands.size(); ++i) {
        int output_fd = STDOUT_FILENO;

        if (i < commands.size() - 1) {
            if (pipe(pipefd) == -1) {
                perror("pipe failed");
                exit(1);
            }
            output_fd = pipefd[1];
        }

        int status = executeSingleCommand(commands[i], input_fd, output_fd, i == 0, isBackground);
        if(status == -1){
            perror(("error executing "+(string)commands[i][0]).c_str());
        }
        if (input_fd != STDIN_FILENO) {
            close(input_fd);
        }
        if (output_fd != STDOUT_FILENO) {
            close(output_fd);
        }
        if (i < commands.size() - 1) {
            close(pipefd[1]);
            input_fd = pipefd[0];
        }
    }
}
void processCommand(vector<char*>& command_parts) {
        // Split commands by pipe '|'
        vector<vector<char*>> pipedCommands;
        vector<char*> currentCommand;

        bool isBackground = !command_parts.empty() && strcmp(command_parts.back(), "&") == 0;
        if (isBackground) {
            command_parts.pop_back();
        }
        for (auto& part : command_parts) {
            if (strcmp(part, "|") == 0) {
                pipedCommands.push_back(currentCommand);
                currentCommand.clear();
            } else {
                currentCommand.push_back(part);
            }
        }

        pipedCommands.push_back(currentCommand); // Add the last command
        
        // for(auto i : pipedCommands) {for(auto j : i)cout<<j<<"::";cout<<endl;}
        
        executeCommands(pipedCommands, isBackground);
}
void handleRedirection(vector<char*>& arguments, int& input_fd, int& output_fd) {
    for (auto it = arguments.begin(); it != arguments.end();) {
        if (strcmp(*it, "<") == 0 && it + 1 != arguments.end()) {
            input_fd = open(*(it + 1), O_RDONLY);
            if (input_fd < 0) {
                cerr << "Failed to open input file: " << *(it + 1) << endl;
                exit(1);
            }
            it = arguments.erase(it, it + 2);
        } else if (strcmp(*it, ">") == 0 && it + 1 != arguments.end()) {
            output_fd = open(*(it + 1), O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (output_fd < 0) {
                cerr << "Failed to open output file: " << *(it + 1) << endl;
                exit(1);
            }
            it = arguments.erase(it, it + 2);
        } else if (strcmp(*it, ">>") == 0 && it + 1 != arguments.end()) {
            output_fd = open(*(it + 1), O_WRONLY | O_CREAT | O_APPEND, 0644);
            if (output_fd < 0) {
                cerr << "Failed to open output file: " << *(it + 1) << endl;
                exit(1);
            }
            it = arguments.erase(it, it + 2);
        } else {
            ++it;
        }
    }
}

