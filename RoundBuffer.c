#include "RoundBuffer.h"

void CreateRoundBufferChar( int Size, struct BufferChar* RoundBuffer )
{
   RoundBuffer->BufferHead = 0;
   RoundBuffer->BufferTail = 0;
   RoundBuffer->MaxSize = Size;
   RoundBuffer->CurrSize = 0;
   RoundBuffer->Buffer = (char*)calloc(Size, sizeof(char));
   if( !RoundBuffer->Buffer )
   {
	perror("Buffer allocation error. ");
	exit(-1);
   }
}

int pushChar( struct BufferChar* InputBuffer, char Input)
{
    if( InputBuffer->CurrSize == InputBuffer->MaxSize )
    {
	return 0;
    }
    
    if( isEmptyChar( *InputBuffer ) )
    {
	InputBuffer->Buffer[ InputBuffer->BufferHead ] = Input;
    }
    else
    {
        InputBuffer->BufferHead++; 
	InputBuffer->BufferHead = InputBuffer->BufferHead % InputBuffer->MaxSize;
	InputBuffer->Buffer[InputBuffer->BufferHead] = Input; 
    }

    InputBuffer->CurrSize++;
    return 1;
}

char popChar(struct BufferChar* InputBuffer)
{
    if( isEmptyChar( *InputBuffer) )
	return '\0';

    char res = InputBuffer->Buffer[InputBuffer->BufferTail++ % InputBuffer->MaxSize ];
    InputBuffer->CurrSize--;
    return res;
}

char tailChar( struct BufferChar InputBuffer )
{ 
    if( isEmptyChar( InputBuffer) )
	return '\0';
    
    return InputBuffer.Buffer[InputBuffer.BufferTail];
}

char headChar( struct BufferChar InputBuffer )
{ 
    if( isEmptyChar( InputBuffer) )
	return '\0';
    
    return InputBuffer.Buffer[InputBuffer.BufferHead];
}

char atChar( struct BufferChar InputBuffer, int position)
{
    if( position > InputBuffer.MaxSize )
	return '\0';

    return InputBuffer.Buffer[position];
}

int isEmptyChar( struct BufferChar InputBuffer)
{
    if( InputBuffer.CurrSize == 0 )
	return 1;
    else 
	return 0;
}

//-----------Round Buffer INT  
void CreateRoundBufferInt( int Size, struct BufferInt* RoundBuffer )
{
   RoundBuffer->BufferHead = 0;
   RoundBuffer->BufferTail = 0;
   RoundBuffer->MaxSize = Size;
   RoundBuffer->CurrSize = 0;
   RoundBuffer->Buffer = (int*)calloc(Size, sizeof(int));
   if( !RoundBuffer->Buffer )
   {
	perror("Buffer allocation error. ");
	exit(-1);
   }

}

int pushInt( struct BufferInt* InputBuffer, int Input)
{
    if( InputBuffer->CurrSize == InputBuffer->MaxSize )
    {
	return 0;
    }
    
    if( isEmptyInt( *InputBuffer ) )
	    InputBuffer->Buffer[ InputBuffer->BufferHead ] = Input;
    else
    {
        InputBuffer->BufferHead = (InputBuffer->BufferHead + 1) % InputBuffer->MaxSize;
	InputBuffer->Buffer[InputBuffer->BufferHead] = Input; 
    }

    InputBuffer->CurrSize++;
    return 1;
}

int popInt(struct BufferInt* InputBuffer)
{
    if( isEmptyInt( *InputBuffer) )
	return 0;

    int res = InputBuffer->Buffer[InputBuffer->BufferTail++ % InputBuffer->MaxSize ];
    InputBuffer->CurrSize--;
    return res;
}

int tailInt( struct BufferInt InputBuffer )
{ 
    if( isEmptyInt( InputBuffer) )
	return '\0';
    
    return InputBuffer.Buffer[InputBuffer.BufferTail];
}

int headInt( struct BufferInt InputBuffer )
{ 
    if( isEmptyInt( InputBuffer) )
	return '\0';
    
    return InputBuffer.Buffer[InputBuffer.BufferHead];
}

int atInt( struct BufferInt InputBuffer, int position)
{
    if( position > InputBuffer.MaxSize )
	return '\0';

    return InputBuffer.Buffer[position];
}

int isEmptyInt( struct BufferInt InputBuffer)
{
    if( InputBuffer.CurrSize == 0 )
	return 1;
    else 
	return 0;
}

