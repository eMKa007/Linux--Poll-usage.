#include <stdlib.h>
#include <stdio.h>

struct Buffer
{
    char* Buffer;
    int BufferHead;
    int BufferTail;
    int MaxSize;
    int CurrSize;
};

struct Buffer CreateRoundBuffer();
int push( struct Buffer, char Input);
char pop( struct Buffer);
char tail( struct Buffer);
char head( struct Buffer);
char at( struct Buffer, int position);
int isEmpty( struct Buffer InputBuffer);

