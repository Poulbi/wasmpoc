/* date = June 10th 2025 1:05 pm */

#ifndef HANDMADE_MATH_H
#define HANDMADE_MATH_H


union v2
{
    struct 
    {
        r32 X, Y;
    };
    r32 E[2];
};

inline v2
V2(r32 X, r32 Y)
{
    v2 Result;
    
    Result.X = X;
    Result.Y = Y;
    
    return(Result);
}

inline v2
operator-(v2 A)
{
    v2 Result;
    
    Result.X = -A.X;
    Result.Y = -A.Y;
    
    return Result;
}

inline v2
operator+(v2 A, v2 B)
{
    v2 Result;
    
    Result.X = A.X + B.X;
    Result.Y = A.Y + B.Y;
    
    return Result;
}

inline v2
operator+(v2 A, r32 B)
{
    v2 Result;
    
    Result.X = A.X + B;
    Result.Y = A.Y + B;
    
    return Result;
}

inline v2
operator-(v2 A, v2 B)
{
    v2 Result;
    
    Result.X = A.X - B.X;
    Result.Y = A.Y - B.Y;
    
    return Result;
}

inline v2
operator-(v2 A, r32 B)
{
    v2 Result;
    Result = A + -B;
    return Result;
}

inline v2
operator*(r32 A, v2 B)
{
    v2 Result;
    
    Result.X = A*B.X;
    Result.Y = A*B.Y;
    
    return Result;
}

inline v2
operator*(v2 A, r32 B)
{
    return B*A;
}

inline v2
operator/(v2 A, r32 B)
{
    v2 Result;
    
    Result.X = A.X/B;
    Result.Y = A.Y/B;
    
    return Result;
}

inline v2 & 
operator*=(v2 &A, r32 B)
{
    A = B * A;
    return A;
}

inline v2 & 
operator+=(v2 &A, v2 B)
{
    A = A + B;
    return A;
}

inline 
r32 Square(r32 A)
{
    r32 Result = A*A;
    return Result;
}

inline 
r32 Inner(v2 A, v2 B)
{
    r32 Result = A.X*B.X + A.Y*B.Y;
    return Result;
}

inline
r32 LengthSq(v2 A)
{
    r32 Result = Inner(A, A);
    return Result;
}

//- v3 
struct v3
{
    union
    {
        struct
        {
            r32 R;
            r32 G;
            r32 B;
        };
        r32 E[3];
    };
};
#define v3(A) v3{(A), (A), (A)}

inline v3
operator*(v3 A, r32 B)
{
    v3 Result;
    
    Result.R = A.R * B;
    Result.G = A.G * B;
    Result.B = A.B * B;
    
    return Result;
}

inline v3
operator*(r32 A, v3 B)
{
    v3 Result = B*A;
    
    return Result;
}

inline v3
operator+(v3 A, r32 B)
{
    v3 Result;
    
    Result.R = A.R + B;
    Result.G = A.G + B;
    Result.B = A.B + B;
    
    return Result;
}

inline v3
operator+(r32 A, v3 B)
{
    v3 Result = B+A;
    
    return Result;
}

inline v3
operator-(v3 A)
{
    v3 Result;
    
    Result.R = -A.R;
    Result.G = -A.G;
    Result.B = -A.B;
    
    return Result;
}

inline v3
operator+(v3 A, v3 B)
{
    v3 Result;
    
    Result.R = A.R + B.R;
    Result.G = A.G + B.G;
    Result.B = A.B + B.B;
    
    return Result;
}

inline v3 & 
operator+=(v3 &A, v3 B)
{
    A = A + B;
    return A;
}

inline v3 & 
operator+=(v3 &A, r32 B)
{
    A = A + B;
    return A;
}

#endif //HANDMADE_MATH_H
