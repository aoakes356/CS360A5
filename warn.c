#include <stdio.h>
#include <signal.h>
#include <ucontext.h>
#include <unistd.h>
#include <string.h>

// Gross global for signal handler.
int fd[2];

void sigIntHandler(int signal){
    struct sigaction s2;
    printf("Recieved sigint\n");
    s2.sa_handler = sigIntHandler;
    sigaction(SIGINT,&s2,NULL);
}
void sigIntHandlerTwo(int signal){
    printf("Recieved sigint in parent\n");
    struct sigaction s2;
    s2.sa_handler = sigIntHandler;
    sigaction(SIGINT,&s2,NULL);
}

void sigAlarmHandler(int signal){
    struct sigaction s1;
    s1.sa_handler = sigAlarmHandler;
    sigaction(SIGALRM,&s1,NULL);
}

void sigUserHandler(int signal){
    
}

int pipeReee(int fileDescriptor, char* buffer, int size){
    FILE* readme = fdopen(fileDescriptor, "r");
    if(fgets(buffer, size, readme) == NULL){
        fclose(readme);
        return 0;
    }else{
        fclose(readme);
        return 1;
    }
}

int pipeWrite(int fileDescriptor, char* buffer){
    FILE* writeme = fdopen(fileDescriptor, "w");
    fprintf(writeme,"%s",buffer);
    fclose(writeme);
}   

int main(){
    struct sigaction s1;
    struct sigaction s2;
    struct sigaction s3;
    // fd[0] -> read
    // fd[1] -> write
    char buffer[512] = "Initial String";
    s1.sa_handler = sigAlarmHandler;
    s2.sa_handler = sigIntHandler;
    s3.sa_handler = sigUserHandler;
    if(pipe(fd) < 0){
        return 1;
    }
    int res = fork();
    if(!res){
        sigaction(SIGALRM,&s1,NULL);
        sigaction(SIGINT,&s2,NULL);
        sigaction(30,&s3,NULL);
        close(fd[1]);
        while(1){
            ualarm(100000,100000);
            pause();
            pipeReee(fd[0],buffer,511);
            printf("%s\n",buffer);
        }
        close(fd[0]);
    }else{
        close(fd[0]);
        sigaction(SIGINT,&s2,NULL);
        while(1){
            pause();
            printf("New string? ");
            if(fgets(buffer,511,stdin) != NULL){
                printf("\n");
                pipeWrite(fd[1],buffer);
                kill(res,30);   // User defined signal.
            }
        } 
        close(fd[1]);
    }
    printf("Exiting\n");
    return 0;
}
