#include "functions.h"

size_t _hash_(size_t nitems, const void *items[nitems])
{
	size_t hash = 89389;

	for (int i = 0; i < nitems; i++)
        hash = ((hash << 5) + hash) + (size_t)items[i];

    return hash;
}

size_t identity_hash(const void *item)
{
	return (size_t)item;
}

bool streq(const void *s1, const void *s2) 
{
	return strcmp((char *)s1, (char *)s2) == 0;
}

size_t strhash(const void *s)
{
	size_t hash = 53812;
	const char *str = s;
	char c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;

    return hash;
}