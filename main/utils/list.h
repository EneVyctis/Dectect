#define INITIAL_LIST_CAPACITY 4
#define LIST_CAPACITY_GROWTH_RATE 2

#define DECLARE_LIST(type, name) \
typedef struct name\
{\
	type* content;\
	int length;\
	int capacity;\
} name;\
\
void initialise_##name(struct name* list);\
type* insert_##name(struct name* list);

#define DEFINE_LIST(type, name)\
void initialise_##name(struct name* list)\
{\
	list->capacity = INITIAL_LIST_CAPACITY;\
	list->content = malloc(list->capacity * sizeof(type));\
}\
\
type* insert_##name(struct name* list)\
{\
	if(list->capacity <= list->length)\
	{\
		list->capacity = list->capacity * LIST_CAPACITY_GROWTH_RATE;\
		list->content = realloc(list->content, list->capacity * sizeof(type));\
	}\
	\
	list->length++;\
	return list->content + (list->length - 1); \
}