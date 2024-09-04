#include "ShellEnv.h"
#include <unistd.h>
#include <pwd.h>
#include <limits.h>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
void ShellEnv::initialize(){
    getShellName();
    getUserName();
    getShellDirectory();
    getCurrentDirectory();
    getHistoryFile();
}


void ShellEnv::addHistory(string &lines_read) {
    if (!history.empty() && history.back() == lines_read) return;
    if(lines_read == "") return;
    
    history.push_back(lines_read);

    if (history.size() > 20) {
        history.erase(history.begin());
    }

    writeHistory();
}

void ShellEnv::writeHistory() {
    FILE* file_stream = fopen((shell_dir + history_path).c_str(), "w");
    if (!file_stream) {
        perror("Error opening history file for writing");
        return;
    }

    for (const auto& entry : history) {
        if (fputs(entry.c_str(), file_stream) == EOF) {
            perror("Error writing to history file");
            fclose(file_stream);
            return;
        }
        if (fputc('\n', file_stream) == EOF) {
            perror("Error writing newline to history file");
            fclose(file_stream);
            return;
        }
    }
    fclose(file_stream);
}


void ShellEnv::getHistoryFile() {
    std::string dir_path = shell_dir + "/.history";
    std::string file_path = dir_path + "/history.txt";

    struct stat info;
    if (stat(dir_path.c_str(), &info) != 0) {
        if (mkdir(dir_path.c_str(), 0755) != 0) {
            perror("Error creating .history directory");
            return;
        }
    } else if (!(info.st_mode & S_IFDIR)) {
        std::cerr << "Error: " << dir_path << " is not a directory" << std::endl;
        return;
    }

    FILE* file_stream = fopen(file_path.c_str(), "r");
    if (!file_stream) {
        std::ofstream new_file(file_path);
        if (!new_file) {
            perror("Error creating history.txt file");
            return;
        }
        new_file.close();
        file_stream = fopen(file_path.c_str(), "r");
        if (!file_stream) {
            perror("Error opening history.txt file for reading after creation");
            return;
        }
    }

    char* line = nullptr;
    size_t len = 0;
    ssize_t read;

    while ((read = getline(&line, &len, file_stream)) != -1) {
        if (line[read - 1] == '\n') {
            line[read - 1] = '\0';
        }
        history.push_back(std::string(line));
    }

    free(line);
    fclose(file_stream);
}

int ShellEnv::setDirectory(const string& new_dir){

    if (chdir(new_dir.c_str()) == 0){
        prev_dir = curr_dir; // Update previous directory
        getCurrentDirectory(); // Update curr_dir and formatted_dir
    } else {
        cerr << "Failed to change directory to: " << new_dir << endl;
        return -1;
    }
    return 0;
}

void ShellEnv::getShellDirectory(){
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != nullptr){
        shell_dir = cwd;
        home_dir=shell_dir; // Comment this line to set HOME=home/user-name --
    } else {
        shell_dir = home_dir;
    }    
}

void ShellEnv::getShellName(){
    char hostname[HOST_NAME_MAX];
    if (gethostname(hostname, sizeof(hostname)) == 0){
        system_name = hostname;
    } else {
        system_name = "Unknown";
    }
}

void ShellEnv::getUserName(){
    struct passwd *pw = getpwuid(getuid());
    if (pw){
        user_name = pw->pw_name;
        home_dir = pw->pw_dir;  
    } else {
        user_name = "Unknown";
        home_dir = "/";
    }
}

void ShellEnv::getCurrentDirectory(){
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != nullptr){
        curr_dir = cwd;

        if (curr_dir.find(home_dir) == 0){
            formatted_dir = curr_dir;
            formatted_dir.replace(0, home_dir.length(), "~");
        } else {
            formatted_dir = curr_dir;
        }
    } else {
        curr_dir = home_dir;
        formatted_dir = "~";
    }
}


string ShellEnv::getFormattedPrompt(){
   return user_name + "@" + system_name + ":" + formatted_dir + ">";
}
