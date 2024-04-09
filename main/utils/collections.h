#include "hashtable.h"
#include "list.h"

struct MAC_address;


DECLARE_HASH_SET(struct MAC_address, MAC_address_hashset)
DECLARE_LIST(struct MAC_address, MAC_address_lst)
