#include "keywordtree.h"

KeywordTree *kt_new(char *key,void *ptr)
{
	KeywordTree *kt = malloc(sizeof(KeywordTree));
	kt->next		= kt->child = NULL;
	kt->str 		= key;
	kt->strL 		= strlen(key);
	kt->ptr 		= ptr;
	return kt;
}

static inline char* kt_clone_key(const char *source)
{
	int l 			= strlen(source);
	char *out 		= malloc(sizeof(char)*(l+1));
	memcpy(out,source,l+1);
	return out;
}

KeywordTree *kt_new_root()
{
	return kt_new(kt_clone_key(""),NULL);
}

void kt_destroy(KeywordTree *kt)
{
	if(kt->child!=NULL) kt_destroy(kt->child);
	if(kt->next!=NULL)
	{
		free(kt->next->str);
		kt_destroy(kt->next);
	}
	free(kt);
}

void kt_flush(KeywordTree *kt)
{
	if(kt->child!=NULL)
	{
		kt_destroy(kt->child);
		kt->child	= NULL;
	}
	if(kt->next!=NULL)
	{
		free(kt->next->str);
		kt_destroy(kt->next);
		kt->next	= NULL;
	}
}

KT_BOOL kt_has(register KeywordTree *kt,register char *key)
{;
	kt_has_ite:
	
	if ( kt->str[0] != key[0] )
	{
		if ((kt = kt->next)!=NULL) goto kt_has_ite;
		return 0;
	}
	
	else if ( kt->child!=NULL )
	{
		kt=kt->child;
		++key;
		goto kt_has_ite;
	}
	
	return (
			kt->strL==strlen(key)
		#ifdef KT_ACCURATE
		&&	!memcmp(kt->str,key,kt->strL)
		#endif
		);
}

void* kt_get(register KeywordTree *kt,register char *key)
{;
	kt_get_ite:
	
	if ( kt->str[0] != key[0] )
	{
		if ((kt = kt->next)!=NULL) goto kt_get_ite;
		return 0;
	}
	
	else if ( kt->child!=NULL )
	{
		kt=kt->child;
		++key;
		goto kt_get_ite;
	}
	
	if (
			kt->strL==strlen(key)
		#ifdef KT_ACCURATE
		&&	!memcmp(kt->str,key,kt->strL)
		#endif
		)
	{
		return kt->ptr;
	}
	else return NULL;
}

#ifdef KT_LOG
	#include <stdio.h>
	char *kt_key=NULL;
#endif

KT_BOOL kt_push(KeywordTree *kt,char *key,void *ptr)
{
	#ifdef KT_LOG
		if(kt_key==NULL) kt_key=key;
	#endif
	
	if		( key[0]==kt->str[0] )
	{
		if		( kt->child!=NULL )	return kt_push(kt->child,&key[1],ptr);
		else if	( strcmp(kt->str,key) )
		{
			kt->child = kt_new( &(kt->str)[1],kt->ptr );
			return kt_push(kt->child,&key[1],ptr);
		}
		else return 0;
	}
	
	else if	( kt->next==NULL )
	{
		kt->next = kt_new(kt_clone_key(key),ptr);
		#ifdef KT_LOG
			printf("KT\tpushed %p (\"%s\")\n",ptr,kt_key);
			kt_key=NULL;
		#endif
		return 1;
	}
	
	else return kt_push(kt->next,key,ptr);
}
