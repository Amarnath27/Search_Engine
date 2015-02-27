#include <stdlib.h>
#include <stdio.h>
#include <string.h> 
#include <unistd.h> 
#include <sys/stat.h>
#include "header.h"
#include "inverted_index.h"
#include "file.h"
#include "html.h"

INVERTED_INDEX* index = NULL;
INVERTED_INDEX* new_index = NULL;
void initLists();
int checkArgs(int n, char* p1, char* r1, char* p2, char* r2);
void cleanupIndex(INVERTED_INDEX* index);
int main(int argc, char* argv[]){
  
  // Make sure the args are valid
  if (checkArgs(argc, argv[1], argv[2], argv[3], argv[4]) == 0)
    {
      return 1;
    }

  // Store the document names
  char** storedDocuments;
  if ((storedDocuments = findFilesInPath(argv[1])) == NULL)
    {
      return 1;
    }
	

  // Initialize the dictionary
  initLists();

  // Loop through the stored documents
  for (int j = 0; storedDocuments[j] != '\0'; j++)
    {
      // Load the document, get its ID, and get the next word from the doc.
      char *loadedDocument = loadDocument(storedDocuments[j], argv[1]);
      int documentId = getDocumentID(storedDocuments[j]);
      char* word = malloc(MAX_WORD_LENGTH);
      MALLOC_CHECK(word);
      BZERO(word, MAX_WORD_LENGTH);
      getNextWordFromHTMLDocument(loadedDocument, word, 1);
      NormalizeWord(word);
      // Update the index with the word and its ID
      updateIndex(word, documentId, index);
      while (getNextWordFromHTMLDocument(loadedDocument, word, 0))
	{
	  NormalizeWord(word);
	  updateIndex(word, documentId, index);
	}

      // Free everything besides the index.
      free(storedDocuments[j]);
      free(word);
      free(loadedDocument);
    }
  free(storedDocuments);
  
  // Save the index to a file and clean up the index.
  saveIndexToFile(index, argv[2]);
  cleanupIndex(index);

  // If the user entered 5 arguments, reload the index and save it again.
  if (argc == 5)
    {
      readIndexFromFile(argv[3], new_index);
      saveIndexToFile(new_index, argv[4]);
      cleanupIndex(new_index);
    }
  return 1;
}

// Initialize the inverted index.
void initLists()
{
  index = malloc(sizeof(INVERTED_INDEX));
  MALLOC_CHECK(index);
  BZERO(index, sizeof(INVERTED_INDEX));
  index->start = index->end = NULL;
  new_index = malloc(sizeof(INVERTED_INDEX));
  MALLOC_CHECK(new_index);
  BZERO(new_index, sizeof(INVERTED_INDEX));
  new_index->start = new_index->end = NULL;
}

// Cleanup the index by looping through and erasing all the DocumentNodes for each WordNode then free the WordNode
void cleanupIndex(INVERTED_INDEX* index)
{

  DocumentNode* prev;
  WordNode* temp;
  int i = 0;
  for (temp = index->start; i != 1;)
    {
      if (temp!=NULL)
	{
	  for (DocumentNode* dTemp = temp->page; dTemp != NULL;)
	    {
	      prev = dTemp;
	      dTemp = dTemp->next;
	      free(prev);
	      prev = NULL;
	    }
	  
	  if (temp == index->end)
	    {
	      free(temp);
	      temp = NULL;
	      i = 1;
	    }
	  else
	    {
	      temp = temp->next;
	      free(temp->prev); 
	      temp->prev = NULL;
	    }
	}
    }
  free(index);
  index = NULL;
      
}

// Check args and make sure the user didn't mess up.  If they did, inform them.
int checkArgs(int n, char* p1, char* r1, char* p2, char* r2)
{
  struct stat statbuf;
  if (!(n == 3 || n == 5))
    {
      fprintf(stdout, "User Input Error: Entered %d arguments.  Must enter 2 or 4 arguments.\n", n);
      return 0;
    }
  else
    {
      if ((stat(p1, &statbuf)) != 0 || !S_ISDIR(statbuf.st_mode))
	{
	  fprintf(stdout, "User Input Error: Entered %s, an invalid directory, as argument 1\n", p1);
	  return 0;
	}
    }

  return 1;
  
}
