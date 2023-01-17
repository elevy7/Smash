# Smash - Small-Shell Project
This "small shell" supports a limited subset of linux shell commands with a special "Jobs-List" and Modified signal Handlers(SIGSTP,SIGINT,SIGALRM).

## Built-In commands 
(built-in commands ignore the '&' character and can't be run in the background)

chprompt <new-prompt> - allow the user to change the prompt displayed by the smash while waiting for the next command.

showpid - prints the smash pid.

pwd - prints the full path of the current working directory.

cd - changed directory command.

jobs - jobs command prints the jobs list which contains:
        1. unfinished jobs (which are running in the background).
        2. stopped jobs (which were stopped by pressing Ctrl+Z while they are running).
        
kill -[signum] [jobid] -  kill command sends a signal whose number is specified by [signum] to a job whose sequence ID in jobs list is [job-id] (same as job-id in jobs                           command), and prints a message reporting that the specified signal was sent to the specified job.  

fg [job-id] - fg command brings a stopped process or a process that runs in the background to the foreground.

bg [job-id] - bg command resumes one of the stopped processes in the background.

tail [-N] <file-name> - tail command prints the last N lines of the file which it is given to the standard output.

touch <file-name> <timestamp> - touch command receives 2 arguments: <timestamp> should contain time in the following format: ss:mm:hh:dd:mm:yyyy 
                                (stands for seconds, minutes, hours, day, month and year respectively).
                                This command will update the file’s last access and modification timestamps to be the time specified in the <timestamp> argument.
                                
timeout <duration> <command> - sets an alarm for ‘duration’ seconds, and runs the given ‘command’ as though it was given to the smash directly, and when the time is up                                  it shall send a SIGKILL to the given command’s process (unless it’s the smash itself).

quit [kill] - quit command exits the smash. If the kill argument was specified, kills all of its unfinished and stopped jobs before exiting.

***Pipes and IO redirection:

This smash code supports simple IO redirection and pipes features. each typed command could have up to one character of pipe or IO redirection. 

Supported IO redirection characters: “>” and “>>”.

Supported Pipe characters: “|” and “|&”.

## External Commands:
any command that is not a built-in command counts as "External Command" and will also be executed by the smash calling "/bin/bash" with the given command 

**for further information and precise commands description view the attached pdf file.
