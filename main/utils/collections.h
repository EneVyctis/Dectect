#ifndef COLLECTIONS_H_GUARD
#define COLLECTIONS_H_GUARD


#include "hashtable.h"
#include "list.h"

struct MAC_address;
struct Tag;


DECLARE_HASH_SET(struct MAC_address, MAC_address_hashset)
DECLARE_LIST(struct MAC_address, MAC_address_lst)
DECLARE_LIST(struct Tag, TAG_lst)

#endif