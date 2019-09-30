#include <QDebug>
#include "evdev.h"
#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <sys/resource.h>
#include <linux/input.h>
#include <dirent.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring>

int get_num_fds(){
     int fd_count;
     char buf[64];
     struct dirent *dp;

     snprintf(buf, 64, "/proc/%i/fd/", getpid());

     fd_count = 0;
     DIR *dir = opendir(buf);
     while ((dp = readdir(dir)) != NULL) {
          fd_count++;
     }
     closedir(dir);
     return fd_count;
}
std::string get_filepath(int fd){
    char procpath[PATH_MAX + 1];
    char filepath[PATH_MAX + 1];
    memset(procpath, 0, PATH_MAX + 1);
    memset(filepath, 0, PATH_MAX + 1);
    snprintf(procpath, PATH_MAX, "/proc/self/fd/%d", fd);
    readlink(procpath, filepath, (size_t)PATH_MAX);
    return std::string(filepath);
}
int ioctl_evdev(const char* path, unsigned long request, int value){
    int res = -1;
    for(int fd = 0; fd < get_num_fds(); fd++){
        if(strcmp(get_filepath(fd).c_str(), path) == 0){
            res = ioctl(fd, request, value);
            if(res != 0){
                return res;
            }
        }
    }
    return res;
}
int grab_evdev(const char* path){
    return ioctl_evdev(path, EVIOCGRAB, 1);
}
int ungrab_evdev(const char* path){
    return ioctl_evdev(path, EVIOCGRAB, 0);
}
void ungrab_wacom(){
    ungrab_evdev("/dev/input/event0");
}
int grab_wacom(){
    int res = grab_evdev("/dev/input/event0");
    if(res == EBUSY){
        qDebug() << "Failed to grab wacom, device is busy";
    }else if(res != 0){
        qDebug() << "Failed to grab wacom: " << res;
    }else{
        qDebug() << "Grabbed wacom";
    }
    return res;
}
void ungrab_touchscreen(){
    ungrab_evdev("/dev/input/event1");
}
int grab_touchscreen(){
    int res = grab_evdev("/dev/input/event1");
    if(res == EBUSY){
        qDebug() << "Failed to grab touchscreen, device is busy";
    }else if(res != 0){
        qDebug() << "Failed to grab touchscreen: " << res;
    }else{
        qDebug() << "Grabbed touchscreen";
    }
    return res;
}
void ungrab_gpio(){
    ungrab_evdev("/dev/input/event2");
}
int grab_gpio(){
     int res = grab_evdev("/dev/input/event2");
     if(res == EBUSY){
         qDebug() << "Failed to grab gpio, device is busy";
     }else if(res != 0){
         qDebug() << "Failed to grab gpio: " << res;
     }else{
         qDebug() << "Grabbed gpio";
     }
     return res;
}
