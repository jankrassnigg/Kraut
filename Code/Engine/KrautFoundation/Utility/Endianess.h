#ifndef AE_FOUNDATION_UTILITY_ENDIANESS_H
#define AE_FOUNDATION_UTILITY_ENDIANESS_H

#include "../Defines.h"

namespace AE_NS_FOUNDATION
{
  union FloatToUInt32
  {
    float f;
    aeUInt32 i;
  };

  union DoubleToUInt64
  {
    double f;
    aeUInt64 i;
  };

  //! Checks whether the current hardware is little-endian.
  AE_INLINE bool IsSystemLittleEndian (void)
  {
    aeUInt16 ui = 0x00FF;
    aeUInt8* pUI = (aeUInt8*) &ui;
    return (*pUI == 0xFF);
  }

  //! Checks whether the current hardware is big-endian.
  AE_INLINE bool IsSystemBigEndian (void)
  {
    return (!IsSystemLittleEndian ());
  }

  //! Swaps the bytes from big to little endian and back.
  AE_INLINE aeUInt8  SwapEndianes (aeUInt8 data)       { return (data); }
  //! Swaps the bytes from big to little endian and back.
  AE_INLINE aeUInt16 SwapEndianes (aeUInt16 data)      { return (((data & 0x00FF) << 8) | ((data & 0xFF00) >> 8)); }
  //! Swaps the bytes from big to little endian and back.
  AE_INLINE aeUInt32 SwapEndianes (aeUInt32 data)      { return (((data & 0x000000FF) << 24) | ((data & 0x0000FF00) << 8) | ((data & 0x00FF0000) >> 8) | ((data & 0xFF000000) >> 24)); }
  //! Swaps the bytes from big to little endian and back.
  AE_INLINE aeUInt64 SwapEndianes (aeUInt64 data)      { return (((data & 0x00000000000000FF) << 56) | ((data & 0x000000000000FF00) << 40) | ((data & 0x0000000000FF0000) << 24) | ((data & 0x00000000FF000000) << 8) | ((data & 0x000000FF00000000) >> 8) | ((data & 0x0000FF0000000000) >> 24) | ((data & 0x00FF000000000000) >> 40) | ((data & 0xFF00000000000000) >> 56)); }

  //! Swaps the bytes from big to little endian and back.
  AE_INLINE aeInt8   SwapEndianes (aeInt8 data)        { return (data); }
  //! Swaps the bytes from big to little endian and back.
  AE_INLINE aeInt16  SwapEndianes (aeInt16 data)       { return (((data & 0x00FF) << 8) | ((data & 0xFF00) >> 8)); }
  //! Swaps the bytes from big to little endian and back.
  AE_INLINE aeInt32  SwapEndianes (aeInt32 data)       { return (((data & 0x000000FF) << 24) | ((data & 0x0000FF00) << 8) | ((data & 0x00FF0000) >> 8) | ((data & 0xFF000000) >> 24)); }
  //! Swaps the bytes from big to little endian and back.
  AE_INLINE aeInt64  SwapEndianes (aeInt64 data)       { return (((data & 0x00000000000000FF) << 56) | ((data & 0x000000000000FF00) << 40) | ((data & 0x0000000000FF0000) << 24) | ((data & 0x00000000FF000000) << 8) | ((data & 0x000000FF00000000) >> 8) | ((data & 0x0000FF0000000000) >> 24) | ((data & 0x00FF000000000000) >> 40) | ((data & 0xFF00000000000000) >> 56)); }

  //! Swaps the bytes from big to little endian and back.
  AE_INLINE float    SwapEndianes (float data0)        { FloatToUInt32  data, res; data.f = data0; res.i = (((data.i & 0x000000FF) << 24) | ((data.i & 0x0000FF00) << 8) | ((data.i & 0x00FF0000) >> 8) | ((data.i & 0xFF000000) >> 24)); return (res.f); }
  //! Swaps the bytes from big to little endian and back.
  AE_INLINE double   SwapEndianes (double data0)       { DoubleToUInt64 data, res; data.f = data0; res.i = (((data.i & 0x00000000000000FF) << 56) | ((data.i & 0x000000000000FF00) << 40) | ((data.i & 0x0000000000FF0000) << 24) | ((data.i & 0x00000000FF000000) << 8) | ((data.i & 0x000000FF00000000) >> 8) | ((data.i & 0x0000FF0000000000) >> 24) | ((data.i & 0x00FF000000000000) >> 40) | ((data.i & 0xFF00000000000000) >> 56)); return (res.f); }


#ifdef AE_BIG_ENDIAN

  //! Converts the data from the CURRENT endianess to big-endian.
  AE_INLINE aeUInt8  ToBigEndian (aeUInt8 data)  	    { return (data); }
  //! Converts the data from the CURRENT endianess to big-endian.
  AE_INLINE aeUInt16 ToBigEndian (aeUInt16 data) 	    { return (data); }
  //! Converts the data from the CURRENT endianess to big-endian.
  AE_INLINE aeUInt32 ToBigEndian (aeUInt32 data) 	    { return (data); }
  //! Converts the data from the CURRENT endianess to big-endian.
  AE_INLINE aeUInt64 ToBigEndian (aeUInt64 data) 	    { return (data); }
  //! Converts the data from the CURRENT endianess to big-endian.
  AE_INLINE aeInt8  	ToBigEndian (aeInt8 data)         { return (data); }
  //! Converts the data from the CURRENT endianess to big-endian.
  AE_INLINE aeInt16 	ToBigEndian (aeInt16 data)        { return (data); }
  //! Converts the data from the CURRENT endianess to big-endian.
  AE_INLINE aeInt32 	ToBigEndian (aeInt32 data)        { return (data); }
  //! Converts the data from the CURRENT endianess to big-endian.
  AE_INLINE aeInt64 	ToBigEndian (aeInt64 data)        { return (data); }
  //! Converts the data from the CURRENT endianess to big-endian.
  AE_INLINE float   	ToBigEndian (float data)          { return (data); }
  //! Converts the data from the CURRENT endianess to big-endian.
  AE_INLINE double  	ToBigEndian (double data)         { return (data); }


  //! Converts the data from the CURRENT endianess to little-endian.
  AE_INLINE aeUInt8  ToLittleEndian (aeUInt8 data)     { return (SwapEndianes (data)); }
  //! Converts the data from the CURRENT endianess to little-endian.
  AE_INLINE aeUInt16 ToLittleEndian (aeUInt16 data)    { return (SwapEndianes (data)); }
  //! Converts the data from the CURRENT endianess to little-endian.
  AE_INLINE aeUInt32 ToLittleEndian (aeUInt32 data)    { return (SwapEndianes (data)); }
  //! Converts the data from the CURRENT endianess to little-endian.
  AE_INLINE aeUInt64 ToLittleEndian (aeUInt64 data)    { return (SwapEndianes (data)); }
  //! Converts the data from the CURRENT endianess to little-endian.
  AE_INLINE aeInt8   ToLittleEndian (aeInt8 data)      { return (SwapEndianes (data)); }
  //! Converts the data from the CURRENT endianess to little-endian.
  AE_INLINE aeInt16  ToLittleEndian (aeInt16 data)     { return (SwapEndianes (data)); }
  //! Converts the data from the CURRENT endianess to little-endian.
  AE_INLINE aeInt32  ToLittleEndian (aeInt32 data)     { return (SwapEndianes (data)); }
  //! Converts the data from the CURRENT endianess to little-endian.
  AE_INLINE aeInt64  ToLittleEndian (aeInt64 data)     { return (SwapEndianes (data)); }
  //! Converts the data from the CURRENT endianess to little-endian.
  AE_INLINE float    ToLittleEndian (float data)       { return (SwapEndianes (data)); }
  //! Converts the data from the CURRENT endianess to little-endian.
  AE_INLINE double   ToLittleEndian (double data)      { return (SwapEndianes (data)); }


  //! Converts the data from big-endian to the CURRENT endianess.
  AE_INLINE aeUInt8  FromBigEndian (aeUInt8 data)      { return (data); }
  //! Converts the data from big-endian to the CURRENT endianess.
  AE_INLINE aeUInt16 FromBigEndian (aeUInt16 data)     { return (data); }
  //! Converts the data from big-endian to the CURRENT endianess.
  AE_INLINE aeUInt32 FromBigEndian (aeUInt32 data)     { return (data); }
  //! Converts the data from big-endian to the CURRENT endianess.
  AE_INLINE aeUInt64 FromBigEndian (aeUInt64 data)     { return (data); }
  //! Converts the data from big-endian to the CURRENT endianess.
  AE_INLINE aeInt8  	FromBigEndian (aeInt8 data)       { return (data); }
  //! Converts the data from big-endian to the CURRENT endianess.
  AE_INLINE aeInt16 	FromBigEndian (aeInt16 data)      { return (data); }
  //! Converts the data from big-endian to the CURRENT endianess.
  AE_INLINE aeInt32 	FromBigEndian (aeInt32 data)      { return (data); }
  //! Converts the data from big-endian to the CURRENT endianess.
  AE_INLINE aeInt64 	FromBigEndian (aeInt64 data)      { return (data); }
  //! Converts the data from big-endian to the CURRENT endianess.
  AE_INLINE float   	FromBigEndian (float data)        { return (data); }
  //! Converts the data from big-endian to the CURRENT endianess.
  AE_INLINE double  	FromBigEndian (double data)       { return (data); }

  //! Converts the data from little-endian to the CURRENT endianess.
  AE_INLINE aeUInt8  FromLittleEndian (aeUInt8 data)   { return (SwapEndianes (data)); }
  //! Converts the data from little-endian to the CURRENT endianess.
  AE_INLINE aeUInt16 FromLittleEndian (aeUInt16 data)  { return (SwapEndianes (data)); }
  //! Converts the data from little-endian to the CURRENT endianess.
  AE_INLINE aeUInt32 FromLittleEndian (aeUInt32 data)  { return (SwapEndianes (data)); }
  //! Converts the data from little-endian to the CURRENT endianess.
  AE_INLINE aeUInt64 FromLittleEndian (aeUInt64 data)  { return (SwapEndianes (data)); }
  //! Converts the data from little-endian to the CURRENT endianess.
  AE_INLINE aeInt8   FromLittleEndian (aeInt8 data)    { return (SwapEndianes (data)); }
  //! Converts the data from little-endian to the CURRENT endianess.
  AE_INLINE aeInt16  FromLittleEndian (aeInt16 data)   { return (SwapEndianes (data)); }
  //! Converts the data from little-endian to the CURRENT endianess.
  AE_INLINE aeInt32  FromLittleEndian (aeInt32 data)   { return (SwapEndianes (data)); }
  //! Converts the data from little-endian to the CURRENT endianess.
  AE_INLINE aeInt64  FromLittleEndian (aeInt64 data)   { return (SwapEndianes (data)); }
  //! Converts the data from little-endian to the CURRENT endianess.
  AE_INLINE float    FromLittleEndian (float data) 	  { return (SwapEndianes (data)); }
  //! Converts the data from little-endian to the CURRENT endianess.
  AE_INLINE double   FromLittleEndian (double data)    { return (SwapEndianes (data)); }
  
#else

  //! Converts the data from the CURRENT endianess to little-endian.
  AE_INLINE aeUInt8  ToLittleEndian (aeUInt8 data)  	  { return (data); }
  //! Converts the data from the CURRENT endianess to little-endian.
  AE_INLINE aeUInt16 ToLittleEndian (aeUInt16 data) 	  { return (data); }
  //! Converts the data from the CURRENT endianess to little-endian.
  AE_INLINE aeUInt32 ToLittleEndian (aeUInt32 data) 	  { return (data); }
  //! Converts the data from the CURRENT endianess to little-endian.
  AE_INLINE aeUInt64 ToLittleEndian (aeUInt64 data) 	  { return (data); }
  //! Converts the data from the CURRENT endianess to little-endian.
  AE_INLINE aeInt8  	ToLittleEndian (aeInt8 data)      { return (data); }
  //! Converts the data from the CURRENT endianess to little-endian.
  AE_INLINE aeInt16 	ToLittleEndian (aeInt16 data)     { return (data); }
  //! Converts the data from the CURRENT endianess to little-endian.
  AE_INLINE aeInt32 	ToLittleEndian (aeInt32 data)     { return (data); }
  //! Converts the data from the CURRENT endianess to little-endian.
  AE_INLINE aeInt64 	ToLittleEndian (aeInt64 data)     { return (data); }
  //! Converts the data from the CURRENT endianess to little-endian.
  AE_INLINE float   	ToLittleEndian (float data)       { return (data); }
  //! Converts the data from the CURRENT endianess to little-endian.
  AE_INLINE double  	ToLittleEndian (double data)      { return (data); }


  //! Converts the data from the CURRENT endianess to big-endian.
  AE_INLINE aeUInt8  ToBigEndian (aeUInt8 data)  		  { return (SwapEndianes (data)); }
  //! Converts the data from the CURRENT endianess to big-endian.
  AE_INLINE aeUInt16 ToBigEndian (aeUInt16 data) 		  { return (SwapEndianes (data)); }
  //! Converts the data from the CURRENT endianess to big-endian.
  AE_INLINE aeUInt32 ToBigEndian (aeUInt32 data) 		  { return (SwapEndianes (data)); }
  //! Converts the data from the CURRENT endianess to big-endian.
  AE_INLINE aeUInt64 ToBigEndian (aeUInt64 data) 		  { return (SwapEndianes (data)); }
  //! Converts the data from the CURRENT endianess to big-endian.
  AE_INLINE aeInt8   ToBigEndian (aeInt8 data)  		    { return (SwapEndianes (data)); }
  //! Converts the data from the CURRENT endianess to big-endian.
  AE_INLINE aeInt16  ToBigEndian (aeInt16 data) 		    { return (SwapEndianes (data)); }
  //! Converts the data from the CURRENT endianess to big-endian.
  AE_INLINE aeInt32  ToBigEndian (aeInt32 data) 		    { return (SwapEndianes (data)); }
  //! Converts the data from the CURRENT endianess to big-endian.
  AE_INLINE aeInt64  ToBigEndian (aeInt64 data) 		    { return (SwapEndianes (data)); }
  //! Converts the data from the CURRENT endianess to big-endian.
  AE_INLINE float    ToBigEndian (float data) 	  		  { return (SwapEndianes (data)); }
  //! Converts the data from the CURRENT endianess to big-endian.
  AE_INLINE double   ToBigEndian (double data)         { return (SwapEndianes (data)); }


  //! Converts the data from little-endian to the CURRENT endianess.
  AE_INLINE aeUInt8  FromLittleEndian (aeUInt8 data)  	{ return (data); }
  //! Converts the data from little-endian to the CURRENT endianess.
  AE_INLINE aeUInt16 FromLittleEndian (aeUInt16 data) 	{ return (data); }
  //! Converts the data from little-endian to the CURRENT endianess.
  AE_INLINE aeUInt32 FromLittleEndian (aeUInt32 data) 	{ return (data); }
  //! Converts the data from little-endian to the CURRENT endianess.
  AE_INLINE aeUInt64 FromLittleEndian (aeUInt64 data) 	{ return (data); }
  //! Converts the data from little-endian to the CURRENT endianess.
  AE_INLINE aeInt8  	FromLittleEndian (aeInt8 data)    { return (data); }
  //! Converts the data from little-endian to the CURRENT endianess.
  AE_INLINE aeInt16 	FromLittleEndian (aeInt16 data)   { return (data); }
  //! Converts the data from little-endian to the CURRENT endianess.
  AE_INLINE aeInt32 	FromLittleEndian (aeInt32 data)   { return (data); }
  //! Converts the data from little-endian to the CURRENT endianess.
  AE_INLINE aeInt64 	FromLittleEndian (aeInt64 data)   { return (data); }
  //! Converts the data from little-endian to the CURRENT endianess.
  AE_INLINE float   	FromLittleEndian (float data)     { return (data); }
  //! Converts the data from little-endian to the CURRENT endianess.
  AE_INLINE double  	FromLittleEndian (double data)    { return (data); }


  //! Converts the data from big-endian to the CURRENT endianess.
  AE_INLINE aeUInt8  FromBigEndian (aeUInt8 data)  		{ return (SwapEndianes (data)); }
  //! Converts the data from big-endian to the CURRENT endianess.
  AE_INLINE aeUInt16 FromBigEndian (aeUInt16 data) 		{ return (SwapEndianes (data)); }
  //! Converts the data from big-endian to the CURRENT endianess.
  AE_INLINE aeUInt32 FromBigEndian (aeUInt32 data) 		{ return (SwapEndianes (data)); }
  //! Converts the data from big-endian to the CURRENT endianess.
  AE_INLINE aeUInt64 FromBigEndian (aeUInt64 data) 		{ return (SwapEndianes (data)); }
  //! Converts the data from big-endian to the CURRENT endianess.
  AE_INLINE aeInt8   FromBigEndian (aeInt8 data)  		  { return (SwapEndianes (data)); }
  //! Converts the data from big-endian to the CURRENT endianess.
  AE_INLINE aeInt16  FromBigEndian (aeInt16 data) 		  { return (SwapEndianes (data)); }
  //! Converts the data from big-endian to the CURRENT endianess.
  AE_INLINE aeInt32  FromBigEndian (aeInt32 data) 		  { return (SwapEndianes (data)); }
  //! Converts the data from big-endian to the CURRENT endianess.
  AE_INLINE aeInt64  FromBigEndian (aeInt64 data) 		  { return (SwapEndianes (data)); }
  //! Converts the data from big-endian to the CURRENT endianess.
  AE_INLINE float    FromBigEndian (float data) 			  { return (SwapEndianes (data)); }
  //! Converts the data from big-endian to the CURRENT endianess.
  AE_INLINE double   FromBigEndian (double data)       { return (SwapEndianes (data)); }

#endif

}

#endif
