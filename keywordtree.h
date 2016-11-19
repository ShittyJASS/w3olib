#ifndef _KEYWORDTREE_H_
#define _KEYWORDTREE_H_

#define KT_ACCURATE
#define KT_LOG

#include <stdlib.h>
#include <string.h>

#define KT_BOOL unsigned char

typedef struct KeywordTree
{
	struct KeywordTree *next,*child;
	void *ptr;
	char *str;
	size_t strL;
} KeywordTree;

KeywordTree *kt_new_root();

void kt_destroy(KeywordTree *kt);

void kt_flush(KeywordTree *kt);

KT_BOOL kt_has(register KeywordTree *kt,register char *key);

void* kt_get(register KeywordTree *kt,register char *key);

KT_BOOL kt_push(KeywordTree *kt,char *key,void *ptr);

#endif // #ifndef _KEYWORDTREE_H_
