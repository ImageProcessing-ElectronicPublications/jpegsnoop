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

typedef char TCHAR;

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
#define _tcscpy_s(A, B, C) strncpy((char *)A, (char *)C, B)
#define _tcsnccmp(A, B, C) strncmp((char *)A, (char *)C, B)
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
        /* Obsolete */
    }

    CFile(void * Data, uint64_t Size, char * Path)
    {
        this->data = (uint8_t *)Data;
        this->position = 0;
        this->size = Size;
        this->path = Path;
    }

    ~CFile()
    {
        return;
    }

    void Close()
    {
        return;
    }

    void Write(void * Data, uint64_t Bytes)
    {
        return;
    }

    CString GetFilePath()
    {
        return path;
    }

    uint64_t Read(void * Out, uint64_t Count)
    {
        memcpy(Out, data+position, Count);
        position += Count;
    }

    uint64_t Seek(int64_t Offset, int From)
    {
        if (From == SEEK_SET)
        {
            position = Offset;
        }
        else if (From == SEEK_CUR)
        {
            position += Offset;
        }
        else if (From == SEEK_END)
        {
            position = size + Offset;
        }

        return 0;
    }

    CString GetFileName()
    {
        return path;
    }

    int64_t GetLength()
    {
        return size;
    }

private:
    uint8_t * data;
    uint64_t size;
    uint64_t position;
    CString path;
};

#endif