#include <string.h>
#include "frames.h"
#include "collections.h"

#define CONST_P 0x5555555555555555ull
#define CONST_C 17316035218449499591ull

#define IMPLIES(a, b) (!(a) || (b)) 

#define xorshift(n, i) (n) ^ ((n) >> (i))
#define COMBINE_HASHES_64B(a, b) (a) ^ ( (b) + 0x517cc1b727220a95 + ((a) << 6) + ((a) >> 2))
#define HASH_64b(a) CONST_C * xorshift(CONST_P * xorshift((a), 32), 32)
#define HASH_32b(a) HASH_64b(((uint64_t)(a)) | (((uint64_t)(a)) << 32))
#define HASH_16b(a) HASH_64b(((uint64_t)(a)) | (((uint64_t)(a)) << 16) | (((uint64_t)(a)) << 32) | (((uint64_t)(a)) << 48))
#define HASH_8b(a) HASH_64b(((uint64_t)(a)) | (((uint64_t)(a)) << 8) | (((uint64_t)(a)) << 16) | (((uint64_t)(a)) << 24) | (((uint64_t)(a)) << 32) | (((uint64_t)(a)) << 40) | (((uint64_t)(a)) << 48) | (((uint64_t)(a)) << 56))


uint64_t hashMacAddress(struct MAC_address* a)
{
	uint64_t hash1 = HASH_16b(*((uint16_t*) &a->addr[0]));
	uint64_t hash2 = HASH_16b(*((uint16_t*) &a->addr[2]));
	uint64_t hash3 = HASH_16b(*((uint16_t*) &a->addr[4]));
	return COMBINE_HASHES_64B(COMBINE_HASHES_64B(hash1, hash2), hash3);
}

bool areMacAddressesEqual(struct MAC_address* a, struct MAC_address* b)
{
	return a->addr[0] == b->addr[0] 
		&& a->addr[1] == b->addr[1]
		&& a->addr[2] == b->addr[2]
		&& a->addr[3] == b->addr[3]
		&& a->addr[4] == b->addr[4]
		&& a->addr[5] == b->addr[5];
}




unsigned long hashProbeRequestIdentifier(struct probe_request_identifier* a)
{
	uint64_t hash = HASH_16b(*(uint16_t*)&a->OUI);
	hash = COMBINE_HASHES_64B(hash, HASH_8b(a->OUI[2]));
	if(a->supported_rates.tag_length == 8)
		hash = COMBINE_HASHES_64B(hash, HASH_64b(*(uint64_t*)a->supported_rates.values));

	return hash;
}

bool areProbeRequestIdentifierEqual(struct probe_request_identifier* a, struct probe_request_identifier* b)
{
	return a->OUI[0] == b->OUI[0]
		&& a->OUI[1] == b->OUI[1]
		&& a->OUI[2] == b->OUI[2]
		&& a->supported_rates.tag_length == b->supported_rates.tag_length
		&& IMPLIES(a->supported_rates.tag_length == 8, *(uint64_t*) a->supported_rates.values == *(uint64_t*) b->supported_rates.values);
}



DEFINE_HASH_SET(struct MAC_address, MAC_address_hashset, hashMacAddress, areMacAddressesEqual)
DEFINE_HASH_SET(struct probe_request_identifier, pri_hashset, hashProbeRequestIdentifier, areProbeRequestIdentifierEqual)


DEFINE_LIST(struct MAC_address, MAC_address_lst)
DEFINE_LIST(struct Tag, TAG_lst)
