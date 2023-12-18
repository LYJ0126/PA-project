#include <stdio.h>
#include <stdint.h>
#include <sys/time.h>
#include <time.h>
#include "../../libndl/include/NDL.h"

int main()
{
	NDL_Init(0);
	uint64_t ttime = 500;
	while(1){
		while(NDL_GetTicks() < ttime);
		uint32_t temptime = NDL_GetTicks();
		printf("s:%u,ms:%u\n",temptime/1000,temptime%1000);
		ttime += 500;
	}
	return 0;
}
