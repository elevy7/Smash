#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;

void ctrlZHandler(int sig_num) {
	std::cout << "smash: got ctrl-Z" << std::endl;

  SmallShell& smash = SmallShell::getInstance();
  pid_t curr_pid = smash.getCurrentPid();

  JobsList* s_jobs_list = smash.getJobsList(); 

  if (curr_pid != -1 && (kill(curr_pid,0) == 0))
  { 
    if(kill(curr_pid,SIGSTOP) == -1)
    {
      perror("smash error: kill failed");
      return;
    }
  
  if (!smash.isFg())
  {
     s_jobs_list->addJob(smash.getCurrentCommand(), true);
  }
  else
  {
    JobsList::JobEntry * job = s_jobs_list->getJobByPid(curr_pid);
    job->setIsStopped(true);
  }
  
  std::cout << "smash: process " << curr_pid << " was stopped" << std::endl;
  smash.setCurrentPid(-1);
  }
}

void ctrlCHandler(int sig_num)
{
	std::cout << "smash: got ctrl-C" << std::endl;
  SmallShell& smash = SmallShell::getInstance();
  pid_t curr_pid= smash.getCurrentPid();
  if (curr_pid != -1 && (kill(curr_pid,0) == 0))
  {
    if(kill(curr_pid,SIGKILL) == -1)
    {
      perror("smash error: kill failed");
      return;
    }
    std::cout << "smash: process " << curr_pid << " was killed" << std::endl;
    smash.setCurrentPid(-1);
    smash.getJobsList()->removeJobByPid(curr_pid);
  }
}

void alarmHandler (int sig_num, siginfo_t *info, void *ucontext)
{
  std::cout << "smash: got an alarm" << std::endl;
  pid_t bro_pid = info->si_pid; //bro_pid = pid of the alarm sending procces

  SmallShell& smash = SmallShell::getInstance();
  JobsList* jobs_list = smash.getJobsList();

  TimedList& s_list = smash.getTimedList(); 
  TimedList::TimedEntry& entry = s_list.getTimedEntryByPid(bro_pid);
  pid_t to_kill = entry.getPidToKill();

  
  //print only if procces was running while alarm was given
  //if its a foreground procces and running time is 0 = the alarm was set before the procces had finished - a valid case of printing
  //if its a background procces we check if it still exists in the joblist while getting the alarm -(was running when got the alarm) a valid case of printing
  jobs_list->removeFinishedJobs();
  if (entry.getRunningTime() == entry.getDuration() || entry.getRunningTime() == 0 || (entry.getRunningTime() == -1 && jobs_list->getJobByPid(to_kill) != nullptr)) 
  { 
    if (kill(to_kill, 0) != 0){ //process doesn't exist - do nothing
    return; 
    }
    if (kill(to_kill, SIGKILL) == -1){
    perror("smash error: kill failed");
    return;
    }
    std::cout << "smash: " << entry.getCmdToKill() << " timed out!" << std::endl;
  }
  
  //process was killed, remove it from jobsList and timedList
  s_list.removeTimedEntry(bro_pid);
  jobs_list->removeJobByPid(to_kill);

}
