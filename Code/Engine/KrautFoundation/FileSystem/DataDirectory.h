#ifndef AE_FOUNDATION_FILESYSTEM_DATADIRECTORY_H
#define AE_FOUNDATION_FILESYSTEM_DATADIRECTORY_H

#include "../Communication/Event.h"
#include "../Strings/HybridString.h"
#include "../Strings/StaticString.h"
#include "../Strings/PathFunctions.h"
#include "../Strings/String.h"
#include "../Containers/Map.h"

namespace AE_NS_FOUNDATION
{
  class aeDataDirectoryHandler;

  //! Interface class for reading from a data-directory.
  class AE_FOUNDATION_DLL aeDataDirectoryFileReader
  {
  public:
    //! Constructor, takes the directory-handler and file-name.
    aeDataDirectoryFileReader (aeDataDirectoryHandler* pHandler, const char* szFile) : m_pDirectoryHandler (pHandler), m_sFilePath (szFile) {}
    //! Virtual Destructor. Does nothing.
    virtual ~aeDataDirectoryFileReader () {}

    //! Opens the file given in the constructor. Returns true, if successful.
    virtual bool Open (void) = 0;
    //! Reads the given number of bytes into the buffer. Returns how many bytes were actually read.
    virtual aeUInt32 Read (void* pBuffer, aeUInt32 uiBytes) = 0;
    //! Closes the file and deletes (!) the file reader. Do not access it afterwards anymore, AT ALL!
    void Close (void);

    //! Returns the full, absolute path of the opened file.
    const aeFilePath& GetFilePath (void) const { return (m_sFilePath); }
    //! Returns the handler through which this reader was created.
    const aeDataDirectoryHandler* GetDataDirectoryHandler (void) const { return (m_pDirectoryHandler); }

  private:
    //! Closes the file reader. Called by "Close".
    virtual void CloseFile (void) = 0;
  
    //! The directory handler. Set in the constructor.
    aeDataDirectoryHandler* m_pDirectoryHandler;
    //! The full, absolute path to the file that is opened by this reader.
    aeFilePath m_sFilePath;

  private:
    aeDataDirectoryFileReader (const aeDataDirectoryFileReader& cc);
    void operator= (const aeDataDirectoryFileReader& rhs);
  };

  //! Interface class for writing to a file in a data-directory.
  class AE_FOUNDATION_DLL aeDataDirectoryFileWriter
  {
  public:
    //! Constructor, takes the directory-handler and file-name.
    aeDataDirectoryFileWriter (aeDataDirectoryHandler* pHandler, const char* szFile) : m_pDirectoryHandler (pHandler), m_sFilePath (szFile) {}
    //! Virtual Destructor. Does nothing.
    virtual ~aeDataDirectoryFileWriter () {}

    //! Opens the file given in the constructor. Returns true, if successful.
    virtual bool Open (void) = 0;
    //! Writes the given number of bytes from the buffer to the file.
    virtual void Write (const void* pBuffer, aeUInt32 uiBytes) = 0;
    //! Closes the file and deletes (!) the file writer. Do not access it afterwards anymore, AT ALL!
    void Close (void);

    //! Returns the full, absolute path of the opened file.
    const aeFilePath& GetFilePath (void) const { return (m_sFilePath); }
    //! Returns the handler through which this writer was created.
    const aeDataDirectoryHandler* GetDataDirectoryHandler (void) const { return (m_pDirectoryHandler); }

  private:
    //! Closes the file reader. Called by "Close".
    virtual void CloseFile (void) = 0;

    //! The directory handler. Set in the constructor.
    aeDataDirectoryHandler* m_pDirectoryHandler;
    //! The full, absolute path to the file that is opened by this reader.
    aeFilePath m_sFilePath;

  private:
    aeDataDirectoryFileWriter (const aeDataDirectoryFileWriter& cc);
    void operator= (const aeDataDirectoryFileWriter& rhs);
  };


  //! Interface class for a data-directory handler.
  class AE_FOUNDATION_DLL aeDataDirectoryHandler
  {
  public:
    //! Constructor. Takes the path to the directory to manage.
    aeDataDirectoryHandler (const char* szDirectory) : m_sDirectory (szDirectory) {}
    //! Virtual Destructor. Does nothing.
    virtual ~aeDataDirectoryHandler () {}

    //! Deletes the given file. Does not guarantee success, some data-directories are read-only.
    virtual void Deletefile (const char* szFile) = 0;

    //! Attempts to create the whole file-path chain from the to a given file or folder. Returns true if succesful (or the path already exists).
    bool CreateFolderStructure (const char* szPathToFileOrFolder);

    //! \todo Implement this
    //virtual aeUInt32 GetFileModificationTime (const char* szFile) = 0;
    //! \todo Implement this
    //virtual void EnumFilesInDirectory (const char* szDirectory, bool bRecursive, CALLBACK, void* pPassThrough = NULL) = 0;

    //! Opens the given file for reading. Returns a directory-reader, if successful. NULL otherwise.
    virtual aeDataDirectoryFileReader* OpenFileToRead (const char* szFile) = 0;
    //! Opens the given file for writing. Returns a directory-writer, if successful. NULL otherwise.
    virtual aeDataDirectoryFileWriter* OpenFileToWrite (const char* szFile) = 0;

    //! Returns the directory that this handler manages.
    const aeFilePath& GetDataDirectory (void) const { return (m_sDirectory); }

  private:
    //! Creates the given directory. Does not need to be able to do this recursively. Returns false if unsuccesful.
    virtual bool CreateFolder (const char* szDirectory, const char* szNewDirectory) = 0;

    //! Path to the directory to manage.
    aeFilePath m_sDirectory;

  private:
    aeDataDirectoryHandler (void);
    aeDataDirectoryHandler (const aeDataDirectoryHandler& cc);
    void operator= (const aeDataDirectoryHandler& rhs);
  };

}

#endif
