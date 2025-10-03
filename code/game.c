// NOTE(luca): Image buffer format is AA BB GG RR

//~ Types
// TODO: figure out how to get those from javascript, because they are going to be different in WASM64
typedef signed char    s8;
typedef signed short   s16;
typedef signed int     s32;
typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;
typedef float          r32;
typedef int            b32;
#define true           1
#define false          0

#define local_persist static
#define global static
#define internal static

#define WIDTH (1920/2)
#define HEIGHT (1080/2)
#define BYTES_PER_PIXEL 4

//~ Libraries
#define STB_SPRINTF_IMPLEMENTATION
#include "libs/stb_sprintf.h"

//~ Global variables
// From WASM
extern u8 __heap_base;

// Memory
u32 BumpPointer = (u32)&__heap_base;
u32 BumpAllocated = 0;

// Image
u8 Buffer[WIDTH*HEIGHT*BYTES_PER_PIXEL];

//~ Functions
//- Platform (js) 
extern void LogMessage(u32 Length, char* message);

//- Memory
void* Malloc(u32 Size)
{
    u32 Result = BumpPointer + BumpAllocated;
    BumpAllocated += Size;
    
    return (void *)Result;
}

void Free(u32 Size)
{
    BumpAllocated -= Size;
}

//- Game
u32 GetBufferWidth() { return WIDTH; }
u32 GetBufferHeight() { return HEIGHT; }
u32 GetBytesPerPixel() { return BYTES_PER_PIXEL; }

void Logf(char *Format, ...)
{
    char MessageBuffer[256] = {0};
    
    va_list Args;
    va_start(Args, Format);
    u32 MessageLength = stbsp_vsprintf(MessageBuffer, Format, Args);
    
    LogMessage(MessageLength, MessageBuffer);
}

void 
RenderRectangle(u8 *Buffer, s32 Pitch, s32 Width, s32 Height, s32 BytesPerPixel,
                s32 MinX, s32 MinY, s32 MaxX, s32 MaxY, u32 Color)
{
    if(MinX < 0)
    {
        MinX = 0;
    }
    
    if(MaxX > Width)
    {
        MaxX = Width;
    }
    
    if(MinY < 0)
    {
        MinY = 0;
    }
    
    if(MaxY > Height)
    {
        MaxY = Height;
    }
    
    u8 *Row = ((u8 *)Buffer) + ((BytesPerPixel*MinX) + (Pitch*MinY));
    
    for(s32 Y = MinY;
        Y < MaxY;
        Y++)
    {
        u32 *Pixel = (u32 *)Row;
        for(s32 X = MinX;
            X < MaxX;
            X++)
        {
            *Pixel++ = Color;
        }
        Row += Pitch;
    }
}

void 
UpdateAndRender(s32 Width, s32 Height, s32 BytesPerPixel, 
                r32 dtForFrame, b32 MouseDown, s32 MouseX, s32 MouseY)
{
#if 1
    // Clear the buffer to black.
    {    
        u32 *Clear = (u32 *)Buffer;
        s32 ClearSize = Width*Height;
        while(ClearSize--) *Clear++ = 0xFF000000;
    }
#endif
    
    u32 Color = 0;
    if(MouseDown)
    {
        Color = 0xFF00FF00;
    }
    
    u32 Pitch = BytesPerPixel * Width;
    
    s32 Size = 5;
    s32 MinX = MouseX - Size;
    s32 MaxX = MouseX + Size;
    s32 MinY = MouseY - Size;
    s32 MaxY = MouseY + Size;
    
    RenderRectangle(Buffer, Pitch, Width, Height, BytesPerPixel, 
                    MinX, MinY, MaxX, MaxY, Color);
    
#if 1    
    Logf("(%d, %d) / %s", 
         MouseX, MouseY, ((MouseDown) ? "down" : "up"));
#endif
    
}