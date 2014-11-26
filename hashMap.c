#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "hashMap.h"


/*the first hashing function you can use*/
int stringHash1(char *str)
{
	int i;
	int r = 0;
	for (i = 0; str[i] != '\0'; i++)
		r += str[i];
	return r;
}

/*the second hashing function you can use*/
int stringHash2(char *str)
{
	int i;
	int r = 0;
	for (i = 0; str[i] != '\0'; i++)
		r += (i+1) * str[i]; /*the difference between stringHash1 and stringHash2 is on this line*/
	return r;
}

/* initialize the supplied hashMap struct*/
void _initMap (struct hashMap *ht, int tableSize)
{
	int index;
	if(ht == NULL)
		return;
	ht->table = (hashLink**)malloc(sizeof(hashLink*) * tableSize);
	ht->tableSize = tableSize;
	ht->count = 0;
	for(index = 0; index < tableSize; index++)
		ht->table[index] = NULL;
}

/* allocate memory and initialize a hash map*/
hashMap *createMap(int tableSize) {
	assert(tableSize > 0);
	hashMap *ht;
	ht = malloc(sizeof(hashMap));
	assert(ht != 0);
	_initMap(ht, tableSize);
	return ht;
}


//Helper function written by Brandon Swanson
int _hashedIndex(int TSize, KeyType k){
    if(HASHING_FUNCTION == 1)
        return (stringHash1(k) + TSize) % TSize;
    else
        return (stringHash2(k) + TSize) % TSize;
}

/*
 Free all memory used by the buckets.
 Note: Before freeing up a hashLink, free the memory occupied by key and value
 */
void _freeMap (struct hashMap *ht)
{  
    assert(ht);

    hashLink *bucket, *temp;
    for(int i=0; i<ht->tableSize; i++){
        bucket = ht->table[i];
        while(bucket){
            temp = bucket;
            bucket = temp->next;
            free(temp->key);
            free(temp);
        }
    }
}

/* Deallocate buckets and the hash map.*/
void deleteMap(hashMap *ht) {
	assert(ht!= 0);
	/* Free all memory used by the buckets */
	_freeMap(ht);
	/* free the hashMap struct */
	free(ht);
}

/* 
Resizes the hash table to be the size newTableSize 
*/
void _setTableSize(struct hashMap *ht, int newTableSize)
{
	assert(ht);

	//allocate new table and init
	hashLink** temp = (hashLink**) malloc(sizeof(hashLink*)*newTableSize);
	assert(temp);
	for(int i=0;i<newTableSize;i++){
	    temp[i] = NULL;
	}

	//move links to new table
	for(int i=0; i<ht->tableSize; i++){
	    while(ht->table[i]){ //check for empty bucket or end of chain
	        int newIndex = _hashedIndex(newTableSize,ht->table[i]->key);
	        ht->table[i]->next = temp[newIndex];
	        temp[newIndex] = ht->table[i];

	        ht->table[i] = ht->table[i]->next;
	    }

	}

	//switch pointer update size//

	hashLink** remember = ht->table;

	ht->table = temp;
	ht->tableSize = newTableSize;

	free(remember);
}

/*
 insert the following values into a hashLink, you must create this hashLink but
 only after you confirm that this key does not already exist in the table. For example, you
 cannot have two hashLinks for the word "taco".
 
 if a hashLink already exists in the table for the key provided you should
 replace that hashLink--this requires freeing up the old memory pointed by hashLink->value
 and then pointing hashLink->value to value v.
 
 also, you must monitor the load factor and resize when the load factor is greater than
 or equal LOAD_FACTOR_THRESHOLD (defined in hashMap.h).
 */
void insertMap (struct hashMap *ht, KeyType k, ValueType v)
{  
    assert(ht);

	//compute hash and index
    int index = _hashedIndex(ht->tableSize,k);

    if(containsKey(ht,k)){
        //find matching key
        hashLink *cur = ht->table[index];
        while(cur && strcmp(cur->key, k)) cur=cur->next;
        assert(cur);

        //replace values in link
        cur->value = v;
        free(k);
        return;
    }

    hashLink* chain = ht->table[index];
    hashLink *newLink = malloc(sizeof(hashLink));
    assert(newLink);

    //setvalues for new link
    newLink->key = k;
    newLink->value = v;
    newLink->next = chain;

    //insert into table
    ht->table[index] = newLink;
    ht->count++;

    //ensure proper load factor
    if(((ht->count / (double) ht->tableSize) > LOAD_FACTOR_THRESHOLD))
        _setTableSize(ht, ht->tableSize * 2);

}

/*
 this returns the value (which is void*) stored in a hashLink specified by the key k.
 
 if the user supplies the key "taco" you should find taco in the hashTable, then
 return the value member of the hashLink that represents taco.
 
 if the supplied key is not in the hashtable return NULL.
 */
ValueType* atMap (struct hashMap *ht, KeyType k)
{ 
    assert(ht);

    //compute hash and index
    int index = _hashedIndex(ht->tableSize,k);

    /*search for element*/
    hashLink *bucket = ht->table[index];
    while(bucket){
      if(strcmp(bucket->key, k) == 0)  return &(bucket->value);
      bucket=bucket->next;
    }
    //key not found
    return (ValueType*) NULL;
}

/*
 a simple yes/no if the key is in the hashtable. 
 0 is no, all other values are yes.
 */
int containsKey (struct hashMap *ht, KeyType k)
{  
    assert(ht);

    //compute hash and index
    int index = _hashedIndex(ht->tableSize,k);

    /*search for element*/
    hashLink *bucket = ht->table[index];
    while(bucket){
      if(strcmp(bucket->key, k) == 0)  return 1;
      bucket=bucket->next;
    }
    //key not found
    return 0;
}

/*
 find the hashlink for the supplied key and remove it, also freeing the memory
 for that hashlink. it is not an error to be unable to find the hashlink, if it
 cannot be found do nothing (or print a message) but do not use an assert which
 will end your program.
 */
void removeKey (struct hashMap *ht, KeyType k)
{  
    assert(ht);

    //compute hash and index
    int index = _hashedIndex(ht->tableSize,k);

    /*search for element*/
    hashLink *bucket = ht->table[index];
    hashLink *prev = bucket;
    while(bucket){
      if(strcmp(bucket->key, k) == 0){
          prev->next = bucket->next;
          free(bucket);
          return;
      }
      prev=bucket;
      bucket=bucket->next;
    }
    //key not found return with no effect
}

/*
 returns the number of hashLinks in the table
 */
int size (struct hashMap *ht)
{  
    assert(ht);
    int numLinks = 0;
	for(int i=0; i<ht->tableSize; i++){
	    hashLink *bucket = ht->table[i];
        while(bucket){
            numLinks++;
            bucket=bucket->next;
        }
	}
	return numLinks;
	
}

/*
 returns the number of buckets in the table
 */
int capacity(struct hashMap *ht)
{  
	assert(ht);
	return ht->tableSize;
}

/*
 returns the number of empty buckets in the table, these are buckets which have
 no hashlinks hanging off of them.
 */
int emptyBuckets(struct hashMap *ht)
{  
    assert(ht);
    int numEmpty = 0;
    for(int i=0; i<ht->tableSize; i++){
        if(!ht->table[i]) numEmpty++;
    }
    return numEmpty;
}

/*
 returns the ratio of: (number of hashlinks) / (number of buckets)
 
 this value can range anywhere from zero (an empty table) to more then 1, which
 would mean that there are more hashlinks then buckets (but remember hashlinks
 are like linked list nodes so they can hang from each other)
 */
float tableLoad(struct hashMap *ht)
{  
    assert(ht);
	return size(ht) / (double) ht->tableSize;
}

void printMap (struct hashMap *ht)
{
	int i;
	struct hashLink *temp;	
	for(i = 0;i < capacity(ht); i++){
		temp = ht->table[i];
		if(temp != 0) {		
			printf("\nBucket Index %d -> ", i);		
		}
		while(temp != 0){			
			printf("Key:%s|", temp->key);
			printValue(temp->value);
			printf(" -> ");
			temp=temp->next;			
		}		
	}
}


