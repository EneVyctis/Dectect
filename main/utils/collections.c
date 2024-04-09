#include "frames.h"
#include "collections.h"

#define xorshift(n, i) (n)^((n)>>(i))

unsigned long hashMacAddress(struct MAC_address* a)
{
	uint64_t p = 0x5555555555555555ull;
	uint64_t c = 17316035218449499591ull;

	uint64_t input = a->addr[0] 
		| a->addr[1] << 8 
		| a->addr[2] << 16
		| a->addr[3] << 24 
		| ((uint64_t) a->addr[4]) << 32
		| ((uint64_t) a->addr[5]) << 40;

	return c*xorshift(p*xorshift(input,32),32); 
}

int areMacAddressesEqual(struct MAC_address* a, struct MAC_address* b)
{
	return a->addr[0] == b->addr[0] 
		&& a->addr[1] == b->addr[1]
		&& a->addr[2] == b->addr[2]
		&& a->addr[3] == b->addr[3]
		&& a->addr[4] == b->addr[4]
		&& a->addr[5] == b->addr[5];
}

DEFINE_HASH_SET(struct MAC_address, MAC_address_hashset, hashMacAddress, areMacAddressesEqual)


DEFINE_LIST(struct MAC_address, MAC_address_lst)
