#include<stdlib.h>
#include<stdbool.h>

#define INITIAL_BUCKET_COUNT 50
#define BUCKET_COUNT_GROWTH_RATE 2
#define INITIAL_BUCKET_CAPACITY 4
#define BUCKET_CAPACITY_GROWTH_RATE 2
#define MAX_LOAD_FACTOR 2

#define DECLARE_HASH_SET(type, name) \
\
struct name##_bucket\
{\
	type* values;\
	int length;\
	int capacity;\
};\
\
typedef struct name\
{\
	struct name##_bucket* buckets;\
	long bucketCount;\
	long objCount;\
} name;\
\
typedef struct name##_iterator\
{\
	struct name* hashtable;\
	long bucketIndex;\
	int valueIndex;\
	int objIterated;\
} name##_iterator;\
\
void name##_init(struct name* hashtable);\
void name##_destroy(struct name* hashtable);\
bool name##_contains(struct name* hashtable, type* value);\
void name##_rehash(struct name* hashtable, int newSize);\
type* name##_insert(struct name* hashtable, type* value);\
bool name##_delete(struct name* hashtable, type* value);\
void name##union_with(struct name* destination, struct name* source);\
\
void name##_init_iterator(struct name* hashtable, struct name##_iterator* iterator);\
bool name##_iterator_has_next(struct name##_iterator* iterator);\
type* name##_iterator_next(struct name##_iterator* iterator);

#define DEFINE_HASH_SET(type, name, hashfunction, equalfunction)\
\
void name##_init(struct name* hashtable)\
{\
	hashtable->buckets = calloc(INITIAL_BUCKET_COUNT, sizeof(struct name##_bucket));\
	hashtable->bucketCount = INITIAL_BUCKET_COUNT;\
}\
\
void name##_destroy(struct name* hashtable)\
{\
	for(int i=0; i<hashtable->bucketCount; i++)\
		if(hashtable->buckets[i].capacity > 0)\
			free(hashtable->buckets[i].values);\
	\
	free(hashtable->buckets);\
	hashtable->bucketCount = 0;\
	hashtable->objCount = 0;\
}\
\
bool name##_contains(struct name* hashtable, type* value)\
{\
	unsigned long long hash = hashfunction(value);\
	struct name##_bucket* bucket = hashtable->buckets + (hash % hashtable->bucketCount);\
	\
	for(int i=0; i<bucket->length; i++)\
		if(equalfunction(bucket->values + i, value))\
			return true;\
	\
	return false;\
}\
\
void name##_rehash(struct name* hashtable, int newSize)\
{\
	struct name##_bucket* newBuckets = calloc(newSize, sizeof(struct name##_bucket));\
	\
	for(int oldBucketIndex=0; oldBucketIndex < hashtable->bucketCount; oldBucketIndex++)\
	{\
		for(int oldElementIndex=0; oldElementIndex < hashtable->buckets[oldBucketIndex].length; oldElementIndex++)\
		{\
			unsigned long long hash = hashfunction(hashtable->buckets[oldBucketIndex].values + oldElementIndex);\
			struct name##_bucket* bucket = newBuckets + (hash % newSize);\
			\
			if(bucket->capacity == 0)\
			{\
				bucket->capacity = INITIAL_BUCKET_CAPACITY;\
				bucket->values = malloc(bucket->capacity * sizeof(type));\
			}\
			else if (bucket->capacity <= bucket->length)\
			{\
				bucket->capacity = bucket->capacity * BUCKET_CAPACITY_GROWTH_RATE;\
				bucket->values = realloc(bucket->values, bucket->capacity * sizeof(type));\
			}\
			\
			bucket->length++;\
			bucket->values[bucket->length - 1] = hashtable->buckets[oldBucketIndex].values[oldElementIndex];\
		}\
		\
		free(hashtable->buckets[oldBucketIndex].values);\
	}\
	\
	free(hashtable->buckets);\
	hashtable->buckets = newBuckets;\
	hashtable->bucketCount = newSize;\
	\
}\
\
type* name##_insert(struct name* hashtable, type* value)\
{\
	if((hashtable->objCount+1) >= MAX_LOAD_FACTOR * (double)hashtable->bucketCount)\
		name##_rehash(hashtable, hashtable->bucketCount * BUCKET_COUNT_GROWTH_RATE);\
	\
	unsigned long long hash = hashfunction(value);\
	struct name##_bucket* bucket = hashtable->buckets + (hash % hashtable->bucketCount);\
	\
	for(int i=0; i<bucket->length;i++)\
		if(equalfunction(bucket->values + i, value))\
			return NULL;\
	\
	if(bucket->capacity == 0)\
	{\
		bucket->capacity = INITIAL_BUCKET_CAPACITY;\
		bucket->values = malloc(bucket->capacity * sizeof(type));\
	}\
	else if (bucket->capacity <= bucket->length)\
	{\
		bucket->capacity = bucket->capacity * BUCKET_CAPACITY_GROWTH_RATE;\
		bucket->values = realloc(bucket->values, bucket->capacity * sizeof(type));\
	}\
	\
	bucket->length++;\
	hashtable->objCount++;\
	bucket->values[bucket->length - 1] = *value;\
	\
	return bucket->values + (bucket->length - 1);\
}\
\
bool name##_delete(struct name* hashtable, type* value)\
{\
	unsigned long long hash = hashfunction(value);\
	struct name##_bucket* bucket = hashtable->buckets + (hash % hashtable->bucketCount);\
	\
	for(int i=0; i<bucket->length; i++)\
	{\
		if(equalfunction(bucket->values + i, value))\
		{\
			for(int j=i; j<bucket->length-1; j++)\
				bucket->values[j] = bucket->values[j+1];\
			bucket->length--;\
			hashtable->objCount--;\
			return true;\
		}\
	}\
	\
	return false;\
}\
\
void name##union_with(struct name* destination, struct name* source)\
{\
	/*find global length*/\
	long globalBucketCount = destination->bucketCount;\
	while(destination->objCount + source->objCount >= globalBucketCount * MAX_LOAD_FACTOR) \
		globalBucketCount *= BUCKET_COUNT_GROWTH_RATE;\
	\
	if(globalBucketCount > destination->bucketCount)\
		name##_rehash(destination, globalBucketCount);\
	\
	for(int sourceBucketIndex=0; sourceBucketIndex < source->bucketCount; sourceBucketIndex++)\
	{\
		for(int sourceValueIndex=0; sourceValueIndex < source->buckets[sourceBucketIndex].length; sourceValueIndex++)\
		{\
			type* value = source->buckets[sourceBucketIndex].values + sourceValueIndex;\
			unsigned long long hash = hashfunction(value);\
			struct name##_bucket* bucket = destination->buckets + (hash % destination->bucketCount);\
			\
			bool isValueAlreadyPresent = false;\
			for(int i=0; i<bucket->length;i++)\
			{\
				if(equalfunction(bucket->values + i, value))\
				{\
					isValueAlreadyPresent = true;\
					break;\
				}\
			}\
			\
			if(isValueAlreadyPresent)\
				break;\
			\
			if(bucket->capacity == 0)\
			{\
				bucket->capacity = INITIAL_BUCKET_CAPACITY;\
				bucket->values = malloc(bucket->capacity * sizeof(type));\
			}\
			else if (bucket->capacity <= bucket->length)\
			{\
				bucket->capacity = bucket->capacity * BUCKET_CAPACITY_GROWTH_RATE;\
				bucket->values = realloc(bucket->values, bucket->capacity * sizeof(type));\
			}\
			\
			\
			bucket->length++;\
			bucket->values[bucket->length - 1] = *value;\
			\
			destination->objCount++;\
		}\
	}\
	\
}\
\
\
\
\
\
void name##_init_iterator(struct name* hashtable, struct name##_iterator* iterator)\
{\
	iterator->hashtable = hashtable;\
	iterator->bucketIndex = 0;\
	iterator->valueIndex = 0;\
	iterator->objIterated = 0;\
}\
\
bool name##_iterator_has_next(struct name##_iterator* iterator)\
{\
	return iterator->objIterated < iterator->hashtable->objCount;\
}\
\
type* name##_iterator_next(struct name##_iterator* iterator)\
{\
	if(iterator->valueIndex < iterator->hashtable->buckets[iterator->bucketIndex].length)\
	{\
		iterator->objIterated++;\
		return iterator->hashtable->buckets[iterator->bucketIndex].values + (iterator->valueIndex++);\
	}\
	\
	while(iterator->hashtable->buckets[++(iterator->bucketIndex)].length == 0);\
	\
	iterator->objIterated++;\
	iterator->valueIndex = 1;\
	return iterator->hashtable->buckets[iterator->bucketIndex].values;\
}


