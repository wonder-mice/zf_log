#include "test_switch.h"

int main(int argc, char *argv[])
{
	(void)argc; (void)argv;
	XLOG_INIT();
	XLOG_STATEMENT();
#ifdef TEST_SEVERAL_STATEMENTS
	XLOG_STATEMENT();
	XLOG_STATEMENT();
	XLOG_STATEMENT();
	XLOG_STATEMENT();
	XLOG_STATEMENT();
	XLOG_STATEMENT();
#endif
#ifdef TEST_EXTRA_STATEMENT
	XLOG_STATEMENT();
#endif
	return 0;
}
