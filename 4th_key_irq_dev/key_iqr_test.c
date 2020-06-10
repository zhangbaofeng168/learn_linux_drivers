#include <stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>

int main()
{
	int fd=0;
	unsigned char key_val;
	fd = open("/dev/buttons",O_RDWR);
	
	while(1)
	{
		read(fd,&key_val,sizeof(key_val));
		printf("key_val = 0x%x\n", key_val);
	}

	close(fd);
	return 0;
}