#include "w3olib.h"

void* MALLOC (size_t s)
{
	printf("MALLOC\t%u\n",s);
	return malloc(s);
}

W3O_File *W3O_new_file ()
{
	W3O_File *f	= malloc(sizeof(W3O_File));
	memset(f,0,sizeof(W3O_File)-sizeof(KeywordTree*));
	f->tree		= kt_new_root();
	return f;
}

static inline unsigned char W3O_rindex (unsigned char in)
{
	return (in<'A') ? in-'0' : in-'A'+10 ;
}

static inline unsigned char W3O_indexr (unsigned char in)
{
	return (in<10) ? in+'0' : in+'A'-10 ;
}

static inline void W3O_itor (uint32_t in, unsigned char *dest)
{
	dest[2]	= W3O_indexr(	in 			% 36 );
	dest[1] = W3O_indexr(	(in/=36)	% 36 );
	dest[0] = W3O_indexr(	in/36 );
}

W3O_Object *W3O_file_get_object (const W3O_File *f,const char *id)
{
	return kt_get(f->tree,W3O_ID2S(id));
}

W3O_Result W3O_objects_append_objects (W3O_Objects *dest,W3O_Objects *src,W3O_File *parent)
{
	const W3O_BOOL custom = parent->objects!=dest;
	
	W3O_INT base		= src->objectCount;
	dest->objectCount  += src->objectCount;
	
	if (!dest->objectCount) return W3O_RESULT_GOOD;
	
	//printf("prev: %p\n",(void*)dest->object);
	
	//printf("cmd: realloc(%p,%u)\n",(void*)dest->object,sizeof(W3O_Object)*dest->objectCount);
	dest->object		= realloc(dest->object,sizeof(W3O_Object)*dest->objectCount);
	
	//printf("result: %p\n",(void*)dest->object);
	
	//printf("%u\n",dest->objectCount);
	//getchar();
	if (dest->object==NULL) return W3O_RESULT_NO_MEMORY;
	
	for (W3O_INT i=0; i<src->objectCount; ++i)
	{
		char key[5];
		key[4]			= '\0';
		memcpy(key,&src->object[i].srcId[custom?4:0],4);
		if(custom)
		{
			return W3O_RESULT_GOOD;
			key[1]			= '\0';
			uint32_t *m		= kt_get(parent->tree,key);
			if (m==NULL)
			{
				m			= malloc(4);
				kt_push(parent->tree,key,m);
				*m			= 0;
			}
			
			uint32_t n		= *m;
			while (W3O_itor(n,(unsigned char*) &key[1]),kt_has(parent->tree,key)) ++n;
			*m				= n+1;
			
			memcpy(&dest->object[base+i],&src->object[i],sizeof(W3O_Object));
			W3O_itor(*m++,(unsigned char*) &dest->object[base+i].newId[1]);
			
			memcpy(&dest->object[base+i].newId[1],&key[1],3);
			kt_push(parent->tree,key,&dest->object[base+i]);
		}
		else
		{
			W3O_Object *o	= kt_get(parent->tree,key);
			if(o==NULL)
			{
				return W3O_RESULT_GOOD;
				dest->object[base+i]	= src->object[i];
				kt_push(parent->tree,key,&dest->object[base+i]);
			}
			else
			{
				printf("%p: %s->%s\n",o,o->srcId,o->newId);
				printf("%p: %s->%s\n",&src->object[i],src->object[i].srcId,src->object[i].newId);
				*o						= src->object[i];
				--dest->objectCount;
				--base;
			}
		}
	}
	
	return W3O_RESULT_GOOD;
}

W3O_Result W3O_file_append_file (W3O_File *dest,W3O_File *src)
{
	if (dest->fileType != src->fileType) return W3O_RESULT_BAD_INPUT;
	
	for(int i=0; i<2; ++i)
	{
		W3O_Result result	= W3O_objects_append_objects(&dest->objects[i],&src->objects[i],dest);
		if (result!=W3O_RESULT_GOOD) return result;
	}
	
	return W3O_RESULT_GOOD;
}

void W3O_destroy_file (W3O_File *f)
{
	#ifdef W3O_KEYWORDTREE
	kt_destroy(f->tree);
	#endif
	free(f);
}

W3O_Object* W3O_objects_get_object (W3O_Objects *src,W3O_INT i)
{
	return &src->object[i];
}

W3O_Value* W3O_object_get_value (W3O_Object *o,W3O_INT i)
{
	return (W3O_Value*) &o->value[sizeof(W3O_Value)*i];
}

W3O_ValueExtended* W3O_object_get_value_ex (W3O_Object *o,W3O_INT i)
{
	return (W3O_ValueExtended*) &o->value[sizeof(W3O_Value)*i];
}

#define W3O_WRITE_FILE_ALLOCATE_BUFFER()				\
	while(buffer-32<*len) 								\
	{ 													\
		size_t probeIndex 	= probe-*dest;				\
		*dest				= realloc(*dest,buffer*=2);	\
		if(*dest==NULL) return W3O_RESULT_NO_MEMORY;	\
		probe				= *dest+probeIndex;			\
	}

W3O_Result W3O_write_from_file (const W3O_File *src,char **dest,size_t *len)
{
	size_t buffer		= 1024;
	*dest				= malloc(1024);
	char *probe			= *dest;
	
	if(probe==NULL) return W3O_RESULT_NO_MEMORY;
	
	const size_t valueExts	= (src->fileType<W3O_FILE_W3D) ? 0 : sizeof(W3O_ValueExtended)-sizeof(W3O_Value);
	const size_t valueSize	= sizeof(W3O_Value) + valueExts;
	
	memcpy(probe,(void*)src,4);
	probe			   += 4;
	*len				= 4;
	
	for (unsigned char i=0; i<2; ++i)
	{
		memcpy(probe,&src->objects[i].objectCount,4);
		probe			   += 4;
		*len			   += 4;
		
		W3O_Object *object	= src->objects[i].object;
		
		for (W3O_INT j=0; j<src->objects[i].objectCount; ++j)
		{
			memcpy(probe,&object[j],12);
			probe			   += 12;
			*len			   += 12;
			
			for (W3O_INT k=0; k<object[j].valueCount; ++k)
			{
				W3O_Value *value		= (W3O_Value*) &object[j].value[valueSize*k];
				
				if (value->valueType==W3O_VALUE_STRING)
				{
					memcpy(probe,value,8+valueExts);
					probe			   += 8+valueExts;
					*len			   += 8+valueExts;
					
					size_t stringLen	= strlen(value->pointer[valueExts/4])+1;
					*len			   += stringLen+4;
					W3O_WRITE_FILE_ALLOCATE_BUFFER()
					
					memcpy(probe,value->pointer[valueExts/4],stringLen);
					probe			   += stringLen;
					
					memcpy(probe,&value->footer,4);
					probe			   += 4;
				}
				else
				{
					*len			   += valueSize;
					W3O_WRITE_FILE_ALLOCATE_BUFFER()
					memcpy(probe,value,valueSize);
					probe			   += valueSize;
				}
			}
		}
	}
	
	*dest				= realloc(*dest,*len);
	if(*dest==NULL) return W3O_RESULT_NO_MEMORY;
	
	return W3O_RESULT_GOOD;
}

W3O_Result W3O_read_to_file (char *src,size_t len,const W3O_FileType fileType,W3O_File *dest)
{
	if (len<12) return W3O_RESULT_BAD_INPUT;
	
	const size_t valueExts	= (fileType<W3O_FILE_W3D) ? 0 : 8;
	const size_t valueSize	= sizeof(W3O_Value) + valueExts;
	
	dest->fileVersion	= *(W3O_INT*)src;
	src 			   += 4;
	len				   -= 4;
	
	kt_flush(dest->tree);
	dest->fileType 		= fileType;
	
	for (unsigned char i=0; i<2; ++i)
	{
		dest->objects[i].objectCount	= *(W3O_INT*)src;
		dest->objects[i].object 		= malloc(*(W3O_INT*)src * sizeof(W3O_Object));
		W3O_Object *object 				= dest->objects[i].object;
		src				   += 4;
		len				   -= 4;
		
		for (W3O_INT j=0; j<dest->objects[i].objectCount; ++j)
		{
			if (len<12) return W3O_RESULT_BAD_INPUT;
			
			char key[5];
			key[4]				= '\0';
			memcpy(key,&src[i?4:0],4);
			if (!kt_push(dest->tree,key,&object[j])) return W3O_RESULT_BAD_INPUT;
			
			if (i)
			{
				uint32_t n		= W3O_rindex(key[1])*1296 + W3O_rindex(key[2])*36 + W3O_rindex(key[3]);
				key[1]			= '\0';
				uint32_t *m 	= kt_get(dest->tree,key);
				if (m==NULL)
				{
					m			= malloc(4);
					*m			= 0;
					kt_push(dest->tree,key,m);
				}
				
				if (n==*m)
				{
					do ++n; while (W3O_itor(n,(unsigned char*) &key[1]),kt_has(dest->tree,key));
					*m			= n;
				}
			}
			
			memcpy(&object[j],src,12);
			src				   += 12;
			len				   -= 12;
			
			object[j].value		= malloc(valueSize*object[j].valueCount);
			
			printf("%s->%s (%d)\n",object[j].srcId,object[j].newId,object[j].valueCount);
			
			for (W3O_INT k=0; k<object[j].valueCount; ++k)
			{
				if (len<valueSize-3) return W3O_RESULT_BAD_INPUT; // --
				
				W3O_Value *value		= (W3O_Value*) &object[j].value[valueSize*k];
				
				memcpy(value,src,8+valueExts);
				src					   += 8+valueExts;
				len					   -= 8+valueExts;
				
				if (value->valueType==W3O_VALUE_STRING)
				{
					char *stringEnd		= memchr(src,'\0',len);
					
					if(stringEnd==NULL) return W3O_RESULT_BAD_INPUT;
					
					size_t stringLen	= stringEnd-src+1;
					
					value->pointer[valueExts/4] = malloc(stringLen);
					memcpy(value->pointer[valueExts/4],src,stringLen);
					src				   += stringLen;
					
					memcpy(&value->pointer[valueExts/4+1],src,4);
					src				   += 4;
					
					len				   -= 4+stringLen;
					
					printf("\t\"%s\"\t%s\n",W3O_ID2S(value->valueId),(char*)value->pointer[valueExts/4]);
				}
				else
				{
					memcpy(&value->pointer[valueExts/4],src,8);
					src 			   += 8;
					len 			   -= 8;
					
					printf("\t\"%s\"\t%p\n",W3O_ID2S(value->valueId),value->pointer[valueExts/4]);
				}
			}
			
			printf("\n");
		}
	}
	
	return (len==0) ? W3O_RESULT_GOOD : W3O_RESULT_NOT_EOF;
}
