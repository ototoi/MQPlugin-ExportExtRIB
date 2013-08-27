#ifndef _FILELIB_H
#define _FILELIB_H

#ifdef __cplusplus

extern "C"
{
#endif

BOOL ChangeFileExtension(LPSTR DstFile, LPCSTR SrcFile, LPCSTR Extend);
BOOL GetFileExtension(LPSTR DstFile, LPCSTR SrcFile);
BOOL GetFileNameFromPath(LPSTR DstFile, LPCSTR SrcFile);
BOOL GetFolderFromPath(LPSTR DstPath, LPCSTR FileName);
BOOL GetFileNameAndExt(LPSTR DstFile, LPCSTR FileName);
BOOL GetUpFolder(LPSTR DstPath, LPCSTR SrcPath);

#ifdef __cplusplus
}
#endif

#endif //_FILELIB_H
