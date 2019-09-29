#include <linux/input.h>
#include <unistd.h>
#include "eventdevice.h"

using namespace std;

EventDevice::EventDevice(std::string path, int flags){
    _flags = flags;
    _path = path;
}
int EventDevice::open_device(){
    _fd = open(_path.c_str(), _flags);
    if(_fd == -1){
        return _fd;
    }
    _filebuff = new __gnu_cxx::stdio_filebuf<char>(_fd, std::ios::in);
    _stream = new std::istream(_filebuff);
    return 0;
}
int EventDevice::close_device(){
    return close(_fd);
}
int EventDevice::grab(){
    return ioctl(_fd, EVIOCGRAB, 1);
}
int EventDevice::ungrab(){
    return ioctl(_fd, EVIOCGRAB, 0);
}
input_event* EventDevice::read_event(){
    streamsize sie = static_cast<streamsize>(sizeof(struct input_event));
    _stream->read((char*)&_ie, sie);
    return &_ie;
}
void EventDevice::write_event(input_event* ie){
    write(_fd, &ie,sizeof(ie));
}
input_event EventDevice::create_event(__u16 type, __u16 code, __s32 value){
    struct input_event ie;
    ie.type = type;
    ie.code = code;
    ie.value = value;
    return ie;

}
