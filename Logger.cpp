//Logger.cpp - Implementing the 4 functions exposed on the header file
//
// 13-Apr-19  G. Wu         Created.
// 14-Apr-19  G. Wu  Added Comments.

#include <arpa/inet.h>
#include <iostream>
#include <net/if.h>
#include <netinet/in.h>
#include <signal.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <ctime>
#include "Logger.h"

using namespace std;

const int BUF_LEN=1024;
const string IP_ADDR = "127.0.0.1";
const int portno = 1153;

int ret, len;
int masterfd;
int filterLvl;
char buf[BUF_LEN];
bool is_running = true;

struct sockaddr_in addr;
pthread_mutex_t m_lock;
pthread_t thread_id;

//receive function to accept commands sent from the server
void *recv_func(void *arg) {
    int fd = *(int *)arg;
    struct sockaddr_in remaddr;
    socklen_t addrlen = sizeof(remaddr);

    memset(&remaddr, 0, sizeof(remaddr));
    remaddr.sin_family = AF_INET; 
    remaddr.sin_port = htons(portno); 
    remaddr.sin_addr.s_addr = INADDR_ANY; 

    char buf[BUF_LEN];
    fd_set readfds;
#ifdef DEBUG    
    cout << "logger: read()" << endl;
#endif    
    //Keep receiving message until being told to shut down
    while(is_running) {
        memset(buf, 0, BUF_LEN);
        memset(&readfds, 0, sizeof(&readfds));
        FD_SET(fd, &readfds);
        
        //Add a 1 second delay
        timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        ret = select(fd + 1, &readfds, NULL, NULL, &tv);
        if(ret > 0 && FD_ISSET(fd, &readfds)) {
            pthread_mutex_lock(&m_lock);
            int len = recvfrom(fd, buf, BUF_LEN, 0, (struct sockaddr *)&remaddr, &addrlen);
            pthread_mutex_unlock(&m_lock);
#ifdef DEBUG
            cout<<"logger: received "<<len<<" bytes from "<<inet_ntoa(remaddr.sin_addr)<<endl;
#endif
            buf[len] = '\0';
            if(len > 0) {
#ifdef DEBUG
                cout<<"logger: received message: "<<buf<<endl;
#endif
                string msg(buf), setLvl = "Set Log Level=";
                if(msg.compare(0, setLvl.length(), setLvl) == 0) { //Check if incoming message is a set log level command               
                    string logLvl = &buf[14]; //get after Level=, calling string()
#ifdef DEBUG              
                    cout << "Setting log level to " << logLvl << endl;
#endif
                    //Setting filter log level
                    if(logLvl == "DEBUG"){
                        SetLogLevel(LOG_LEVEL::DEBUG);
                    }else if(logLvl == "WARNING"){
                        SetLogLevel(LOG_LEVEL::WARNING);
                    }else if(logLvl == "CRITICAL"){
                        SetLogLevel(LOG_LEVEL::CRITICAL);     
                    }else if(logLvl == "ERROR"){
                        SetLogLevel(LOG_LEVEL::ERROR);
                    }
                }
            }

        } 
    }
#ifdef DEBUG
    cout << "thread exiting" << endl;
#endif    
    pthread_exit(NULL);
}

//Initalize the logger
int InitializeLog() {
    memset(&addr, 0, sizeof(addr));
#ifdef DEBUG
    cout << "Creating socket..." << endl;
#endif    
    //Create the socket
    if ((masterfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        cout << "logger: " << strerror(errno) << endl;
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET; 
    addr.sin_port = htons(portno); 
    inet_pton(AF_INET, IP_ADDR.c_str(), &addr.sin_addr);
   
    pthread_mutex_init(&m_lock, NULL);
    //Create a receive thread and pass the file descriptor to it
    ret = pthread_create(&thread_id, NULL, recv_func, &masterfd);
    if(ret != 0) {
        cout << "logger: " << strerror(errno) << endl;
        close(masterfd);
        return -1;
    }
    return 0;
}

//Setting the filter log level
void SetLogLevel(int level) {
    filterLvl = level;
}

//Send a log the server
void Log(int level, const char *prog, const char *func, int line, const char *message) {
    if(level >= filterLvl) { //Check the severity first; anything lower than the filter severity would be discarded
        memset(buf, 0, BUF_LEN);
        char levelStr[][16]={"DEBUG", "WARNING", "ERROR", "CRITICAL"};
        
        //Create a timestamp
        time_t now = time(0);
        char* dt = ctime(&now);
        
        len = sprintf(buf, "%s %s %s:%s:%d %s\n", dt, levelStr[level], prog, func, line, message)+1;
        buf[len - 1] = '\0';
#ifdef DEBUG        
        cout << "logger: sending" << endl;
#endif        
        ret = sendto(masterfd, buf, len, 0, (struct sockaddr *)&addr, sizeof(addr));
#ifdef DEBUG        
        cout << "logger: sent" << endl;
#endif
    }
}

//Terminate the logger
void ExitLog() {
    is_running = false;
#ifdef DEBUG    
    cout << "waiting for thread sync" << endl;
#endif    
    pthread_join(thread_id, NULL);//Join the receiving threads
#ifdef DEBUG    
    cout << "bye" << endl;
#endif    
    close(masterfd);//Close the file descriptor
}



