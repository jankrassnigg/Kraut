#ifndef VERTEXBUFFER_H
#define VERTEXBUFFER_H

#include <vector>
#include <string>
#include <deque>
#include <map>

class kvShader;

// Abstracts away an OpenGL Vertex Buffer with Index Buffer
class kvVertexBuffer
{
public:
  kvVertexBuffer ();
  ~kvVertexBuffer ();

  enum StreamType
  {
    R32G32B32A32_FLOAT,   // float: x,y,z,w
    R32G32B32_FLOAT,      // float: x,y,z (position etc.)
    R32G32_FLOAT,         // float: x,y (like texcoords)
    R32_FLOAT,            // single float
    R8G8B8A8_UNORM,       // unsigned byte: r,g,b,a (color etc.)
  };

  // Adds another stream to the buffer. Give the stream a name with which it should be bound to the shader later.
  unsigned int AddVertexStream (const char* szName, StreamType Type);

  // Defines how many vertices and triangles the buffer should hold.
  void SetupBuffer (unsigned int uiVertexCount, unsigned int uiTriangleCount);

  // Sets the data for one vertex in one of the attribute-arrays.
  void SetVertexAttribute1f (unsigned int uiStream, unsigned int uiVertex, float d1);
  // Sets the data for one vertex in one of the attribute-arrays.
  void SetVertexAttribute2f (unsigned int uiStream, unsigned int uiVertex, float d1, float d2);
  // Sets the data for one vertex in one of the attribute-arrays.
  void SetVertexAttribute3f (unsigned int uiStream, unsigned int uiVertex, float d1, float d2, float d3);
  // Sets the data for one vertex in one of the attribute-arrays.
  void SetVertexAttribute4f (unsigned int uiStream, unsigned int uiVertex, float d1, float d2, float d3, float d4);
  // Sets the data for one vertex in one of the attribute-arrays.
  void SetVertexAttribute4ub (unsigned int uiStream, unsigned int uiVertex, unsigned char d1, unsigned char d2, unsigned char d3, unsigned char d4);

  // Sets the three indices for one triangle.
  void SetTriangle (unsigned int uiTriangle, unsigned int uiVertex1, unsigned int uiVertex2, unsigned int uiVertex3);

  // After all the data has been setup, call this to finally create the GPU side buffer.
  void UploadToGPU (void);

  // Call this to bind this vertex buffer.
  void Activate (void);

  // Returns the currently active vertex buffer.
  static kvVertexBuffer* GetActiveBuffer (void) { return s_pActiveBuffer; }

  // Sets the vertex arrays up so that the given shader can use them.
  void ConnectBufferWithShader (kvShader* pShader);

  // Returns the number of triangles that this buffer stores
  unsigned int GetNumTriangles (void) const { return m_uiTriangleCount; }

private:
  unsigned int m_uiVertexCount;
  unsigned int m_uiTriangleCount;
  unsigned int m_uiVertexDataSize;

  struct kvStream
  {
    string m_sStreamName;
    StreamType m_Type;
    unsigned int m_uiOffset;
  };

  deque<kvStream> m_Streams;
  vector<unsigned char> m_Data;
  vector<unsigned int> m_IndexData;

  unsigned int m_uiBufferID;
  unsigned int m_uiIndexID;

  static kvVertexBuffer* s_pActiveBuffer;
};

#endif

