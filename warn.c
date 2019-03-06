#include <stdio.h>
#include <signal.h>
#include <ucontext.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

// Gross global for signal handler.
int fd[2];
int pflag = 0;
int closeFlag = 1;

int errorHandler(const char* message, int err){
    fprintf(stderr,"%s: %s\n",message, strerror(err));
    return 1;
}

int pipeReee(int fileDescriptor, char* buffer, int size){
    int newfd = dup(fileDescriptor);
    if(dup < 0){
        return errorHandler("Failed to duplicate file descriptor",errno);
    }
    FILE* readme = fdopen(newfd, "r");
    if(fgets(buffer, size, readme) == NULL){
        if(fclose(readme) == EOF){
            return errorHandler("Failed to close file Stream",errno);
        }
        return errorHandler("No string read in?!?",errno);
    }else{
        if(fclose(readme) == EOF){
            return errorHandler("Failed to close file Stream",errno);
        }
        return 0;
    }
}

void sigIntHandler(int signal){
    struct sigaction s2;
    s2.sa_handler = sigIntHandler;
    sigaction(SIGINT,&s2,NULL);
}
void sigIntHandlerTwo(int signal){
    ualarm(0,0);
    pflag = 1;
    struct sigaction s2;
    s2.sa_handler = sigIntHandlerTwo;
    sigaction(SIGINT,&s2,NULL);
}

void sigAlarmHandler(int signal){
    struct sigaction s1;
    s1.sa_handler = sigAlarmHandler;
    sigaction(SIGALRM,&s1,NULL);
}

void sigUserHandler(int signal){
    struct sigaction s3;
    s3.sa_handler = sigUserHandler;
    sigaction(SIGFPE,&s3,NULL);
}

void sigCloseHandler(int signal){
    closeFlag = 0;
    alarm(0);
    ualarm(1,1); // Smash through any pauses in the while loop.
    struct sigaction s3;
    s3.sa_handler = sigUserHandler;
    sigaction(31,&s3,NULL);
}


int pipeWrite(int fileDescriptor, char* buffer){
    int newfd = dup(fileDescriptor);
    if(dup < 0){
        return errorHandler("Failed to duplicate file descriptor",errno);
    }
    FILE* writeme = fdopen(newfd, "w");
    if(writeme == NULL){
        return errorHandler("Failed to open file with file descriptor",errno);
    }
    fputs(buffer,writeme);
    fclose(writeme);
    return 0;
}   

int main(){
    char buffer[512] = "";
    struct sigaction s1;
    struct sigaction s2;
    struct sigaction s3;
    struct sigaction s4;
    struct sigaction s5;
    // fd[0] -> read
    // fd[1] -> write
    s1.sa_handler = sigAlarmHandler;
    s2.sa_handler = sigIntHandler;
    s3.sa_handler = sigUserHandler;
    s4.sa_handler = sigCloseHandler;
    s5.sa_handler = sigIntHandlerTwo;
    if(pipe(fd) < 0){
        return 1;
    }
    int res = fork();
    if(res < 0) return errorHandler("Failed to create child process",errno);
    if(!res){
        int waitTime = 5;
        sigaction(SIGALRM,&s1,NULL);
        sigaction(SIGINT,&s5,NULL);
        sigaction(SIGFPE,&s3,NULL);
        if(close(fd[1]) < 0) return errorHandler("Failed to close file descriptor",errno);
        while(closeFlag){
            if(pflag){
                pause();
                pipeReee(fd[0],buffer,511);
                // Used %[\001-\377] because %s wouldn't get the whole string >:I
                if(sscanf(buffer,"%d %[\001-\377]\n",&waitTime,buffer) != 2){
                    sscanf(buffer,"%[\001-\377]\n",buffer);
                    waitTime = 5;
                }
                if(!strcmp("exit\n",buffer)) {
                    printf("taking out the parent\n");
                    kill(res,31);   // Tell the parent to kill itself.
                    break;
                    //kill(res,SIGKILL);
                }
                pflag = 0;
            }else{
                if(closeFlag){
                    alarm(waitTime);
                    pause();
                }else{
                    break;
                }
                if(strlen(buffer) != 0){
                    printf("%s",buffer);
                }
            }
        }
        if(close(fd[0]) < 0) return errorHandler("Failed to close file descriptor",errno);
    }else{
        if(close(fd[0]) < 0) return errorHandler("Failed to close file descriptor",errno);
        sigaction(SIGINT,&s2,NULL);
        sigaction(31,&s4,NULL);
        sigaction(SIGALRM,&s1,NULL);
        while(closeFlag){
            pause();
            printf("New string: ");
            if(fgets(buffer,512,stdin) != NULL){
                printf("\n");
                pipeWrite(fd[1],buffer);
                kill(res,SIGFPE);   // User defined signal.
            }
        }
    if(close(fd[1]) < 0) return errorHandler("Failed to close descriptor",errno);
    //wait(NULL);
    }
    printf("Exiting\n");
    return 0;
}
