/*--------------------------------------------------------------------

File: dp.c
Student name: 
Student id:   

Description:  Double pipe program.  To pipe the output from the standard
          output to the standard input of two other processes.
          Usage:  dp  <cmd1> : <cmd2> : <cmd3
          Output from process created with cmd1 is piped to
          processes created with cmd2 and cmd3

-------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#define READ_END 0
#define WRITE_END 1
/* prototypes */
const int BUF_SIZE = 4096;
int doublePipe(char **,char **,char **);
void handleStatus(int);
/*--------------------------------------------------------------------

File: dp.c

Description: Main will parse the command line arguments into three arrays
         of strings one for each command to execv().
--------------------------------------------------------------------*/

int main(int argc, char *argv[])
{

   int i,j;         /*indexes into arrays */
   char *cmd1[10];  /*array for arguments of first command */
   char *cmd2[10];  /*array for arguments of second command */
   char *cmd3[10];  /*array for arguments of third command */
   if(argc == 1)
   {
     printf("Usage: dp <cmd1 arg...> : <cmd2 arg...> : <cmd3 arg....>\n");
     exit(1);
   }

   /* get the first command */
   for(i=1,j=0 ; i<argc ; i++,j++)
   {
      if(!strcmp(argv[i],":")) break; /* found first command */
      cmd1[j]=argv[i];
   }
   cmd1[j]=NULL;
   if(i == argc) /* missing : */
   {
      printf("Bad command syntax - only one command found\n");
      exit(1);
   }
   else i++;

   /* get the second command */
   for(j=0 ; i<argc ; i++,j++)
   {
      if(!strcmp(argv[i],":")) break; /* found second command */
      cmd2[j]=argv[i];
   }
   cmd2[j]=NULL;
   if(i == argc) /* missing : */
   {
      printf("Bad command syntax - only two commands found\n");
      exit(1);
   }
   else i++;

   /* get the third command */
   for(j=0 ; i<argc ; i++,j++) cmd3[j]=argv[i];
   cmd3[j]=NULL;
   if(j==0) /* no command after last : */
   {
      printf("Bad command syntax - missing third command\n");
      exit(1);
   }

   exit(doublePipe(cmd1,cmd2,cmd3));
}

/*--------------------------------------------------------------------------
  ----------------- You have to implement this function --------------------
  --------------------------------------------------------------------------
Function: doublePipe()

Description:  Starts three processes, one for each of cmd1, cmd2, and cmd3.
          The parent process will receive the output from cmd1 and
          copy the output to the other two processes.
-------------------------------------------------------------------------*/

int doublePipe(char **cmd1, char **cmd2, char **cmd3)
{
  int fd[6];
  int status;
  pid_t pid_1, pid_2, pid_3;

  // File Descriptors for 3 processes
  for (int i=0; i<6; i+=2) {
    if (pipe(&fd[i]) == -1) {
      fprintf(stderr, "Pipe creation failed");
      exit(1);
    }
  }

  pid_1 = fork();
  if (pid_1 == -1) {
    fprintf(stderr, "Fork failed!");
    exit(1);
  }
  else if (pid_1 == 0) {
    // Child Process
    close(fd[READ_END]);
    dup2(fd[WRITE_END], STDOUT_FILENO);
    execvp(cmd1[0], cmd1);

    // Only here if execvp failed
    exit(1);
  }
  close(fd[WRITE_END]);
  waitpid(pid_1, &status, 0);
  handleStatus(status);

  // Capture output of cmd1
  char buf[BUF_SIZE];
  int nbytes = read(fd[READ_END], &buf, sizeof(buf));
  
  // Pipe to next two commands
  pid_2 = fork();
  if (pid_2 == -1) {
    fprintf(stderr, "Fork failed!");
    exit(1);
  }
  else if (pid_2 == 0) {
    // Child Process
    close(fd[WRITE_END+2]);
    dup2(fd[READ_END+2], STDIN_FILENO);
    execvp(cmd2[0], cmd2);

    // Only here if execvp failed
    exit(1);
  }
  close(fd[READ_END+2]);
  write(fd[WRITE_END+2], &buf, nbytes);
  close(fd[WRITE_END+2]);

  waitpid(pid_2, &status, 0);
  handleStatus(status);

  pid_3 = fork();
  if (pid_3 == -1) {
    fprintf(stderr, "Fork failed!");
    exit(1);
  }
  else if (pid_3 == 0) {
    // Child Process
    close(fd[WRITE_END+4]);
    dup2(fd[READ_END+4], STDIN_FILENO);
    execvp(cmd3[0], cmd3);

    // Only here if execvp failed
    exit(1);
  }
  close(fd[READ_END+4]);
  write(fd[WRITE_END+4], &buf, nbytes);
  close(fd[WRITE_END+4]);
  
  waitpid(pid_3, &status, 0);
  handleStatus(status);

	return 0;
}

void handleStatus(int status) {
  if (status != 0) {
    fprintf(stderr, "Error in child process, exiting\n");
    exit(1);
  }
}