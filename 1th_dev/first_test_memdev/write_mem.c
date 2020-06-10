#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(void)
{
	int fd=0;
	int src=2015;
	fd=open("/dev/memdev1",O_RDWR);
	
	write(fd,&src,sizeof(int));

	close(fd);

	return 0;
}
