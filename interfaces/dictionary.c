#include <stdio.h>
#include <stdlib.h>
#include "dictionary.h"
#include "header.h"
#include <string.h>

/*
  Method to initialize dict data structure
  @return 1 - upon success
*/
DICTIONARY *initDict() {
  DICTIONARY *dict = (DICTIONARY *)malloc( sizeof(DICTIONARY) );
  MALLOC_CHECK(dict);
  BZERO(dict, sizeof(DICTIONARY));
  //Have the start and end pointer be NULL initially
  dict->start = dict->end = NULL;
  
  return dict;
}

/*
  Method to create a URLNODE and DNODE for a url
  @param url - url to be added
  @param depth - current search depth
  @return DNODE that has been created
*/
DNODE *dadd(char *url, int depth) {
  URLNODE *urlnode = (URLNODE *)malloc( sizeof(URLNODE) );
  MALLOC_CHECK(urlnode);
  BZERO(urlnode, sizeof(URLNODE));
  strcpy(urlnode->url, url);
  urlnode->depth = depth;
  urlnode->visited = 0;

  DNODE *dnode = (DNODE *)malloc( sizeof(DNODE) );
  MALLOC_CHECK(dnode);
  BZERO(dnode, sizeof(DNODE));
  dnode->data = urlnode;
  strcpy(dnode->key, url);

  return dnode;
}

/*
  Method to clean up the dict that has been malloc'ed throughout running the program
*/
void cleanupdict(DICTIONARY *dict) {
  //Start at the first dnode in dict and loop through, freeing each element until we're at the end
  DNODE *current_dnode = dict->start;
  while ( current_dnode != dict->end ) {
    current_dnode = current_dnode->next;
    free(current_dnode->prev->data);
    free(current_dnode->prev);
  }
  //That loop won't delete the final element in dict, so do that here
  free(dict->end->data);
  free(dict->end);
  
  //Free dict itself
  free(dict);
}
