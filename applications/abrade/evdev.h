#ifndef EVDEV_H
#define EVDEV_H

#include <string>

int get_num_fds();
std::string get_filepath(int fd);
int ioctl_evdev(const char* path, unsigned long request, int value);
int grab_evdev(const char* path);
int ungrab_evdev(const char* path);
void ungrab_wacom();
int grab_wacom();
void ungrab_touchscreen();
int grab_touchscreen();
void ungrab_gpio();
int grab_gpio();
#endif // EVDEV_H
