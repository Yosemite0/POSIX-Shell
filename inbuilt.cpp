#include "./env/ShellEnv.h"

using namespace std;

extern ShellEnv shellEnv;

extern map<pid_t, int> backgroundProcesses;
extern pid_t foregroundPid;


typedef int (*BuiltInCommandFunc)(vector<char*>&);

int cd(vector<char*>& arguments);
int pwd(vector<char*>& arguments);
int echo(vector<char*>& arguments);
int ls(vector<char*>& arguments);
int pinfo(vector<char*>& arguments);
int fg(vector<char*>& arguments);
int history(vector<char*>& arguments);
int search(vector<char*>& arguments);

unordered_map<string, BuiltInCommandFunc> builtInCommands = {
    {"cd", &cd},
    {"pwd", &pwd},
    {"echo", &echo},
    {"pinfo", &pinfo},
    {"ls", &ls},
    {"fg",&fg},
    {"search", &search},
    {"history",&history}
};
unordered_map<string, BuiltInCommandFunc> notChild = {
    {"cd", &cd},
    {"pwd", &pwd},
    {"fg",&fg},
    {"history",&history}
};


void addBackgroundProcess(pid_t pid, int jobNumber) {
    backgroundProcesses[pid] = jobNumber;
}

void removeBackgroundProcess(pid_t pid) {
    backgroundProcesses.erase(pid);
}



void fgCommand(int processPid) {
    pid_t pid = processPid;

    if (pid == -1) {
        std::cerr << "fg: no such job" << std::endl;
        return;
    }

    // Bring the process group to the foreground
    tcsetpgrp(STDIN_FILENO, pid);

    kill(-pid, SIGCONT);

    int status;
    waitpid(pid, &status, WUNTRACED);

    tcsetpgrp(STDIN_FILENO, getpgrp());

    if (WIFEXITED(status) || WIFSIGNALED(status)) {
        removeBackgroundProcess(pid);
    }
}

int fg(vector<char*>& arguments){
    if (arguments.size() > 1 || arguments.size() == 0) {
        cerr << "Invalid Argument" << endl;
        return -1;
    }
    else fgCommand(atoi(arguments[0]));
    return 0;
}

int history(vector<char*>& arguments){
    if (arguments.size() > 1) {
        cerr << "Invalid Argument" << endl;
        return -1;
    }  
    int len;
    if(arguments.size()==0) len = 10;
    else len = atoi(arguments[0]);
    
    for(int i = max(0,(int)shellEnv.history.size() - len); i < (int)shellEnv.history.size(); i++){
        cout<<shellEnv.history[i]<<endl;
    };
    return 0;
}

// pinfo command function
int pinfo(vector<char*>& arguments) {
    pid_t pid;
    if (arguments.empty()) {
        pid = getpid(); // Current shell process ID
    } else {
        pid = atoi(arguments[0]);
        if (pid <= 0) {
            cerr << "Invalid PID: " << arguments[0] << endl;
            return -1;
        }
    }

    string pidStr = to_string(pid);
    string statusFilePath = "/proc/" + pidStr + "/status";
    string statFilePath = "/proc/" + pidStr + "/stat";
    string exeFilePath = "/proc/" + pidStr + "/exe";

    // Read process status
    ifstream statusFile(statusFilePath);
    string line;
    string processStatus;
    string memory;
    
    if (statusFile.is_open()) {
        while (getline(statusFile, line)) {
            if (line.find("State:") == 0) {
                istringstream iss(line);
                string label;
                iss >> label >> processStatus;
            } else if (line.find("VmSize:") == 0) {
                istringstream iss(line);
                string label;
                iss >> label >> memory;
                memory = memory + " kB";
            }
        }
        statusFile.close();
    } else {
        cerr << "Could not open " << statusFilePath << endl;
        return -1;
    }

    // Determine if the process is in the foreground
    ifstream statFile(statFilePath);
    if (statFile.is_open()) {
        string word;
        for (int i = 1; i <= 7; ++i) {
            statFile >> word;
        }
        if (word != "0") {
            processStatus += "+";
        }
        statFile.close();
    } else {
        cerr << "Could not open " << statFilePath << endl;
        return -1;
    }

    // Get executable path
    char exePath[PATH_MAX];
    ssize_t len = readlink(exeFilePath.c_str(), exePath, sizeof(exePath) - 1);
    if (len != -1) {
        exePath[len] = '\0';
    } else {
        cerr << "Could not get executable path for PID: " << pid << endl;
        return -1;
    }

    // Print the process information
    cout << "pid -- " << pid << endl;
    cout << "Process Status -- " << processStatus << endl;
    cout << "memory -- " << memory << endl;
    cout << "Executable Path -- " << exePath << endl;

    return 0;
}

int cd(vector<char*>& arguments) {
    if (arguments.size() > 1) {
        cerr << "Invalid Argument" << endl;
        return -1;
    }
    if (arguments.empty()) {
        return shellEnv.setDirectory(shellEnv.home_dir);
    }
    string new_dir = arguments[0];
    if (new_dir == "-") {
        cout << shellEnv.prev_dir << endl;
        return shellEnv.setDirectory(shellEnv.prev_dir);
    }
    if (new_dir[0] == '~') new_dir.replace(0, 1, shellEnv.home_dir);
    return shellEnv.setDirectory(new_dir);
}

int pwd(vector<char*>& arguments) {
    cout << shellEnv.curr_dir << endl;
    return 0;
}

int echo(vector<char*>& arguments) {
    for (auto argument : arguments)
        cout << argument << " ";
    cout << endl;
    return 0;
}

// Helper function to get absolute path
string getAbsolutePath(string& relativePath) {
    char absolutePath[PATH_MAX];
    if (relativePath[0] == '~') relativePath.replace(0, 1, (shellEnv.home_dir + '/').c_str());
    if (realpath(relativePath.c_str(), absolutePath) != NULL) {
        return absolutePath;
    } else {
        perror("realpath");
        return "";
    }
}

// Helper function to print file information
void printFileInformation(const string& dir, string name) {
    struct stat st;
    if (stat((dir + "/" + name).c_str(), &st) == 0) {
        cout << setw(10) << st.st_size << " ";
        cout << put_time(localtime(&st.st_mtime), "%b %d %H:%M") << " ";
        cout << setw(5) << st.st_nlink << " ";
        cout << (S_ISDIR(st.st_mode) ? "d" : "-");
        cout << (st.st_mode & S_IRUSR ? "r" : "-");
        cout << (st.st_mode & S_IWUSR ? "w" : "-");
        cout << (st.st_mode & S_IXUSR ? "x" : "-");
        cout << (st.st_mode & S_IRGRP ? "r" : "-");
        cout << (st.st_mode & S_IWGRP ? "w" : "-");
        cout << (st.st_mode & S_IXGRP ? "x" : "-");
        cout << (st.st_mode & S_IROTH ? "r" : "-");
        cout << (st.st_mode & S_IWOTH ? "w" : "-");
        cout << (st.st_mode & S_IXOTH ? "x" : "-");
        cout << " ";
        cout << name << endl;
    } else {
        perror("stat");
    }
}

// Helper function to list directory contents
void listDirectory(const string& directory, bool showAll, bool longFormat) {
    DIR* dir = opendir(directory.c_str());
    if (dir == NULL) {
        perror("opendir");
        return;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        // Skip hidden files if -a is not specified
        if (!showAll && entry->d_name[0] == '.') {
            continue;
        }

        if (longFormat) {
            printFileInformation(directory, entry->d_name);
        } else {
            cout << entry->d_name << " ";
        }
    }

    if (!longFormat) {
        cout << endl;
    }

    closedir(dir);
}

int ls(vector<char*>& arguments) {
    vector<string> directories;
    bool showAll = false;
    bool longFormat = false;

    // Parse command-line arguments from vector<char*>
    for (auto arg : arguments) {
        string argument(arg);
        if (argument == "-a") {
            showAll = true;
        } else if (argument == "-l") {
            longFormat = true;
        } else if (argument == "-la" || argument == "-al") {
            showAll = true;
            longFormat = true;
        } else if (argument[0] == '-') {
            cerr << "Unknown option: " << argument << endl;
            return 1;
        } else {
            directories.push_back(argument);
        }
    }

    // If no directory is specified, use the current directory
    if (directories.empty()) {
        directories.push_back(".");
    }

    // Process each directory
    for (size_t i = 0; i < directories.size(); ++i) {
        string directory = getAbsolutePath(directories[i]);
        if (directory.empty()) {
            cerr << "Failed to get absolute path for " << directories[i] << endl;
            continue;
        }

        // Print directory name if there are multiple directories
        if (directories.size() > 1) {
            if (i > 0) cout << endl; // Separate listings with a newline
            cout << directory << ":" << endl;
        }

        listDirectory(directory, showAll, longFormat);
    }

    return 0;
}
bool isNotChildable(string& command) {
    if(notChild.find(command) != notChild.end()){
        return true;
    }    
    else return false;
}

bool isBuiltInCommand(string& command) {
    if(builtInCommands.find(command) != builtInCommands.end()){
        return true;
    }
    else return false;
}

int executeBuiltInCommand(string& command, vector<char*>& arguments) {
    auto it = builtInCommands.find(command);
    if (it != builtInCommands.end()) {
        BuiltInCommandFunc func = it->second;
        return func(arguments);
    } else {
        cerr << "Unknown built-in command: " << command << endl;
        return -1;
    }
}

bool searchFileOrDir(const string& dirPath, const string& name) {
    DIR* dir = opendir(dirPath.c_str());
    if (dir == nullptr) {
        return false;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (string(entry->d_name) == "." || string(entry->d_name) == "..") {
            continue;
        }

        string fullPath = dirPath + "/" + entry->d_name;

        if (entry->d_name == name) {
            closedir(dir);
            return true;
        }

        if (entry->d_type == DT_DIR) {
            if (searchFileOrDir(fullPath, name)) {
                closedir(dir);
                return true;
            }
        }
    }

    closedir(dir);
    return false;
}

int search(vector<char*>& arguments) {
    if (arguments.size() != 1) {
        cerr << "Usage: search <filename>" << endl;
        return -1;
    }
    string name = arguments[0];
    string currentDir = ".";
    if (searchFileOrDir(currentDir, name)) {
        cout << "True" << endl;
    } else {
        cout << "False" << endl;
    }

    return 0;
}