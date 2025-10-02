
// TODO: figure out how to get those from javascript, because they are going to be different in WASM64
typedef int s32;
typedef unsigned int u32;
typedef unsigned char u8;
typedef float r32;
typedef int b32;
#define true 1
#define false 0

#define local_persist static
#define global static
#define internal static

#define WIDTH (1920/2)
#define HEIGHT (1080/2)
#define BYTES_PER_PIXEL 4

#define STB_SPRINTF_IMPLEMENTATION
#include "libs/stb_sprintf.h"

//- Global variables
extern u8 __heap_base;
u32 BumpPointer = (u32)&__heap_base;
u8 Buffer[WIDTH*HEIGHT*BYTES_PER_PIXEL];

//- Extern (js) functions
extern void LogMessage(u32 Length, char* message);

//- Memory
void* Malloc(u32 Size)
{
    u32 Result = BumpPointer;
    Result += Size;
    return (void *)Result;
}

void Free(u32 Size)
{
    BumpPointer -= Size;
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

void RenderGradient(s32 Width, s32 Height, s32 BytesPerPixel, 
                    r32 dtForFrame, b32 MouseDown, s32 MouseX, s32 MouseY)
{
    local_persist s32 Counter = 0;
    local_persist b32 Toggle = true;
    for(s32 Y = 0; Y < Height; Y++)
    {
        for(s32 X = 0; X < Width; X++)
        {
            r32 R = 0;
            r32 G = 0;
            r32 B = 0;
            
            if(Toggle)
            {
                R = 1.0f - ((r32)Counter/(r32)Width);
                G = 1.0f - ((r32)Counter/(r32)Width);
            }
            else
            {
                G = (r32)Counter/(r32)Width;
                R = (r32)Counter/(r32)Width;
            }
            
            // AA BB GG RR
            u32 Color = ((0xFF << 24) |
                         ((u32)(0xFF * B) << 16) |
                         ((u32)(0xFF * G) << 8)  |
                         ((u32)(0xFF * R) << 0)); 
            
            ((u32 *)Buffer)[Y*Width + X] = Color;
        }
    }
    
    u32 Color = 0;
    
    if(MouseDown)
    {
        Color = 0xFFFF0000;
    }
    
    u32 Pitch = BytesPerPixel * Width;
    
    s32 Size = 5;
    s32 MinX = MouseX - Size;
    s32 MaxX = MouseX + Size;
    s32 MinY = MouseY - Size;
    s32 MaxY = MouseY + Size;
    
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
    
    
    Counter += 1000 * dtForFrame * 0.5;
    if(Counter > Width)
    {
        Counter -= Width;
        Toggle = !Toggle;
    }
    
#if 1    
    Logf("(%d, %d) / %s", 
         MouseX, MouseY, ((MouseDown) ? "down" : "up"));
#endif
    
}