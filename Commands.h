#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include <time.h>
#include <cmath>
#include <cctype>
#include <limits.h>
#include <fcntl.h>
#include <utime.h>
#include <list>


#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)
#define N 10

class JobsList;
 
class Command
{
protected:
  char** c_args;
  std::string c_cmd_line;
  pid_t c_pid;
  int c_num_of_args;
  bool isANumber(const char* str);
  bool is_built_in;

public:
  Command(const char *cmd_line, bool is_built_in = false);  
  virtual ~Command();
  virtual void execute() = 0;
  virtual pid_t getPid() const; 
  virtual void setPid(pid_t pid);
  virtual std::string getCmdLine(); 
};

class BuiltInCommand : public Command
{
public:
  BuiltInCommand(const char *cmd_line);
  virtual ~BuiltInCommand() = default;
};

class ExternalCommand : public Command
{
  JobsList* c_jobs;
public:
  ExternalCommand(const char *cmd_line, JobsList* jobs);
  virtual ~ExternalCommand() = default;
  void execute() override; 
};

class PipeCommand : public Command
{
  bool ch_stdout;
  size_t pos;
public:
  PipeCommand(const char *cmd_line, bool ch_stdout, size_t pos);
  virtual ~PipeCommand() {}
  void execute() override;
};

// redirect output of first command to give out put file after " < " or " << " ( spaces are necessary for identification of command)
// we assume redirected commands are ignoring &(background sign), and cannot be killed or stoped
class RedirectionCommand : public Command
{
  bool append;
  size_t pos; 
public:
  explicit RedirectionCommand(const char *cmd_line, bool _append, size_t pos);
  virtual ~RedirectionCommand() {}
  void execute() override;
};

class ChangeDirCommand : public BuiltInCommand
{
  public:
  ChangeDirCommand(const char *cmd_line);
  virtual ~ChangeDirCommand() = default;
  void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand
{
public:
  GetCurrDirCommand(const char *cmd_line);
  virtual ~GetCurrDirCommand() = default;
  void execute() override;
};

class ShowPidCommand : public BuiltInCommand
{
public:
  ShowPidCommand(const char *cmd_line);
  virtual ~ShowPidCommand() = default;
  void execute() override;
};

class JobsList;
class QuitCommand : public BuiltInCommand
{
  JobsList* c_jobs;
  public:
  QuitCommand(const char *cmd_line, JobsList *jobs);
  virtual ~QuitCommand() = default;
  void execute() override;
};

class ChangePromptCommand : public BuiltInCommand
{
public:
  ChangePromptCommand(const char *cmd_line);
  virtual ~ChangePromptCommand() = default;
  void execute() override;
};

class JobsList
{

public:
  class JobEntry
  { 
    std::string cmd;
    pid_t job_id;
    pid_t proccess_id;
    time_t init_time; 
    bool is_stopped; 
    bool is_finished;

  public:
    JobEntry() = default; 
    ~JobEntry() = default;
    void setJobID(pid_t job_id); 
    pid_t getJobID() const; 
    void setCmd(std::string cmd);
    std::string getCmd() const;
    void setProccessId(pid_t proccess_id); 
    pid_t getProccessId() const;
    void setInitTime(time_t init_time);
    time_t getInitTime() const; 
    bool getIsStopped() const;
    void setIsStopped(bool isStopped);
    bool getIsFinished() const; 
    void setIsFinished(bool isFinished);
  };
  std::vector<JobEntry*> jobs_list;

  JobsList() = default;
  ~JobsList(); 
  void addJob(Command *cmd, bool isStopped = false);
  void printJobsList();
  void killAllJobs(); //print all jobs
  void removeFinishedJobs();
  JobEntry *getJobById(int jobId);
  void removeJobById(int jobId);
  void removeJobByPid(pid_t p);
  JobEntry *getLastJob(int *lastJobId);
  JobEntry *getLastStoppedJob(int *jobId);

  JobEntry *getJobByPid(int job_pid);
  pid_t getMaxJobID();
  bool isJobsListEmpty();   
};

class TimedList{

public:
 class TimedEntry{
    pid_t pid; //pid of son counting time
    pid_t pid_to_kill; //pid of his "sibling" to be killed
    std::string cmd_to_kill;
    time_t start_time;
    int duration;
    double running_time;

  public:
    TimedEntry() = default; 
    TimedEntry(pid_t _pid, pid_t _pid_to_kill, std::string _cmd_to_kill, int duration, double running_time = 0);
    ~TimedEntry() = default;
    pid_t getPid() const; 
    void setPid(pid_t _pid);
    pid_t getPidToKill() const; 
    void setPidToKill(pid_t _pid_to_kill);
    std::string getCmdToKill() const; 
    void setCmdToKill(std::string _cmd_to_kill);
    int getDuration() const;
    time_t getStartTime() const;
    double getRunningTime() const;
    void setRunningTime(double running_time);
 };

 std::list<TimedEntry> timedList;

 TimedList() = default; 
 ~TimedList() = default; 
 void addTimedEntry(pid_t _pid, pid_t _pid_to_kill, std::string _cmd, int _duration);
 void removeTimedEntry (pid_t _pid);
 TimedEntry& getTimedEntryByPid(pid_t pid);
 bool isTimedOut(pid_t _pid); // returns true if (current_time - start_time) <= duration_time
};


class JobsCommand : public BuiltInCommand
{
  JobsList* c_jobs; 
public:
  JobsCommand(const char *cmd_line, JobsList *jobs);
  virtual ~JobsCommand() {}
  void execute() override;
};

class KillCommand : public BuiltInCommand
{
  JobsList* c_jobs;
public:
  KillCommand(const char *cmd_line, JobsList *jobs);
  virtual ~KillCommand() {}
  void execute() override;
};

class ForegroundCommand : public BuiltInCommand
{
  JobsList* c_jobs;
public:
  ForegroundCommand(const char *cmd_line, JobsList *jobs);
  virtual ~ForegroundCommand() {}
  void execute() override;
};

class BackgroundCommand : public BuiltInCommand
{
  JobsList* c_jobs;
public:
  BackgroundCommand(const char *cmd_line, JobsList *jobs);
  virtual ~BackgroundCommand() {}
  void execute() override;
};

class TailCommand : public BuiltInCommand
{
public:
  TailCommand(const char *cmd_line);
  virtual ~TailCommand() = default;
  void execute() override;
};

class TouchCommand : public BuiltInCommand
{
public:
  TouchCommand(const char *cmd_line);
  virtual ~TouchCommand() {}
  void execute() override;
};

class SmallShell
{
private:
  std::string current_prompt;
  char* lastwd;
  JobsList* s_jobs;
  pid_t s_current_pid;
  bool s_quit;
  Command* s_current_command;
  bool s_is_piped;
  bool s_is_fg;
  TimedList s_timedlist;  

  SmallShell();

public:
  Command *CreateCommand(const char *cmd_line);
  SmallShell(SmallShell const &) = delete;     // disable copy ctor
  void operator=(SmallShell const &) = delete; // disable = operator
  static SmallShell &getInstance()             // make SmallShell singleton
  {
    static SmallShell instance; // Guaranteed to be destroyed.
    // Instantiated on first use.
    return instance;
  }
  ~SmallShell();
  void executeCommand(const char *cmd_line);

  void setPrompt(std::string new_prompt);
  std::string getPrompt();
  void setLastWD(const char *new_lastwd);
  char* getLastWD();
  void setCurrentPid(pid_t curr_pid);
  pid_t getCurrentPid();
  JobsList* getJobsList();
  TimedList& getTimedList();

  void setQuit(bool quit_);
  bool getQuit() const;
  Command* getCurrentCommand() const;
  void setCurrentCommand(Command* command); 
  bool isPiped();
  void setIsPiped(bool is_piped);
  bool isFg();
  void setIsFg(bool is_fg);
  pid_t getPidToKill () const;
  void setPidToKill (pid_t pid);
  std::string getCmdToKill () const;
  void setCmdToKill (std::string cmd);
};

#endif // SMASH_COMMAND_H_
