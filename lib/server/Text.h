#ifndef TEXT_H
#define TEXT_H

#include <stdio.h>

typedef struct _buf
{
    char* buf;
    size_t size;
} extened_buf;

typedef struct _str
{
    char* string;
    size_t len;
} str;

typedef struct _txt
{
    extened_buf* buffer;
    str* str_array;
    size_t num_of_str;
    FILE* file;
} TEXT;

size_t DevideToStrings (char* buf, str* struct_array); 

size_t GetFileSize (FILE* file);

extened_buf* ReadToBuffer (FILE* file);

void TxtCreate  (TEXT* txt, const char* file_name, const char* mode);

void TxtDestroy (TEXT* txt);

#endif