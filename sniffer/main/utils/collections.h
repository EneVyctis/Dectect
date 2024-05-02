#ifndef COLLECTIONS_H_GUARD
#define COLLECTIONS_H_GUARD


#include "hashtable.h"
#include "list.h"
#include "frames.h"

struct MAC_address;
struct probe_request_identifier;
struct Tag;


DECLARE_HASH_SET(struct MAC_address, MAC_address_hashset)
DECLARE_HASH_MAP(struct MAC_address, int64_t, MAC_address_hashmap)
DECLARE_HASH_SET(struct probe_request_identifier, pri_hashset)
DECLARE_LIST(struct MAC_address, MAC_address_lst)
DECLARE_LIST(struct Tag, TAG_lst)

#endif