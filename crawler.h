#ifndef _CRAWLER_H_
#define _CRAWLER_H_

// *****************Impementation Spec********************************
// File: crawler.c
// This file contains useful information for implementing the crawler:
// - DEFINES
// - MACROS
// - DATA STRUCTURES
// - PROTOTYPES

// DEFINES

// define how long in seconds we should wait between webpage fetches
#define INTERVAL_PER_FETCH 1

// The URLs we crawl should all start with this prefix. 
// You could remove this limitation but not before testing!!!
// The danger is a site may block you and or all of us!

#define URL_PREFIX "www.sjsu.edu"


// Unlikely to have more than an 1000 URLs in page

#define MAX_URL_PER_PAGE 1000

// DATA STRUCTURES.

// This is the table that keeps pointers to a list of URL extracted
// from the current HTML page. 
//NULL pointer represents the end of the list of URLs.

char **url_list;

// getPage: Assumption: if you are dowloading the file it must be unique. 
// Two cases. First its the SEED URL so its unique. Second, wget only gets a page 
// once a URL is determined to be unique. Get the HTML file saved in TEMP 
// and read it into a string that is returned by getPage. Store TEMP
// to a file 1..N after writing the URL and depth on the first and second 
// lines respectively.

char *getPage(char* url, int depth,  char* path);

// extractURL: Given a string of the HTML page, parse it (you have the code 
// for this GetNextURL) and store all the URLs in the url_list (defined above).
// NULL pointer represents end of list (no more URLs). Return the url_list

char **extractURLs(char* html_buffer, char* current);

// setURLasVisited: Mark the URL as visited in the URLNODE structure.

void setURLasVisited(char* url);

// updateListLinkToBeVisited:
// the url_list and for each URL in the list it first determines if it is unique.
// If it is then it creates a DNODE (using malloc) and URLNODE (using malloc).
// It copies the URL to the URLNODE and initialises it and then links it into the
// DNODE (and initialise it). Then it links the DNODE into the linked list dict.
// at the point in the list where its key cluster is (assuming that there are
// elements hashed at the same slot and the URL was found to be unique. It does
// this for *every* URL in the url_list

void updateListLinkToBeVisited(int depth);

// getAddressFromTheLinksToBeVisited: Scan down thie hash table (part of dict) and
// find the first URL that has not already been visited and return the pointer 
// to that URL. Note, that the pointer to the depth is also passed. Update the
// depth using the depth of the URLNODE that is next to be visited. 

char *getAddressFromTheLinksToBeVisited(int *depth);

#endif

