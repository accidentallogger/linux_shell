#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdbool.h>

char *read_line(){
	int bufs = 1024;
	int position=0;
	char *buffer = malloc(sizeof(char) *bufs);
	int c;
	
	if(!buffer){
	fprintf(stderr,"memory allocation error\n");
	exit(EXIT_FAILURE);
	}
	
	while(1){
		c=getchar();
		if(c==EOF || c == '\n'){
			buffer[position]='\0';
			return buffer;
		}else{
			buffer[position]=c;
		}
		
		position++;
		
		if(position>=bufs){
		bufs+= 1024;
		buffer=realloc(buffer,bufs);
		if (!buffer) {
        fprintf(stderr, "allocation error\n");
        exit(EXIT_FAILURE);
      }
}
	}
}

char **split_line(char *line){
	int buf = 64,position=0;
	char **tokens=malloc(buf * sizeof(char*));
	char *token;
	
	if(!tokens){
	fprintf(stderr,"memory allocation error\n");
	exit(EXIT_FAILURE);}
	
	token = strtok(line," \t\r\n\a");
	while(token!=NULL){
		tokens[position]=token;
		position++;
		
	
	
	
	if(position>=buf){
		buf+=64;
		tokens=realloc(tokens,buf * sizeof(char*));
		if(!tokens){
			fprintf(stderr,"memory allocation error\n");
			exit(EXIT_FAILURE);	
		
		}
	
	}
	token=strtok(NULL," \t\r\n\a");
	}
	
	tokens[position]=NULL;
	//puts(*tokens);
	return tokens;
}

int launch(char **args){
	pid_t pid, wpid;
	int status;
 	int input_fd, output_fd;

   int i = 0;
   int j=0;

   
    while (args[i] != NULL) {
    signal(SIGQUIT, SIG_DFL);
        if (strcmp(args[i], "<") == 0) {
            input_fd = open(args[i+1], O_RDONLY);
            if (input_fd < 0) {
                perror("open");
                return 1;
            }
            dup2(input_fd, STDIN_FILENO);
            close(input_fd);
            args[i] = NULL;
        }
        else if (strcmp(args[i], ">") == 0) {
            output_fd = open(args[i+1], O_WRONLY | O_CREAT | O_TRUNC, 1024);
            if (output_fd < 0) {
                perror("open");
                return 1;
            }

            dup2(output_fd, STDOUT_FILENO);
            close(output_fd);
            args[i] = NULL;
        }
        i++;
    }   //for handling i/o stream redirection.
    
    
    
	pid = fork();
	if(pid==0){
		if(execvp(args[0],args)==-1){
			perror("err");
		
		}
		exit(EXIT_FAILURE);
	}else if(pid<0){
		perror("err");
	}else{
	
	do{
	wpid=waitpid(pid,&status,WUNTRACED);
	}while(!WIFEXITED(status)&&!WIFSIGNALED(status));}
	return 1;
}

int help(char **args);
int exit_this();
int cd_this(char **args);

char *builtin_str[] = {
  "help",
  "exit_this",
  "cd_this"
};

int (*builtin_func[]) (char **) = {
  &help,
  &exit_this,
  &cd_this
};

int num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}
int cd_this(char **args){
if (args[1] == NULL) {
    fprintf(stderr, "expected argument to \"cd\"\n");
  } else {
  chdir(args[1]);
    if (chdir(args[1]) != 0) {
      perror("err");
    }
  }
  return 1;
} //not working
int help(char **args)
{
  int i;
  printf("execute all installed commands.\n");
  printf("The following are built in functions:\n");

  for (i = 0; i < num_builtins(); i++) {
    printf("  %s\n", builtin_str[i]);
  }

  printf("Use the man command for extra info.\n");
  return 1;
}

int exit_this()
{
exit(EXIT_FAILURE);
  return 0;
}     // this function not working actual exit handled in execute function.

int execute(char **args){





if(args[0]==NULL){

return 1;
}

//check for background process &

int k=0;
bool bgp = false;
while(args[k]!=NULL){
if(strcmp(args[k],"&")==0){
args[k]=NULL;
bgp=true;
break;
}k++;}

if(bgp){
pid_t pid123 =fork();
if(pid123==0){
exit(EXIT_FAILURE);

}else if(pid123<0){
perror("could not fork");
}else{
printf("Background process with PID %d started\n", pid123);
            return 1;
}
}else{


    // Check for pipe symbol "|" in the command
    int j = 0;
    while (args[j] != NULL) {
        if (strcmp(args[j], "|") == 0) {
            // Split the command into two parts before and after the pipe symbol
            args[j] = NULL; // remove pipe
            char **args1 = args; // First part before the pipe symbol
            char **args2 = args + j + 1; // Second part after the pipe symbol

            int pipefd[2];
            if (pipe(pipefd) == -1) {
                perror("pipe");
                return 1;
            }

            pid_t pid1 = fork();
            if (pid1 == 0) {
                // Child process for the first command
                close(pipefd[0]); // Close the read end of the pipe
                dup2(pipefd[1], STDOUT_FILENO); // Redirect stdout to the write end of the pipe
                close(pipefd[1]); // Close the write end of the pipe
                // Execute the first command
               if (execvp(args1[0], args1) == -1) {
                    perror("execvp");
                    exit(EXIT_FAILURE);
                }
            } else if (pid1 > 0) {
                // Parent process
                pid_t pid2 = fork();
                if (pid2 == 0) {
                    // Child process for the second command
                    close(pipefd[1]); // Close the write end of the pipe
                    dup2(pipefd[0], STDIN_FILENO); // Redirect stdin to the read end of the pipe
                    close(pipefd[0]); // Close the read end of the pipe
                    // Execute the second command
                    if (execvp(args2[0], args2) == -1) {
                        perror("execvp");
                        exit(EXIT_FAILURE);
                    }
                } else if (pid2 > 0) {
                    // Parent process
                    close(pipefd[0]); // Close the read end of the pipe in the parent process
                    close(pipefd[1]); // Close the write end of the pipe in the parent process
                    // Wait for both child processes to finish
                    waitpid(pid1, NULL, 0);
                    waitpid(pid2, NULL, 0);
                } else {
                    perror("fork");
                    return 1;
                }
            } else {
                perror("fork");
                return 1;
            }
            return 1; // Command executed with pipe, no need to proceed further
        }
        j++;
    }



if(strcmp(args[0],"cd_this")==0){
if (chdir(args[1]) != 0) {
      perror("err");
      return 1;
    }else{
chdir(args[1]);
 return 1;
}
    
   
}
if (strcmp(args[0], "exit_this") == 0) {
return 0;
   //     exit(EXIT_SUCCESS);
    }  // handles exit. 
//puts(*args);
for(int i=0;i<num_builtins();i++){
if(strcmp(args[0],builtin_str[i])==0){
return (*builtin_func[i])(args);

}
return launch(args);
}










}




}




void loop(){
	char *line;
	char **args;
	int status=1;
	
	while(status==1){
	char *bufmdir;
bufmdir=(char *)malloc(100*sizeof(char));
getcwd(bufmdir,100);
		printf("%s $ ",bufmdir);
		line = read_line();
		args=split_line(line);
		status = execute(args);
		free(line);
		free(args);
	}
}







int main(){


loop();

return 0;



}
