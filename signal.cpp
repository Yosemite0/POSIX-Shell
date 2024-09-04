#include "env/ShellEnv.h"

extern ShellEnv shellEnv;

extern pid_t foregroundPid;
extern map<pid_t, int> backgroundProcesses;


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

void signalHandler(){
    signal(SIGTSTP, handleSigTSTP); // Handle CTRL-Z
    signal(SIGINT, handleSigINT);   // Handle CTRL-C
}