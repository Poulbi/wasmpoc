#ifndef HANDMADE_FONT_H
#define HANDMADE_FONT_H
#include "stb_truetype.h"

struct game_font
{
    stbtt_fontinfo Info;
    s32 Ascent;
    s32 Descent;
    s32 LineGap;
    v2 BoundingBox[2];
    b32 Initialized; // For debugging.
};

#endif //HANDMADE_FONT_H

#ifdef HANDMADE_FONT_IMPLEMENTATION
//~ Libraries
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#define RoundReal32ToInt32 round
#define FloorReal32ToInt32 floor
#define free Free

//~ Implementation
//- Loading  
internal void
InitFontFromBuffer(game_font *Font, char *Buffer)
{
    if(Buffer)
    {
        if(stbtt_InitFont(&Font->Info, (u8 *)Buffer, stbtt_GetFontOffsetForIndex((u8 *)Buffer, 0)))
        {
            Font->Info.data = (u8 *)Buffer;
            
            s32 X0, Y0, X1, Y1;
            stbtt_GetFontBoundingBox(&Font->Info, &X0, &Y0, &X1, &Y1);
            Font->BoundingBox[0] = v2{(r32)X0, (r32)Y0};
            Font->BoundingBox[1] = v2{(r32)X1, (r32)Y1};
            stbtt_GetFontVMetrics(&Font->Info, &Font->Ascent, &Font->Descent, &Font->LineGap);
            Font->Initialized = true;
        }
        else
        {
            // TODO(luca): Logging
        }
    }
    else
    {
        // TODO(luca): Logging
    }
}

//- Rendering 
internal void
DrawCharacter(game_offscreen_buffer *Buffer,  u8 *FontBitmap,
              int FontWidth, int FontHeight, 
              int XOffset, int YOffset,
              v3 Color)
{
    s32 MinX = 0;
    s32 MinY = 0;
    s32 MaxX = FontWidth;
    s32 MaxY = FontHeight;
    
    if(XOffset < 0)
    {
        MinX = -XOffset;
        XOffset = 0;
    }
    if(YOffset < 0)
    {
        MinY = -YOffset;
        YOffset = 0;
    }
    if(XOffset + FontWidth > Buffer->Width)
    {
        MaxX -= ((XOffset + FontWidth) - Buffer->Width);
    }
    if(YOffset + FontHeight > Buffer->Height)
    {
        MaxY -= ((YOffset + FontHeight) - Buffer->Height);
    }
    
    u8 *Row = (u8 *)(Buffer->Memory) + 
    (YOffset*Buffer->Pitch) +
    (XOffset*Buffer->BytesPerPixel);
    
    for(int  Y = MinY;
        Y < MaxY;
        Y++)
    {
        u32 *Pixel = (u32 *)Row;
        for(int X = MinX;
            X < MaxX;
            X++)
        {
            u8 Brightness = FontBitmap[Y*FontWidth+X];
            r32 Alpha = ((r32)Brightness/255.0f);
            
            r32 DR = (r32)((*Pixel >> 16) & 0xFF);
            r32 DG = (r32)((*Pixel >> 8) & 0xFF);
            r32 DB = (r32)((*Pixel >> 0) & 0xFF);
            
            r32 R = Color.R*255.0f*Alpha + DR*(1-Alpha);
            r32 G = Color.G*255.0f*Alpha + DG*(1-Alpha);
            r32 B = Color.B*255.0f*Alpha +  DB*(1-Alpha);
            
            u32 Value = ((0xFF << 24) |
                         ((u32)(R) << 16) |
                         ((u32)(G) << 8) |
                         ((u32)(B) << 0));
            *Pixel++ = Value;
        }
        
        Row += Buffer->Pitch;
    }
}


internal void
DrawText(game_offscreen_buffer *Buffer, game_font *Font, r32 HeightPixels,
         u32 TextLen, u8 *Text, v2 Offset, v3 Color, b32 IsUTF8)
{
    Assert(Font->Initialized);
    
    Offset.X = RoundReal32ToInt32(Offset.X);
    Offset.Y = RoundReal32ToInt32(Offset.Y);
    
    r32 FontScale = stbtt_ScaleForPixelHeight(&Font->Info, HeightPixels);
    
    for(u32 TextIndex = 0;
        TextIndex < TextLen;
        TextIndex++)
    {
        rune CharAt = 0;
        if(IsUTF8)
        {
            CharAt = *(((rune *)Text) + TextIndex);
        }
        else
        {
            CharAt = Text[TextIndex];
        }
        
        s32 FontWidth, FontHeight;
        s32 AdvanceWidth, LeftSideBearing;
        s32 X0, Y0, X1, Y1;
        u8 *FontBitmap = 0;
        // TODO(luca): Get rid of malloc.
        FontBitmap = stbtt_GetCodepointBitmap(&Font->Info, 
                                              FontScale, FontScale, 
                                              CharAt, 
                                              &FontWidth, &FontHeight, 0, 0);
        stbtt_GetCodepointBitmapBox(&Font->Info, CharAt, 
                                    FontScale, FontScale, 
                                    &X0, &Y0, &X1, &Y1);
        stbtt_GetCodepointHMetrics(&Font->Info, CharAt, &AdvanceWidth, &LeftSideBearing);
        
        s32 XOffset = FloorReal32ToInt32(Offset.X + LeftSideBearing*FontScale);
        s32 YOffset = Offset.Y + Y0;
        
        DrawCharacter(Buffer, FontBitmap, FontWidth, FontHeight, XOffset, YOffset, Color);
        
        Offset.X += RoundReal32ToInt32(AdvanceWidth*FontScale);
        free(FontBitmap);
    }
}

// 1. First pass where we check each character's size.
// 2. Save positions where we need to wrap.
// Wrapping algorithm
// 1. When a character exceeds the box maximum width search backwards for whitespace.
//    I. Whitespace found?
//       Y -> Length of string until whitespace would fit?
//            Y -> Save whitespace's position.  This becomes the new searching start position.
//            N -> Break on the character that exceeds the maximum width.
//       N -> Break on the character that exceeds the maximum width.
//    II. Continue until end of string.

internal void
DrawTextInBox(memory_arena *Arena, game_offscreen_buffer *Buffer, game_font *Font, 
              string Text, r32 HeightPx, v3 Color,
              v2 BoxMin, v2 BoxMax, b32 Centered)
{
    psize ArenaStartUsed = Arena->Used;
    s32 *CharacterPixelWidths = PushArray(Arena, Text.Count, s32);
    u32 *WrapPositions = PushArray(Arena, 0, u32);
    u32 WrapPositionsCount = 0;
    
    r32 FontScale = stbtt_ScaleForPixelHeight(&Font->Info, HeightPx);
    
    // TODO(luca): UTF8 support
    // e.g. (https://en.wikipedia.org/wiki/Whitespace_character) these are all whitespace characters that we might want to support.
    for(u32 TextIndex = 0;
        TextIndex < Text.Count;
        TextIndex++)
    {
        u8 CharAt = Text.Data[TextIndex];
        
        s32 AdvanceWidth, LeftSideBearing;
        stbtt_GetCodepointHMetrics(&Font->Info, CharAt, &AdvanceWidth, &LeftSideBearing);
        
        CharacterPixelWidths[TextIndex] = RoundReal32ToInt32(FontScale*AdvanceWidth);
    }
    
    s32 MaxWidth = BoxMax.X - BoxMin.X;
    Assert(MaxWidth >= 0);
    
    u32 SearchStart = 0;
    while(SearchStart < Text.Count)
    {
        s32 CumulatedWidth = 0;
        u32 SearchIndex = SearchStart;
        for(;
            ((SearchIndex < Text.Count) &&
             (CumulatedWidth <= MaxWidth));
            SearchIndex++)
        {
            s32 Width = CharacterPixelWidths[SearchIndex];
            CumulatedWidth += Width;
        }
        
        if(CumulatedWidth > MaxWidth)
        {
            // We need to search backwards for wrapping.
            SearchIndex--;
            u32 SearchIndexStop = SearchIndex;
            
            while(SearchIndex > SearchStart)
            {
                if(Text.Data[SearchIndex] == ' ')
                {
                    PushArray(Arena, 1, u32);
                    WrapPositions[WrapPositionsCount++] = SearchIndex;
                    break;
                }
                
                SearchIndex--;
            }
            
            if(SearchIndex > SearchStart)
            {
                Assert(SearchIndex > SearchStart);
                // luca: We skip the character we wrapped on.
                SearchStart = SearchIndex + 1;
            }
            else if(SearchIndex == SearchStart)
            {
                Assert(SearchIndexStop > SearchStart);
                PushArray(Arena, 1, u32);
                WrapPositions[WrapPositionsCount++] = SearchIndexStop;
                SearchStart = SearchIndexStop;
            }
            else
            {
                Assert(0);
            }
            
        }
        else
        {
            // luca: We don't need to wrap, we've reached the end of the text.
            break;
        }
    }
    
    s32 YAdvance = FontScale*(Font->Ascent - Font->Descent + 
                              Font->LineGap);
    
    v2 TextOffset = BoxMin;
    
    // Add baseline
    TextOffset.Y += FontScale*Font->Ascent;
    
    if(Centered)
    {
        s32 TextHeight = YAdvance * (WrapPositionsCount + 1);
        s32 CenterHOffset = ((BoxMax.Y - BoxMin.Y) - TextHeight)/2;
        if(CenterHOffset >= 0)
        {
            TextOffset.Y += CenterHOffset;
        }
    }
    
    u32 Start = 0;
    for(u32 WrapIndex = 0;
        WrapIndex < WrapPositionsCount;
        WrapIndex++)
    {
        u32 Position = WrapPositions[WrapIndex];
        
        if(TextOffset.Y - FontScale*Font->Descent < BoxMax.Y)
        {
            
            b32 DoCenter = (Centered && 
                            ((WrapIndex == 0) ||
                             (Text.Data[(WrapPositions[WrapIndex - 1])] == ' ')));
            if(DoCenter)
            {
                s32 TextWidth = 0;
                for(u32 WidthIndex = Start;
                    WidthIndex < Position;
                    WidthIndex++)
                {
                    TextWidth += CharacterPixelWidths[WidthIndex];
                }
                TextOffset.X = BoxMin.X + ((MaxWidth - TextWidth)/2);
            }
            
            DrawText(Buffer, Font, HeightPx, 
                     Position - Start, Text.Data + Start, 
                     TextOffset, Color, false);
        }
        
        TextOffset.Y += YAdvance;
        
        if(Text.Data[Position] == ' ')
        {
            Position++;
        }
        
        Start = Position;
    }
    
    TextOffset.X = BoxMin.X;
    
    b32 DoCenter = (Centered &&
                    ((WrapPositionsCount == 0) || (Text.Data[WrapPositions[WrapPositionsCount - 1]] == ' ')));
    if(DoCenter)
    {                
        s32 TextWidth = 0;
        for(u32 WidthIndex = Start;
            WidthIndex < Text.Count;
            WidthIndex++)
        {
            TextWidth += CharacterPixelWidths[WidthIndex];
        }
        TextOffset.X = BoxMin.X + ((MaxWidth - TextWidth)/2);
    }
    
    if(TextOffset.Y - FontScale*Font->Descent < BoxMax.Y)
    {
        DrawText(Buffer, Font, HeightPx, 
                 Text.Count - Start, Text.Data + Start, 
                 TextOffset, Color, false); 
    }
    
    Arena->Used = ArenaStartUsed;
}

#endif //HANDMADE_FONT_IMPLEMENTATION
