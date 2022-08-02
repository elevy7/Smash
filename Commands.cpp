#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"
#include <time.h>
#include <utime.h>

using namespace std;

#if 0
#define FUNC_ENTRY() \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT() \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

const std::string WHITESPACE = " \n\r\t\f\v";

string _ltrim(const std::string &s)
{
  size_t start = s.find_first_not_of(WHITESPACE);
  return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string &s)
{
  size_t end = s.find_last_not_of(WHITESPACE);
  return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string &s)
{
  return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char *cmd_line, char **args)
{
  FUNC_ENTRY()
  int i = 0;
  std::istringstream iss(_trim(string(cmd_line)).c_str());
  for (std::string s; iss >> s;)
  {
    args[i] = (char *)malloc(s.length() + 1);
    memset(args[i], 0, s.length() + 1);
    strcpy(args[i], s.c_str());
    args[++i] = NULL;
  }
  return i;

  FUNC_EXIT()
}

bool _isBackgroundComamnd(const char *cmd_line)
{
  const string str(cmd_line);
  return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char *cmd_line)
{
  const string str(cmd_line);
  // find last character other than spaces
  unsigned int idx = str.find_last_not_of(WHITESPACE);
  // if all characters are spaces then return
  if (idx == string::npos)
  {
    return;
  }
  // if the command line does not end with & then return
  if (cmd_line[idx] != '&')
  {
    return;
  }
  // replace the & (background sign) with space and then remove all tailing spaces.
  cmd_line[idx] = ' ';
  // truncate the command line string up to the last non-space character
  cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}

Command::Command(const char *cmd_line, bool built_in) 
{

  if (cmd_line == nullptr)
    c_cmd_line = "";
  else
    c_cmd_line = cmd_line;
  if(cmd_line!= nullptr)
  {
    char* non_const_cmd_line = new char[strlen(cmd_line)+1]; //create a non-const version of cmd_line //needs to be delete
    strcpy(non_const_cmd_line,cmd_line);

    if (built_in)
    {
      _removeBackgroundSign(non_const_cmd_line);
    }
    
    c_args=new char*[COMMAND_MAX_ARGS];
    c_num_of_args = _parseCommandLine(non_const_cmd_line,c_args);
    delete[] non_const_cmd_line;
  }
}

Command::~Command()
{
  for (int i=0; i< c_num_of_args; i++){
    if (c_args[i] != nullptr){
      free (c_args[i]);
    }
  }
  delete[] c_args; 
}

std::string Command::getCmdLine()
{
  return c_cmd_line; 
} 

pid_t Command::getPid() const
{
  return c_pid;
}

void Command::setPid(pid_t pid)
{
  c_pid = pid; 
}

bool Command::isANumber(const char* str)
{ 
  for (unsigned int i = (str[0] == '-') ? 1 : 0 ; i < strlen(str); i++)
  {
    if (!std::isdigit(str[i]))
        return false; 
  }
  return true;
}

BuiltInCommand::BuiltInCommand(const char *cmd_line) : Command(cmd_line,true) {}

/*CHPROMPT COMMAND***************/
ChangePromptCommand::ChangePromptCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}
void ChangePromptCommand::execute()
{
  SmallShell &smash = SmallShell::getInstance();
  std::string new_prompt = c_args[1] == nullptr ? std::string() : c_args[1];
  smash.setPrompt(new_prompt);
}
/******************CHPROMPT COMMAND*/

/*SHOWPID COMMAND***************/
ShowPidCommand::ShowPidCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}
void ShowPidCommand::execute()
{

  SmallShell &smash = SmallShell::getInstance();
  
  if (smash.isPiped())
  {
    std::cout << "smash pid is " << getppid() << std::endl;
    return;
  }
  std::cout << "smash pid is " << getpid() << std::endl;
}
/******************SHOWPID COMMAND*/

/*PWD COMMAND***************/
GetCurrDirCommand::GetCurrDirCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}
void GetCurrDirCommand::execute()
{
  char buf[PATH_MAX];
  if (getcwd(buf, PATH_MAX)==NULL)
  {
    perror("getcwd failed");
    return;
  }
  std::cout << buf << std::endl;
}
/******************PWD COMMAND*/

/*CD COMMAND***************/
ChangeDirCommand::ChangeDirCommand(const char *cmd_line /*, char **plastPwd*/) : BuiltInCommand(cmd_line) {}

void ChangeDirCommand::execute()
{
  SmallShell &smash = SmallShell::getInstance();
  const char *path = c_args[1];

  if (c_num_of_args > 2)
  {
    std::cerr << "smash error: cd: too many arguments" << std::endl;
    return;
  }
  if (c_args[1] == nullptr)
    return;

  char *current_dir = get_current_dir_name();

  if (strcmp(c_args[1], "-") == 0)
  {
    if (smash.getLastWD() == nullptr)
    {
      std::cerr << "smash error: cd: OLDPWD not set" << std::endl;
      free(current_dir);
      return;
    }
    else if (chdir(smash.getLastWD()) == -1)
    {
      perror("smash error: chdir failed");
      free(current_dir);
      return;
    }
    else
      smash.setLastWD(current_dir);
  }
  else
  {
    if (chdir(path) == -1)
    {
       perror("smash error: chdir failed");
      free(current_dir);
      return;
    }
    else
      smash.setLastWD(current_dir);
  }
  free(current_dir);
}
/******************CD COMMAND*/

/*JOBS COMMAND***************/
JobsCommand::JobsCommand(const char *cmd_line, JobsList* jobs) : BuiltInCommand(cmd_line), c_jobs(jobs) {}
void JobsCommand::execute()
{
  c_jobs->printJobsList();
}
/******************JOBS COMMAND*/

/*KILL COMMAND***************/
KillCommand::KillCommand(const char *cmd_line, JobsList* jobs) : BuiltInCommand(cmd_line), c_jobs(jobs) {}
void KillCommand::execute() 
{
  SmallShell &smash = SmallShell::getInstance();
  c_jobs->removeFinishedJobs(); 
  int signal = 0;
  char* minus_signal = nullptr;
  if(c_args[1] != nullptr)
  {
    minus_signal = c_args[1];
    char* substr = (char*)malloc(strlen(minus_signal));
    strcpy(substr, minus_signal+1);
    signal = std::atoi(substr);
    if ((!isANumber(substr)))
    {
      std::cerr << "smash error: kill: invalid arguments" << endl; 
      free(substr);
      return;
    }
    free(substr); 
  }
  
  if (c_num_of_args != 3 || strcmp(c_args[0],"kill")!=0 || minus_signal[0] != '-' || signal < 1 || signal > 31 || (!isANumber(c_args[2])) ){
    std::cerr << "smash error: kill: invalid arguments" << endl; 
    return;
  }

  int job_id = std::stoi(c_args[2]);
  JobsList::JobEntry* job = c_jobs->getJobById(job_id);
  if (job == nullptr){
    std::cerr << "smash error: kill: job-id " << job_id << " does not exist" << endl; 
    return;
  }

  int proccess_id = job->getProccessId();
  if (kill(proccess_id,signal) == -1) {
    perror("smash error: kill failed");
    return; 
  }
  else {
    std::cout << "signal number " << signal << " was sent to pid " << proccess_id << endl;
    if(signal == 9) 
    {
      smash.getJobsList()->removeJobByPid(proccess_id);
    } 
  }
}
/******************KILL COMMAND*/

/*FOREGROUND COMMAND***************/
ForegroundCommand::ForegroundCommand(const char *cmd_line, JobsList* jobs) : BuiltInCommand(cmd_line), c_jobs(jobs) {}
void ForegroundCommand::execute()
{
  SmallShell& smash = SmallShell::getInstance();
  c_jobs->removeFinishedJobs();
  pid_t job_id_to_fg;
  int status = 0;
  JobsList::JobEntry* job_entry = nullptr;
  if(c_num_of_args > 2 || (c_num_of_args == 2 && (!isANumber(c_args[1]))))
  {
    std::cerr << "smash error: fg: invalid arguments" << std::endl;
    return;
  }
  if(c_num_of_args==1) // case: fg max job id
  {
    if (c_jobs->isJobsListEmpty())
    {
      std::cerr << "smash error: fg: jobs list is empty" << std::endl;
      return;
    }
    job_id_to_fg = c_jobs->getMaxJobID();
    job_entry = c_jobs->getJobById(job_id_to_fg);
  }
  if(c_num_of_args==2) // case fg given job id
  {
    int job_id = atoi(c_args[1]);
    job_entry = c_jobs->getJobById(job_id);
    if (job_entry==nullptr)
    {
      std::cerr << "smash error: fg: job-id " << job_id << " does not exist" << std::endl;
      return;
    }
    job_id_to_fg = job_id;
  }
  // send signal (cont) and wait for procces to finish, remove from jobs and
  // if stopped again by CTRLZ the singal handler will add it back to the jobs list
  int pid_to_fg = job_entry->getProccessId();
  int kill_result = kill(pid_to_fg,SIGCONT);
  if (kill_result == -1)
  {
   perror("smash error: kill failed");
   return;
  }

  smash.setIsFg(true);
  smash.setCurrentPid(pid_to_fg);
  Command* cmd = new ExternalCommand(job_entry->getCmd().c_str(),c_jobs);
  cmd->setPid(pid_to_fg);
  smash.setCurrentCommand(cmd);

  //print command info and update smash's current pid
  std::cout<< job_entry->getCmd() << " : " << job_entry->getProccessId() << std::endl;

  waitpid(pid_to_fg,&status,WUNTRACED); 
  smash.setCurrentPid(-1);
  smash.setIsFg(false);
  
  delete cmd;
}
/******************FOREGROUND COMMAND*/

/*BACKGROUND COMMAND***************/
BackgroundCommand::BackgroundCommand(const char *cmd_line, JobsList* jobs) : BuiltInCommand(cmd_line), c_jobs(jobs) {}
void BackgroundCommand::execute()
{
  if (c_num_of_args > 2 || (c_num_of_args == 2 && (!isANumber(c_args[1])))){
    std::cerr << "smash error: bg: invalid arguments" << std::endl;
    return;
  }

  pid_t job_id_to_bg;
  JobsList::JobEntry* job_entry = nullptr;

  if(c_num_of_args==1) // case: bg max stopped job id
  {
    job_entry = c_jobs->getLastStoppedJob(&job_id_to_bg);
    if (job_entry == nullptr){
      std::cerr << "smash error: bg: there is no stopped jobs to resume" << std::endl;
      return;
    } 
  }
  if(c_num_of_args==2) // case bg given a job id
  {
    int job_id = atoi(c_args[1]);
    job_entry = c_jobs->getJobById(job_id);
    if (job_entry==nullptr)
    {
      std::cerr << "smash error: bg: job-id " << job_id << " does not exist" << std::endl;
      return;
    }
    else{
      bool is_stopped = job_entry->getIsStopped();
      if (is_stopped == false){
        std::cerr << "smash error: bg: job-id " << job_id << " is already running in the background" << std::endl;
        return; 
      }
    }
    job_id_to_bg = job_id;
  }

  //first print the cmdline of the job to be resumed, then send signal (cont)
  job_entry->setIsStopped(false);
  std::cout<< job_entry->getCmd() << " : " << job_entry->getProccessId() << std::endl;
  int pid_to_bg = job_entry->getProccessId();
  int kill_result = kill(pid_to_bg,SIGCONT);
  if (kill_result == -1){
   perror("smash error: kill failed");
   return;
  }
}
/******************BACKGROUND COMMAND*/

/*QUIT COMMAND***************/
QuitCommand::QuitCommand(const char *cmd_line, JobsList* jobs) : BuiltInCommand(cmd_line), c_jobs(jobs) {}
void QuitCommand::execute()
{
  SmallShell& smash = SmallShell::getInstance();
  if(c_args[1] != nullptr)
  {
    if(strcmp(c_args[1],"kill")==0) 
    {
      c_jobs->removeFinishedJobs();
      c_jobs->killAllJobs();
      smash.setQuit(true);
    }
  }
  else
  {
    smash.setQuit(true);
  }
}
/******************QUIT COMMAND*/

/*EXTERNAL COMMAND***************/
ExternalCommand::ExternalCommand(const char *cmd_line, JobsList* jobs) : Command(cmd_line), c_jobs(jobs) {}  //changed
void ExternalCommand::execute()
{
  if (c_args[0] == nullptr)
  {
    return;
  }
  SmallShell& smash = SmallShell::getInstance();
  bool is_bg = _isBackgroundComamnd(c_cmd_line.c_str());
  const char* cmd_line = c_cmd_line.c_str();
  int status = 0;
  char* ex_cmd_line; 
  bool is_timed = false;
  int duration; 
  
  if (strcmp(c_args[0],"timeout") == 0)//case is timeout command
  {
    if(c_num_of_args < 3 || atoi(c_args[1])<=0 ) //handle wrong syntax of timeout
    {
      cerr << "smash error: timeout: invalid arguments" << endl;
      return;
    }
    is_timed = true;
    duration = atoi(c_args[1]); 

    int pos = c_cmd_line.find("timeout");
    string fixed_cmd = c_cmd_line.substr(c_cmd_line.find_first_of(" ", pos));
    fixed_cmd = fixed_cmd.substr(fixed_cmd.find_first_not_of(" "));
    fixed_cmd = fixed_cmd.substr(fixed_cmd.find_first_of(" "));
    fixed_cmd = fixed_cmd.substr(fixed_cmd.find_first_not_of(" "));

    ex_cmd_line = new char[fixed_cmd.size()+1];
    strcpy(ex_cmd_line, fixed_cmd.c_str());
  }
  else{
    ex_cmd_line = new char[strlen(cmd_line)+1]; //create a non-const version of cmd_line //needs to be delete
    strcpy(ex_cmd_line,cmd_line);
  }
  pid_t p2;
  pid_t p = fork();
  if(p == -1)
  {
    perror("smash error: fork failed");
    delete[] ex_cmd_line;
    return;
  }
  else 
  {
    if (p == 0) // son
    {
      setpgrp();
      if(is_bg) //remove & from arguments
      {
        _removeBackgroundSign(ex_cmd_line);
        char* bash_args[] = {(char *)"/bin/bash",(char *)"-c", ex_cmd_line, NULL};
        c_pid = getpid();
        execv("/bin/bash", bash_args); 
      }
      else
      {
        char* bash_args[] = {(char *)"/bin/bash",(char *)"-c", ex_cmd_line, NULL};
        smash.setCurrentPid(getpid());
        c_pid = getpid();
        smash.setCurrentCommand(this);
        execv("/bin/bash", bash_args); 
      } 
    }
    if(p > 0) // parent
    { 
      delete[] ex_cmd_line;
      if (is_timed)
      {
        pid_t smash_pid = getpid(); 
        p2 = fork(); //another son for countdown
        if (p2 == -1)
        {
          perror("smash error: fork failed");
          return;
        }
        if (p2 == 0) //son whose aim is to count duration time untill sending alarm to smash
        {
          setpgrp();
          sleep(duration);
          if (kill(smash_pid, SIGALRM) == -1)
          {
            perror("smash error: kill failed"); 
            return;
          }
          exit(0); 
        }
        if (p2 > 0){
          TimedList& s_list = smash.getTimedList();
          s_list.addTimedEntry(p2, p, c_cmd_line, duration);
        }
      }
    
      c_pid = p;
      smash.setCurrentPid(p);
      smash.setCurrentCommand(this);

      TimedList& s_list = smash.getTimedList();
      TimedList::TimedEntry& entry = s_list.getTimedEntryByPid(p2);
      if (is_bg)
      {
        c_jobs->addJob(this);
        smash.setCurrentPid(-1);
        entry.setRunningTime(-1); // if is a background command put -1 in running time 
      }
      else  //foreground
      {  
        waitpid(p,&status,WUNTRACED); // also return if child 'p' was STOPPED
        if(is_timed) // if is a timeout command calculate actual running time of procces
        {
          entry.setRunningTime(difftime(time(nullptr),entry.getStartTime()));
        }
      }
    }
  }
}
/******************EXTERNAL COMMAND*/

/*REDIRECTION COMMAND***************/

RedirectionCommand::RedirectionCommand(const char* cmd_line, bool _append, size_t _pos): Command(cmd_line), append(_append), pos(_pos) {}
void RedirectionCommand::execute()
{
  size_t x = (append == true) ? 2 : 1;
  std::string cmd_string = c_cmd_line.substr(0,pos);
  std::string output_file_string = _trim(c_cmd_line.substr(pos+x ,c_cmd_line.size() - cmd_string.size() - x));

  if(dup2(1,5)==-1)
  {
    perror("smash error: dup2 failed");
    return;
  }
  if (close(1) == -1)
  {
    perror("smash error: close failed");
    return;
  }

  int fd;
  if (append)
  {
    fd =open(output_file_string.c_str(),O_CREAT | O_RDWR | O_APPEND, 0655); //-rw-r-xr-x
    if(fd == -1)
    {
      perror("smash error: open failed");
      dup2(5,1);
    }
  }
  else
  {
    fd = open(output_file_string.c_str(),O_CREAT | O_RDWR | O_TRUNC, 0655);
    if(fd == -1)
    {
      perror("smash error: open failed");
      dup2(5,1);
      return;
    }
  }

  SmallShell& smash = SmallShell::getInstance();
  smash.executeCommand(cmd_string.c_str());
  if(close(fd) == -1)
  {
    perror("smash error: close failed");
    dup2(5,1);
    return;
  }
  if(dup2(5,1)==-1)
  {
    perror("smash error: dup2 failed");
    return;
  }
}
/******************REDIRECTION COMMAND*/

/*PIPE COMMANDS***************/

PipeCommand::PipeCommand(const char* cmd_line, bool _ch_stdout, size_t _pos): Command(cmd_line), ch_stdout(_ch_stdout), pos(_pos) {}
void PipeCommand::execute()
{
  //divide the cmd line to its command 1 and command 2
  size_t x = (ch_stdout == true) ? 3 : 4;
  std::string cmd_1 = c_cmd_line.substr(0,pos);
  std::string cmd_2 = c_cmd_line.substr(pos+x ,c_cmd_line.size() - cmd_1.size() - x);
  
  SmallShell &smash = SmallShell::getInstance();
  int my_pipe[2]; // close shell pipe
  if (pipe(my_pipe) == -1)
  {
    perror("smash error: pipe failed");
    return;
  }
  pid_t p = fork();
  //fork command 1 and wait for it to finish
  if (p == -1){
    perror("smash error: fork failed");
    return; 
  }
  if (p == 0) { // child = execute command 1  
    setpgrp();
    if (close(my_pipe[0]) == -1) // close cmd1 pipe's read channel
    {
       perror("smash error: close failed");
    }
    
    if (ch_stdout)  // got " | "
    { 
      if (dup2(my_pipe[1],1)==-1) //cmd1 stdout -> pipe's write channel
      {
        perror("smash error: dup2 failed");
        return;
      }
    }
    else  // got " |& "
    {
      if (dup2(my_pipe[1],2)==-1)// cmd1 stderr -> pipe's write channel
      {
        perror("smash error: dup2 failed");
        return;
      }
    }
    close(my_pipe[1]);
    smash.executeCommand(cmd_1.c_str());
    exit(0);
    }
    if (p > 0) //father(smash) wait for cmd1 to finish and execute cmd2
    {
      if (close(my_pipe[1]) == -1) // close smash's pipe's write channel
      {
        perror("smash error: close failed");
        return;
      }
      if (dup2(0,5)==-1) { //save smash's stdin in fd 5 to restore it later
        perror("smash error: dup2 failed");
        return;
      }
      if (dup2(my_pipe[0],0)==-1) // pipe's read channel -> cmd2(smash) stdin
      {
        perror("smash error: dup2 failed");
        return;
      }
      if (close(my_pipe[0]) == -1) //smash doesnt need pipe anymore close its read channel
      {
       perror("smash error: close failed");
       return;
      }

      int cmd1_status = 0;
      waitpid(p,&cmd1_status,0);
      smash.setIsPiped(false);
      smash.executeCommand(cmd_2.c_str());
      if (dup2(5,0)==-1) //restore pipe's stdin
      { 
       perror("smash error: dup2 failed");
       return;
      }
    }     
}
/******************PIPE COMMANDS*/

/*TAIL COMMAND***************/
TailCommand::TailCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}
void TailCommand::execute()
{
  if (c_num_of_args > 3 || c_num_of_args < 2)
  {
    std::cerr << "smash error: tail: invalid arguments" << std::endl;
    return;
  }

  const char* minus_N = c_args[1]; 
  char* last_lines = (char*)malloc(strlen(minus_N));
  strcpy(last_lines, minus_N+1);
  
  if (c_num_of_args == 3 && ( (minus_N[0] != '-') || (!isANumber(last_lines)) ) )
  {
    std::cerr << "smash error: tail: invalid arguments" << std::endl;
    free(last_lines); 
    return;
  }
  int num_lines;
  char* file_path;
  if(c_num_of_args == 2)
  {
    num_lines = N;
    file_path = c_args[1];
  }
  else //3 args
  {
    file_path = c_args[2];
    num_lines= atoi(last_lines);   
  }
  free(last_lines);

  int fd = open(file_path, O_RDONLY);
  if (fd == -1)
  {
    perror("smash error: open failed"); 
    return;
  }

  int total_lines_counter=0, total_bytes_counter=0;
  char buf[1], prev_char[1]; //prev char will be used for counting lines 
  int res = 0;
  res = read(fd,prev_char,1);
  if (res == -1)
  {
    perror("smash error: read failed");
    close(fd);
    return;
  }
  ++total_bytes_counter;
  if (res == 0) // case file is empty (read syscall returned EOF) close file and return
  {
    if(close(fd) == -1)
    {
      perror("smash error: close failed");
    }
    return;
  }
  
  do        //counting num of lines and bytes in given file
  { 
    res = read(fd, buf, 1);
    if (res == 1 && (prev_char[0] == '\n') && (buf[0] != EOF)){   //doesnt count empty lines at the end (= count a line only if prev char is '/n' and we havent reached EOF)
      total_lines_counter++;
    }  
    prev_char[0] = buf[0];
    total_bytes_counter = (res == 1) ? total_bytes_counter + 1 : total_bytes_counter;
  } while (res == 1);

  if(res == -1)
  {
    perror("smash error: read failed");
    close(fd);
    return;
  }

  //moving seek pointer to begenning of file and moving it forward to where it should start reading from
  lseek(fd, 0, SEEK_SET);
  char* txt_buf = new char[total_bytes_counter];
  int unread_bytes =0;
  if(total_lines_counter < num_lines) //case reading the whole file
  {
    if (read(fd,txt_buf,total_bytes_counter) != total_bytes_counter)
    {
      perror("smash error: read failed");
      close(fd);
      delete[] txt_buf;
      return;
    }
   
  }
  else{ //case reading given number of lines
    int unread_lines_cnt = total_lines_counter - num_lines;
    do
    {
      res = read(fd, buf, 1);
      if (res == 1 && (buf[0] == '\n')){ 
        unread_lines_cnt--;
      }
      unread_bytes = (res == 1) ? unread_bytes + 1 : unread_bytes;     

    } while(res == 1 && unread_lines_cnt >= 0);
    if(res == -1)
    {
     perror("smash error: read failed");
     close(fd);
     delete[] txt_buf;
     return;
    }
  
    if (read(fd,txt_buf,total_bytes_counter) == -1) 
    {
      perror("smash error: read failed");
      close(fd);
      delete[] txt_buf;
    }
  }

  //here, txt buf has needed info, write it to the screen  
  if(close(fd) == -1)
  {
    perror("smash error: close failed"); 
    return;
  }

  if (write(1, txt_buf, total_bytes_counter - unread_bytes) == -1)
  {
    perror("smash error: write failed");
    return;
  }
  delete[] txt_buf;
}
/******************TAIL COMMAND*/

/*TOUCH COMMAND***************/
TouchCommand::TouchCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}
void TouchCommand::execute()
{
  if (c_num_of_args != 3){
    std::cerr << "smash error: touch: invalid arguments" << std::endl;
    return;
  }
  struct tm info = {0};
  const char* file = c_args[1];

  char delim[] = ":";
  char *ptr = strtok(c_args[2], delim);
  char* date_cmps[6];
  int i = 0;
  while(ptr != nullptr)
	{
		date_cmps[i] = new char[strlen(ptr) + 1]; 
    strcpy(date_cmps[i],ptr);
    ptr = strtok(nullptr, delim);
    ++i;
	}

  i=0;
  info.tm_sec = atoi(date_cmps[i++]);
  info.tm_min = atoi(date_cmps[i++]);
  info.tm_hour = atoi(date_cmps[i++]);
  info.tm_mday = atoi(date_cmps[i++]);
  info.tm_mon = atoi(date_cmps[i++]) -1;
  info.tm_year = atoi(date_cmps[i]) - 1900;

  for (int i = 0; i < 6; i++)
  {
    delete[] date_cmps[i];
  }

  time_t ret = mktime(&info);
  if (ret == -1)
  {
    perror("smash error: mktime failed");
    return;
  }

  struct utimbuf time;
  time.actime = ret;
  time.modtime = ret;

  if(utime(file, &time) == -1)
  {
    perror("smash error: utime failed");
    return;
  }
}
/******************TOUCH COMMAND*/

/*JOBLIST COMMANDS***************/

//JOB ENTRY COMMANDS
void JobsList::JobEntry::setJobID(pid_t jobId)
{
  job_id = jobId; 
}

pid_t JobsList::JobEntry::getJobID() const
{
  return job_id;
}

void JobsList::JobEntry::setCmd(std::string cmd_line)
{
  cmd = cmd_line; 
}

std::string JobsList::JobEntry::getCmd() const
{
  return cmd;
}

void JobsList::JobEntry::setProccessId(pid_t proccessId)
{
  proccess_id = proccessId; 
} 

pid_t JobsList::JobEntry::getProccessId() const
{
  return proccess_id;
} 

void JobsList::JobEntry::setInitTime(time_t initTime)
{
  init_time = initTime;
}

time_t JobsList::JobEntry::getInitTime() const
{
  return init_time;
}

bool JobsList::JobEntry::getIsStopped() const
{
  return is_stopped;
}

void JobsList::JobEntry::setIsStopped(bool isStopped)
{
  is_stopped = isStopped;
}
bool JobsList::JobEntry::getIsFinished() const
{
  return is_finished; 
}

void JobsList::JobEntry::setIsFinished(bool isFinished) 
{
  is_finished = isFinished; 
}

//JobsList functions
JobsList::~JobsList()
{
  for (int i = jobs_list.size()-1; i>=0; i--)
  {
    JobEntry* temp;
    temp = jobs_list[i];
    jobs_list.erase(jobs_list.begin()+i);
    delete temp; 
  }
}

void JobsList::addJob(Command *cmd, bool isStopped)
{
  removeFinishedJobs();
  JobsList::JobEntry* new_job = new JobsList::JobEntry();
  pid_t curr_job_id = getMaxJobID()+1;

  new_job->setJobID(curr_job_id); 
  new_job->setProccessId(cmd->getPid());
  new_job->setCmd(cmd->getCmdLine());
  time_t init_time;
  time(&init_time); 
  new_job->setInitTime(init_time);
  new_job->setIsStopped(isStopped);
  new_job->setIsFinished(false);

  // insert to vector
  if (jobs_list.empty()){
    jobs_list.push_back(new_job);
  }
  else{
    std::vector<JobEntry*>::iterator it = jobs_list.begin(); 
    for (unsigned int i = 0; i < jobs_list.size(); i++)
    {
      if (jobs_list[i]->getJobID() < curr_job_id)
      {
        it++;
      }
    }
    jobs_list.insert(it, new_job); 
  }  
}

void JobsList::killAllJobs()
{
  int kill_result;
  std::cout << "smash: sending SIGKILL signal to " << jobs_list.size()  << " jobs:" << std::endl;
  for (size_t i=0; i< jobs_list.size(); i++)
  {
    pid_t pid_to_kill = jobs_list[i]->getProccessId();
    kill_result = kill(pid_to_kill,SIGKILL);
    if(kill_result == -1)
    {
      perror("smash error: kill failed");
      return;     
    }
    std::cout << jobs_list[i]->getProccessId() << ": " << jobs_list[i]->getCmd() << std::endl;
  }  
}

void JobsList::printJobsList()
{
  removeFinishedJobs();

  for (size_t i=0; i< jobs_list.size(); i++)
  {
    time_t print_time; 
    time(&print_time);
    std::cout << "[" << jobs_list[i]->getJobID() << "] "   
    << jobs_list[i]->getCmd() << " : " << jobs_list[i]->getProccessId() << " "
    << difftime(print_time ,jobs_list[i]->getInitTime()) << " secs"; 
    if (jobs_list[i]->getIsStopped()){
       std::cout << " (stopped)";
    }
    std::cout << endl;   
  }
}

void JobsList::removeFinishedJobs()
{
  pid_t p = waitpid(-1,nullptr,WNOHANG);
  while(p>0) {
      removeJobByPid(p);
      p = waitpid(-1 , nullptr, WNOHANG);
  }
}

void JobsList::removeJobByPid(pid_t p)
{
  for (int i = jobs_list.size()-1; i>=0; i--)
  {
    JobEntry* temp;
    if (jobs_list[i]-> getProccessId()== p){
      temp = jobs_list[i];
      jobs_list.erase(jobs_list.begin()+i);
      delete temp; 
    }
  }
}

pid_t JobsList::getMaxJobID()
{
  if (jobs_list.empty())
    return 0;
                                          
  pid_t max_job_id = jobs_list[0]->getJobID(); 

  for (size_t i=0 ; i< jobs_list.size(); i++)
  {
    if (jobs_list[i]->getJobID() > max_job_id){
      max_job_id = jobs_list[i]->getJobID(); 
    }
  }
  return max_job_id; 
}

JobsList::JobEntry* JobsList::getJobById(int jobId)
{
  for (size_t i=0; i< jobs_list.size(); i++)
  {
    if (jobs_list[i]->getJobID() == jobId){
      return jobs_list[i];
    }
  }
  return  nullptr; 
}

JobsList::JobEntry* JobsList::getJobByPid(int job_pid)
{
  for (size_t i=0; i< jobs_list.size(); i++)
  {
    if (jobs_list[i]->getProccessId() == job_pid){
      return jobs_list[i];
    }
  }
  return  nullptr; 
}

bool JobsList::isJobsListEmpty()
{
  return (jobs_list.empty());
}

JobsList::JobEntry* JobsList::getLastStoppedJob(int *jobId) ///changed
{
  if (isJobsListEmpty()){
    jobId = nullptr;
    return nullptr; 
  }
  pid_t max_stopped_job_id = jobs_list[0]->getJobID();
  bool is_first_stopped = jobs_list[0]->getIsStopped();

  for (size_t i=0; i< jobs_list.size(); i++)
  {
    if ((jobs_list[i]->getJobID() > max_stopped_job_id) && (jobs_list[i]->getIsStopped() == true) ){
      max_stopped_job_id = jobs_list[i]->getJobID(); 
    }
  }

  if (max_stopped_job_id == jobs_list[0]->getJobID())
  {
    if (is_first_stopped == true){
      *jobId = jobs_list[0]->getJobID();
      return getJobById(jobs_list[0]->getJobID());
    }
    else{ //first is not stopped
      jobId = nullptr;
      return nullptr;
    }
  }

  *jobId = max_stopped_job_id; 
  return getJobById(max_stopped_job_id);
}
/******************JOBLIST COMMANDS*/

/*TIMEOUT COMMANDS***************/
TimedList::TimedEntry::TimedEntry(pid_t _pid, pid_t _pid_to_kill, std::string _cmd_to_kill, int _duration, double _running_time) : pid(_pid), pid_to_kill(_pid_to_kill), cmd_to_kill(_cmd_to_kill), duration(_duration), running_time(_running_time) 
{
  time(&start_time);
}

pid_t TimedList::TimedEntry::getPid() const
{
  return pid;
}

void TimedList::TimedEntry::setPid(pid_t _pid)
{
  pid = _pid; 
}

pid_t TimedList::TimedEntry::getPidToKill() const
{
  return pid_to_kill; 
}

void TimedList::TimedEntry::setPidToKill(pid_t _pid_to_kill)
{
  pid_to_kill = _pid_to_kill;
}

std::string TimedList::TimedEntry::getCmdToKill() const
{
  return cmd_to_kill;
}

void TimedList::TimedEntry::setCmdToKill(std::string _cmd_to_kill)
{
  cmd_to_kill = _cmd_to_kill; 
}

int TimedList::TimedEntry::getDuration() const
{
  return duration; 
}

time_t TimedList::TimedEntry::getStartTime() const
{
  return start_time; 
}

double TimedList::TimedEntry::getRunningTime() const
{
  return running_time;
}
void TimedList::TimedEntry::setRunningTime(double _running_time)
{
  running_time = _running_time;
}

void TimedList::addTimedEntry(pid_t _pid, pid_t _pid_to_kill, std::string _cmd, int _duration)
{
  TimedEntry entry(_pid, _pid_to_kill, _cmd, _duration); 
  timedList.push_front(entry);
}

void TimedList::removeTimedEntry (pid_t _pid)
{
  list<TimedList::TimedEntry>::iterator itr;
  
  for (itr = timedList.begin();  itr != timedList.end(); itr++)
  { 
    if (itr->getPid() == _pid){
      timedList.erase(itr);
      return;

    }
  }
}

TimedList::TimedEntry& TimedList::getTimedEntryByPid(pid_t pid)
{
  list<TimedList::TimedEntry>::iterator itr;
  
  for (itr = timedList.begin();  itr != timedList.end(); itr++)
  { 
    if (itr->getPid() == pid)
    {
      return *itr; 
    }
  }
  return *itr; // wont get here
}

bool TimedList::isTimedOut(pid_t _pid)
{
  TimedEntry& entry = getTimedEntryByPid(_pid);
  double running_time = difftime(time(nullptr),entry.getStartTime());
  cout << running_time << endl;
  return (running_time <= entry.getDuration());
}
/******************TIMEOUT COMMANDS*/

/*SMALLSHELL COMMANDS***************/
SmallShell::SmallShell() : current_prompt("smash> "), lastwd(nullptr), s_jobs(nullptr), s_quit(false), s_is_piped(false), s_timedlist()
{
  s_jobs = new JobsList();
}

SmallShell::~SmallShell()
{
  if(lastwd!=nullptr)
  {
    free(lastwd);
  }
  if(s_jobs != nullptr)
  {
    delete s_jobs;
  }
}

/**
 * Creates and returns a pointer to Command class which matches the given command line (cmd_line)
 */
Command *SmallShell::CreateCommand(const char *cmd_line)
{
  string cmd_s = _trim(string(cmd_line));
  string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));

  size_t pos = cmd_s.find(">>");
  bool append=false;
  bool redirected = false;
  if(pos != string::npos)
  {
    redirected = true;
    append = true;
  }
  else{
    pos = cmd_s.find(">");  
    if (pos != string::npos)
    { 
    redirected = true;
    }  
  }
  if  (redirected == true){
    return new RedirectionCommand(cmd_line, append, pos); 
  }

  pos = cmd_s.find(" | ");
  bool ch_stdout = false; 
  bool piped = false; 
  if(pos != string::npos)
  {
    piped = true;
    ch_stdout = true;
  }
  else{
    pos = cmd_s.find(" |& ");  
    if (pos != string::npos)
    { 
      piped = true;
    }  
  }
  if  (piped == true){
    s_is_piped = true;
    return new PipeCommand(cmd_line, ch_stdout, pos); 
  }

  if (firstWord.compare("pwd") == 0 || firstWord.compare("pwd&") == 0)
  {
    return new GetCurrDirCommand(cmd_line);
  }
  if (firstWord.compare("showpid") == 0 || firstWord.compare("showpid&") == 0)
  {
    return new ShowPidCommand(cmd_line);
  }
  if (firstWord.compare("chprompt") == 0 || firstWord.compare("chprompt&") == 0)
  {
    return new ChangePromptCommand(cmd_line);
  }
  if (firstWord.compare("cd") == 0 || firstWord.compare("cd&") == 0)
  {
    return new ChangeDirCommand(cmd_line);
  }
  if (firstWord.compare("jobs") == 0  || firstWord.compare("jobs&") == 0)
  {
    return new JobsCommand(cmd_line,s_jobs);
  }
  if (firstWord.compare("kill") == 0 || firstWord.compare("kill&") == 0) 
  {
    return new KillCommand(cmd_line,getJobsList());
  }
  if (firstWord.compare("fg") == 0 || firstWord.compare("fg&") == 0)
  {
    return new ForegroundCommand(cmd_line,getJobsList());
  }
  if (firstWord.compare("bg") == 0 || firstWord.compare("bg&") == 0) 
  {
    return new BackgroundCommand(cmd_line,getJobsList());
  }
  if (firstWord.compare("tail") == 0 || firstWord.compare("tail&") == 0)
  {
    return new TailCommand(cmd_line);
  }
  if (firstWord.compare("quit") == 0 || firstWord.compare("quit&") == 0)
  {
    return new QuitCommand(cmd_line,getJobsList());
  }
  if (firstWord.compare("touch") == 0 || firstWord.compare("touch&") == 0)
  {
    return new TouchCommand(cmd_line);
  }
  else {
    return new ExternalCommand(cmd_line, s_jobs);
  }
  return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line)
{
  Command *cmd = CreateCommand(cmd_line);
  cmd->execute();
  delete cmd;
}

void SmallShell::setPrompt(std::string new_prompt)
{
  if (new_prompt.size() == 0)
    current_prompt = "smash> ";
  else
    current_prompt = new_prompt + "> ";
}

std::string SmallShell::getPrompt()
{
  return current_prompt;
}

void SmallShell::setLastWD(const char *new_lastwd)
{
  if (lastwd != nullptr)
  {
    free(lastwd);
  }
  lastwd = (char *)malloc(strlen(new_lastwd) + 1);
  strcpy(lastwd, new_lastwd);
}

char *SmallShell::getLastWD()
{
  if (lastwd == nullptr)
  {
    return nullptr;
  }
  return lastwd;
}

void SmallShell::setCurrentPid(pid_t curr_pid)
{
  s_current_pid = curr_pid;
}

pid_t SmallShell::getCurrentPid()
{
  return s_current_pid;
}

JobsList* SmallShell::getJobsList()
{
  return s_jobs;
}

TimedList& SmallShell::getTimedList()
{
  return s_timedlist; 
}

bool SmallShell::isPiped()
{
  return s_is_piped;
}

void SmallShell::setIsPiped(bool is_piped)
{
  s_is_piped = is_piped;
}

bool SmallShell::isFg()
{
  return s_is_fg;
}
void SmallShell::setIsFg(bool is_fg)
{
  s_is_fg = is_fg;
}

void SmallShell::setQuit(bool quit_)
{
    s_quit=quit_;
}

bool SmallShell::getQuit() const
{
return s_quit;
} 

Command* SmallShell::getCurrentCommand() const
{
  return s_current_command;
}

void SmallShell::setCurrentCommand(Command* command)
{
  s_current_command=command;
} 
/***************SMALLSHELL COMMANDS*/