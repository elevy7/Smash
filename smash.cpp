#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include "Commands.h"
#include "signals.h"

int main(int argc, char* argv[]) {
  
    struct sigaction sa = {0};
    
    sa.sa_sigaction = alarmHandler; 
    sigisemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_SIGINFO; 

    if(signal(SIGTSTP , ctrlZHandler)==SIG_ERR) {
        perror("smash error: failed to set ctrl-Z handler");
    }
    if(signal(SIGINT , ctrlCHandler)==SIG_ERR) {
        perror("smash error: failed to set ctrl-C handler");
    }
    if(sigaction(SIGALRM, &sa, nullptr) == -1){
        perror("smash error: failed to set alarm handler");
    }

    SmallShell& smash = SmallShell::getInstance();
    smash.setCurrentPid(-1);
    
    while(!smash.getQuit()) {
        std::cout << smash.getPrompt();
        std::string cmd_line;
        std::getline(std::cin, cmd_line);
        smash.executeCommand(cmd_line.c_str());
    }
    return 0;
}