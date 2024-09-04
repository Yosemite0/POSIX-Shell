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
    {"history",&history},
    {"pinfo",&pinfo}
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
        pid = getpid(); 
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

    string processStatus;
    string memory;

    ifstream statusFile(statusFilePath);
    if (statusFile.is_open()) {
        string line;
        while (getline(statusFile, line)) {
            if (line.find("State:") == 0) {
                istringstream iss(line);
                string label;
                iss >> label >> processStatus;
            } else if (line.find("VmSize:") == 0) {
                istringstream iss(line);
                string label;
                iss >> label >> memory;
                memory += " kB (Virtual Memory)";
            }
        }
        statusFile.close();
    } else {
        cerr << "Could not open " << statusFilePath << endl;
        return -1;
    }

    ifstream statFile(statFilePath);
    if (statFile.is_open()) {
        string word;
        vector<string> statFields;
        while (statFile >> word) {
            statFields.push_back(word);
        }
        statFile.close();

        // Check if the process is in the foreground
        pid_t tty_pgrp = tcgetpgrp(STDIN_FILENO);
        pid_t process_pgrp = stoi(statFields[4]);
        if (tty_pgrp == process_pgrp) {
            processStatus += "+";
        }
    } else {
        cerr << "Could not open " << statFilePath << endl;
        return -1;
    }

    char exePath[PATH_MAX];
    ssize_t len = readlink(exeFilePath.c_str(), exePath, sizeof(exePath) - 1);
    if (len != -1) {
        exePath[len] = '\0';
    } else {
        cerr << "Could not get executable path for PID: " << pid << endl;
        return -1;
    }

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

string getFilePermissions(mode_t mode) {
    string permissions;

    permissions += (S_ISDIR(mode)) ? 'd' : '-';
    permissions += (mode & S_IRUSR) ? 'r' : '-';
    permissions += (mode & S_IWUSR) ? 'w' : '-';
    permissions += (mode & S_IXUSR) ? 'x' : '-';
    permissions += (mode & S_IRGRP) ? 'r' : '-';
    permissions += (mode & S_IWGRP) ? 'w' : '-';
    permissions += (mode & S_IXGRP) ? 'x' : '-';
    permissions += (mode & S_IROTH) ? 'r' : '-';
    permissions += (mode & S_IWOTH) ? 'w' : '-';
    permissions += (mode & S_IXOTH) ? 'x' : '-';

    return permissions;
}

// Helper function to print file information in long format
void printFileInformation(const string& directory, const string& fileName) {
    struct stat fileStat;
    string filePath = directory + "/" + fileName;

    if (stat(filePath.c_str(), &fileStat) < 0) {
        perror("stat");
        return;
    }

    string permissions = getFilePermissions(fileStat.st_mode);
    string owner = getpwuid(fileStat.st_uid)->pw_name;
    string group = getgrgid(fileStat.st_gid)->gr_name;

    // Get last modified time
    char timeStr[20];
    strftime(timeStr, sizeof(timeStr), "%b %d %H:%M", localtime(&fileStat.st_mtime));

    cout << permissions << " "
         << fileStat.st_nlink << " "
         << owner << " "
         << group << " "
         << setw(8) << fileStat.st_size << " "
         << timeStr << " "
         << fileName << endl;
}

int calculateTotalBlocks(const string& directory, bool showAll) {
    DIR* dir = opendir(directory.c_str());
    if (dir == NULL) {
        perror("opendir");
        return 0;
    }

    int totalBlocks = 0;
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (!showAll && entry->d_name[0] == '.') {
            continue;
        }

        struct stat fileStat;
        string filePath = directory + "/" + entry->d_name;
        if (stat(filePath.c_str(), &fileStat) == 0) {
            totalBlocks += fileStat.st_blocks;
        }
    }

    closedir(dir);
    return totalBlocks / 2;
}

// Helper func -- list directory
void listDirectory(const string& directory, bool showAll, bool longFormat) {
    DIR* dir = opendir(directory.c_str());
    if (dir == NULL) {
        perror("opendir");
        return;
    }

    if (longFormat) {
        int totalBlocks = calculateTotalBlocks(directory, showAll);
        cout << "total " << totalBlocks << endl;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
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

    if (directories.empty()) {
        directories.push_back(".");
    }

    for (size_t i = 0; i < directories.size(); ++i) {
        string directory = getAbsolutePath(directories[i]);
        if (directory.empty()) {
            perror(("Directort not found: "+directories[i]).c_str());
            continue;
        }
        if (directories.size() > 1) {
            if (i > 0) cout << endl; 
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