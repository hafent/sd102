#include <string.h>
#include <stdio.h>
#include <time.h>
#include "define.h"
#include "loopbuf.h"
#include "sd102_struct.h"
int main(void)
{
	struct Ta Ta;
	struct Tb Tb;
	struct power_caiji x;
	x.zy.val=1;
	Ta.day=1;
	Tb.ms=999;
	Tb.second=9;
	printf("Ta %d\n",sizeof(struct Ta));
	printf("Tb %d\n",sizeof(struct Tb));
	return 0;
}
