#include "RoundBuffer.h"

struct Buffer CreateRoundBuffer()
{
   struct Buffer RoundBuffer;
   RoundBuffer.BufferHead = 0;
   RoundBuffer.BufferTail = 0;
   RoundBuffer.MaxSize = 1250000/sizeof(char);
   RoundBuffer.CurrSize = 0;
   RoundBuffer.Buffer = (char*)calloc(RoundBuffer.MaxSize, sizeof(char));
   if( !RoundBuffer.Buffer )
   {
	perror("Buffer allocation error. ");
	exit(-1);
   }

   return RoundBuffer;
}

int push( struct Buffer InputBuffer, char Input)
{
    if( InputBuffer.CurrSize == InputBuffer.MaxSize )
    {
	return 0;
    }
    
    if( isEmpty( InputBuffer ) )
	    InputBuffer.Buffer[ InputBuffer.BufferHead ] = Input;
    else
    {
        InputBuffer.BufferHead = ++InputBuffer.BufferHead % InputBuffer.MaxSize;
	InputBuffer.Buffer[InputBuffer.BufferHead] = Input; 
    }

    InputBuffer.CurrSize++;
    return 1;
}

char pop(struct Buffer InputBuffer)
{
    if( isEmpty( InputBuffer) )
	return '\0';

    char res = InputBuffer.Buffer[InputBuffer.BufferTail++ % InputBuffer.MaxSize ];
    InputBuffer.CurrSize--;
    return res;
}

char tail( struct Buffer InputBuffer )
{ 
    if( isEmpty( InputBuffer) )
	return '\0';
    
    return InputBuffer.Buffer[InputBuffer.BufferTail];
}

char head( struct Buffer InputBuffer )
{ 
    if( isEmpty( InputBuffer) )
	return '\0';
    
    return InputBuffer.Buffer[InputBuffer.BufferHead];
}

char at( struct Buffer InputBuffer, int position)
{
    if( position > InputBuffer.MaxSize )
	return '\0';

    return InputBuffer.Buffer[position];
}

int isEmpty( struct Buffer InputBuffer)
{
    if( InputBuffer.CurrSize == 0 )
	return 1;
    else 
	return 0;
}

