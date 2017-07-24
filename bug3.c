void api_putchar(int c);
void api_end(void);

void RubbMain(void)
{
	for (;;) { api_putchar('a'); }
}
