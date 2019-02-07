#include <stdlib.h>
#include <stdio.h>

struct BufferChar
{
    char* Buffer;
    int BufferHead;
    int BufferTail;
    int MaxSize;
    int CurrSize;
};

void CreateRoundBufferChar(int Size, struct BufferChar* RoundBuffer);
int pushChar( struct BufferChar*, char Input);
char popChar( struct BufferChar*);
char tailChar( struct BufferChar);
char headChar( struct BufferChar);
char atChar( struct BufferChar, int position);
int isEmptyChar( struct BufferChar InputBuffer);

//----------- Buffer Int
struct BufferInt
{
    int* Buffer;
    int BufferHead;
    int BufferTail;
    int MaxSize;
    int CurrSize;
};

void CreateRoundBufferInt(int Size, struct BufferInt* ToSend);
int pushInt( struct BufferInt*, int Input);
int popInt( struct BufferInt*);
int tailInt( struct BufferInt);
int headInt( struct BufferInt);
int atInt( struct BufferInt, int position);
int isEmptyInt( struct BufferInt InputBuffer);
