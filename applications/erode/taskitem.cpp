#include "taskitem.h"
#include <iostream>
#include <fstream>

TaskItem::TaskItem(int pid) : QObject(nullptr), _pid(pid){
    std::string execpath = getprop("exename");
    _name = QString(execpath.substr(execpath.find_last_of("/\\") + 1).c_str());
    _path = QString(execpath.c_str());
}
bool TaskItem::ok(){
    return true;
}
std::string TaskItem::getprop(std::string name){
    std::string path = "/proc/" + std::to_string(_pid) + "/" + name;
    std::ifstream filestream(path);
    std::string propvalue(
        (std::istreambuf_iterator<char>(filestream)),
        std::istreambuf_iterator<char>());
    return propvalue;
}
