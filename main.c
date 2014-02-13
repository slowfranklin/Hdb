#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

int main(){
	int a;
	uint32_t u=0;
	a=u-1;
	printf("%d\n",a);
	a++;
	printf("%d\n",a);
	//printf("%f,%f,%f\n",pow(2,0),pow(2,7),pow(2,8));
	//printf("%d,%d\n",2<<0,2<<7);
	return 1;
}
