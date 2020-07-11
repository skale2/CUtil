#include <stdio.h>
#include <string.h>
#include "debug.h"
#include "data_struct.h"

#undef malloc
#undef calloc
#undef realloc
#undef free

enum alloc_type
{
	ALLOC_TYPE_MALLOC,
	ALLOC_TYPE_CALLOC,
	ALLOC_TYPE_REALLOC,
	ALLOC_TYPE_FREE
};

struct alloc_op
{
	enum alloc_type op_type;
	char *file;
	size_t line;
	struct alloc_op *next, *prev;
	size_t size;
	void *new_resource;
	void *old_resource;
};

struct alloc_op *first_op, *last_op;

void add_alloc_op(struct alloc_op *op) 
{
	if (first_op == NULL)
		first_op = last_op = op;
	else
	{
		last_op->next = op;
		last_op = last_op->next;
	}
}

void debug_mem_setup()
{
	debug_mem_reset();
}

void debug_mem_reset()
{
	first_op = last_op = NULL;
}

void *debug_mem_malloc(size_t size, char *file, size_t line)
{
	void *resource = malloc(size);

	struct alloc_op *op = (struct alloc_op *)malloc(sizeof(struct alloc_op));
	*op = (struct alloc_op){
		.op_type = ALLOC_TYPE_MALLOC,
		.file = file,
		.line = line,
		.next = NULL,
		.prev = last_op,
		.size = size,
		.new_resource = resource,
		.old_resource = NULL};

	add_alloc_op(op);
	return resource;
}

void *debug_mem_calloc(size_t nitems, size_t size, char *file, size_t line)
{
	void *resource = calloc(nitems, size);
	struct alloc_op *op = (struct alloc_op *)malloc(sizeof(struct alloc_op));
	*op = (struct alloc_op){
		.op_type = ALLOC_TYPE_CALLOC,
		.file = file,
		.line = line,
		.next = NULL,
		.prev = last_op,
		.size = nitems * size,
		.new_resource = resource,
		.old_resource = NULL
	};

	add_alloc_op(op);
	return resource;
}

void *debug_mem_realloc(void *ref, size_t size, char *file, size_t line)
{
	void *resource = realloc(ref, size);

	struct alloc_op *op = (struct alloc_op *)malloc(sizeof(struct alloc_op));
	*op = (struct alloc_op){
		.op_type = ALLOC_TYPE_REALLOC,
		.file = file,
		.line = line,
		.next = NULL,
		.prev = last_op,
		.size = size,
		.new_resource = resource,
		.old_resource = ref};

	add_alloc_op(op);
	return resource;
}

void debug_mem_free(void *ref, char *file, size_t line)
{
	free(ref);

	struct alloc_op *op = (struct alloc_op *)malloc(sizeof(struct alloc_op));
	*op = (struct alloc_op){
		.op_type = ALLOC_TYPE_FREE,
		.file = file,
		.line = line,
		.next = NULL,
		.prev = last_op,
		.size = -1,
		.new_resource = NULL,
		.old_resource = ref};

	add_alloc_op(op);
}

void debug_mem_print()
{
	printf("\n-------------- Memory allocation history ---------------\n\n");

	for (struct alloc_op *op = first_op; op != NULL; op = op->next)
	{
		switch (op->op_type)
		{
		case ALLOC_TYPE_MALLOC:
			printf(
				"- malloc @ %p\n"
				"    loc: %s[%zu]\n"
				"    size: %zuB\n\n",
				op->new_resource,
				op->file,
				op->line,
				op->size);
			break;

		case ALLOC_TYPE_CALLOC:
			printf(
				"- calloc @ %p\n"
				"    loc: %s[%zu]\n"
				"    size: %zuB\n\n",
				op->new_resource,
				op->file,
				op->line,
				op->size);
			break;

		case ALLOC_TYPE_REALLOC:
			printf(
				"- realloc @ %p -> %px\n"
				"    loc: %s[%zu]\n"
				"    size: %zuB\n\n",
				op->old_resource,
				op->new_resource,
				op->file,
				op->line,
				op->size);
			break;

		case ALLOC_TYPE_FREE:
			printf(
				"- free @ %p\n"
				"    loc: %s[%zu]\n\n",
				op->old_resource,
				op->file,
				op->line);
			break;

		default:
			break;
		}
	}

	printf("---------------------------------------------------------\n");
}

void debug_mem_dump(void *start, size_t nitems, size_t item_size, char *fmt_spec)
{
	printf("\nAddress        | Value"
		   "\n---------------+---------------------------------------\n");

	for (uint8_t *ptr = start;
		 ptr < (uint8_t *)(start + nitems * item_size);
		 ptr += item_size)
	{
		printf("%p | ", ptr);
		printf(fmt_spec, *ptr);
		printf("\n");
	}
}