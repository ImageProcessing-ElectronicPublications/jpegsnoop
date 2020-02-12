// cstr.h [02 Oct 1998]
//

#ifndef _CSTR_INCLUDED_
#define _CSTR_INCLUDED_

// // Define windows.h in case we're the first header. If MFC is used,
// //   stdafx.h (or an equivalent) MUST be included before us, otherwise
// //   terrible things may happen.  Most people use precompiled headers
// //   with MFC, so I don't expect this to be a huge problem.
// #include <windows.h>
#include <stdarg.h>

// Define CSTR_LARGE_STRINGS to support >65534 chars
#ifndef CSTR_LARGE_STRINGS
typedef WORD CPOS;
#else
typedef UINT CPOS;
#endif

// Define CSTR_NOT_MT_SAFE to increase speed if you use strings
//   in _only_ one thread (or you're not multithreaded at all)

// Callers define CSTR_NO_WINSTUFF to exclude MFC window add-ons
#ifndef __AFX_H__
#define CSTR_NO_WINSTUFF
#endif

// If calling LoadString and not using MFC, implement get_stringres
HANDLE get_stringres();

// The structure below is implementation specific, do not
//   attempt to use it directly!
#pragma pack(1)
typedef struct SDesc {
	// Data zone
	UINT	m_Usage;	// Usage counter, 0 in empty slots, always 2 in m_Null
	char*	m_Text;		// 0-terminated string, or next free SDesc* if m_Usage==0
	CPOS	m_Length;	// Actual string length, excl. 0
	CPOS	m_Alloc;	// Allocated length, excl. 0
} SDesc;
#pragma pack()
#define CSTR_DITEMS 320		/* Max bytes to be kept in blocks pool */


/*********************************************************************
* Class:	CStr
* Purpose:	Represents a string in your program.  Does not have virtual
*			methods and contains exactly 4 bytes (a pointer to a
*			data structure held internally by the string manager).
*			Can be typecasted to 'const char*' to obtain a read-only
*			zero-terminated copy of the string.  Can be copied and
*			assigned freely.
*********************************************************************/

class CStr
{
// Data block and internal access methods
private:
	SDesc* data;
	static void  freestr(SDesc* tofree);
	static void  freestr_nMT(SDesc* tofree);

// Construction, copying, assignment
public:
	CStr();
	CStr(const CStr& source);
	CStr(const char* s, CPOS prealloc = 0);
	CStr(CPOS prealloc);
	const CStr& operator=(const CStr& source);
	const CStr& operator=(const char* s);
	~CStr();
#ifdef __AFX_H__
	CStr(const CString& source, CPOS prealloc = 0);
	const CStr& operator=(const CString& source);
#endif

// Get attributes, get data, compare
#ifndef CSTR_LARGE_STRINGS
	enum { MAXCHARS = 65530 };
#endif
	BOOL IsEmpty() const;
	CPOS GetLength() const;
	operator const char* () const;
	const char* GetString() const;			// Same as above
	char GetFirstChar() const;
	char GetLastChar() const;
	char operator[](CPOS idx) const;
	char GetAt(CPOS idx) const;				// Same as above
	void GetLeft (CPOS chars, CStr& result);
	void GetRight (CPOS chars, CStr& result);
	void GetMiddle (CPOS start, CPOS chars, CStr& result);
	int  Find (char ch, CPOS startat = 0) const;
	int  ReverseFind (char ch, CPOS startat = (CPOS) -1) const;
	int  Compare (const char* match) const;			// -1, 0 or 1
	int  CompareNoCase (const char* match) const;	// -1, 0 or 1
	// Operators == and != are also predefined

// Global modifications
	void Empty();			// Sets length to 0, but keeps buffer around
	void Reset();			// This also releases the buffer
	void GrowTo(CPOS size);
	void Compact(CPOS only_above = 0);
	static void CompactFree();
	void Format(const char* fmt, ...);
	void FormatRes(UINT resid, ...);
	BOOL LoadString(UINT resid);

// Catenation, truncation
	void operator += (const CStr& obj);
	void operator += (const char* s);
	void operator += (const char ch);		// Same as AddChar
	void AddString(const CStr& obj);		// Same as +=
	void AddString(const char* s);			// Same as +=
	void AddChar(char ch);
	void AddChars(const char* s, CPOS startat, CPOS howmany);
	void AddStringAtLeft(const CStr& obj);
	void AddStringAtLeft(const char* s);
	void AddInt(int value);
	void AddDouble(double value, UINT after_dot);
	void RemoveLeft(CPOS count);
	void RemoveMiddle(CPOS start, CPOS count);
	void RemoveRight(CPOS count);
	void TruncateAt(CPOS idx);
	friend CStr operator+(const CStr& s1, const CStr& s2);
	friend CStr operator+(const CStr& s, const char* lpsz);
	friend CStr operator+(const char* lpsz, const CStr& s);
	friend CStr operator+(const CStr& s, const char ch);
	friend CStr operator+(const char ch, const CStr& s);
	void Trim(const char* charset = NULL);		// Remove all trailing
	void LTrim(const char* charset = NULL);		// Remove all leading
	void AllTrim(const char* charset = NULL);	// Remove both

// Window operations and other utilities
#ifndef CSTR_NO_WINSTUFF
	void GetWindowText (CWnd* wnd);
#endif

// Miscellaneous implementation methods
protected:
	void NewFromString(const char* s, CPOS slen, CPOS prealloc);
	void  Buffer (CPOS newlength);
	void  CoreAddChars(const char* s, CPOS howmany);
	void FormatCore (const char* x, va_list& marker);
	BOOL FmtOneValue (const char*& x, va_list& marker);
public:
	// These are public to avoid friend function definitions...
	static BYTE*  m_Cache[];			// Of size CSTR_DITEMS >> 2
	static SDesc  m_Null;
	static int m_CacheArbiter;
	static bool m_CacheArbiterInited;
	static UINT m_ArbiterWithinCS;
	// These may be reimplemented, see below
	static void ThrowIfNull(void* p);
	static void ThrowPgmError();
	static void ThrowNoUnicode();
	static void ThrowBadIndex();
#ifndef CSTR_LARGE_STRINGS
	static void ThrowTooLarge();
#endif
};

BOOL operator ==(const CStr& s1, const CStr& s2);
BOOL operator ==(const CStr& s1, LPCTSTR s2);
BOOL operator ==(LPCTSTR s1, const CStr& s2);
BOOL operator !=(const CStr& s1, const CStr& s2);
BOOL operator !=(const CStr& s1, LPCTSTR s2);
BOOL operator !=(LPCTSTR s1, const CStr& s2);


/*********************************************************************
* Procs:	CStr::ThrowXXX()
* Purpose:	Called when a fatal event is encountered, such as not
*			enough memory or out-of-range character.  In large
*			applications you will typically want to reimplement these
*			to throw an exception, or at least abort the program
*			gracefully.  To do this, just define CSTR_OWN_ERRORS
*			and implement these functions elsewhere.  It may be
*			useful to place ASSERT-s in some of these during debugging.
*********************************************************************/

#ifndef CSTR_OWN_ERRORS

void cstr_abort_application(int fatal_type);
inline void CStr::ThrowBadIndex()		{ cstr_abort_application(2); }
inline void CStr::ThrowPgmError()		{ cstr_abort_application(3); }
inline void CStr::ThrowNoUnicode()		{ cstr_abort_application(4); }
#ifndef CSTR_LARGE_STRINGS
inline void CStr::ThrowTooLarge()		{ cstr_abort_application(5); }
#endif

// Contemporary apps typically don't check for 'no memory'
//   situations when just a few bytes are concerned.  If this
//   is a requirement, redefine the function below as
//      void CStr::ThrowIfNull(void*)
//      { if (p == NULL)
//           ThrowSomeFatalError();
//      }
inline void CStr::ThrowIfNull(void*)  { /* no-op */ }


#endif


// Inline CStr method implementations
#include "cstrimp.h"


#endif		// _CSTR_INCLUDED_
