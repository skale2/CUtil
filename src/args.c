#include <limits.h>
#include <float.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include "args.h"
#include "panic.h"

char *cmdline_inp_kind_repr[] = {
	"byte",			  // CMDLINE_OPT_BYTE
	"short",		  // CMDLINE_OPT_SHORT
	"int",			  // CMDLINE_OPT_INT
	"long",			  // CMDLINE_OPT_LONG
	"unsigned byte",  // CMDLINE_OPT_UBYTE
	"unsigned short", // CMDLINE_OPT_USHORT
	"unsigned int",	  // CMDLINE_OPT_UINT
	"unsigned long",  // CMDLINE_OPT_ULONG
	"float",		  // CMDLINE_OPT_FLOAT
	"double",		  // CMDLINE_OPT_DOUBLE
	"bool",			  // CMDLINE_OPT_BOOL
	"char",			  // CMDLINE_OPT_CHAR
	"string",		  // CMDLINE_OPT_STRING
};

struct cmdline_opt_index
{
	enum
	{
		CMDLINE_OPT_INDEX_ID,
		CMDLINE_OPT_INDEX_NAME
	} kind;
	union cmdline_opt_index_key {
		char id;
		char *name;
	} key;
};

bool cmdline_opt_index_eq(const void *a, const void *b)
{
	const struct cmdline_opt_index *indexa = a;
	const struct cmdline_opt_index *indexb = b;

	return indexa->kind == indexb->kind &&
		   ((indexa->kind == CMDLINE_OPT_INDEX_ID && indexa->key.id == indexb->key.id) ||
			(indexa->kind == CMDLINE_OPT_INDEX_ID && streq(indexa->key.name, indexb->key.name)));
}

size_t cmdline_opt_index_hash(const void *x)
{
	const struct cmdline_opt_index *index = x;
	switch (index->kind)
	{
	case CMDLINE_OPT_INDEX_ID:
		return hash(_(index->kind), _(index->key.id));
		break;

	case CMDLINE_OPT_INDEX_NAME:
		return hash(_(index->kind), _(strhash(index->key.name)));
		break;

	default:
		break;
	}
}

struct cmdline_opt
{
	enum cmdline_inp_kind kind;
	void **buf;
	char id;
	char *name;
	char *doc;
};

struct cmdline_posarg
{
	enum cmdline_inp_kind kind;
	void **buf;
	char *name;
	char *doc;
};

struct cmdline_varargs
{
	enum cmdline_inp_kind kind;
	arraylist_t *list; // C type of args.kind
	char *name;
	char *doc;
};

struct cmdline
{
	char *program_name;
	cmdline_version_t version;
	struct cmdline_varargs *varargs;
	hashmap_t *posargs; // <char *, struct cmdline_posarg *>
	hashmap_t *opts; // <struct cmdline_opt_index *, struct cmdline_opt *>
};

cmdline_t _cmdline_new_(char *program_name)
{
	return (cmdline_t){
		.program_name = program_name,
		.version = {
			.number = {0},
			.release = ""},
		.varargs = $new(struct cmdline_varargs, .kind = -1, .list = arraylist_new(0)),
		.posargs = hashmap_new(.key_equal_fn = streq, .hash_fn = strhash),
		.opts = hashmap_new(.key_equal_fn = cmdline_opt_index_eq, .hash_fn = cmdline_opt_index_hash)};
}

void cmdline_register_opt(cmdline_t *cla, enum cmdline_inp_kind kind, void **buf, char id, char *name, char *desc)
{
	hashmap_set_at(
		cla->opts,
		$new(struct cmdline_opt_index,
			 .kind = CMDLINE_OPT_INDEX_ID,
			 .key = {.id = id}),
		$new(struct cmdline_opt,
			 .kind = kind,
			 .buf = buf,
			 .id = id,
			 .name = name,
			 .doc = desc));

	hashmap_set_at(
		cla->opts,
		$new(struct cmdline_opt_index,
			 .kind = CMDLINE_OPT_INDEX_NAME,
			 .key = {.name = name}),
		$new(struct cmdline_opt,
			 .kind = kind,
			 .buf = buf,
			 .id = id,
			 .name = name,
			 .doc = desc));
}

void print_version(cmdline_t *cla)
{
	printf("%zu.%zu.%zu%s%s",
		   cla->version.number[0],
		   cla->version.number[1],
		   cla->version.number[2],
		   cla->version.release[0] != '\0' ? "-" : "",
		   cla->version.release);
}

void print_help(cmdline_t *cla)
{
	char *posargs_str;
	char *options_str;

	printf("usage: %s [options] %s %s%s\n"
		   "options:\n%s",
		   cla->program_name,
		   posargs_str,
		   cla->varargs != NULL ? cla->varargs->name : "",
		   cla->varargs != NULL ? "..." : "",
		   options_str);
}

void set_opt_value(cmdline_t *cla, struct cmdline_opt *opt, char *value)
{
	#define SET_OPT_VALUE(parsed_value, start_type, type, typename) \
	({                                                              \
		if (value == NULL)                                          \
			panic("%s >>> %s option \"%s\" requires an argument",   \
				  cla->program_name,                                \
				  cmdline_inp_kind_repr[(typename)],                \
				  opt->name);                                       \
                                                                    \
		start_type i = parsed_value;                                \
                                                                    \
		if (errno == EINVAL)                                        \
			panic("%s >>> %s option \"%s\" is invalid",             \
				  cla->program_name,                                \
				  cmdline_inp_kind_repr[(typename)],                \
				  opt->name);                                       \
		else if (errno == ERANGE || i != (start_type)(type)i)       \
			panic("%s >>> %s option \"%s\" is out of range",        \
				  cla->program_name,                                \
				  cmdline_inp_kind_repr[(typename)],                \
				  opt->name);                                       \
		else                                                        \
			*((type *)opt->buf) = (type)i;                          \
	})

	switch (opt->kind)
	{
	case CMDLINE_OPT_BYTE:
		SET_OPT_VALUE(strtol(value, NULL, 10), long, int8_t, opt->kind);
		break;

	case CMDLINE_OPT_SHORT:
		SET_OPT_VALUE(strtol(value, NULL, 10), long, short, opt->kind);
		break;

	case CMDLINE_OPT_INT:
		SET_OPT_VALUE(strtol(value, NULL, 10), long, int, opt->kind);
		break;

	case CMDLINE_OPT_LONG:
		SET_OPT_VALUE(strtol(value, NULL, 10), long, long, opt->kind);
		break;

	case CMDLINE_OPT_UBYTE:
		SET_OPT_VALUE(strtoul(value, NULL, 10), unsigned long, uint8_t, opt->kind);
		break;

	case CMDLINE_OPT_USHORT:
		SET_OPT_VALUE(strtoul(value, NULL, 10), unsigned long, unsigned short, opt->kind);
		break;

	case CMDLINE_OPT_UINT:
		SET_OPT_VALUE(strtoul(value, NULL, 10), unsigned long, unsigned int, opt->kind);
		break;

	case CMDLINE_OPT_ULONG:
		SET_OPT_VALUE(strtoul(value, NULL, 10), unsigned long, unsigned long, opt->kind);
		break;

	case CMDLINE_OPT_FLOAT:
		SET_OPT_VALUE(strtod(value, NULL), double, float, opt->kind);
		break;

	case CMDLINE_OPT_DOUBLE:
		SET_OPT_VALUE(strtod(value, NULL), double, double, opt->kind);
		break;

	case CMDLINE_OPT_BOOL:
		if (value == NULL)
			*((bool *)opt->buf) = true;
		else
			panic("%s >>> %s option is invalid");
		break;

	case CMDLINE_OPT_CHAR:
		if (value == NULL)
			panic("%s >>> character option \"%s\" requires a value",
				  cla->program_name,
				  opt->name);
		else if (value[1] != '\0')
			panic("%s >>> argument for option \"%s\" must be a single character",
				  cla->program_name,
				  opt->name);
		else
			*((char *)opt->buf) = value[0];
		break;

	case CMDLINE_OPT_STRING:
		if (value == NULL)
			panic("%s >>> string option \"%s\" requires a value",
				  cla->program_name,
				  opt->name);
		else
			*((char **)opt->buf) = value;
	}
}

void cmdline_parse(cmdline_t *cla, int argc, char *argv[])
{
	hashmap_ref_t *posarg_ref = hashmap_ref(cla->posargs);
	bool accept_opts_mode = true;

	for (int argi = 0; argi < argc; argi++)
	{
		char *arg = argv[argi];

		if (accept_opts_mode && streq(arg, "--version"))
			print_version(cla);

		else if (accept_opts_mode && streq(arg, "--help"))
			print_help(cla);

		else if (streq(arg, "--"))
			accept_opts_mode = false;

		/* Short-form option */
		else if (accept_opts_mode && arg[0] == '-' && strlen(arg) > 1)
		{
			arg += 1;

			struct cmdline_opt *opt = hashmap_get_at(
				cla->opts,
				&(struct cmdline_opt_index){
					.kind = CMDLINE_OPT_INDEX_ID,
					.key = {.id = arg[0]}});

			if (opt == NULL)
				panic("%s >>> unknown option: \"%s\"",
					  cla->program_name,
					  &(char[]){arg[0], '\0'});

			/* Non-argumentative (boolean) options. */
			if (opt->kind == CMDLINE_OPT_BOOL)
			{
				set_opt_value(cla, opt, NULL);

				/* Allow multiple non-argumentative options under the same
				 * hyphen delimiter. */
				for (char *optid = &arg[1]; *optid != '\0'; optid++)
				{
					opt = hashmap_get_at(
						cla->opts,
						&(struct cmdline_opt_index){
							.kind = CMDLINE_OPT_INDEX_ID,
							.key = {.id = *optid}});

					if (opt == NULL)
						panic("%s >>> unknown option: \"%s\"",
							  cla->program_name,
							  &(char[]){arg[0], '\0'});

					else if (opt->kind != CMDLINE_OPT_BOOL)
						panic("%s >>> %s option \"%s\" requires an argument "
							  "(can't share a hyphen delimiter with other options)",
							  cmdline_inp_kind_repr[opt->kind],
							  cla->program_name,
							  &(char[]){arg[0], '\0'});

					set_opt_value(cla, opt, NULL);
				}
			}

			/* Option with argument in same token */
			else if (strlen(arg) > 1)
				set_opt_value(cla, opt, &arg[1]);

			/* Option with argument in next token. */
			else
			{
				if (argi + 1 == argc)
					panic("%s >>> %s option \"%s\" requires an argument",
						  cmdline_inp_kind_repr[opt->kind],
						  cla->program_name,
						  &arg[1]);
				else
					set_opt_value(cla, opt, argv[++argi]);
			}
		}

		/* Long-form option */
		else if (accept_opts_mode && arg[0] == '-' && arg[1] == '-')
		{
			char *optname = strtok(arg, "=");
			char *optarg = strtok(NULL, "=");

			struct cmdline_opt *opt = hashmap_get_at(
				cla->opts,
				&$(struct cmdline_opt_index,
				   .kind = CMDLINE_OPT_INDEX_NAME,
				   .key = {.name = optname}));

			if (opt == NULL)
				panic("%s >>> unknown option \"%s\"", cla->program_name, optname);

			/* Also handles non-argumentative (boolean) options - optarg
			 * will be NULL if no '=' sign found. */
			set_opt_value(cla, opt, optarg);
		}

		else if (hashmap_ref_is_valid(posarg_ref))
		{
			struct cmdline_posarg *posarg = hashmap_ref_get_value(posarg_ref);
			// set_opt_value(cla, posarg, arg); // TODO incorrect
			hashmap_ref_next(posarg_ref);
		}
	}
}