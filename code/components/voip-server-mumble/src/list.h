#ifndef LIST_H_87698769870987
#define LIST_H_87698769870987

struct dlist {
	struct dlist *next, *prev;
};

#define init_list_entry(ptr) \
        do { \
            (ptr)->prev = ptr; (ptr)->next = ptr; } \
        while(0);
#define declare_list(_name_) struct dlist _name_ = {&(_name_), &(_name_) }

static inline void list_add_head(struct dlist *neww, struct dlist *list)
{
	list->next->prev = neww;
	neww->prev = list;
	neww->next = list->next;
	list->next = neww;
}

static inline void list_add_tail(struct dlist *neww, struct dlist *list)
{
	list->prev->next = neww;
	neww->prev = list->prev;
	neww->next = list;
	list->prev = neww;
}
static inline void list_del(struct dlist *entry)
{
	entry->next->prev = entry->prev;
	entry->prev->next = entry->next;
}

static inline int list_empty(struct dlist *list)
{
	return list->next == list;
}

#define list_get_first(listhead) \
        ((listhead)->next)

#define list_get_next(_entry_) \
        ((_entry_)->next)

#define list_get_entry(entry, type, structmember) \
        ((type *) ((char *)(entry) - (unsigned long)(&((type *)0)->structmember)))

#define list_iterate(entry, listhead) \
        for(entry = (listhead)->next; entry != (listhead); entry = entry->next)

#define list_iterate_safe(entry, save, listhead) \
        for(entry = (listhead)->next, save = entry->next; entry != (listhead); \
            entry = save, save = save->next)


#endif /* #ifndef LIST_H_87698769870987 */
