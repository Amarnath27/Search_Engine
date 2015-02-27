#ifndef _INVERTED_INDEX__
#define _INVERTED_INDEX__
/* inverted_index.h
Description: Implements an inverted index
*/
// Max number of slots in the hash table.
#define MAX_HASH_SLOT 10000

// The longest word someone would want to search for is less than 100 characters long.
#define MAX_WORD_LENGTH 100

// This is the data structure that holds the information for each Document. DocumentNodes are parts of singly-linked lists.
typedef struct _DocumentNode {
  struct _DocumentNode *next;        // pointer to the next member of the list.
  int document_id;                   // document identifier
  int page_word_frequency;           // number of occurrences of the word
} 
  DocumentNode;

// This is the general Node placed in the index's double-linked list.  It also holds a pointer to the first item in a single-linked list of DocumentNode pointers.
typedef struct _WordNode {
  struct _WordNode *prev;           // pointer to the previous word
  struct _WordNode *next;           // pointer to the next word
  char word[MAX_WORD_LENGTH];           // the word
  DocumentNode  *page;              // pointer to the first element of the page list.
}
 WordNode;

// The inverted index holds both a double-linked list of WordNode pointers and a hash table with slots pointing to individual WordNode pointers.
typedef struct _INVERTED_INDEX {
                                        // Start and end pointer of the dynamic links.
  WordNode *start;                      // start of the list
  WordNode *end;                        // end of the list
  WordNode *hash[MAX_HASH_SLOT];  // hash slot
} 
 INVERTED_INDEX;



int hash(char* s);
void addWNode(char* word, WordNode* prev, INVERTED_INDEX* index, int hashed);
void addDNode(WordNode* wnode, int documentId, int page_word_frequency);
int updateIndex(char *word, int documentId, INVERTED_INDEX *index);
int saveIndexToFile(INVERTED_INDEX* index, char* path);
int readIndexFromFile(char* fname, INVERTED_INDEX* index);
int updateNewIndex(char *word, int documentId, int page_word_frequency, INVERTED_INDEX *index);
#endif
