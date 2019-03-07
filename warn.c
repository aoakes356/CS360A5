#include <stdio.h>
#include <stdlib.h>
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
int written = 0;
int alrm = 0;

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
    pflag = 1;
    struct sigaction s2;
    s2.sa_handler = sigIntHandlerTwo;
    sigaction(SIGINT,&s2,NULL);
}

void sigAlarmHandler(int signal){
    alrm = 1;
    struct sigaction s1;
    s1.sa_handler = sigAlarmHandler;
    sigaction(SIGALRM,&s1,NULL);
}

void sigUserHandler(int signal){
    written = 1;
    struct sigaction s3;
    s3.sa_handler = sigUserHandler;
    sigaction(SIGFPE,&s3,NULL);
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

// I aplogize for the mess, I didn't understand the assignment well enough to split it into pieces before I started.

int main(){
    char buffer[2048] = "";
    char buffer2[2048] = "";
    struct sigaction s1;
    struct sigaction s2;
    struct sigaction s3;
    // fd[0] -> read
    // fd[1] -> write
    s1.sa_handler = sigAlarmHandler;
    s2.sa_handler = sigIntHandlerTwo;
    s3.sa_handler = sigUserHandler;
    if(pipe(fd) < 0){
        return 1;
    }
    int res = fork();
    if(res < 0) return errorHandler("Failed to create child process",errno);
    int waitTime = 5;
    if(!res){   // Child process
        char* word;
        int ex = 0;
        sigaction(SIGALRM,&s1,NULL);
        sigaction(SIGINT,&s2,NULL);
        sigaction(SIGFPE,&s3,NULL);
        if(close(fd[1]) < 0) return errorHandler("Failed to close file descriptor",errno);
        while(1){
            if(pflag){
                alarm(0);
                while(!written){
                    pause();
                }
                written = 0;
                if(!pflag){
                    printf("\n");
                }
                while(pflag){
                    pflag = 0;
                    pipeReee(fd[0],buffer,513);
                    strncpy(buffer2,buffer,512);
                    // Used %[\001-\377] because %s wouldn't get the whole string >:I
                    if(sscanf(buffer,"%d",&waitTime) == 1){
                        word = strtok(buffer," ");
                        word = strtok(NULL,"");
                        if(word == NULL){
                            waitTime = 5;
                            word = buffer2;
                        }
                    }else{
                        waitTime = 5;
                        word = buffer2;
                    } 
                    if(!strcmp("exit\n",word)) {
                        ex = 1;
                        break;
                        //kill(res,SIGKILL);
                    }
                }
                
            }
            if(ex) break;
            if(waitTime < 0) waitTime = -waitTime;
            if(waitTime == 0) waitTime = 1;
            alarm(waitTime);
            pause();
            if(strlen(buffer) != 0 && !pflag){
                printf("%s",word);
                fflush(stdout);
            }
        }
        if(close(fd[0]) < 0) return errorHandler("Failed to close file descriptor",errno);
        printf("Exiting\n");
        exit(0);
    }else{
        if(close(fd[0]) < 0) return errorHandler("Failed to close file descriptor",errno);

        sigaction(SIGINT,&s2,NULL);
        //sigaction(SIGALRM,&s1,NULL);
        char* line = NULL;
        int len, res1;
        size_t n = 0;
        while(1){
            line = NULL;
            n = 0;
            while(!pflag){
                pause();
            }
            pflag = 0;
            printf("New string: ");
            fflush(stdout);
            if((res1 = (getline(&line,&n,stdin))) >= 0){
                if(pflag){
                    printf("\n");
                    pflag = 0;
                    continue;
                }
                //fflush(stdin); // Can't use on input stream....
                //while(getchar() != '\n'); // Loops indefinitely or something?
                strncpy(buffer,line,512);
                free(line);
                n = 0;
                len = strlen(buffer);
                buffer[len-1] = '\n';
                printf("\n");
                pipeWrite(fd[1],buffer);
                kill(res,SIGFPE);
                char* word = strtok(buffer," ");
                word = strtok(NULL,"");
                if(word == NULL){
                    if(!strcmp("exit\n", buffer)) {
                        break;
                    }
                }
                else if(!strcmp("exit\n", word)) {
                    break;
                }
            }else{
                errorHandler("Failed to read a line",errno);
                pflag = 0;
            }
        }

        if(close(fd[1]) < 0) return errorHandler("Failed to close descriptor",errno);
        wait(NULL);
    }
    return 0;
}
