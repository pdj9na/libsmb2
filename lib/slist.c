
#include <slist.h>

void SMB2_LIST_prepend(void *list, void *item)
{
	void **l = (void **)list;
	*(void **)item = *l, *l = item;
}

void SMB2_LIST_append(void *list, void *item)
{
	void **l = (void **)list;
	if (!*l)
		SMB2_LIST_prepend(list, item);
	else
	{
		void *p = *l, *p2;
		while ((p2 = *(void **)p))
			p = p2;
		*(void **)p = item, *(void **)item = NULL;
	}
}

void SMB2_LIST_remove(void *list, void *item)
{
	void **l = (void **)list;
	if (*l == item)
		*l = *(void **)item;
	else
	{
		void *p = *l, *p2 = NULL;
		while (p && (p2 = *(void **)p) && p2 != item)
			p = p2;
		if (p2)
			*(void **)p = *(void **)p2;
	}
}

// size_t SMB2_LIST_length(void *list)
// {
// 	size_t length = 0;
// 	void *p = *(void **)list;
// 	while (p && (++length, p = *(void **)p))
// 		;
// 	return length;
// }
