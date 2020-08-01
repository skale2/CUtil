#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <fts.h>
#include "debug.h"

// Honestly this entire program needs to be refactored from the ground up.

#define SOURCE_DIR "src"
#define TEST_DIR "tst"
#define OBJECT_DIR "obj"
#define TARGET_DIR "bin"

/* XXX: shamelessly copied from Stack Overflow. */
int recursive_delete(const char *dir) 
{
    int ret = 0;
    FTS *ftsp = NULL;
    FTSENT *curr;

    // Cast needed (in C) because fts_open() takes a "char * const *", instead
    // of a "const char * const *", which is only allowed in C++. fts_open()
    // does not modify the argument.
    char *files[] = { (char *) dir, NULL };

    // FTS_NOCHDIR  - Avoid changing cwd, which could cause unexpected behavior
    //                in multithreaded programs
    // FTS_PHYSICAL - Don't follow symlinks. Prevents deletion of files outside
    //                of the specified directory
    // FTS_XDEV     - Don't cross filesystem boundaries
    ftsp = fts_open(files, FTS_NOCHDIR | FTS_PHYSICAL | FTS_XDEV, NULL);
    if (!ftsp) {
        fprintf(stderr, "%s: fts_open failed: %s\n", dir, strerror(curr->fts_errno));
        ret = -1;
        goto finish;
    }

    while ((curr = fts_read(ftsp))) {
        switch (curr->fts_info) {
        case FTS_NS:
        case FTS_DNR:
        case FTS_ERR:
            fprintf(stderr, "%s: fts_read error: %s\n",
                    curr->fts_accpath, strerror(curr->fts_errno));
            break;

        case FTS_DC:
        case FTS_DOT:
        case FTS_NSOK:
            // Not reached unless FTS_LOGICAL, FTS_SEEDOT, or FTS_NOSTAT were
            // passed to fts_open()
            break;

        case FTS_D:
            // Do nothing. Need depth-first search, so directories are deleted
            // in FTS_DP
            break;

        case FTS_DP:
        case FTS_F:
        case FTS_SL:
        case FTS_SLNONE:
        case FTS_DEFAULT:
            if (remove(curr->fts_accpath) < 0) {
                fprintf(stderr, "%s: Failed to remove: %s\n",
                        curr->fts_path, strerror(curr->fts_errno));
                ret = -1;
            }
            break;
        }
    }

finish:
    if (ftsp) {
        fts_close(ftsp);
    }

    return ret;
}

int main(int argc, char *argv[])
{
	bool debug = false;
	size_t pos = 1;
	size_t debug_offset = 0;

	if (argc < 2)
	{
		printf("testing >> usage: %s <suite> [<test> [<test> [...]]]\n", argv[0]);
		exit(1);
	}

	if (strcmp(argv[1], "-d") == 0 || strcmp(argv[1], "--debug") == 0)
	{
		debug = true;
		pos++;
	}

	char *suite_name = argv[pos++];
	size_t suite_name_len = strlen(suite_name);

	char suite_fname[strlen("%s/%s.c") + strlen(TEST_DIR) + suite_name_len + 5];
	sprintf(suite_fname, "%s/%s.c", TEST_DIR, suite_name);

	if (access(suite_fname, F_OK) == -1)
	{
		printf("testing >> no test suite named %s\n", suite_fname);
		exit(1);
	}

	/* Build the tests-to-run string. */

	size_t tests_nchars = 5;
	for (int i = pos; i < argc; i++)
		tests_nchars += strlen(argv[i]) + 1; /* factor in the joining space character. */

	char tests_buf[tests_nchars];

	if (debug && pos < argc)
		printf("testing >> Can't pass specific tests to gdb; "
			   "use gdb command `run [<test> [<test> [...]]]` instead\n\n");
	else
	{
		memset(tests_buf, 0, tests_nchars);

		if (pos < argc)
			strcpy(tests_buf, argv[pos++]);

		for (; pos < argc; pos++)
		{
			strcat(tests_buf, " ");
			strcat(tests_buf, argv[pos]);
		}
	}

	/* Build the command. */

	char *cmd_base = "shopt -s extglob\n" /* Set bash to extended globbing mode, allowing it to process more complicated regex. */
					 "gcc %s-o %s/%s %s/!(test_runner).c %s/%s.c\n"
					 "%s%s/%s %s\n";

	char cmd[4 * suite_name_len +
			 3 * strlen(TARGET_DIR) +
			 strlen(SOURCE_DIR) +
			 strlen(TEST_DIR) +
			 tests_nchars +
			 strlen("-g") +
			 strlen("lldb") +
			 strlen(cmd_base)];

	sprintf(cmd, cmd_base,
			(debug ? "-g " : ""), TARGET_DIR, suite_name, SOURCE_DIR, TEST_DIR, suite_name,
			(debug ? "lldb " : ""), TARGET_DIR, suite_name, (debug ? "" : tests_buf));

	/* Run the command. */

	system(cmd);

	/* Cleanup. */

	char suite_bin_fname[strlen("%s/%s") + strlen(TARGET_DIR) + suite_name_len + 5];
	sprintf(suite_bin_fname, "%s/%s", TARGET_DIR, suite_name);
	remove(suite_bin_fname);

	if (debug)
	{
		char suite_dSYM_bin_fname[strlen("%s/%s.dSYM") + strlen(TARGET_DIR) + suite_name_len + 5];
		sprintf(suite_dSYM_bin_fname, "%s/%s.dSYM", TARGET_DIR, suite_name);
		recursive_delete(suite_dSYM_bin_fname);
	}
}