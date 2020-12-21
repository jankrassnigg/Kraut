#include "Main.h"

kvVertexBuffer* kvVertexBuffer::s_pActiveBuffer = NULL;

kvVertexBuffer::kvVertexBuffer ()
{
  m_uiVertexCount = 0;
  m_uiTriangleCount = 0;
  m_uiVertexDataSize = 0;
  m_uiBufferID = 0;
  m_uiIndexID = 0;
}

kvVertexBuffer::~kvVertexBuffer ()
{
  // who's going to cleanup here ?
}


static unsigned int GetStreamTypeElements (kvVertexBuffer::StreamType Type)
{
  switch (Type)
  {
  case kvVertexBuffer::R32_FLOAT:
    return 1;
  case kvVertexBuffer::R32G32_FLOAT:
    return 2;
  case kvVertexBuffer::R32G32B32_FLOAT:
    return 3;
  case kvVertexBuffer::R32G32B32A32_FLOAT:
    return 4;
  case kvVertexBuffer::R8G8B8A8_UNORM:
    return 4;
  }

  throw exception ("Unknown kvVertexBuffer::StreamType");
}

static GLenum GetStreamTypeElementType (kvVertexBuffer::StreamType Type)
{
  switch (Type)
  {
  case kvVertexBuffer::R32_FLOAT:
  case kvVertexBuffer::R32G32_FLOAT:
  case kvVertexBuffer::R32G32B32_FLOAT:
  case kvVertexBuffer::R32G32B32A32_FLOAT:
    return GL_FLOAT;
  case kvVertexBuffer::R8G8B8A8_UNORM:
    return GL_UNSIGNED_BYTE;
  }

  throw exception ("Unknown kvVertexBuffer::StreamType");
}

static GLenum GetStreamTypeElementsNormalized (kvVertexBuffer::StreamType Type)
{
  switch (Type)
  {
  case kvVertexBuffer::R32_FLOAT:
  case kvVertexBuffer::R32G32_FLOAT:
  case kvVertexBuffer::R32G32B32_FLOAT:
  case kvVertexBuffer::R32G32B32A32_FLOAT:
    return GL_FALSE;
  case kvVertexBuffer::R8G8B8A8_UNORM:
    return GL_TRUE;
  }

  throw exception ("Unknown kvVertexBuffer::StreamType");
}

static unsigned int GetStreamTypeSize (kvVertexBuffer::StreamType Type)
{
  switch (Type)
  {
  case kvVertexBuffer::R32_FLOAT:
    return sizeof (float);
  case kvVertexBuffer::R32G32_FLOAT:
    return sizeof (float) * 2;
  case kvVertexBuffer::R32G32B32_FLOAT:
    return sizeof (float) * 3;
  case kvVertexBuffer::R32G32B32A32_FLOAT:
    return sizeof (float) * 4;
  case kvVertexBuffer::R8G8B8A8_UNORM:
    return sizeof (unsigned char) * 4;
  }

  throw exception ("Unknown kvVertexBuffer::StreamType");
}


unsigned int kvVertexBuffer::AddVertexStream (const char* szName, StreamType Type)
{
  if (m_uiVertexCount > 0 || m_uiTriangleCount > 0)
    throw exception ("kvVertexBuffer::AddVertexStream cannot be called after kvVertexBuffer::SetupBuffer.");

  kvStream s;
  s.m_sStreamName = szName;
  s.m_uiOffset = m_uiVertexDataSize;
  s.m_Type = Type;

  m_Streams.push_back (s);

  m_uiVertexDataSize += GetStreamTypeSize (Type);

  return ((unsigned int) (m_Streams.size () - 1));
}

void kvVertexBuffer::SetupBuffer (unsigned int uiVertexCount, unsigned int uiTriangleCount)
{
  if (m_Streams.empty ())
    throw exception ("kvVertexBuffer::SetupBuffer must be called after some vertex streams have been added via kvVertexBuffer::AddVertexStream");

  m_uiVertexCount = uiVertexCount;
  m_uiTriangleCount = uiTriangleCount;

  // m_uiVertexDataSize contains the size of the data for one vertex
  const unsigned int uiVertexDataSize = m_uiVertexCount * m_uiVertexDataSize; // in bytes
  const unsigned int uiIndexDataSize = m_uiTriangleCount * 3; // in ints

  m_Data.resize (uiVertexDataSize);
  m_IndexData.resize (uiIndexDataSize);
}

void kvVertexBuffer::SetTriangle (unsigned int uiTriangle, unsigned int uiVertex1, unsigned int uiVertex2, unsigned int uiVertex3)
{
  if (uiTriangle >= m_uiTriangleCount)
    throw exception ("kvVertexBuffer::SetTriangle: The given triangle index is larger than the buffer was setup for.");
  if ((uiVertex1 >= m_uiVertexCount) || (uiVertex2 >= m_uiVertexCount) || (uiVertex3 >= m_uiVertexCount))
    throw exception ("kvVertexBuffer::SetTriangle: One of the given vertex indices is larger than the number of allocated vertices.");

  unsigned int uiFirstIndex = uiTriangle * 3;
  m_IndexData[uiFirstIndex + 0] = uiVertex1;
  m_IndexData[uiFirstIndex + 1] = uiVertex2;
  m_IndexData[uiFirstIndex + 2] = uiVertex3;
}

void kvVertexBuffer::SetVertexAttribute1f (unsigned int uiStream, unsigned int uiVertex, float d1)
{
  if (uiStream >= m_Streams.size ())
    throw exception ("kvVertexBuffer::SetVertexAttribute1f: The given stream index is larger than the number of allocated streams.");
  if (uiVertex >= m_uiVertexCount)
    throw exception ("kvVertexBuffer::SetVertexAttribute1f: The given vertex index is larger than the number of allocated vertices.");

  // this offset is in bytes
  const unsigned int uiDataOffset = uiVertex * m_uiVertexDataSize + m_Streams[uiStream].m_uiOffset;

  float* pData = (float*) &m_Data[uiDataOffset];
  pData[0] = d1;
}

void kvVertexBuffer::SetVertexAttribute2f (unsigned int uiStream, unsigned int uiVertex, float d1, float d2)
{
  if (uiStream >= m_Streams.size ())
    throw exception ("kvVertexBuffer::SetVertexAttribute2f: The given stream index is larger than the number of allocated streams.");
  if (uiVertex >= m_uiVertexCount)
    throw exception ("kvVertexBuffer::SetVertexAttribute2f: The given vertex index is larger than the number of allocated vertices.");

  // this offset is in bytes
  const unsigned int uiDataOffset = uiVertex * m_uiVertexDataSize + m_Streams[uiStream].m_uiOffset;

  float* pData = (float*) &m_Data[uiDataOffset];
  pData[0] = d1;
  pData[1] = d2;
}

void kvVertexBuffer::SetVertexAttribute3f (unsigned int uiStream, unsigned int uiVertex, float d1, float d2, float d3)
{
  if (uiStream >= m_Streams.size ())
    throw exception ("kvVertexBuffer::SetVertexAttribute3f: The given stream index is larger than the number of allocated streams.");
  if (uiVertex >= m_uiVertexCount)
    throw exception ("kvVertexBuffer::SetVertexAttribute3f: The given vertex index is larger than the number of allocated vertices.");

  // this offset is in bytes
  const unsigned int uiDataOffset = uiVertex * m_uiVertexDataSize + m_Streams[uiStream].m_uiOffset;

  float* pData = (float*) &m_Data[uiDataOffset];
  pData[0] = d1;
  pData[1] = d2;
  pData[2] = d3;
}

void kvVertexBuffer::SetVertexAttribute4f (unsigned int uiStream, unsigned int uiVertex, float d1, float d2, float d3, float d4)
{
  if (uiStream >= m_Streams.size ())
    throw exception ("kvVertexBuffer::SetVertexAttribute4f: The given stream index is larger than the number of allocated streams.");
  if (uiVertex >= m_uiVertexCount)
    throw exception ("kvVertexBuffer::SetVertexAttribute4f: The given vertex index is larger than the number of allocated vertices.");

  // this offset is in bytes
  const unsigned int uiDataOffset = uiVertex * m_uiVertexDataSize + m_Streams[uiStream].m_uiOffset;

  float* pData = (float*) &m_Data[uiDataOffset];
  pData[0] = d1;
  pData[1] = d2;
  pData[2] = d3;
  pData[3] = d4;
}

void kvVertexBuffer::SetVertexAttribute4ub (unsigned int uiStream, unsigned int uiVertex, unsigned char d1, unsigned char d2, unsigned char d3, unsigned char d4)
{
  if (uiStream >= m_Streams.size ())
    throw exception ("kvVertexBuffer::SetVertexAttribute4ub: The given stream index is larger than the number of allocated streams.");
  if (uiVertex >= m_uiVertexCount)
    throw exception ("kvVertexBuffer::SetVertexAttribute4ub: The given vertex index is larger than the number of allocated vertices.");

  // this offset is in bytes
  const unsigned int uiDataOffset = uiVertex * m_uiVertexDataSize + m_Streams[uiStream].m_uiOffset;

  unsigned char* pData = (unsigned char*) &m_Data[uiDataOffset];
  pData[0] = d1;
  pData[1] = d2;
  pData[2] = d3;
  pData[3] = d4;
}

void kvVertexBuffer::UploadToGPU (void)
{
  glGenBuffers (1, &m_uiBufferID);
  glBindBuffer (GL_ARRAY_BUFFER, m_uiBufferID);
  glBufferData (GL_ARRAY_BUFFER, m_Data.size (), &m_Data[0], GL_STATIC_DRAW);

  glGenBuffers (1, &m_uiIndexID);
  glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, m_uiIndexID);
  glBufferData (GL_ELEMENT_ARRAY_BUFFER, m_IndexData.size () * sizeof (int), &m_IndexData[0], GL_STATIC_DRAW);

  Activate ();
}

void kvVertexBuffer::Activate (void)
{
  s_pActiveBuffer = this;

  glBindBuffer (GL_ARRAY_BUFFER, m_uiBufferID);
  glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, m_uiIndexID);
}

void kvVertexBuffer::ConnectBufferWithShader (kvShader* pShader)
{
  Activate ();

  if (pShader == NULL)
    return;

  // disable all arrays
  for (int i = 0; i < 16; ++i)
    glDisableVertexAttribArray (i);
  
  // now enable all the used ones and set their respective pointers
  for (size_t i = 0; i < m_Streams.size (); ++i)
  {
    int iLocation = pShader->GetAttributeLocation (m_Streams[i].m_sStreamName.c_str ());

    if (iLocation >= 0)
    {
      const StreamType Type = m_Streams[i].m_Type;
      const unsigned int uiStride = m_uiVertexDataSize;
      const unsigned int uiOffset = m_Streams[i].m_uiOffset;

      glEnableVertexAttribArray (iLocation);
      glVertexAttribPointer (iLocation, GetStreamTypeElements (Type), GetStreamTypeElementType (Type), GetStreamTypeElementsNormalized (Type), uiStride, (void *) uiOffset);
    }
  }
}

