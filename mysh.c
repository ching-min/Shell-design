#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <signal.h> 
#include <sys/wait.h> 

 
#define PATH_BUFSIZE 1024 
#define COMMAND_BUFSIZE 1024 
#define TOKEN_BUFSIZE 64 
#define TOKEN_DELIMITERS " \t\r\n\a" 
#define BACKGROUND_EXECUTION 0 
#define FOREGROUND_EXECUTION 1 
#define PIPELINE_EXECUTION 2 
 
 
struct command_segment { //#7
    char **args;   // arguments array 
    struct command_segment *next; 
    pid_t pid;   // process ID 
    pid_t pgid;   // process group ID 
}; 

 
struct command { //#4
    struct command_segment *root;   // a linked list 
    int mode;   // BACKGROUND_EXECUTION or FOREGROUND_EXECUTION 
}; 

 
int mysh_cd(char *path) { //#3
    /* Implement cd command */  
	return chdir(path);
} 

 
int mysh_fg(pid_t pid) { //#7
    /* Implement fg command */ 

 
} 

 
int mysh_bg(pid_t pid) { //#7
    /* Implement bg command */ 

 
} 

 
int mysh_kill(pid_t pid) { //#7
    /* Implement kill command */ 

 
} 

 
int mysh_exit() { //#3
    /* Release all the child processes */ 

 

 
    /* Exit the program */ 
    exit(0); 
} 

 
int mysh_execute_builtin_command(struct command_segment *segment) { //#3
    /* Match if command name (i.e. segment->args[0]) is a internal command */ 
	
	if(strcmp(segment->args[0],"cd")==0){
		int task=mysh_cd(segment->args[1]);
		if(task==-1){printf("\E[0;31;40mcd %s: No such file or directiry\E[0;37;40m\n",segment->args[1]);return 1;}
		return 1;
	}else if(strcmp(segment->args[0],"exit")==0){printf("Goodbye!\n");mysh_exit();return 1;}
	return 0;
} 

 
int mysh_execute_command_segment(struct command_segment *segment, int in_fd, int out_fd, int mode, int pgid) { //#3
    // Check if it's a built-in command first 
    if (mysh_execute_builtin_command(segment)==1) { 
        return 0; 
    }else{
		segment->pid=fork();
		pid_t p=segment->pid;
		if(p<0){
			printf("\E[0;31;40mfork failed\E[0;37;40m\n");
		}else if(p==0){
			printf("\E[0;35;40mCommand executed by pid=%d\E[0;37;40m\n",segment->pid);
			sleep(3);
			int e;
			e=execvp(segment->args[0],segment->args);
			if(e==-1){printf("\E[0;31;40m-mysh: %s: command not found\E[0;37;40m\n",segment->args[0]);}
		}else{
			int status;
			if(mode==1){
				printf("\E[0;35;40mCommand executed by pid=%d\E[0;37;40m\n",segment->pid);
				int status;
				waitpid(p, &status, 0); /* wait for child process terminated */
			}else{
				printf("\E[0;35;40mCommand executed by pid=%d in background\E[0;37;40m\n",segment->pid);
			}
		}
	} 

 
    /* Fork a process and execute the program */ 

 
} 

 
int mysh_execute_command(struct command *command) { //#3 #5
    struct command_segment *cur_segment,*c; 
	int cou=0;
	for (c = command->root; c != NULL; c = c->next){cou++;}
    // Iterate the linked list of command segment 
    // If this is not a pipeline command, there is only a root segment. 
    for (cur_segment = command->root; cur_segment != NULL; cur_segment = cur_segment->next) { 
        /* Create pipe if necessary */ 
		if(cou>1){
			printf("pipe\n");
			pid_t childpid;
			int fd[2];//stdin stdout
			pipe(fd);
			childpid=fork();
			if(childpid<0){printf("\E[0;31;40mfailed\E[0;37;40m\n");}
			else if(childpid==0){
				printf("\E[0;35;40mCommand executed by pid=%d\E[0;37;40m\n",childpid);
//				dup2(fd[1],1);
				dup2(fd[1], STDOUT_FILENO);
				mysh_execute_command_segment(cur_segment,fd[0],fd[1],command->mode,getpgid(0));
//				execvp(cur_segment->args[0],cur_segment->args);
				
			}else{
				printf("\E[0;35;40mCommand executed by pid=%d\E[0;37;40m\n",childpid);
//				dup2(fd[0], 0); /* replace stdin */
				dup2(fd[0], STDIN_FILENO);
				mysh_execute_command_segment(cur_segment,fd[0],fd[1],command->mode,getpgid(0));
				int status;
				waitpid(childpid, &status, 0);
			}
//			execvp(cur_segment->args[0],cur_segment->args);
//			printf("count>2end\n");
		}else{
		/* Call mysh_execute_command_segment(...) to execute command segment */ 
			mysh_execute_command_segment(cur_segment,0,1,command->mode,getpgid(0));
		}
    } 

 
    /* Return status */ 
//	return 0;
 
} 

 
struct command_segment* mysh_parse_command_segment(char *segment) { //#2
    /* Parse command segment from a pipeline command */ 
	struct command_segment *newcomseg=(struct command_segment*)malloc(sizeof(struct command_segment));
	
	newcomseg->args=(char**)malloc(sizeof(char*)*100);
	int i=0,j=0;
	for(j=0;j<100;j++){
		newcomseg->args[j]=(char*)malloc(sizeof(char)*100);
	}
	char* st;
	st=strtok(segment,TOKEN_DELIMITERS);
	while(st!=NULL){
		strcpy(newcomseg->args[i],st);
		i++;
		st=strtok(NULL,TOKEN_DELIMITERS);
	}

	return newcomseg;
} 

 
struct command* mysh_parse_command(char *line) { //#2
    /* Parse line as command structure */ 
	struct command *newcomm=(struct command*)malloc(sizeof(struct command));
	newcomm->mode=1;
	newcomm->root=NULL;
	struct command_segment *comm_seg=(struct command_segment*)malloc(sizeof(struct command_segment));//comm_seg:new segment
	char data[100];
	int k=0,flag=1;//k:data,flag=1:root point
	int i;
	i=0;
	char *d;
	for(i=0;i<strlen(line);i++){
		if(line[i]==' '&&i!=0){
			data[k]=line[i];
			k++;
		}else if(line[i]=='&'){
			newcomm->mode=0;
			data[k]='\0';
			comm_seg=mysh_parse_command_segment(data);
			if(newcomm->root==NULL){
				comm_seg->next=NULL;
				newcomm->root=comm_seg;
				
			}else{
				struct command_segment *curr=newcomm->root;
				while(curr->next!=NULL){
					curr=curr->next;
				}
				comm_seg->next=NULL;
				curr->next=comm_seg;
				
			}
			data[0]='\0';
			k=0;
		}
		else if(line[i]=='|'||i==strlen(line)-1){//segment
			if(i==strlen(line)-1){
				data[k]=line[i];
				k++;
			}
			data[k]='\0';
			comm_seg=mysh_parse_command_segment(data);
			if(newcomm->root==NULL){
				comm_seg->next=NULL;
				newcomm->root=comm_seg;
				
			}else{
				struct command_segment *curr=newcomm->root;
				while(curr->next!=NULL){
					curr=curr->next;
				}
				comm_seg->next=NULL;
				curr->next=comm_seg;
				
			}
			data[0]='\0';
			k=0;
		}else{
			data[k]=line[i];
			k++;
			
		}
		
		
		
	}
	
	return newcomm;
} 

char* mysh_read_line() { //#2
   int bufsize = COMMAND_BUFSIZE; 
    int position = 0; 
    char *buffer = (char*)malloc(sizeof(char) * bufsize); 
    int c; 
 
    if (!buffer) { 
        fprintf(stderr, "-mysh: allocation error\n"); 
        exit(EXIT_FAILURE); 
    } 
    while (1) { 
        c = getchar(); 
		
        if (c == EOF || c == '\n') {    // read just one line per time 
			buffer[position] = '\0'; 
            return buffer; 
        } else { 
            buffer[position] = c; 
        } 
        position++; 

 
        if (position >= bufsize) {   // handle overflow case 
            bufsize += COMMAND_BUFSIZE; 
            buffer = (char*)realloc(buffer, bufsize); 
            if (!buffer) { 
                fprintf(stderr, "-mysh: allocation error\n"); 
                exit(EXIT_FAILURE); 
            } 
        } 
    } 
} 


 
void mysh_print_promt() { //#1
    /* Print "<username> in <current working directory>" */ 
    char curr_dic[100],username[20];
	getcwd(curr_dic, sizeof(curr_dic));
	getlogin_r(username, sizeof(username));
	printf("\E[0;36;40m%s\E[0;37;40m in\E[0;33;40m %s\E[0;37;40m \n", username,curr_dic );
    /* Print "mysh> " */ 
	printf("\E[0;32;40mmysh>\E[0;37;40m ");
} 

 
void mysh_print_welcome() { //#1
    /* Print "Welcome to mysh by <student ID>!" */ 
	printf("\nWelcome to mysh by 0216019\n");
 
} 

 
void mysh_loop() { 
    char *line; 
    struct command *command; 
    int status = 1; 
 
    do {   // an infinite loop to handle commands 
		mysh_print_promt(); 
        line = mysh_read_line();   // read one line from terminal 
        if (strlen(line) == 0) {
            continue; 
        }
        command = mysh_parse_command(line);   // parse line as command structure 
        status = mysh_execute_command(command);   // execute the command
        free(line); 
    } while (status >= 0); 
} 

 
void mysh_init() { 
    /* Do any initializations here. You may need to set handlers for signals */ 
	 
 
} 

 
int main(int argc, char **argv) { 
    mysh_init(); 
	mysh_print_welcome();
    mysh_loop(); 

 
    return EXIT_SUCCESS; 
} 



