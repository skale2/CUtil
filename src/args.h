#pragma once

#include <stdlib.h>
#include "data_struct.h"
#include "functions.h"

enum cmdline_inp_kind
{
    CMDLINE_OPT_BYTE,
    CMDLINE_OPT_SHORT,
    CMDLINE_OPT_INT,
    CMDLINE_OPT_LONG,
    CMDLINE_OPT_UBYTE,
    CMDLINE_OPT_USHORT,
    CMDLINE_OPT_UINT,
    CMDLINE_OPT_ULONG,
    CMDLINE_OPT_FLOAT,
    CMDLINE_OPT_DOUBLE,
    CMDLINE_OPT_BOOL,
    CMDLINE_OPT_CHAR,
    CMDLINE_OPT_STRING,
};

typedef struct cmdline_version
{
    size_t number[3];
    char *release;
} cmdline_version_t;

typedef struct cmdline cmdline_t;

#define cmdline_new() (_cmdline_new_(__FILENAME__))
cmdline_t _cmdline_new_(char *program_name);

arraylist_t(char *) cmdline_retrieve_args(cmdline_t *cla);
void cmdline_register_opt(cmdline_t *cla, enum cmdline_inp_kind kind, void **buf, char id, char *name, char *desc);
void cmdline_parse(cmdline_t *, int argc, char *argv[]);