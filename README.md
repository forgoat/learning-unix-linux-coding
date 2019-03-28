# learning-unix-linux-coding
一些unix/linux的命令，
在ubuntu16.04上能运行。
代码参考《unix/linux编程实践教程》
read-read from a file descriptor
#include<unistd.h>
ssize_t read(int fd,void *buf,size_t count);
