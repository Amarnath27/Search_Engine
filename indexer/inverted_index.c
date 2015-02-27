#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "header.h"
#include "hash.h"
#include "inverted_index.h"

// Returns a hash value between 0 and MAX_HASH_SLOT
int hash(char *s)
{
  return (hash1(s) % MAX_HASH_SLOT);
}
 

// Adds a word node to the specified index after the prev WordNode
void addWNode(char *word, WordNode* prev, INVERTED_INDEX* index, int hashed)
{
  int h = hash(word);
  WordNode* wnode = malloc(sizeof(WordNode));
  MALLOC_CHECK(wnode);
  BZERO(wnode, sizeof(WordNode));
  wnode->page = NULL;
  strncpy(wnode->word, word, MAX_WORD_LENGTH);
  if (index->start == NULL)
    {
      MYASSERT(index->end == NULL);
      index->hash[h] = wnode;
      wnode->next = wnode;
      wnode->prev = wnode;
      strncpy(wnode->word, word, MAX_WORD_LENGTH);
      index->start = index->end = wnode;
    }
  else
    {
      wnode->next = prev->next;
      prev->next->prev = wnode;
      prev->next = wnode;
      wnode->prev = prev;
      if (prev == index->end)
	index->end = wnode;
      if (hashed)
	index->hash[h] = wnode;
    }
}

// Adds a DocumentNode to the wnode's SLL specified with the params documentID and page_word_frequency
void addDNode(WordNode* wnode, int documentId, int page_word_frequency)
{
  DocumentNode* dnode = malloc(sizeof(DocumentNode));
  DocumentNode* temp = NULL;
  MALLOC_CHECK(dnode);
  dnode->document_id = documentId;
  dnode->page_word_frequency = page_word_frequency;
  if (wnode->page == NULL)
    {
      wnode->page = dnode;
      wnode->page->next = NULL;
    }
  else
    {
      for (temp = wnode->page; temp -> next != NULL; temp = temp->next)
	{
	  if (documentId == temp->document_id)
	    {
	      free(dnode);
	      temp->page_word_frequency++;
	      return;
	    }
	}
      temp->next = dnode;
      dnode->next = NULL;
    }
}

// Updates an existing WordNode or returns 0
int updateWord(WordNode* wnode, int documentId, INVERTED_INDEX* index)
{
  for (DocumentNode* temp = wnode->page; temp != NULL; temp = temp->next)
    {
      if (temp->document_id == documentId)
	{
	  (temp->page_word_frequency)++;
	  return 1;
	}
    }
  return 0;
}

// Saves an index to a file using a homogeneous format.
int saveIndexToFile(INVERTED_INDEX* index, char* path)
{
  FILE* fp = fopen(path, "w");
  int i = 0;
  for (WordNode* wTemp = index->start; wTemp != index->end; wTemp = wTemp->next)
    {  
      fprintf(fp, "%s ", wTemp->word);
      for (DocumentNode* dTemp =  wTemp->page; dTemp != NULL; dTemp = dTemp->next)
	{
	  if (dTemp->page_word_frequency > 0)
	    i++;
	}
      fprintf(fp, "%d ", i);
      for (DocumentNode* dTemp =  wTemp->page; dTemp != NULL; dTemp = dTemp->next)
	{
	  if (dTemp->document_id >= 0 && dTemp->page_word_frequency != 0)
	    fprintf(fp, "%d %d ", dTemp->document_id, dTemp->page_word_frequency);
	}
      fprintf(fp, "\n");
      i = 0;
    }    
  if (index->end != NULL)
    {
      fprintf(fp, "%s ", index->end->word);
      for (DocumentNode* dTemp =  index->end->page; dTemp != NULL; dTemp = dTemp->next)
	{
	  if (dTemp->page_word_frequency > 0)
	i++;
	}
      fprintf(fp, "%d ", i);
      for (DocumentNode* dTemp =  index->end->page; dTemp != NULL; dTemp = dTemp->next)
	{
	  if (dTemp->document_id >= 0 && dTemp->page_word_frequency != 0)
	    fprintf(fp, "%d %d ", dTemp->document_id, dTemp->page_word_frequency);
	}
    }
  fprintf(fp, "\n");
  fclose(fp);
  return 1;
}

// Reads an index from a file and calls updateNewIndex as it processes the file.
int readIndexFromFile(char* fname, INVERTED_INDEX* index)
{
  FILE* fp = fopen(fname, "r");
  char line[100000];
  char* search =" ";
  char* sDocNum;
  char *sPageFreq;
  char* word;
  int docNum, pageFreq, docsCount;
  while (((fgets(line, 100000, fp)) != NULL))
    {
      word = strtok(line, search);
      docsCount = strtol(strtok(NULL, search), NULL, 10);
      for (int i = 0; i < (2 * docsCount); i++)
	{
	  if ((sDocNum = strtok(NULL, search)) != NULL)
	    docNum = strtol(sDocNum, NULL, 10);
	  if ((sPageFreq = strtok(NULL, search)) != NULL)
	    pageFreq = strtol(sPageFreq, NULL, 10);
	  if (pageFreq != 0)
	    updateNewIndex(word, docNum, pageFreq, index);
	  docNum = pageFreq = 0;
	}
      docsCount = 0;
    }
  fclose(fp);
  return 1;
}

// Updates the new index that is used for the reload of the original index.
int updateNewIndex(char *word, int documentId, int page_word_frequency, INVERTED_INDEX *index)
{
  /* Check if the word is a null character */
  if (word == '\0')
    return 0;
  
  /* If something in hash slot already, perform collision procedure */
  if ((index->hash[hash(word)] != NULL))
    {
      WordNode* hwnode = index->hash[hash(word)];
      for (WordNode* tempW = hwnode; hash(word) == hash(tempW->word); tempW = tempW->next)
	{
	 
	  if (tempW == NULL)
	    return 0;
	  if (strncmp(tempW->word, word, strlen(word)) == 0)
	    {
		{
		  addDNode(tempW, documentId, page_word_frequency);
		  return 1;
		}
	    }
	  if (tempW->next == hwnode)
	    break;
	}
      addWNode(word, hwnode, index, 0);
      addDNode(index->hash[hash(word)]->next, documentId, page_word_frequency);
      return 1;
    }

  /* Otherwise, add a node at that hash position */
  else
    {
      addWNode(word, index->end, index, 1);
      addDNode(index->end, documentId, page_word_frequency);
      return 1;
    }
}

// Update the original index by either updating an existing node or adding a new one.
int updateIndex(char *word, int documentId, INVERTED_INDEX *index)
{
  if (word[0] == '\0')
    return 0;
  /* Check if the word is already in the index or if there is just a collision. */
  if ((index->hash[hash(word)] != NULL))
    {
      WordNode* hwnode = index->hash[hash(word)];
      if (hwnode == NULL)
	return 0;
      for (WordNode* tempW = hwnode; hash(word) == hash(tempW->word) ; tempW = tempW->next)
	{
	  if (tempW == NULL)
	    {
	      fprintf(stdout, "[UPDATEINDEX] Node to be updated is null");
	      return 0;
	    }
	  if (strncmp(tempW->word, word, strlen(word)) == 0)
	    {
	      if (updateWord(hwnode, documentId, index))
		return 1;
	      else 
		{
		  addDNode(tempW, documentId, 1);
		  return 1;
		}
	    }
	  if (tempW->next == hwnode || tempW->next == NULL)
	    break;
	}
      addWNode(word, hwnode, index, 0);
      addDNode(index->hash[hash(word)]->next, documentId, 1);
      return 1;
    }

  /* If not, add a new word node in that hash slot. */
  else
    {
      addWNode(word, index->end, index, 1);
      addDNode(index->end, documentId, 1);
      return 1;
    }
}
