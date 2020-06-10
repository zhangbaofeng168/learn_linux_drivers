#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
     int fd;
     int value;
     if(strcmp(argv[1], "on") ==0)
	value = 1;
     else 
	value = 0;
     fd = open("/dev/leddev0",O_RDWR);

     write(fd, &value, sizeof(int));
     close(fd);
     return 0;
}