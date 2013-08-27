#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>

//------------------------------------------------------------------
//  ChangeFileExtension       1999/3/1
//    拡張子を変更する(Extendは先頭にピリオドを含む文字列)
//------------------------------------------------------------------
BOOL ChangeFileExtension(LPSTR DstFile, LPCSTR SrcFile, LPCSTR Extend)
{
#if 1
	int len;
	const char *ptr;
	char path[MAX_PATH];

	len = lstrlen(SrcFile);
	memcpy(path, SrcFile, len+1);
	for(ptr=&path[len]; ptr>path; )
	{
		ptr = CharPrev(path, ptr);
		if(*ptr == '.') {
			lstrcpy(DstFile, path);
			lstrcpy(&DstFile[(unsigned)(ptr-path)], Extend);
			return TRUE;
		}
		if(*ptr=='\\' || *ptr=='/'){
			lstrcpy(DstFile, path);
			lstrcpy(&DstFile[len], Extend);
			return TRUE;
		}
	}
	// not find either period or \, so add extension
	lstrcpy(DstFile, path);
	lstrcpy(&DstFile[len], Extend);
	return TRUE;
#else
	int i;
	for(i=lstrlen(SrcFile)-1; i>=0; i--){
		if(SrcFile[i] == '.')break;
	}
	if(i == -1) i = lstrlen(SrcFile);
	lstrcpy(DstFile, SrcFile);
	lstrcpy(&DstFile[i], Extend);
	return TRUE;
#endif
}	//ChangeFileExtension()

//------------------------------------------------------------------
//  GetFileExtension       1999/3/1
//    パスから拡張子を取り出す
//------------------------------------------------------------------
BOOL GetFileExtension(LPSTR DstFile, LPCSTR SrcFile)
{
#if 1
	int len;
	const char *ptr;
	char path[MAX_PATH];

	len = lstrlen(SrcFile);
	memcpy(path, SrcFile, len+1);
	for(ptr=&path[len]; ptr>path; )
	{
		ptr = CharPrev(path, ptr);
		if(*ptr == '.') {
			lstrcpy(DstFile, ptr+1);
			return TRUE;
		}
		if(*ptr=='\\' || *ptr=='/')
			break;
	}
	DstFile[0] = 0;
	return TRUE;
#else
	int i;
	for(i=lstrlen(SrcFile)-1; i>=0; i--){
		if(SrcFile[i] == '.')break;
	}
	if(i==-1 || i==lstrlen(SrcFile)-1){DstFile[0] = 0; return FALSE;}
	lstrcpy(DstFile, &SrcFile[i+1]);
	return TRUE;
#endif
}	//GetFileExtension()

//------------------------------------------------------------------
//  GetFileNameFromPath       1999/3/1
//    パスからファイル名(拡張子除く)を取り出す
//------------------------------------------------------------------
BOOL GetFileNameFromPath(LPSTR DstFile, LPCSTR SrcFile)
{
#if 1
	int len;
	const char *ptr, *endptr;
	char path[MAX_PATH];

	len = lstrlen(SrcFile);
	memcpy(path, SrcFile, len+1);
	endptr = &path[len];
	for(ptr=endptr; ptr>path; )
	{
		ptr = CharPrev(path, ptr);
		if(*ptr == '.' && endptr == &path[len])
			endptr = ptr;
		if(*ptr=='\\' || *ptr=='/'){
			ptr = CharNext(ptr);
			break;
		}
	}
	if(ptr >= endptr)
		return FALSE;
	memcpy(DstFile, ptr, (int)(endptr-ptr));
	DstFile[(unsigned)(endptr-ptr)] = 0;
	return TRUE;
#else
	int i, j, period;
	for(i=lstrlen(SrcFile)-1, j=0, period=0; i>=0; i--, j++){
		if(SrcFile[i]=='\\' || SrcFile[i]=='/')break;
		if(SrcFile[i]=='.' && !period){j=0; period=1;}
	}
	if(i == lstrlen(SrcFile)-1)
		{DstFile[0] = 0; return FALSE;}
	lstrcpyn(DstFile, &SrcFile[i+1], j);
	return TRUE;
#endif
}	//GetFileNameFromPath()

//------------------------------------------------------------------
//  GetFolderFromPath   1999/3/1
//    パスからフォルダ名(終端に必ず\を含む)のみを取り出す
//------------------------------------------------------------------
BOOL GetFolderFromPath(LPSTR DstPath, LPCSTR FileName)
{
#if 1
	int len;
	const char *ptr;
	char path[MAX_PATH];

	len = lstrlen(FileName);
	memcpy(path, FileName, len+1);
	for(ptr=&path[len]; ptr>path; )
	{
		ptr = CharPrev(path, ptr);
		if(*ptr=='\\' || *ptr=='/'){
			memcpy(DstPath, path, (int)(ptr-path));
			if(DstPath[(unsigned)(ptr-path)-1] != '\\'
			&& DstPath[(unsigned)(ptr-path)-1] != '/'){
				DstPath[(unsigned)(ptr-path)] = '\\';
				DstPath[(unsigned)(ptr-path)+1] = 0;
			} else
				DstPath[(unsigned)(ptr-path)] = 0;
			return TRUE;
		}
	}
	return FALSE;
#else
	int r, w, d=0;
	char path[MAX_PATH];
	for(w=r=0; FileName[r]!=0; r++){
		path[r] = FileName[r];
		if(FileName[r]=='\\' || FileName[r]=='/'){
			w = r;
			d++;
		}
	}
	if(d<=1){
		path[w] = '\\';
		path[w+1] = 0;
	} else
		path[w] = 0;
	lstrcpy(DstPath, path);
	return TRUE;
#endif
}	//GetFolderFromPath()

//------------------------------------------------------------------
//  GetFileNameAndExt   1999/3/1
//    パスからファイル名と拡張子を取り出す
//------------------------------------------------------------------
BOOL GetFileNameAndExt(LPSTR DstFile, LPCSTR FileName)
{
#if 1
	int len;
	const char *ptr, *endptr;
	char path[MAX_PATH];

	len = lstrlen(FileName);
	memcpy(path, FileName, len+1);
	endptr = &path[len];
	for(ptr=endptr; ptr>path; )
	{
		ptr = CharPrev(path, ptr);
		if(*ptr=='\\' || *ptr=='/'){
			ptr = CharNext(ptr);
			break;
		}
	}
	if(ptr >= endptr)
		return FALSE;
	lstrcpy(DstFile, ptr);
	return TRUE;
#else
	int r, w;
	for(w=r=0; FileName[r]!=0; r++){
		if(FileName[r]=='\\' || FileName[r]=='/')
			w = r+1;
	}
	lstrcpy(DstFile, &FileName[w]);
	return TRUE;
#endif
}	//GetFileNameAndExt()

//------------------------------------------------------------------
//  GetUpFolder   1999/9/9
//    一つ上のフォルダ名(終端に\含む)を得る。
//------------------------------------------------------------------
BOOL GetUpFolder(LPSTR DstPath, LPCSTR SrcPath)
{
	int len;
	const char *ptr, *endptr;

	len = lstrlen(SrcPath);
	if(len == 0)
		return FALSE;
	endptr = &SrcPath[len];
	ptr = CharPrev(SrcPath, endptr);
	if(*ptr == '\\' || *ptr=='/')
		ptr = CharPrev(SrcPath, ptr);
	for(; ptr>SrcPath; ptr=CharPrev(SrcPath,ptr))
	{
		if(*ptr=='\\' || *ptr=='/') {
			len = (int)(ptr - SrcPath) + 1;
			memcpy(DstPath, SrcPath, len);
			DstPath[len] = 0;
			return TRUE;
		}
	}
	return FALSE;
}
