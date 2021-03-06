#include "PCH.h"

#include <KrautGraphics/FileSystem/FileSystem.h>

bool ConvertTGAtoDDS(const char* szFile, bool bNormalMap)
{
  STARTUPINFO si;
  PROCESS_INFORMATION pi;

  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(STARTUPINFO);
  ZeroMemory(&pi, sizeof(pi));

  char szProgram[2048];

  aeString p = aePathFunctions::ChangeExistingFileExtension(szFile, "dds");

  aeString sEXE;
  if (!aeFileSystem::MakeValidPath("DDS\\nvdxt.exe", false, &sEXE, nullptr))
    return false;

  aeString sOutDir = aePathFunctions::GetFileDirectory(p);
  aeString sOutFile = aePathFunctions::GetFileNameAndExtension(p);

  if (bNormalMap)
    aeStringFunctions::Format(szProgram, 2048, "\"%s\" -file \"%s\" -outdir \"%s\" -outfile \"%s\" -quality_production -profile normalmap.dpf", sEXE.c_str(), szFile, sOutDir.c_str(), sOutFile.c_str());
  else
    aeStringFunctions::Format(szProgram, 2048, "\"%s\" -file \"%s\" -outdir \"%s\" -outfile \"%s\" -quality_production -profile diffuse.dpf", sEXE.c_str(), szFile, sOutDir.c_str(), sOutFile.c_str());

  aeString sCWD = aePathFunctions::GetFileDirectory(sEXE);

  if (!CreateProcess(nullptr, (LPSTR)szProgram, nullptr, nullptr, false, CREATE_NO_WINDOW, nullptr, sCWD.c_str(), &si, &pi))
    return (false);

  // Wait until child process exits.
  WaitForSingleObject(pi.hProcess, INFINITE);

  DWORD ret = 0;
  GetExitCodeProcess(pi.hProcess, &ret);

  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);

  if (ret != 0)
    return false;

  return true;
}
