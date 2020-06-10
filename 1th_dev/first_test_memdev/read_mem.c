#include <stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>

int main()
{
	int fd=0;
	int rd =0;
	fd = open("/dev/memdev1",O_RDWR);

	read(fd,&rd,sizeof(int));
	
	printf("rd is %d\n",rd);
	close(fd);
	return 0;
}
