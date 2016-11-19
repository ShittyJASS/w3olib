#ifndef _W3OLIB_H_
#define _W3OLIB_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "keywordtree.h"

#define W3O_BOOL	unsigned char
#define W3O_INT		int32_t
#define W3O_REAL	float
#define W3O_UNREAL	float
#define W3O_STRING	char*	// 4 bytes
#define W3O_BYTE 	(sizeof(char))

#define W3O_ID2S(id) \
	(char[5]){id[0],id[1],id[2],id[3],'\0'}

typedef enum
{
	W3O_RESULT_GOOD 		= 0,
	W3O_RESULT_NOT_EOF		= 1,
	W3O_RESULT_BAD_INPUT	= 2,
	W3O_RESULT_NO_MEMORY	= 3,
	W3O_RESULT_SYSTEM_FAIL	= 4
} W3O_Result;

typedef enum
{
	W3O_FILE_W3U		= 0,	// units
	W3O_FILE_W3T		= 1,	// items
	W3O_FILE_W3B		= 2,	// destructables
	W3O_FILE_W3H		= 3,	// buffs
	W3O_FILE_W3D		= 4,	// doodads
	W3O_FILE_W3A		= 5,	// abilities
	W3O_FILE_W3Q		= 6		// upgrades
} W3O_FileType;

typedef enum
{
	W3O_VALUE_INTEGER = 0,
	W3O_VALUE_REAL = 1,
	W3O_VALUE_UNREAL = 2,
	W3O_VALUE_STRING = 3
} W3O_ValueType;

typedef union
{
	W3O_INT 	integer;
	W3O_REAL 	real;
	char		*string;
} W3O_Data;

typedef struct
{
	char valueId[4];
	W3O_ValueType valueType;
	union
	{
		W3O_Data;
		void* pointer[1];
	};
	W3O_INT footer;
} W3O_Value;

typedef struct
{
	char valueId[4];
	W3O_ValueType valueType;
	W3O_INT dataLevel,dataLetter;	// doodads, abilities, upgrades
	W3O_Data;
	W3O_INT footer;
} W3O_ValueExtended;

typedef struct
{
	char srcId[4],newId[4];
	W3O_INT valueCount;
	char *value;
} W3O_Object;

typedef struct
{
	W3O_INT objectCount;
	W3O_Object *object;
} W3O_Objects;

typedef struct
{
	W3O_INT fileVersion;
	W3O_FileType fileType;
	union
	{
		struct
		{
			W3O_Objects originals,customs;
		};
		W3O_Objects objects[2];
	};
	KeywordTree *tree;
} W3O_File;

W3O_File* W3O_new_file ();
W3O_Result W3O_read_to_file (char *src,size_t len,const W3O_FileType fileType,W3O_File *dest);
W3O_Result W3O_write_from_file (const W3O_File *src,char **dest,size_t *len);
W3O_Object* W3O_file_get_object (const W3O_File *f,const char *id);
W3O_Result W3O_file_append_file (W3O_File *dest,W3O_File *src);
void W3O_destroy_file (W3O_File *f);

W3O_Result W3O_objects_append_objects (W3O_Objects *dest,W3O_Objects *src,W3O_File *parent);
W3O_Object* W3O_objects_get_object (W3O_Objects *src,W3O_INT i);

W3O_Value* W3O_object_get_value (W3O_Object *o,W3O_INT i);
W3O_ValueExtended* W3O_object_get_value_ex (W3O_Object *o,W3O_INT i);

#endif //ifndef _W3OLIB_H_
