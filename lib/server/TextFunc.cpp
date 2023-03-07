#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <malloc/malloc.h>
#include <string.h>
#include "Text.h"

const void* ERR_PTR = (const void*)0x78;
const char ERR_BYTE = 0x87;

#define CALLOC( num, type )  ( (type*) calloc ((num), sizeof (type)) )
#define   FREE( ptr )        { memset ( (ptr), ERR_BYTE, malloc_size( ptr ) ); free (ptr); *(const void**) &(ptr) = ERR_PTR; }


void TxtDestroy (TEXT* txt) 
{
    FREE (txt -> buffer -> buf);
    FREE (txt -> buffer)
    FREE (txt -> str_array);
}

void TxtCreate (TEXT* txt, const char* file_name, const char* mode)
{
    FILE* file = fopen (file_name, mode);
    
    extened_buf* e_buf = ReadToBuffer (file);

    str* struct_array = CALLOC (e_buf -> size, str);
    assert (struct_array != NULL);

    size_t num_of_str = DevideToStrings (e_buf -> buf, struct_array);

    struct_array = (str*)realloc (struct_array, sizeof(struct_array[0]) * num_of_str);

    txt -> str_array  = struct_array;
    txt -> buffer     = e_buf;
    txt -> num_of_str = num_of_str;
    
    fclose (file);
}

extened_buf* ReadToBuffer (FILE* file)
{   
    assert (file != NULL);

    extened_buf* e_buf = CALLOC (1, extened_buf);

    e_buf -> size = GetFileSize (file);
    
    char* buffer = CALLOC (e_buf -> size + 1, char);   
    assert (buffer != NULL);

    fread (buffer, sizeof(char), e_buf -> size, file);
    e_buf -> buf = buffer;

    return e_buf;
}

size_t GetFileSize (FILE* file) 
{
    assert (file != NULL);
    int size = 0;
    int current_pos = ftell (file);

    fseek (file, 0, SEEK_END);
    size = ftell (file);
    fseek (file, 0, current_pos);

    return size;
}

size_t DevideToStrings (char* buf, str* struct_array) 
{
    assert (buf != NULL);
    assert (struct_array != NULL);

    int elem_num = 0;
    struct_array[0].string = buf;

    for (int i = 0; buf[i] != '\0'; i++)
    {
        if (buf [i] == '\n' || buf[i] == '\r') 
        {
            buf [i] = '\0';

            elem_num ++;
            struct_array[elem_num].string = buf + i + 1;
            struct_array[elem_num - 1].len = struct_array[elem_num].string - struct_array[elem_num - 1].string - 1;
        }
    }
    return elem_num;
}