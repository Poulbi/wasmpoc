// NOTE(luca): Image buffer format is AA BB GG RR

#include "game.h"

#define WIDTH (1920/2)
#define HEIGHT (1080/2)
#define BYTES_PER_PIXEL 4

//~ Libraries
#define STB_SPRINTF_IMPLEMENTATION
#include "libs/stb_sprintf.h"
#include "libs/handmade_math.h"

//~ Global variables
// From WASM
extern u8 __heap_base;

// From Linker
extern u8 jetbrains_mono_regular_ttf[];

// Memory
psize BumpPointer = (psize)&__heap_base;
psize BumpAllocated = 0;

// Image
u8 GlobalImageBuffer[WIDTH*HEIGHT*BYTES_PER_PIXEL];

//~ Functions
//- Platform (js) 
#define external extern "C"
external void LogMessage(u32 Length, char *Message);
#define S_Len(String) (sizeof(String) - 1), (String)

external void JS_DrawText(u32 Length, char *Message, s32 X, s32 Y);

external r32 floor(r32 X);
external r32 ceil(r32 X);
external r32 sqrt(r32 X);
external r32 pow(r32 X, r32 Y);
external r32 fmod(r32 X, r32 Y);
external r32 cos(r32 X);
external r32 acos(r32 X);
external r32 fabs(r32 X);
external r32 round(r32 X);

//- Memory
void* Malloc(psize Size)
{
    psize Result = BumpPointer + BumpAllocated;
    BumpAllocated += Size;
    
    return (void *)Result;
}

void Free(void *Memory)
{
    
#if 0    
    u32 Size = (psize)Memory;
    BumpAllocated -= Size;
#endif
    
}

//- Game
external u32 GetBufferWidth() { return WIDTH; }
external u32 GetBufferHeight() { return HEIGHT; }
external u32 GetBytesPerPixel() { return BYTES_PER_PIXEL; }

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

external void 
UpdateAndRender(s32 Width, s32 Height, s32 BytesPerPixel, 
                r32 dtForFrame, b32 MouseDown, s32 MouseX, s32 MouseY)
{
    u8 *Buffer = GlobalImageBuffer;
    u32 Pitch = BytesPerPixel * Width;
    
#if 0    
    game_offscreen_buffer OffscreenBuffer = {};
    OffscreenBuffer.Pitch = Pitch;
    OffscreenBuffer.BytesPerPixel = BytesPerPixel;
    OffscreenBuffer.Width = Width;
    OffscreenBuffer.Height = Height;
    OffscreenBuffer.Memory = GlobalImageBuffer;
#endif
    
    local_persist b32 MemoryInitialized = false;
    if(!MemoryInitialized)
    {
#if 0
        InitFontFromBuffer(&GlobalFont, (char *)data_font_ttf);
#endif
        
        MemoryInitialized = true;
    }
    
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
        Color = 0xFF00FF;
    }
    
    v2 Min = {};
    
    s32 ColumnsCount = 6;
    
    s32 ColumnWidth = Width/ColumnsCount;
    
    b32 Toggle = true;
    for(s32 ColumnIndex = 0;
        ColumnIndex < ColumnsCount;
        ColumnIndex++)
    {
        s32 ColumnMinX = ColumnIndex*ColumnWidth;
        s32 ColumnMinY = 0;
        s32 ColumnMaxX = (1 + ColumnIndex)*ColumnWidth;
        s32 ColumnMaxY = Height;
        
        s32 ColumnWidth = ColumnMaxX - ColumnMinX;
        s32 ColumnHeight = ColumnMaxY - ColumnMinY;
        
        u32 Color = (Toggle) ? 0xFF333333 : 0xFF888888;
        
#if 1        
        RenderRectangle(Buffer, Pitch, Width, Height, BytesPerPixel,
                        ColumnMinX, ColumnMinY, ColumnMaxX, ColumnMaxY, Color);
#endif
        // Draw a button
        {        
            s32 Height = 0.10*ColumnHeight;
            s32 HMargin = 0.05*ColumnWidth;
            s32 MinX = ColumnMinX + HMargin;
            s32 MaxX = ColumnMaxX - HMargin;
            s32 MinY = ColumnMinY + 10;
            s32 MaxY = MinY + Height;
            
            b32 Selected = ((MouseX >= MinX && MouseX <= MaxX) &&
                            (MouseY >= MinY && MouseY <= MaxY));
            
            u32 Color = ((Selected) ? 
                         ((MouseDown) ? 0xFFBBBBBB : 0xFFCCCCCC) :
                         0xFFEEEEEE);
            
            if(Selected && MouseDown)
            {
                LogMessage(S_Len("Clicked"));
            }
            
            RenderRectangle(Buffer, Pitch, Width, Height, BytesPerPixel, 
                            MinX, MinY, MaxX, MaxY,
                            Color);
            
            
            
#if 0            
            JS_DrawText(S_Len("hello world world world"), MinX, MinY + 16);
#endif
            
            
        }
        
        Toggle = !Toggle;
    }
    
#if 0    
    // Draw mouse pointer
    {    
        s32 Size = 5;
        s32 MinX = MouseX - Size;
        s32 MaxX = MouseX + Size;
        s32 MinY = MouseY - Size;
        s32 MaxY = MouseY + Size;
        RenderRectangle(Buffer, Pitch, Width, Height, BytesPerPixel, 
                        MinX, MinY, MaxX, MaxY, Color);
    }
#endif
    
#if 1    
    Logf("(%d, %d) / %s", 
         MouseX, MouseY, ((MouseDown) ? "down" : "up"));
#endif
    
}
