void io_hlt();

void RubbMain(void)
{
fin:
	io_hlt();
	goto fin;
}
