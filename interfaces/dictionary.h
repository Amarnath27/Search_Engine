#ifndef _DICTIONARY_H
#define _DICTIONARY_H

//Constants
//// The max length of each URL path.
//// Since there is no defined limit by IETF or W3C, we'll go with the 
//// effective limit imposed by popular browsers, plus a little:
////    2048 characters + a NULL
//
#define MAX_URL_LENGTH 2049


// The KEY is the same as a URL. The term KEY is just a
// general Dictionary/hash function term 

#define KEY_LENGTH 2049

// Make the hash large in comparison to the number of URLs found
// for example depth of 2 on www.cs.dartmouth.edu approx 200.
// This will minimize collisions and mostly likely slots will be
// empty or have only one or two DNODES. Access is O(1). Fast.

#define MAX_HASH_SLOT 10000

// This is the key data structure that holds the information of each URL.

typedef struct _URL{
  char url[MAX_URL_LENGTH];      // e.g., www.cs.dartmouth.edu
  int depth;                     //  depth associated with this URL.
  int visited;                   //  crawled or not, marked true(1), otherwise false(0)
} URLNODE;

// Dictionary Node. This is a general double link list structure that
// holds the key (URL - we explained into today's lecture why this is there)
// and a pointer to void that points to a URLNODE in practice. 
// key is the same as URL recall.

typedef struct _DNODE {
  struct _DNODE  *next;
  struct _DNODE  *prev;
  void    *data;        //  actual data points to URLNODE
  char key[KEY_LENGTH]; //  actual (URL) key 
} DNODE;

// The DICTIONARY holds the hash table and the start and end pointers into a double 
// link list. This is a unordered list with the exception that DNODES with the same key (URL) 
// are clusters along the list. So you hash into the list. Check for uniqueness of the URL.
// If not found add to the end of the cluster associated wit the same URL. You will have
// to write an addElement function.

typedef struct _DICTIONARY {
  DNODE *hash[MAX_HASH_SLOT]; // the hash table of slots, each slot points to a DNODE
  DNODE *start;               // start of double link list of DNODES terminated by NULL pointer
  DNODE *end;                 // points to the last DNODE on this list
} DICTIONARY;
  
// Define the dict structure that holds the hash table 
// and the double linked list of DNODES. Each DNODE holds
// a pointer to a URLNODE. This list is used to store
// unique URLs. The search time for this list is O(n).
// To speed that up to O(1) we use the hash table. The
// hash table holds pointers into the list where 
// DNODES with the same key are maintained, assuming
// the hash(key) is not NULL (which implies the URL has
// not been seen before). The hash table provide quick
// access to the point in the list that is relevant
// to the current URL search. 

//extern DICTIONARY *dict;



DICTIONARY *initDict();

/*
  Method to create a URLNODE and DNODE for a url
  @param url - url to be added
  @param depth - current search depth
  @return DNODE that has been created
*/
DNODE *dadd(char *url, int depth);

/*
  Method to clean up the dict that has been malloc'ed throughout running the program
*/
void cleanupdict(DICTIONARY *dict);

#endif
