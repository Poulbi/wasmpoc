/* date = October 4th 2025 11:46 am */

#ifndef GAME_H
#define GAME_H

//- Macro's 
#define Assert(Expression) do { if(!(Expression)) { *(psize *)((psize)-1) = 0; } } while(0)

//~ Types
// TODO: figure out how to get those from javascript, because they are going to be different in WASM64
#include <stddef.h>
#include <stdint.h>
typedef int8_t   s8;
typedef int16_t  s16;
typedef int32_t  s32;
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef float    r32;
typedef s32      b32;
typedef u32      rune;
typedef size_t   psize;
#define true     1
#define false    0

#define local_persist static
#define global static
#define internal static

struct string
{
    psize Count;
    u8 *Data;
};

struct memory_arena
{
    psize Used;
    psize Size;
    void *Base;
};
void
InitializeArena(memory_arena *Arena, psize Size, void *Base)
{
    Arena->Size = Size;
    Arena->Base = (u8 *)Base;
    Arena->Used = 0;
}

#define PushStruct(Arena, type) ((type *)PushSize((Arena), (sizeof(type))))
#define PushArray(Arena, Count, type) (type *)PushSize((Arena), (sizeof(type))*(Count)) 
void *
PushSize(memory_arena *Arena, psize Size)
{
    Assert((Arena->Used + Size) < Arena->Size);
    
    void *Result = ((u8 *)Arena->Base + Arena->Used);
    Arena->Used += Size;
    
    return Result;
}

struct game_offscreen_buffer
{
    u8 *Memory;
    s32 Width;
    s32 Height;
    s32 Pitch;
    s32 BytesPerPixel;
};

#endif //GAME_H
