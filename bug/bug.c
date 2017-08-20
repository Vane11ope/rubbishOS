#include "../app.h"

void RubbMain(void)
{
	char a[100];
	a[100] = 'A';
	api_putchar(a[130]);
	api_end();
	return;
}
