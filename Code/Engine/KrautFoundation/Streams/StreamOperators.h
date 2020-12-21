#ifndef AE_FOUNDATION_FILESYSTEM_STREAMOPERATORS_H
#define AE_FOUNDATION_FILESYSTEM_STREAMOPERATORS_H

#include "../Defines.h"
#include "../Streams/Streams.h"
#include "../Utility/Endianess.h"
#include "../Utility/Flags.h"
#include "../Strings/String.h"
#include "../Math/Vec3.h"
#include "../Math/Plane.h"
#include "../Math/Quaternion.h"
#include "../Math/Matrix.h"

namespace AE_NS_FOUNDATION
{
  // ******** Stream Out Operators ********

  // *** unsigned ints ***

  AE_INLINE void operator<< (aeStreamOut& s, aeUInt8 data)
  {
    data = ToLittleEndian (data);
    s.Write (&data, sizeof (data));
  }

  AE_INLINE void operator<< (aeStreamOut& s, aeUInt16 data)
  {
    data = ToLittleEndian (data);
    s.Write (&data, sizeof (data));
  }

  AE_INLINE void operator<< (aeStreamOut& s, aeUInt32 data)
  {
    data = ToLittleEndian (data);
    s.Write (&data, sizeof (data));
  }

  AE_INLINE void operator<< (aeStreamOut& s, aeUInt64 data)
  {
    data = ToLittleEndian (data);
    s.Write (&data, sizeof (data));
  }

  // *** signed ints ***

  AE_INLINE void operator<< (aeStreamOut& s, aeInt8 data)
  {
    data = ToLittleEndian (data);
    s.Write (&data, sizeof (data));
  }

  AE_INLINE void operator<< (aeStreamOut& s, aeInt16 data)
  {
    data = ToLittleEndian (data);
    s.Write (&data, sizeof (data));
  }

  AE_INLINE void operator<< (aeStreamOut& s, aeInt32 data)
  {
    data = ToLittleEndian (data);
    s.Write (&data, sizeof (data));
  }

  AE_INLINE void operator<< (aeStreamOut& s, aeInt64 data)
  {
    data = ToLittleEndian (data);
    s.Write (&data, sizeof (data));
  }

  // *** float / double ***

  AE_INLINE void operator<< (aeStreamOut& s, float data)
  {
    data = ToLittleEndian (data);
    s.Write (&data, sizeof (data));
  }

  AE_INLINE void operator<< (aeStreamOut& s, double data)
  {
    data = ToLittleEndian (data);
    s.Write (&data, sizeof (data));
  }

  // *** bool ***

  AE_INLINE void operator<< (aeStreamOut& s, bool data)
  {
    s.Write (&data, sizeof (data));
  }


  // *** Strings ***

  AE_INLINE void operator<< (aeStreamOut& s, const aeBasicString& data)
  {
    const aeUInt32 uiSize = data.length ();
    s.Write (&uiSize, sizeof (aeUInt32));
    s.Write (data.c_str (), sizeof (char) * uiSize);
  }


  // *** Math Classes ***

  AE_INLINE void operator<< (aeStreamOut& s, const aeVec3& data)
  {
    s.Write (&data.x, sizeof (float) * 3);
  }

  AE_INLINE void operator<< (aeStreamOut& s, const aePlane& data)
  {
    s.Write (&data.m_fComponents[0], sizeof (float) * 4);
  }

  AE_INLINE void operator<< (aeStreamOut& s, const aeMatrix& data)
  {
    s.Write (data.m_fElements, sizeof (float) * 16);
  }

  AE_INLINE void operator<< (aeStreamOut& s, const aeQuaternion& data)
  {
    s.Write (data.m_Components, sizeof (float) * 4);
  }

  // *** Utility Classes ***

  template<class BASETYPE>
  void operator<< (aeStreamOut& s, const aeFlags<BASETYPE>& data)
  {
    s << data.GetData ();
  }


  // ******** Stream In Operators ********

  // *** unsigned ints ***

  AE_INLINE void operator>> (aeStreamIn& s, aeUInt8& data)
  {
    s.Read (&data, sizeof (data));
    data = FromLittleEndian (data);
  }

  AE_INLINE void operator>> (aeStreamIn& s, aeUInt16& data)
  {
    s.Read (&data, sizeof (data));
    data = FromLittleEndian (data);
  }

  AE_INLINE void operator>> (aeStreamIn& s, aeUInt32& data)
  {
    s.Read (&data, sizeof (data));
    data = FromLittleEndian (data);
  }

  AE_INLINE void operator>> (aeStreamIn& s, aeUInt64& data)
  {
    s.Read (&data, sizeof (data));
    data = FromLittleEndian (data);
  }

  // *** signed ints ***

  AE_INLINE void operator>> (aeStreamIn& s, aeInt8& data)
  {
    s.Read (&data, sizeof (data));
    data = FromLittleEndian (data);
  }

  AE_INLINE void operator>> (aeStreamIn& s, aeInt16& data)
  {
    s.Read (&data, sizeof (data));
    data = FromLittleEndian (data);
  }

  AE_INLINE void operator>> (aeStreamIn& s, aeInt32& data)
  {
    s.Read (&data, sizeof (data));
    data = FromLittleEndian (data);
  }

  AE_INLINE void operator>> (aeStreamIn& s, aeInt64& data)
  {
    s.Read (&data, sizeof (data));
    data = FromLittleEndian (data);
  }

  // *** float / double ***

  AE_INLINE void operator>> (aeStreamIn& s, float& data)
  {
    s.Read (&data, sizeof (data));
    data = FromLittleEndian (data);
  }

  AE_INLINE void operator>> (aeStreamIn& s, double& data)
  {
    s.Read (&data, sizeof (data));
    data = FromLittleEndian (data);
  }

  // *** bool ***

  AE_INLINE void operator>> (aeStreamIn& s, bool& data)
  {
    s.Read (&data, sizeof (data));
  }


  // *** Strings ***

  AE_INLINE void operator>> (aeStreamIn& s, aeString& data)
  {
    aeUInt32 uiSize;
    s.Read (&uiSize, sizeof (aeUInt32));

    data.resize (uiSize);

    if (uiSize > 0)
      s.Read (&data[0], sizeof (char) * uiSize);
  }


  // *** Math Classes ***

  AE_INLINE void operator>> (aeStreamIn& s, aeVec3& data)
  {
    s.Read (&data.x, sizeof (float) * 3);
  }

  AE_INLINE void operator>> (aeStreamIn& s, aePlane& data)
  {
    s.Read (&data.m_fComponents[0], sizeof (float) * 4);
  }

  AE_INLINE void operator>> (aeStreamIn& s, aeMatrix& data)
  {
    s.Read (data.m_fElements, sizeof (float) * 16);
  }

  AE_INLINE void operator>> (aeStreamIn& s, aeQuaternion& data)
  {
    s.Read (data.m_Components, sizeof (float) * 4);
  }

  // *** Utility Classes ***

  template<class BASETYPE>
  void operator>> (aeStreamIn& s, aeFlags<BASETYPE>& data)
  {
    BASETYPE d;
    s >> d;
    data.SetFlags (d);
  }


}

#endif
