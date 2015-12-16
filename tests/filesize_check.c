#if defined(_WIN32) || defined(_WIN64)
	#define _CRT_SECURE_NO_WARNINGS
#endif
#include <stdio.h>
#include <stdlib.h>

static long filesize(const char *const path)
{
	FILE *const f = fopen(path, "rb");
	if (0 == f)
	{
		fprintf(stderr, "Bad file: %s\n", path);
		exit(1);
	}
	fseek(f, 0, SEEK_END);
	const long sz = (unsigned)ftell(f);
	fclose(f);
	return sz;
}

int main(int argc, const char *argv[])
{
	if (2 >= argc)
	{
		fprintf(stderr, "Usage: prog f1_path f2_path\n");
		return 1;
	}
	const long f1_sz = filesize(argv[1]);
	const long f2_sz = filesize(argv[2]);
	if (f1_sz < f2_sz)
	{
		fprintf(stderr, "New size is larger: %li <  %li\n", f1_sz, f2_sz);
		return 1;
	}
	return 0;
}
