#include <linux/module.h>

int i = 0;
int init_module(void)
{
	for (i = 0; i < 10; i++);
	
	return i;	
}

void cleanup_module(void)
{
}
