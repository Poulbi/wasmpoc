typedef unsigned int u32;
typedef unsigned char u8;

#define WIDTH 600
#define HEIGHT 600
u32 BUFFER[WIDTH * HEIGHT];

extern u8 __heap_base;
u32 BumpPointer = (u32)&__heap_base;

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

void go(u32 *Pixels, int Width, int Height) {
    for(int Y = 0; Y < Height; Y++)
    {
        for(int X = 0; X < Width; X++)
        {
            u32 Color = 0xFFFF0000;
            Pixels[Y*Width + X] = Color;
        }
    }
}
