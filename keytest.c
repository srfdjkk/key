#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
 
#define DEVICE_NAME  "/dev/key"
int main(int argc, char * * argv)
{
	int fd;
	unsigned char key_vals[3] ;
	fd = open(DEVICE_NAME, O_RDWR);
	if(fd<0)
	{
		printf("open DEVICE_NAME faild \n ");
	}
	printf("open successful ! \n");
	while(1)
	{
		read(fd, key_vals,sizeof(key_vals));
		
		if(!key_vals[0]||!key_vals[1]||!key_vals[2])
		{
			printf("key status  is   key1: key2: key3:    %d     %d     %d  \n ",key_vals[0],key_vals[1],key_vals[2]);
		}
	}
	return 0;
}
