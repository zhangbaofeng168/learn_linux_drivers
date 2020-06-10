#include <stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>

int main()
{
	int fd=0;
	int i;
	unsigned char key_val[6];
	fd = open("/dev/keydev0",O_RDWR);
	while(1)
	{
		read(fd,&key_val,sizeof(key_val));
		for(i=0;i<6;i++)
		{
			if(!key_val[i])
			{
				printf("pressed key is XIENT%d\n",i);
				while(!key_val[i])
					read(fd,&key_val,1);
			}
		}
		
	}

	close(fd);
	return 0;
}