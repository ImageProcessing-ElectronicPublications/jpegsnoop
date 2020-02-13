#ifndef _WindowsClasses_h_
#define _WindowsClasses_h_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <wchar.h>
#include <algorithm>



#define TRUE 1
#define FALSE 0

typedef void CDC;
typedef void CStdioFile;
typedef void CFont;

typedef char * LPTSTR;
typedef uint8_t BYTE;
#define byte BYTE
typedef BYTE * PBYTE;
typedef uint32_t DWORD, *PDWORD, *LPDWORD;
typedef DWORD UINT;
typedef uint16_t WORD;
typedef char * LPCTSTR;
typedef wchar_t* LPWSTR, *PWSTR;
 typedef char* PSTR, *LPSTR;
typedef int BOOL;
typedef uint64_t ULONGLONG;
typedef DWORD COLORREF;
#define RGB(R, G, B) ((COLORREF)(((B) << 16) |  ((G) << 8) | (R)))

typedef struct tagRGBQUAD {
  BYTE rgbBlue;
  BYTE rgbGreen;
  BYTE rgbRed;
  BYTE rgbReserved;
} RGBQUAD;

typedef void * HANDLE;

#include "stdstr.h"
typedef CStdStr<char>       CString;
typedef CStdStr<char>		CStringA;	// a better std::string
typedef CStdStr<uint16_t>	CStringW;	// a better std::wstring
typedef CStdStr<OLECHAR>	CStringO;	// almost always CStdStringW

#define min(a,b) ( ((a)<(b)) ? (a):(b) )
#define max(a,b) ( ((a)>(b)) ? (a):(b) )

class CObject {};

/* Do Nothing */
#define _T(FGJKL) (FGJKL)


#define STRING_LENGTH 4096


#define StringAppend(_String, _Append) strcpy(_String+strlen(_String), _Append)
#define StringAppendChar(_String, _AppendChar) _String[strlen(_String)] = _AppendChar

#define _tcscmp(A, B) strcmp(A, B)
#define _tcslen(A) strlen(A)
#define _tcscpy_s(A, B, C) wcsncpy((wchar_t *)A, (wchar_t *)C, B)
#define _tcsnccmp(A, B, C) strcmp((wchar_t *)A, (wchar_t *)C, B)
/* "returns nonzero if c is a particular representation of a printable character" */
#define _istprint(c) ((c < 10 && c >= 0) ? 1 : 0)

class CSize
{
public:
    CSize(int x, int y)
    {
        this->cx = x;
        this->cy = y;
    }
public:
    int cx,cy;
};

class CPoint
{
public:
    CPoint(int initX, int initY)
    {
        this->x = initX;
        this->y = initY;
    }
    CPoint()
    {
        this->x = 0;
        this->y = 0;
    }

public:
    int x, y;
};

class CRect
{
public:
    CRect(int l, int t, int r, int b)
    {
        this->l = l;
        this->t = t;
        this->r = r;
        this->b = b;
    }
    CRect(CPoint point, CSize size)
    {
        this->l = point.x;
        this->t = point.y + size.cy;
        this->r = point.x + size.cx;
        this->b = point.y;
    }
    CRect()
    {
        this->l = 0;
        this->t = 0;
        this->r = 0;
        this->b = 0;
    }
    CRect(const CRect & srcRect)
    {
        this->l = srcRect.l;
        this->t = srcRect.t;
        this->r = srcRect.r;
        this->b = srcRect.b;
    }

public:
    int l, t, r, b;
};


class CFile
{
public:

    enum {modeRead,modeWrite,modeReadWrite,modeCreate};
    enum {typeBinary,typeText,typeUnicode};
    enum {shareDenyNone,shareDenyRead,shareDenyWrite,shareExclusive};
    enum {begin=SEEK_SET, current=SEEK_CUR, end=SEEK_END};

    CFile(char * filepath, UINT OpenFlags)
    {
        this->c_file = fopen(filepath, "rb+");
        this->path = (char *)malloc(strlen(filepath)+1);
        strcpy(this->path, filepath);
    }

    ~CFile()
    {
        if (this->c_file) fclose(this->c_file);
    }

    void Close()
    {
        if (this->c_file) fclose(this->c_file);
        this->c_file = NULL;
    }

    void Write(void * Data, uint64_t Bytes)
    {
        fwrite(Data, 1, Bytes, this->c_file);
    }

    CString GetFilePath()
    {
        return CString(this->path);
    }

    uint64_t Read(void * Out, uint64_t Count)
    {
        return fread(Out, 1, Count, this->c_file);
    }

    uint64_t Seek(uint64_t Offset, int From)
    {
        int ret = fseek(c_file, Offset, From);

        if (!ret) return ftell(c_file);
        else return -10000;
    }

    CString GetFileName()
    {
        char * filepath = this->path;
        filepath += strlen(filepath);
        while (filepath[-1] != '/') filepath--;
        return CString(filepath);
    }

    int64_t GetLength()
    {
        int64_t original_pos = ftell(c_file);
        fseek(c_file, 0, SEEK_END);
        int64_t retval = ftell(c_file);
        fseek(c_file, original_pos, SEEK_SET);
        return retval;
    }

private:
    FILE * c_file;
    char * path;
};

#endif