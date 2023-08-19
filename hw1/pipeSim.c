#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>


int main(int argc, char *argv[]){
    
    printf("I'm SHELL process, with PID: %d - Main command is: man df | grep -a \n", getpid());
    
    int fd[2];
    pipe(fd);
    int outFile = open("output.txt", O_CREAT|O_APPEND|O_WRONLY, 0666);
    
    int child1 = fork();
    if(child1 < 0){
        //Fork failed
        fprintf(stderr, "fork failed!\n");
        exit(1);
    }
    else if(child1 == 0){ //If child1 process fork is successful
        printf("I'm MAN process, with PID: %d - My command is: man df \n", getpid());
        
        dup2(fd[1], STDOUT_FILENO);
        
        char* myargs1[3];
        myargs1[0] = strdup("man");
        myargs1[1] = strdup("df");
        myargs1[2] = NULL;
        execvp(myargs1[0],myargs1);
    }
    else{ //Going back to main process
        
        int child2 = fork();
        if(child2 < 0){
            fprintf(stderr, "fork failed!\n");
            exit(1);
        }
        else if(child2 == 0){ //If the child2 process fork is successful
            printf("I'm GREP process, with PID: %d - My command is: grep -a \n", getpid());
            dup2(fd[0], STDIN_FILENO);
            
            dup2(outFile, STDOUT_FILENO);
            dup2(outFile, STDERR_FILENO);
            char *myargs2[3];
            myargs2[0] = strdup("grep");
            myargs2[1] = strdup("-a");
            myargs2[2] = NULL;
            execvp(myargs2[0],myargs2);
        }
        else{//Going back to main process
            waitpid(child1,NULL,0);
            waitpid(child2,NULL,0);
            
            printf("I'm SHELL process, with PID: %d - execution is completed, you can find the results in output.txt\n", getpid());
            close(outFile);
        }
    }
    return 0;
}

