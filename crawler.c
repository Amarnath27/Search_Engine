#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include "crawler.h"
#include "hash.h"
#include "html.h"
#include "input.h"
#include "header.h"
#include "dictionary.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

int initlists();
int urlnotpresent(char **url_list, char *url, int numextracted);
void cleanup(char *path);


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

DICTIONARY* dict = NULL; 


// This is the table that keeps pointers to a list of URL extracted
// from the current HTML page. NULL pointer represents the end of the
// list of URLs.

char **url_list; 

//This number is used when determining what html file to store the page in
int htmlnum = 1;

//This number keeps track of if the wget has been successful
int wgetsuccess = 0;



int main( int argc, char *argv[] ) {
  char *url;
  int max_depth;
  int current_depth = 0;
  char *path;
  char *page;

  checknuminputsone((argc-1), 3);

  testDir(argv[2]);
  
  //Check SEED URL. 
  //First, make sure it's a cs.dartmouth.edu one
  if ( NULL == strcasestr(argv[1], URL_PREFIX) ) {
    fprintf(stderr, "--------------------------------------------------------------\n");
    fprintf(stderr, "Invalid URL. Not cs.dartmouth.edu domain. Exiting the program.\n");
    fprintf(stderr, "--------------------------------------------------------------\n");
    exit(1);
  }


  //Do a system call to try wget'ing the page, and redirect its output to /dev/null so we don't get the file in the folder
  char checkurlbuffer[(MAX_URL_LENGTH + 30)] = {0};
  strcat(checkurlbuffer, "wget -q -T 10 --tries=2 ");
  strcat(checkurlbuffer, argv[1]);
  strcat(checkurlbuffer, " -O /dev/null");
  if ( system(checkurlbuffer) != 0 ) {
    fprintf(stderr, "Invalid URL. Exiting the program.\n");
    exit(1);
  }

  //Check depth, make sure its between 0 and 4
  if ( atoi(argv[3]) > 4 || atoi(argv[3]) < 0 ) {
    fprintf(stderr, "Invalid depth. Exiting the program.\n");
    exit(1);
  }

  if ( (atoi(argv[3]) == 0) && (isalpha(*argv[3])) ) {
    fprintf(stderr, "Invalid depth. Don't enter a string. Exiting the program.\n");
    exit(1);
  }

  //Set the variables equal to the input parameters
  url = argv[1];
  NormalizeURL(url);
  path = argv[2];
  max_depth = atoi(argv[3]);

  //Initialize data structures. Quit if the initialization goes wrong
  if ( initlists(dict) != 1) {
    fprintf(stderr, "Initialization gone wrong\n");
    exit(1);
  }

  //Add the seed URL to dict
  
  DNODE *seed_url_dnode = dadd(url, current_depth);
  seed_url_dnode->next = seed_url_dnode->prev = seed_url_dnode;

  //Have both the start and the end of dict initially point to the seed_url
  dict->start = dict->end = seed_url_dnode;

  //Get the seed_url's hash number and put it into that slot in dict's hash table
  long hashnum = (hash1(url) % MAX_HASH_SLOT);
  dict->hash[hashnum] = seed_url_dnode;

  page = getPage(url, current_depth, path);

  //Quit the program if the seed url comes back null, since there isn't much you can do after
  if ( page == NULL ) {
    fprintf(stderr, "Issue downloading seed url. It came back empty. Exiting the program.\n");
    exit(1);
  }

  url_list = extractURLs(page, url);

  free(page);

  updateListLinkToBeVisited(current_depth + 1);
  
  setURLasVisited(url);
  
  char *URLToBeVisited; //A pointer to the next URL we need to look at
  while ( (URLToBeVisited = getAddressFromTheLinksToBeVisited(&current_depth)) ) {
    //Check the next url if the depth is beyond max_depth
    if ( current_depth > max_depth ) {
      setURLasVisited(URLToBeVisited);
      continue;
    }

    page = getPage(URLToBeVisited, current_depth, path);

    //If the page comes back null, let the user know and go to the next url
    if ( page == NULL ) {
      fprintf(stderr, "Issue downloading url: %s. It came back empty. Moving on to next url\n", URLToBeVisited);
      setURLasVisited(URLToBeVisited);
      continue;
    }
    
    url_list = extractURLs(page, URLToBeVisited);
    
    free(page);
    updateListLinkToBeVisited(current_depth + 1);

    setURLasVisited(URLToBeVisited);

    sleep(INTERVAL_PER_FETCH);
  }

  printf("Nothing else to do");
    
  //Clean up what's left
  cleanup(path);
  
  return 0;
}

int initlists() {
  dict = initDict();
  if ( NULL == dict ) {
    return 0;
  }

  //url_list has memory for a number, MAX_URL_PER_PAGE, char pointers   
  url_list = (char **)malloc(MAX_URL_PER_PAGE * sizeof(char *));
  MALLOC_CHECK(url_list);
  BZERO(url_list, (sizeof(char *) * MAX_URL_PER_PAGE));

  return 1;
}

/*
  Method to retrieve the contents of a page given a url, a path to put the contents in, and the depth
  @param url - url to retrieve contents from
  @param depth - current search depth
  @param path - destination of file containing page contents
  @return buffer containing contents of the page
*/
char *getPage(char* url, int depth, char* path) {
  //Make sure the URL is normalized first
  NormalizeURL(url);

  //urlbuffer stores the string that is the system call to download the url using wget and store the
  //page in path/TEMP
  char *urlbuffer = (char *)malloc(MAX_URL_LENGTH + 40);
  MALLOC_CHECK(urlbuffer);
  BZERO(urlbuffer, (MAX_URL_LENGTH + 40));
  strcat(urlbuffer, "wget -q -T 10 --tries=2 -O ");
  strcat(urlbuffer, path);
  strcat(urlbuffer, "TEMP \"");
  strcat(urlbuffer, url);
  strcat(urlbuffer, "\"");
  wgetsuccess = system(urlbuffer); //System call
  
  if ( wgetsuccess != 0 ) {
    free(urlbuffer);
    wgetsuccess = 0;
    return NULL;
  }

  //pathname stores a string that is of the form path/TEMP, and is used to access the TEMP file
  char *pathname = malloc(strlen(path) + 15);
  MALLOC_CHECK(pathname);
  BZERO(pathname, (strlen(path) +  15));
  strcat(pathname, path);
  strcat(pathname, "TEMP");

  //Use stat to see how many bytes are in the page
  struct stat st;
  stat(pathname, &st);
  int numbytes = st.st_size;

  //filebuffer stores the information that is contained in the webpage
  char *filebuffer = (char *)malloc(numbytes + 1);
  MALLOC_CHECK(filebuffer);
  BZERO(filebuffer, (numbytes + 1));
  
  //This file is the TEMP file we've created
  FILE *fp = fopen(pathname, "r");

  //Get one character at a time from TEMP and put it in the filebuffer
  int c = getc(fp);
  for ( int x = 0; c != EOF; x++ ){
    filebuffer[x] = c;
    c = getc(fp);
  }
  //Add a NULL character to the end
  strcat(filebuffer, "\0");

  //filenum just stores the number of the output file with the page's info.
  //5 should have enough space, and then some
  char *filenum = malloc( 5 );
  MALLOC_CHECK(filenum);
  BZERO(filenum, 5);

  //filename has the address for the output file, so for example, if path is ./data and filenum is 1, filename is ./data/1
  char *filename = malloc( 100 + strlen(path) );
  MALLOC_CHECK(filename);
  BZERO(filename, (100 + strlen(path)));
  strcat(filename, path);
  sprintf(filenum, "%d", htmlnum);
  strcat(filename, filenum);

  //depthstring stores the number representing the current depth. Once again, 5 should be more than enough space
  char *depthstring = malloc( 5 );
  MALLOC_CHECK(depthstring);
  BZERO(depthstring, 5);
  sprintf(depthstring, "%d", depth);
  
  //This is the file that has the first line with the URL, the second with the depth, and from then on the page
  FILE *outfile = fopen(filename, "w");  
   
  //Put in the url, then a new line
  for (int a = 0; a < strlen(url); a++) {
    fputc(url[a], outfile);
  }
  fputc('\n', outfile);

  //Put in the depth, then a new line
  for (int b = 0; b < strlen(depthstring); b++) {
    fputc(depthstring[b], outfile);
  }
  fputc('\n', outfile);

  //Put in the page info
  for (int x = 0; x < strlen(filebuffer); x++) {
    fputc(filebuffer[x], outfile);
  }

  //Increment the global variable for the next page we get
  htmlnum += 1;


  //Make sure all files are closed and there's no memory free
  fclose(fp);
  fclose(outfile);

  free(urlbuffer);
  free(filenum);
  free(depthstring);
  free(filename);
  free(pathname);

  return filebuffer;
  
}

/*
  Method to get the URLs from a buffer containing the contents of a web page, storing them in url_list
  @param html_buffer - contains the contents of the page
  @param current - the current web page we're looking at
  @return url_list - an array of pointer to character arrays containing all the URLs
*/
char **extractURLs(char* html_buffer, char* current) {
  //Reset url_list at the beginning of the function by freeing all the char *'s within it and then BZERO'ing it
  for ( int x = 0; url_list[x] != NULL; x++ ) {
    free(url_list[x]);
  }

  BZERO( url_list, MAX_URL_PER_PAGE * sizeof( char *) );

  //numextracted keeps track of the number of URLs we take out of the page
  int numextracted = 0;

  //result is a temporary string that is passed to GetNextUrl to store the resulting url
  char result[MAX_URL_LENGTH];
  BZERO(result, MAX_URL_LENGTH);

  //Set currentpos and get the first url from the page
  int currentpos = 0;
  currentpos = GetNextURL(html_buffer, current, result, currentpos);

  //While we're within the page getting URLs, without exceeding the max number of URLs we can get
  while ( (numextracted < MAX_URL_PER_PAGE) && (currentpos >= 0) ) {
    //Conditions are: each URL has cs.dartmouth.edu, url_list doesn't already contain the URL, and the URL has been normalized
    if ( ( NULL != strcasestr(result, URL_PREFIX) ) && ( urlnotpresent(url_list, result, numextracted) == 0 ) && ( NormalizeURL(result) == 1 ) ) {
      char *resultcopy = (char *)malloc( MAX_URL_LENGTH );
      MALLOC_CHECK(resultcopy);
      BZERO(resultcopy, MAX_URL_LENGTH);
      //Copy result into a newly malloc'ed string that will be inserted into the url_list with this url
      strcpy(resultcopy, result);
      url_list[numextracted] = resultcopy;
      numextracted++;
      BZERO(result, MAX_URL_LENGTH); //Reset url
    }
    //If those conditions aren't met, go to the next url on the page and reset result
    else {
       BZERO(result, MAX_URL_LENGTH);
     }
    //Get the next url
    currentpos = GetNextURL(html_buffer, current, result, currentpos);
  }

  return url_list;
}

/*
  Helper method that's called within extractURLs to see if the url is already in url_list
  @param url_list - the array of urls
  @param url - the url to check
  @param numextracted - the number of urls already in url_list
  @return 1 if the url is in url_list, 0 otherwise
*/
int urlnotpresent(char **url_list, char *url, int numextracted) {
  for (int x = 0; x < numextracted; x++) {
    if ( strcmp(url_list[x], url) == 0 ) {
	return 1;
    }
  }
  return 0;
}

/*
  Method to add elements to the link list that have to be visited
  @param depth - the current search depth
*/
void updateListLinkToBeVisited(int depth) {
  //skipadd is used later when we want to skip a url to add to the dict
  int skipadd = 0;
  long hashnum = 0;

  //Loop through url_list
  for ( int x = 0; url_list[x] != NULL; x++ ) {
    //Mod with MAX_HASH_SLOT as to not exceed the index of url_list
    hashnum = ( hash1(url_list[x]) % MAX_HASH_SLOT );
    
    //If dict doesn't have the url, add it in
    if( dict->hash[hashnum] == NULL ) {
        //Allocate a new URLNODE and DNODE, and index them into the dict structure properly
        DNODE *url_dnode = dadd(url_list[x], depth);
	dict->hash[hashnum] = url_dnode;
	dict->end->next = url_dnode;
	dict->end = url_dnode;
	url_dnode->prev = dict->start->prev;
	url_dnode->next = dict->start;
	dict->start->prev = url_dnode;
    }

    //If it's already in the dict, do this
    else {
      //If this url isn't the same as the current url, index to the next one. The second condition only applies after there is only one entry, the seed_url, and prevents an infinite loop
      if ( ( strcmp((dict->hash[hashnum]->key), url_list[x]) != 0 ) && ( dict->hash[hashnum] != dict->hash[hashnum]->next ) ) {
	DNODE *current_dnode = dict->hash[hashnum]->next;

	//While we're at DNODE's that have this hash number, keep looking to see if the URL is there. If we do find it, set skipadd to true since we don't have to add it in.
	//If we eventually get past the entries with this hash number, then the url isn't there, so add it in
	while ( (hash1(current_dnode->key)%MAX_HASH_SLOT) == hashnum ) {
	  if ( strcmp(current_dnode->key, url_list[x]) == 0 ) {
	    skipadd = 1;
	    break;
	  }
	  current_dnode = current_dnode->next;
	}
	
	if ( skipadd == 0 ) {
	  DNODE *url_dnode = dadd(url_list[x], depth);

	  current_dnode->prev->next = url_dnode;
	  url_dnode->prev = current_dnode->prev;
	  url_dnode->next = current_dnode;
	  current_dnode->prev = url_dnode;
	}

	//Reset skipadd if we've just skipped 
	else {
	  skipadd = 0;
        }
      }
    }
  }
}


/*
  Method to set a URL as visited once it's been crawled
  @param url - url to be marked visited
*/
void setURLasVisited(char* url) {
  //Get the has number of the url and index to that spot in dict
  long hashnum = ( hash1(url) % MAX_HASH_SLOT );
  DNODE *current_dnode = dict->hash[hashnum];

  //Since other url's can have this hash number, keep going through until we get to this url's DNODE
  while (( strcmp(current_dnode->key, url) != 0 ) && ( hashnum == (hash1(current_dnode->key)%MAX_HASH_SLOT) )) {
    current_dnode = current_dnode->next;
  }
 
  //We should eventually get to the DNODE with the url, but check anyway. If we're there, set visited to one. Else, printf an error
  if ( strcmp(current_dnode->key,url) == 0 ) {
    ((URLNODE *)current_dnode->data)->visited = 1;
  }
  else {
    fprintf(stderr, "Error with URL, not found in dict");
  }
}

/*
  Method to get an address to visit
  @param depth - the current_search depth to be updated
  @return url - address to be visited
*/
char *getAddressFromTheLinksToBeVisited(int *depth) {
  DNODE *current_dnode = dict->start;

  //While the current DNODE has been visited already, go to the next one
  while ( ((URLNODE *)current_dnode->data)->visited == 1 ) {
    current_dnode = current_dnode->next;
    
    //This is triggered when everything in the dict is visited, to prevent an infinite loop. It just returns NULL since we've visited everything
    if ( current_dnode == dict->start ) {
      return NULL;
    }
  }

  //If the current dnode ends at the start, then everything is visited, so return NULL
  if ( current_dnode == dict->start ) {
    return NULL;
  }
  
  //Otherwise, return the url to be visited and update depth
  else {
    *depth = ((URLNODE *)current_dnode->data)->depth;
    return current_dnode->key;
  } 
}

/*
  Method to clean up all the data structures that have been malloc'ed throughout running the program
*/
void cleanup(char *path) {
  //First, clean up the dictionary
  cleanupdict(dict);


  //Free url_list. First loop through and free all the char * pointers in it. Then free the structure itself
  for ( int x = 0; url_list[x] != NULL; x++ ) {
    free(url_list[x]);
  }
  free(url_list);

  //Finally, remove the temp file. tempbuffer will store the call to remove the temp file
  char *tempbuffer = (char *)malloc(strlen(path) + 40);
  MALLOC_CHECK(tempbuffer);
  BZERO(tempbuffer, (strlen(path) + 40));
  strcat(tempbuffer, "rm ");
  strcat(tempbuffer, path);
  strcat(tempbuffer, "TEMP");
  system(tempbuffer);

  free(tempbuffer);
}
