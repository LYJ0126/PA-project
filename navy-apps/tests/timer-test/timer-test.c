#include <stdio.h>
#include <stdint.h>
#include <sys/time.h>
#include <time.h>
#include "../../libndl/include/NDL.h"

int main()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	uint64_t ttime = 500;
	printf("start\n");
	printf("%llu\n", ttime);
	while(1){
		while((tv.tv_sec * 1000 + tv.tv_usec / 1000) < ttime) gettimeofday(&tv, NULL);
		printf("timer interrupt\n");
		printf("tv_sec:%ld, tv_usec:%ld\n", tv.tv_sec, tv.tv_usec);
		ttime += 500;
		printf("%llu\n", ttime);
	}
	return 0;
}
