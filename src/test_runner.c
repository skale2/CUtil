#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SOURCE_DIR "src"
#define TEST_DIR "tst"
#define OBJECT_DIR "obj"
#define TARGET_DIR "bin"

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		printf("testing >> usage: %s <suite 1> [<suite 2> [...]]\n", argv[0]);
		exit(1);
	}

	for (int i = 1; i < argc; i++)
	{
		char *test_name = argv[i];
		size_t test_name_len = strlen(test_name);

		char test_fname[strlen("%s/%s.c") + strlen(TEST_DIR) + test_name_len + 5];
		sprintf(test_fname, "%s/%s.c", TEST_DIR, test_name);

		if (access(test_fname, F_OK) == -1)
		{
			printf("testing >> no test named %s\n", test_fname);
			exit(1);
		}

		char *cmd_base = "shopt -s extglob\n"
						 "gcc -o %s/%s %s/!(test_runner).c %s/%s.c\n"
						 "%s/%s\n";

		char cmd[4 * test_name_len +
				 3 * strlen(TARGET_DIR) +
				 strlen(SOURCE_DIR) +
				 strlen(TEST_DIR) +
				 strlen(cmd_base)];

		sprintf(cmd, cmd_base,
				TARGET_DIR, test_name, SOURCE_DIR, 
				TEST_DIR, test_name, TARGET_DIR,
				test_name, TARGET_DIR, test_name);

		system(cmd);

		char test_bin_fname[strlen("%s/%s") + strlen(TARGET_DIR) + test_name_len + 5];
		sprintf(test_bin_fname, "%s/%s", TARGET_DIR, test_name);
		remove(test_bin_fname);
	}
}