#ifndef AE_FOUNDATION_FILESYSTEM_FILESYSTEM_H
#define AE_FOUNDATION_FILESYSTEM_FILESYSTEM_H

#include "../Communication/Event.h"
#include "../Strings/HybridString.h"
#include "../Strings/StaticString.h"
#include "../Strings/PathFunctions.h"
#include "../Strings/String.h"
#include "../Containers/Map.h"
#include "../FileSystem/DataDirectory.h"

namespace AE_NS_FOUNDATION
{
  //! Function-Pointer: Provides a file-reader for some file. Called for a registered file-type to overwrite the default policy.
  typedef aeDataDirectoryFileReader* (*AE_FILE_TYPE_READER_HOOK)(const char* szFile);
  //! Function-Pointer: Called upon aeFileSystem::AddDataDirectory to create a aeDataDirectoryHandler. Returns true, if successful.
  typedef bool (*AE_DATA_DIRECTORY_HANDLER_PROVIDER)(const char* szFile, const char* szCategory, bool bAllowWrite);

  //! Enum that describes the type of file-event that occured.
  struct aeFileEventType
  {
      enum Enum
    {
      None,												//!< None. Should not occure.
      OpenFile,										//!< A file was opened for reading.
      OpenFileHook,							  //!< A file hook is to be executed.
      OpenFileFailed,						  //!< Opening a file for reading failed. Probably because it doesn't exist.
      CreateFile,									//!< A file was opened for writing.
      CreateFileFailed,					  //!< Opening a file for writing failed.
      CloseFile,									//!< A file was closed.
      AddDataDirectory,					  //!< A data directory was added.
      AddDataDirectoryFailed,		  //!< Adding a data directory failed. No provider could handle it (or there were none).
      AddDataDirectoryHandler,	  //!< A data-directory handler was added. This usually happens together with aeFileEventType::AddDataDirectory, but can happen more than once each time.
      RemoveDataDirectory,				//!< A data directory was removed. IMPORTANT: This is where ResourceManagers should check if some loaded resources need to be purged.
      // TODO delete file event
      // TODO delete file hook ??
    };
  };


  //! Static class that handles file-accesses.
  class AE_FOUNDATION_DLL aeFileSystem
  {
  public:
    //! Call this to register some directory as a "data-directory".
    static void AddDataDirectory (const char* szPath, const char* szDirCategory = "", bool bAllowWrite = true);
    //! Adds a data-directory by directly specifying a handler. Usually only used by AE_DATA_DIRECTORY_HANDLER_PROVIDER's.
    static void AddDataDirectoryHandler (aeDataDirectoryHandler* pDirectoryHandler, const char* szDirCategory, const char* szDataDirectoryGroup, bool bAllowWrite);
    //! Call this to register a data-directory-provider, which will be called when AddDataDirectory is executed.
    static void AddDataDirectoryHandlerProvider (AE_DATA_DIRECTORY_HANDLER_PROVIDER Provider);
    //! Remove the data-directory with the given path. Must be exactly the same path, as provided earlier to AddDataDirectory.
    static void RemoveDataDirectory (const char* szPath);

    //! Opens a file for writing. Returns a directory-writer or NULL. Usually not to be called directly, but by some file-abstraction.
    static aeDataDirectoryFileWriter* OpenFileToWrite (const char* szFile/*, bool bReplacingTokens = false*/);
    //! Opens a file for reading. Returns a directory-reader or NULL. Usually not to be called directly, but by some file-abstraction.
    static aeDataDirectoryFileReader* OpenFileToRead (const char* szFile, bool bAllowReaderHooks = true/*, bool bReplacingTokens = false*/);

    //! Sets the hook (callback) to be called whenever a file of a certain type shall be opened. Pass NULL to remove a hook.
    static void SetFileTypeReaderHook (const char* szFileExtension, AE_FILE_TYPE_READER_HOOK Hook);

    //! Allows to register a function as an event receiver. All receivers will be notified in the order that they registered.
    static void RegisterEventReceiver (AE_EVENT_NOTIFICATION_CALLBACK callback, void* pPassThrough)   { m_FileEvent.RegisterEventReceiver   (callback, pPassThrough); }
    //! Unregisters a previously registered receiver. It is an error to unregister a receiver that was not registered.
    static void UnregisterEventReceiver (AE_EVENT_NOTIFICATION_CALLBACK callback, void* pPassThrough) { m_FileEvent.UnregisterEventReceiver (callback, pPassThrough); }

    //! Registers a path-token. Allows to use $TOKENS$ in a path, that will be successively replaced, until an existing file is found. The token must be upper-case.
    //static void AddPathToken (const char* szToken, const char* szReplacement);

    //! Deletes the given file from all data-directories where possible.
    static void Deletefile (const char* szFile);
    //! Checks if the given file exists (by trying to read it).
    static bool ExistsFile (const char* szFile);

    //! Attempts to create the given folder-structure (tree). Returns true if it could be created in ANY data-directory.
    static bool CreateFolders (const char* szFile);

    
    static bool MakeValidPath (const char* szPath, bool bForWriting, aeFilePath* out_sAbsolutePath, aeFilePath* out_sDataDirRelativePath);

    //void EnumFilesInDirectory (const char* szDirectory, bool bRecursive, CALLBACK cb, void* pPassThrough = NULL);

    //! The data that is sent through the event interface.
    struct aeFileEvent
    {
      aeFileEvent (void);

      //! The exact event that occured.
      aeFileEventType::Enum m_EventType;
      //! Path to the file or directory that was involved.
      const char* m_szFileOrDirectory;
      //! Additional Path / Name that might be of interest.
      const char* m_szOther;
      //! The data-directory handler, that was involved.
      const aeDataDirectoryHandler* m_pDataDirHandler;
    };

  private:
    static aeUInt32 aeFileSystem::ExtractDataDirsToSearch (const aeBasicString& sPath, aeArray<aeHybridString<16> >& SearchDirs);

    //! Data for each data-directory.
    struct aeDataDir
    {
      //! The category name.
      aeHybridString<32> m_sCategory;
      //! The handler for the data-directory.
      aeDataDirectoryHandler* m_pHandler;
      //! A group-name linking data-directories together, such that when one is removed, the dependent ones are also removed.
      aeFilePath m_sDataDirectoryGroup;
      //! True if data is allowed to be written to that directory.
      bool m_bAllowWrite;
    };

    //! Data for one path-token.
    //struct aePathToken
    //{
    //  //! Name of the token. Is always upper-case.
    //  aeHybridString<16> m_sPathToken;
    //  //! The replacement for the token.
    //  aeHybridString<32> m_sReplacement;
    //};

    friend void aeDataDirectoryFileReader::Close (void);
    friend void aeDataDirectoryFileWriter::Close (void);

    //! Event object for all file-events.
    static aeEvent m_FileEvent;

    //! All currently registered data-directories.
    static aeArray<aeDataDir> m_DataDirectories;
    //! All currently registered file-reader hooks.
    static aeMap<aeFileExtension, AE_FILE_TYPE_READER_HOOK> m_FileTypeHooks;
    //! All data-directory providers.
    static aeArray<AE_DATA_DIRECTORY_HANDLER_PROVIDER> m_DataDirHandlerProviders;
    //! All known path-tokens.
    //static aeArray<aePathToken> s_PathTokens;

    //typedef bool (*AE_PATH_ACTION)(const char* szFile, void* pPassThrough);

    //static bool ExecuteActionOnPath (const char* szPathWithTokens, AE_PATH_ACTION Action, void* pPassThrough);
  };

  //! Describes the status of the file-open operation
  struct aeThreadedFileStatus
  {
    enum Enum
    {
      Pending,			//!< File is not yet opened
      Success,			//!< Openinig the file was successful
      Failure,			//!< Could not open/find the file
    };
  };

}

#endif
