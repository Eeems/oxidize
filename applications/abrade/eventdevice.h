#ifndef EVENTDEVICE_H
#define EVENTDEVICE_H

#include <string>
#include <ext/stdio_filebuf.h>
#include <linux/input.h>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <fcntl.h>


//#include <fcntl.h>
//#include <stdlib.h>
//#include <unistd.h>
//#include <cstring>
//#include <unordered_map>
//#include <sys/time.h>

class EventDevice {
public:
    EventDevice(std::string path, int flags);
    int open_device();
    int close_device();
    int grab();
    int ungrab();
    input_event* read_event();
    void write_event(input_event* ie);
    input_event create_event(__u16 type, __u16 code, __s32 value);
private:
    std::string _path;
    int _flags;
    int _fd;
    __gnu_cxx::stdio_filebuf<char>* _filebuff;
    std::istream* _stream;
    input_event _ie;
};

#endif // EVENTDEVICE_H
