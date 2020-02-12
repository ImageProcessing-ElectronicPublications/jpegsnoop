// CStrMgr.cpp [02 Oct 1998]
//

#include "CStr.h"
#include "stdio.h"

#include "../WindowsClasses.h"


/*********************************************************************
* Proc:		cstr_abort_application
* Purpose:	Used only if the caller does not supply his own version
*			of the ThrowXXX methods.  Displays a system-modal message
*			box, then aborts the application.
*********************************************************************/

#ifndef CSTR_OWN_ERRORS
void cstr_abort_application(int fatal_type)
{
	// Display message and abort
	char msg[64];
	sprintf (msg, "CStr fatal %d", fatal_type);
	::MessageBox (NULL, msg, NULL, MB_SYSTEMMODAL | MB_ICONSTOP);
	abort();
}
#endif


// Define specific type of ASSERT
#ifdef _DEBUG
#define CSTR_ASSERT(x)  { if ((x) == 0)  cstr_abort_application (1111); }
#else
#define CSTR_ASSERT(x)  { }
#endif


/*********************************************************************
* Proc:		i_blkalloc / i_blkverify / i_blkrealfree
* Purpose:	Helpers
*********************************************************************/

#ifdef _DEBUG

// Debug alloc: prepend byte length, append protective mask
BYTE* i_blkalloc (UINT bytes)
{
	BYTE* x = (BYTE*) malloc (bytes + 8);
	((UINT*)x)[0] = bytes;
	x += 4;
	memset (x+bytes, 0x78, 4);
	return x;
}

// Debug verify: make sure byte length matches and protective mask is OK
void i_blkverify (BYTE* block, UINT bytes)
{
	BYTE* realblock = block-4;
	CSTR_ASSERT (((UINT*)realblock)[0] == bytes);
	CSTR_ASSERT (block[bytes+0] == 0x78  &&  block[bytes+1] == 0x78);
	CSTR_ASSERT (block[bytes+2] == 0x78  &&  block[bytes+3] == 0x78);
}

// Debug real free: call free() 4 bytes earlier
void i_blkrealfree(BYTE* block, UINT bytes)
{
	i_blkverify(block, bytes);
	BYTE* realblock = block-4;
	free (realblock);
}

#else

// Release alloc: just do malloc
inline BYTE* i_blkalloc (UINT bytes)
{
	return (BYTE*) malloc (bytes);
}

// Release verify: do nothing
inline void i_blkverify (BYTE*, UINT)
{
}

// Release real free: just call free()
void i_blkrealfree(BYTE* block, UINT)
{
	free (block);
}

#endif


/*********************************************************************
* Class:	cache_arb
* Purpose:	Helper class for maintaining the cache consistent
*			across threads
*********************************************************************/

struct cache_arb
{
	cache_arb();
	~cache_arb();
};

#ifndef CSTR_NOT_MT_SAFE

// Thread-safe cache arbiter
cache_arb::cache_arb()
{
 return;
}

inline cache_arb::~cache_arb()
{
return;
}

#else

// Thread-unsafe cache arbiter
inline cache_arb::cache_arb()
{ }
inline cache_arb::~cache_arb()
{ }

#endif


/*********************************************************************
* Proc:		blkalloc
* Purpose:	Allocates a memory block for the string manager.  If the
*			block falls below the max bytes threshold (and almost all
*			do), the function attempts to return a block from the
*			cache.  Otherwise, it acts as a regular malloc.
* In:		bytes - how many bytes to be allocated (MUST be divisible
*					by four, and must be >0!)
* Out:		Ptr to allocated (or obtained) block, NULL if no memory
* Rems:		In debug build, 8 additional bytes are allocated before
*			and after the block, to facilitate memory overwrite
*			detection.
*********************************************************************/

static BYTE* blkalloc(UINT bytes)
{
	CSTR_ASSERT (bytes != 0);
	CSTR_ASSERT ((bytes & 3) == 0);
	BYTE* x;
	// Large block?
	if (bytes >= CSTR_DITEMS)
		return i_blkalloc (bytes);
	// No block in cache?
	UINT cindex = bytes >> 2;
	cache_arb keeper;
	x = CStr::m_Cache[cindex];
	if (x == NULL)
		return i_blkalloc (bytes);
	// Return block from cache, update it
	BYTE** x_fp = (BYTE**) x;
	CStr::m_Cache[cindex] = *x_fp;
	return x;
}


/*********************************************************************
* Proc:		blkfree
* Purpose:	Deallocates a memory block used by the string manager.
*			Does not release the block by free(), but instead forms
*			a linked list of blocks of a given size, storing a ptr
*			to the list head in m_Cache.
* In:		blk   - a block previously allocated with blkalloc
*			bytes - the exact same value passed to blkalloc
*********************************************************************/

static void blkfree (void* blk, UINT bytes)
{
	// Check the block; translates to nothing in release builds
	CSTR_ASSERT (blk != NULL  &&  bytes != 0  &&  (bytes & 3) == 0);
	i_blkverify ((BYTE*) blk, bytes);
	// Large blocks are returned directly to the memory manager
	if (bytes >= CSTR_DITEMS) {
		i_blkrealfree ((BYTE*) blk, bytes);
		return;
	}
	// Put reference to next free block in first 4 bytes
	UINT cindex = bytes >> 2;
	BYTE** blk_fp = (BYTE**) blk;
	cache_arb keeper;
	*blk_fp = CStr::m_Cache[cindex];
	// And add block as head of cache
	CStr::m_Cache[cindex] = (BYTE*) blk;
}


BYTE* CStr::m_Cache[CSTR_DITEMS / 4] = { NULL };
SDesc CStr::m_Null = { 2, "", 0, 1 };
bool  CStr::m_CacheArbiterInited = false;
#ifndef CSTR_NOT_MT_SAFE
UINT  CStr::m_ArbiterWithinCS = 0;
#endif

int CStr::m_CacheArbiter = { 0 };

#ifdef __AFX_H__
HANDLE get_stringres()		{ return AfxGetResourceHandle(); }
#endif


/*********************************************************************
* Proc:		find_desc
* Purpose:	Helper.  Allocates (from the cache or memory) a SDesc block
*********************************************************************/

inline SDesc* find_desc()
{
	BYTE* x = blkalloc (sizeof(SDesc));
	CStr::ThrowIfNull(x);
	return (SDesc*) x;
}


/*********************************************************************
* Proc:		CStr::freestr (static method)
* Purpose:	Decrements the usage count of the specified string.  When
*			the count reaches zero, blkfrees the data bytes and moves
*			the SDesc block to the pool of available blocks.
*********************************************************************/

void  CStr::freestr_nMT(SDesc* tofree)
{
#ifndef CSTR_NOT_MT_SAFE
	CSTR_ASSERT (m_ArbiterWithinCS > 0);
#endif
	// Check validity
#ifdef _DEBUG
	if (tofree == NULL  ||  tofree->m_Usage == 0)
		ThrowPgmError();
#endif
	// Decrement counter, don't deallocate anything if we're using m_Null
	tofree->m_Usage--;
	if (tofree == &CStr::m_Null) {
#ifdef _DEBUG
		if (tofree->m_Usage < 2)
			ThrowPgmError();
#endif
		return;
	}
	// When the usage count reaches 0, blkfree the block
	if (tofree->m_Usage == 0) {
		// Deallocate associated data block
		blkfree (tofree->m_Text, tofree->m_Alloc+1);
		// Deallocate the descriptor itself
		blkfree (tofree, sizeof(SDesc));
	}
}

void  CStr::freestr(SDesc* tofree)
{
	cache_arb keeper;
	freestr_nMT (tofree);
}


/*********************************************************************
* Proc:		CStr::destructor
*********************************************************************/

CStr::~CStr()
{
	// Note that we don't have to check for m_Null here, because
	//   the usage count there will always be 2 or more.
	// MT note: InterlockedDecrement() is used instead of --
	cache_arb keeper;
	if (--data->m_Usage == 0) {
#ifdef _DEBUG
		if (data == &CStr::m_Null)
			CStr::ThrowPgmError();
#endif
		// blkfree associated text block and descriptor
		blkfree (data->m_Text, data->m_Alloc+1);
		blkfree (data, sizeof(SDesc));
	}
}


/*********************************************************************
* Proc:		CStr::NewFromString
* Purpose:	Core code for 'copy char* constructor' and '= operator'.
*			Assumes our 'data' field is garbage on entry.
*********************************************************************/

void CStr::NewFromString(const char* s, CPOS slen, CPOS prealloc)
{
	// Determine actual size of buffer that needs to be allocated
	if (slen > prealloc)
		prealloc = slen;
	// String or buffer too large?
#ifndef CSTR_LARGE_STRINGS
	if (prealloc > MAXCHARS)
		ThrowTooLarge();
#endif
	// Empty string?
	if (slen == 0  &&  prealloc == 0) {
		SET_data_EMPTY();
		return;
	}
	// Round up to blkalloc() granularity
	up_alloc (prealloc);
	// Get memory for string
	char* target = (char*) blkalloc (prealloc+1);
	ThrowIfNull(target);
	// Locate blkfree descriptor, fill it in
	data = find_desc();
	data->m_Text = target;
	data->m_Usage = 1;
	data->m_Length = slen;
	data->m_Alloc = prealloc;
	// Copy the string bytes, including the null
	memcpy (target, s, slen+1);
}


/*********************************************************************
* Proc:		CStr::CStr (UINT prealloc)
* Purpose:	Instantiates an empty string, but with the specified
*			number of bytes in the preallocated buffer.
* In:		prealloc - number of bytes to reserve
*********************************************************************/

CStr::CStr(CPOS prealloc)
{

#ifndef CSTR_LARGE_STRINGS
	if (prealloc > MAXCHARS)
		ThrowTooLarge();
#endif
	up_alloc (prealloc);
	// Get memory for string
	char* target = (char*) blkalloc (prealloc+1);
	ThrowIfNull(target);
	target[0] = 0;
	// Locate blkfree descriptor, fill it in
	data = find_desc();
	data->m_Text = target;
	data->m_Usage = 1;
	data->m_Length = 0;
	data->m_Alloc = prealloc;
}


/*********************************************************************
* Proc:		CStr::CStr(const char*)
* Purpose:	Construct an instance that exactly copies the specified
*			string.  An optional second parameter specifies the buffer
*			size (will be ignored if it is less than what's needed)
*********************************************************************/

CStr::CStr(const char* s, CPOS prealloc /*= 0*/)
{
	// No need to check for limit in CSTR_LARGE_STRINGS, because
	//   NewFromString will blow up because of the buffer size
	UINT slen = strlen(s);
	NewFromString(s, (CPOS) slen, prealloc);
}


/*********************************************************************
* Proc:		CStr::copy constructor
*********************************************************************/

CStr::CStr(const CStr& source)
{
	if (this == &source)
		return;
	data = source.data;
	data->m_Usage++;
}


/*********************************************************************
* Proc:		CStr::CStr(const CString&)
* Purpose:	Implemented only when using MFC.  Instantiates the string
*			from an MFC-provided one.
*********************************************************************/

#ifdef __AFX_H__
CStr::CStr(const CString& source, CPOS prealloc /*= 0*/)
{
	NewFromString((const char*) source, (CPOS) source.GetLength(), prealloc);
}
#endif


/*********************************************************************
* Proc:		CStr::operator = CStr
* Purpose:	Copies a string into another string, destroying the
*			previous content.
*********************************************************************/

const CStr& CStr::operator=(const CStr& source)
{
	if (this != &source) {
		freestr(data);
		data = source.data;
		data->m_Usage++;
	}
	return *this;
}


/*********************************************************************
* Proc:		CStr::operator = [const CString&]
* Purpose:	Implemented only when MFC is used.  
*********************************************************************/

#ifdef __AFX_H__
const CStr& CStr::operator=(const CString& source)
{
	*this = (const char*) source;
	return *this;
}
#endif


/*********************************************************************
* Proc:		CStr::Buffer
* Purpose:	Helper.  Makes sure that our internal buffer has
*			the specified number of bytes available, and that
*			we can overwrite it (i.e. m_Usage is 1).  If this
*			isn't so, prepares a copy of our internal data.
*********************************************************************/

void  CStr::Buffer (CPOS newlength)
{
#ifndef CSTR_LARGE_STRINGS
	if (newlength > MAXCHARS)
		ThrowTooLarge();
#endif
	up_alloc(newlength);
	// Reallocate. First handle case where we cannot just
	//   touch the buffer.  We don't need to test for m_Null
	//   here because it's m_Usage field is always >1
	cache_arb keeper;
	if (data->m_Usage == 1) {
		// Buffer already big enough?
		if (data->m_Alloc >= newlength)
			return;
		// Nope. We need to reallocate here.
	}
	SDesc* prevdata = data;
	if (newlength < prevdata->m_Length)
		newlength = prevdata->m_Length;
	NewFromString(prevdata->m_Text, prevdata->m_Length, newlength);
	freestr_nMT(prevdata);
}


/*********************************************************************
* Proc:		CStr::Compact
* Purpose:	If m_Alloc is bigger than m_Length, shrinks the
*			buffer to hold only as much memory as necessary.
* In:		only_above - will touch the object only if the difference
*				is greater than this value.  By default, 0 is passed,
*				but other good values might be are 1 to 7.
* Rems:		Will compact even the buffer for a string shared between
*			several CStr instances.
*********************************************************************/

void CStr::Compact(CPOS only_above /*= 0*/)
{
	// Nothing to do?
	if (data == &CStr::m_Null)
		return;
	cache_arb keeper;
	CPOS diff = (CPOS) int(data->m_Alloc - data->m_Length);
	if (diff <= only_above)
		return;
	// Shrink buffer.  The up_alloc() call is because it is no good
	//   to allocate 14 or 15 bytes when we can get 16; the memory
	//   manager will waste the few remaining bytes anyway.
	CPOS to_shrink = data->m_Length;
	up_alloc (to_shrink);
	// Using realloc to shrink a memory block might cause heavy
	//   fragmentation.  Therefore, a preferred method is to
	//   allocate a new block and copy the data there.
	char* xnew = (char*) blkalloc (to_shrink+1);
	if (xnew == NULL)
		return;				// No need to throw 'no mem' here
	// Copy data, substitute fields
	char* xold = data->m_Text;
	UINT  xoal = data->m_Alloc;
	data->m_Text = xnew;
	memcpy (xnew, xold, data->m_Length+1);
	blkfree ((BYTE*) xold, xoal+1);
	data->m_Alloc = to_shrink;
}


/*********************************************************************
* Proc:		CStr::CompactFree (static)
* Purpose:	Releases any unused blocks we might cache.
*********************************************************************/

void CStr::CompactFree()
{
	for (UINT i=0; i < (CSTR_DITEMS / 4); i++) {
		if (m_Cache[i] != NULL) {
			cache_arb keeper;
			while (m_Cache[i] != NULL) {
				BYTE** x = (BYTE**) m_Cache[i];
				m_Cache[i] = *x;
				i_blkrealfree ((BYTE*) x, i << 2);
			}
		}
	}
}


/*********************************************************************
* Proc:		CStr::operator = const char*
* Purpose:	Copies a string into our internal buffer, if it is big
*			enough and is used only by us; otherwise, blkfrees the
*			current instance and allocates a new one.
*********************************************************************/

const CStr& CStr::operator=(const char* s)
{
	// Check for zero length string.
	UINT slen = strlen(s);
	if (slen == 0) {
		if (data == &CStr::m_Null)
			return *this;
		freestr(data);
		SET_data_EMPTY();
		return *this;
	}
#ifndef CSTR_LARGE_STRINGS
	if (slen > MAXCHARS)
		ThrowTooLarge();
#endif
	// Can we handle this without reallocations?  NOTE: we do
	//   not use Buffer() here because if the string needs to
	//   be expanded, the old one will be copied, and we don't
	//   care about it anyway.
	cache_arb keeper;
	if (data->m_Usage == 1  &&  data->m_Alloc >= slen) {
		// Yes, copy bytes and get out
		memcpy (data->m_Text, s, slen+1);
		data->m_Length = (CPOS) slen;
		return *this;
	}
	// No. blkfree old string, allocate new one.
	freestr_nMT(data);
	NewFromString(s, (CPOS) slen, 0);
	return *this;
}


/*********************************************************************
* Proc:		CStr::Empty
* Purpose:	Sets the string to NULL.  However, the allocated buffer
*			is not released.
*********************************************************************/

void CStr::Empty()
{
	// Already empty, and with buffer zero?
	if (data == &CStr::m_Null)
		return;
	// More than one instance served?
	cache_arb keeper;
	if (data->m_Usage != 1) {
		// Get a copy of the buffer size, release shared instance
		UINT to_alloc = data->m_Alloc;
		freestr_nMT(data);
		// Get memory for string
		char* target = (char*) blkalloc (to_alloc+1);
		ThrowIfNull(target);
		// Locate blkfree descriptor, fill it in
		data = find_desc();
		data->m_Text = target;
		data->m_Usage = 1;
		data->m_Length = 0;
		data->m_Alloc = (CPOS) to_alloc;
		target[0] = 0;
		return;
	}
	// Only one instance served, so just set the charcount to zero.
	data->m_Text[0] = 0;
	data->m_Length = 0;
}


/*********************************************************************
* Proc:		CStr::Reset
* Purpose:	Sets the string to NULL, deallocates buffer
*********************************************************************/

void CStr::Reset()
{
	if (data != &CStr::m_Null) {
		freestr(data);
		SET_data_EMPTY();
	}
}


/*********************************************************************
* Proc:		CStr::AddChar
* Purpose:	Appends a single character to the end of the string
*********************************************************************/

void CStr::AddChar(char ch)
{
	// Get a copy if m_Usage>1, expand buffer if necessary
	UINT cur_len = data->m_Length;
	Buffer (CPOS(cur_len + 1));
	// And append the character (no need for cache_arb because of Buffer)
	data->m_Text[cur_len] = ch;
	data->m_Text[cur_len+1] = 0;
	data->m_Length = CPOS(cur_len+1);
}


/*********************************************************************
* Proc:		CStr::AddInt
* Purpose:	Appends a decimal signed integer, possibly with - sign
*********************************************************************/

void CStr::AddInt(int value)
{
	char buf[32];
	itoa (value, buf, 10);
	AddString (buf);
}


/*********************************************************************
* Proc:		CStr::AddDouble
* Purpose:	Appends a signed double value, uses specified # of digits
*********************************************************************/

void CStr::AddDouble(double value, UINT after_dot)
{
	if (after_dot > 48)
		after_dot = 48;
	char fmt[16];
	sprintf (fmt, "%%.%df", (int) after_dot);
	char buf[256];
	sprintf (buf, fmt, value);
	AddString (buf);
}


/*********************************************************************
* Proc:		CStr::CoreAddChars
* Purpose:	Core code for AddChars() and operators +=; assumes
*			howmany is bigger than 0
*********************************************************************/

void  CStr::CoreAddChars(const char* s, CPOS howmany)
{
#ifndef CSTR_LARGE_STRINGS
	if ((UINT(data->m_Length) + howmany) > MAXCHARS)
		ThrowTooLarge();
#endif
	// Prepare big enough buffer
	Buffer (CPOS(data->m_Length + howmany));
	// And copy the bytes
	char* dest = data->m_Text + data->m_Length;
	memcpy (dest, s, howmany);
	dest[howmany] = 0;
	data->m_Length = CPOS(data->m_Length + howmany);
}


/*********************************************************************
* Proc:		CStr::operator += (both from const char* and from CStr)
* Purpose:	Append a string to what we already contain.
*********************************************************************/

void CStr::operator += (const CStr& obj)
{
	if (obj.data->m_Length != 0)
		CoreAddChars (obj.data->m_Text, obj.data->m_Length);
}

void CStr::operator += (const char* s)
{
	UINT slen = strlen(s);
	if (slen != 0) {
#ifndef CSTR_LARGE_STRINGS
		if (slen > MAXCHARS)
			ThrowTooLarge();
#endif
		CoreAddChars (s, (CPOS) slen);
	}
}


/*********************************************************************
* Proc:		CStr::AddChars
* Purpose:	Catenates a number of characters to our internal data.
*********************************************************************/

void CStr::AddChars(const char* s, CPOS startat, CPOS howmany)
{
	if (howmany != 0) {
#ifndef CSTR_LARGE_STRINGS
		if ((UINT(GetLength()) + howmany) > MAXCHARS)
			ThrowTooLarge();
#endif
		CoreAddChars (s+startat, howmany);
	}
}


/*********************************************************************
* Proc:		CStr::AddStringAtLeft
* Purpose:	Prepend a string before us
*********************************************************************/

void CStr::AddStringAtLeft(const char* s)
{
	UINT slen = strlen(s);
	if (slen == 0)
		return;
	// Make buffer big enough
#ifndef CSTR_LARGE_STRINGS
	if ((UINT(GetLength()) + slen) > MAXCHARS)
		ThrowTooLarge();
#endif
	Buffer (CPOS(GetLength() + slen));
	// Move existing data -- do NOT use memcpy!!
	memmove (data->m_Text+slen, data->m_Text, GetLength()+1);
	// And copy bytes to be prepended
	memcpy (data->m_Text, s, slen);
	data->m_Length = CPOS(data->m_Length + slen);
}


/*********************************************************************
* Proc:		operator +LPCSTR for CStr
*********************************************************************/

CStr operator+(const char* lpsz, const CStr& s)
{
	CStr temp (CPOS(s.GetLength() + strlen(lpsz)));
	temp  = lpsz;
	temp += s;
	return temp;
}


/*********************************************************************
* Proc:		operator +char for CStr
*********************************************************************/

CStr operator+(const char ch, const CStr& s)
{
	CStr temp (CPOS(s.GetLength() + 1));
	temp  = ch;
	temp += s;
	return temp;
}


/*********************************************************************
* Proc:		CStr::GetLastChar
*********************************************************************/

char CStr::GetLastChar() const
{
	UINT l = GetLength();
	if (l < 1)
		ThrowBadIndex();
	return data->m_Text[l-1];
}


/*********************************************************************
* Proc:		CStr::GetLeft
*********************************************************************/

void CStr::GetMiddle (CPOS start, CPOS chars, CStr& result)
{
	result.Empty();
	// Nothing to return?
	CPOS l = GetLength();
	if (l == 0  ||  (UINT(start)+chars) == 0)
		return;
	// Do not return data beyond the end of the string
	if (start >= l)
		return;
	if ((UINT(start)+chars) >= l)
		chars = CPOS(l - start);
	// Copy bytes
	result.CoreAddChars(GetString()+start, chars);
}

void CStr::GetLeft (CPOS chars, CStr& result)
{
	GetMiddle(0, chars, result);
}

void CStr::GetRight (CPOS chars, CStr& result)
{
	if (chars >= GetLength()) {
		result = *this;
		return;
	}
	GetMiddle(CPOS(GetLength()-chars), chars, result);
}


/*********************************************************************
* Proc:		CStr::TruncateAt
* Purpose:	Cuts off the string at the character with the specified
*			index.  The allocated buffer remains the same.
*********************************************************************/

void CStr::TruncateAt (CPOS idx)
{
	if (idx >= GetLength())
		return;
	// Spawn a copy if necessary
	Buffer (data->m_Alloc);		// Preserve buffer size
	// And do the truncation
	data->m_Text[idx] = 0;
	data->m_Length = idx;
}


/*********************************************************************
* Proc:		CStr::Find and ReverseFind
* Purpose:	Scan the string for a particular character (must not
*			be 0); return the index where the character is found
*			first, or -1 if cannot be met
*********************************************************************/

int CStr::Find (char ch, CPOS startat /*= 0*/) const
{
	// Start from middle of string?
	if (startat > 0) {
		if (startat >= GetLength())
			ThrowBadIndex();
	}
	char* scan = strchr (data->m_Text+startat, ch);
	if (scan == NULL)
		return -1;
	else
		return scan - data->m_Text;
}

int CStr::ReverseFind (char ch, CPOS startat /*= -1*/) const
{
	if (startat == (UINT) -1) {
		// Scan entire string
		char* scan = strrchr (data->m_Text, ch);
		if (scan)
			return scan - data->m_Text;
	}
	else {
		// Make sure the index is OK
		if (startat >= GetLength())
			ThrowBadIndex();
		for (int findex = (int) startat-1; findex >= 0; findex--) {
			if (data->m_Text[findex] == ch)
				return findex;
		}
	}
	return -1;
}


/*********************************************************************
* Proc:		CStr::Compare and CompareNoCase
*********************************************************************/

int CStr::Compare (const char* match) const
{
	int i = strcmp (data->m_Text, match);
	if (i == 0)
		return 0;
	else if (i < 0)
		return -1;
	else
		return 1;
}

int CStr::CompareNoCase (const char* match) const
{
	int i = stricmp (data->m_Text, match);
	if (i == 0)
		return 0;
	else if (i < 0)
		return -1;
	else
		return 1;
}


/*********************************************************************
* Proc:		CStr::GrowTo
* Purpose:	If the buffer is smaller than the amount of characters
*			specified, reallocates the buffer.  This function cannot
*			reallocate to a buffer smaller than the existing one.
*********************************************************************/

void CStr::GrowTo(CPOS size)
{
	Buffer (size);
}


/*********************************************************************
* Proc:		CStr::operator == (basic forms, the rest are inline)
*********************************************************************/

BOOL operator ==(const CStr& s1, const CStr& s2)
{
	UINT slen = s2.GetLength();
	if (s1.GetLength() != slen)
		return FALSE;
	return memcmp(s1.GetString(), s2, slen) == 0;
}

BOOL operator ==(const CStr& s1, LPCTSTR s2)
{
	UINT slen = strlen(s2);
	if (s1.GetLength() != slen)
		return FALSE;
	return memcmp(s1.GetString(), s2, slen) == 0;
}


/*********************************************************************
* Proc:		CStr::RemoveLeft
*********************************************************************/

void CStr::RemoveLeft(CPOS count)
{
	if (GetLength() <= count) {
		Empty();
		return;
	}
	if (count == 0)
		return;
	Buffer (data->m_Alloc);		// Preserve buffer size
	memmove (data->m_Text, data->m_Text+count, GetLength()-count+1);
	data->m_Length = CPOS(data->m_Length - count);
}

void CStr::RemoveMiddle(CPOS start, CPOS count)
{
	if (GetLength() <= start) {
		Empty();
		return;
	}
	Buffer (data->m_Alloc);		// Preserve buffer size
	char* pstart = data->m_Text + start;
	if (GetLength() <= (start+count)) {
		pstart[0] = 0;
		data->m_Length = start;
		return;
	}
	memmove (pstart, pstart+count, GetLength()-(start+count)+1);
	data->m_Length = CPOS(data->m_Length - count);
}

void CStr::RemoveRight(CPOS count)
{
	if (GetLength() <= count)
		Empty();
	else
		TruncateAt (CPOS(GetLength() - count));
}


/*********************************************************************
* Proc:		CStr::FmtOneValue
* Purpose:	Helper for CStr::Format, formats one %??? item
* In:		x - ptr to the '%' sign in the specification; on exit,
*				will point to the first char after the spec.
* Out:		True if OK, False if should end formatting (but also copy
*			the remaining characters at x)
*********************************************************************/

BOOL CStr::FmtOneValue (const char*& x, va_list& marker)
{
	// Start copying format specifier to a local buffer
	char fsbuf[64];
	fsbuf[0] = '%';
	int fsp = 1;
GetMoreSpecifiers:
	// Get one character
#ifdef _DEBUG
	if (fsp >= sizeof(fsbuf)) {
		CStr::ThrowPgmError();
		return FALSE;
	}
#endif
	char ch = x[0];
	if (ch == 0)
		return FALSE;		// unexpected end of format string
	x++;
	// Chars that may exist in the format prefix
	const char fprefix[] = "-+0 #*.123456789hlL";
	if (strchr (fprefix, ch) != NULL) {
		fsbuf[fsp] = ch;
		fsp++;
		goto GetMoreSpecifiers;
	}
	// 's' is the most important parameter specifier type
	if (ch == 's') {
		// Find out how many characters should we actually print.
		//   To do this, get the string length, but also try to
		//   detect a .precision field in the format specifier prefix.
		const char* value = va_arg (marker, const char*);
		UINT slen = strlen(value);
		fsbuf[fsp] = 0;
		char* precis = strchr (fsbuf, '.');
		if (precis != NULL  &&  precis[1] != 0) {
			// Convert value after dot, put within 0 and slen
			char* endptr;
			int result = (int) strtol (precis+1, &endptr, 10);
			if (result >= 0  &&  result < int(slen))
				slen = (UINT) result;
		}
		// Copy the appropriate number of characters
		if (slen > 0)
			CoreAddChars (value, (CPOS) slen);
		return TRUE;
	}
	// '!' is our private extension, allows direct passing of CStr*
	if (ch == '!') {
		// No precision characters taken into account here.
		const CStr* value = va_arg (marker, const CStr*);
		*this += *value;
		return TRUE;
	}
	// Chars that format an integer value
	const char intletters[] = "cCdiouxX";
	if (strchr (intletters, ch) != NULL) {
		fsbuf[fsp] = ch;
		fsbuf[fsp+1] = 0;
		char valbuf[64];
		int value = va_arg (marker, int);
		sprintf (valbuf, fsbuf, value);
		*this += valbuf;
		return TRUE;
	};
	// Chars that format a double value
	const char dblletters[] = "eEfgG";
	if (strchr (dblletters, ch) != NULL) {
		fsbuf[fsp] = ch;
		fsbuf[fsp+1] = 0;
		char valbuf[256];
		double value = va_arg (marker, double);
		sprintf (valbuf, fsbuf, value);
		*this += valbuf;
		return TRUE;
	}
	// 'Print pointer' is supported
	if (ch == 'p') {
		fsbuf[fsp] = ch;
		fsbuf[fsp+1] = 0;
		char valbuf[64];
		void* value = va_arg (marker, void*);
		sprintf (valbuf, fsbuf, value);
		*this += valbuf;
		return TRUE;
	};
	// 'store # written so far' is obscure and unsupported
	if (ch == 'n') {
		ThrowPgmError();
		return FALSE;
	}
	// 'Print unicode string' is not supported also
	if (ch == 'S') {
		ThrowNoUnicode();
		return FALSE;
	}
	// If we fall here, the character does not represent an item
	AddChar (ch);
	return TRUE;
}


/*********************************************************************
* Proc:		CStr::Format
* Purpose:	sprintf-like method
*********************************************************************/

void CStr::FormatCore (const char* x, va_list& marker)
{
	for (;;) {
		// Locate next % sign, copy chunk, and exit if no more
		LPCSTR next_p = strchr (x, '%');
		if (next_p == NULL)
			break;
		if (next_p > x)
			CoreAddChars (x, CPOS(next_p-x));
		x = next_p+1;
		// We're at a parameter
		if (!FmtOneValue (x, marker))
			break;		// Copy rest of string and return
	}
	if (x[0] != 0)
		*this += x;
}

void CStr::Format(const char* fmt, ...)
{
	Empty();
	// Walk the string
	va_list marker;
	va_start(marker, fmt);
	FormatCore (fmt, marker);
	va_end(marker);
}

void CStr::FormatRes(UINT resid, ...)
{
	Empty();
	// Get a stack-based buffer
	char buffer[512];
	if (::LoadString((HINSTANCE) get_stringres(), resid, buffer, sizeof(buffer)) == 0)
		return;
	// Walk the string
	va_list marker;
	va_start(marker, resid);
	FormatCore (buffer, marker);
	va_end(marker);
}


/*********************************************************************
* Proc:		operator + (CStr and CStr, CStr and LPCSTR)
*********************************************************************/

CStr operator+(const CStr& s1, const CStr& s2)
{
	CStr out (CPOS(s1.GetLength() + s2.GetLength()));
	out  = s1;
	out += s2;
	return out;
}

CStr operator+(const CStr& s, const char* lpsz)
{
	UINT slen = strlen(lpsz);
	CStr out (CPOS(s.GetLength() + slen));
	out.CoreAddChars(s.data->m_Text, s.GetLength());
	out += lpsz;
	return out;
}

CStr operator+(const CStr& s, const char ch)
{
	CStr out (CPOS(s.GetLength() + 1));
	out.CoreAddChars(s.data->m_Text, s.GetLength());
	out += ch;
	return out;
}


/*********************************************************************
* Proc:		CStr::LoadString
* Purpose:	Loads a string from the stringtable.
* In:		resid - resource ID
* Out:		True if OK, FALSE if could not find such a string
*********************************************************************/

BOOL CStr::LoadString(UINT resid)
{
	HANDLE hLoad = get_stringres();
	// Try smaller resources first
	char buffer[96];
	if (::LoadString((HINSTANCE) hLoad, resid, buffer, sizeof(buffer)) == 0)
		return FALSE;
	UINT slen = strlen(buffer);
	if (slen < sizeof(buffer)-1) {
		Empty();
		CoreAddChars(buffer, (CPOS) slen);
		return TRUE;
	}
	// We have a large string, use a big buffer
	const UINT s_big = 16384;
	char* bigbuf = (char*) blkalloc (s_big);
	if (!bigbuf)
		return FALSE;
	if (::LoadString((HINSTANCE) hLoad, resid, bigbuf, s_big) == 0) {
		blkfree(bigbuf, s_big);
		return FALSE;
	}
	*this = (const char*) bigbuf;
	blkfree (bigbuf, s_big);
	return TRUE;
}


/*********************************************************************
* Proc:		CStr::GetWindowText and GetDlgItemText
* Purpose:	Helpers for MFC programs
*********************************************************************/

#ifndef CSTR_NO_WINSTUFF
void CStr::GetWindowText (CWnd* wnd)
{
	Empty();
	// How big a buffer should we have?
	HWND w = wnd->GetSafeHwnd();
	CPOS tl = (CPOS) ::SendMessage (w, WM_GETTEXTLENGTH, 0, 0L);
	if (tl == 0)
		return;
	Buffer((CPOS) (tl+2));
	// And get the text
	::SendMessage (w, WM_GETTEXT, (WPARAM) tl+1, (LPARAM) data->m_Text);
	data->m_Length = tl;
}
#endif


/*********************************************************************
* Proc:		CStr::LTrim
* Purpose:	Remove leading characters from a string.  All characters
*			to be excluded are passed as a parameter; NULL means
*			'truncate spaces'
*********************************************************************/

void CStr::LTrim(const char* charset /*= NULL*/)
{
	CPOS good = 0;
	if (charset) {
		while (strchr (charset, data->m_Text[good]) != NULL)
			good++;
	}
	else {
		while (data->m_Text[good] == ' ')
			good++;
	}
	if (good > 0)
		RemoveLeft (good);
}


/*********************************************************************
* Proc:		CStr::Trim
* Purpose:	Remove trailing characters; see LTrim
*********************************************************************/

void CStr::Trim(const char* charset /*= NULL*/)
{
	CPOS good = data->m_Length;
	if (good == 0)
		return;
	if (charset) {
		while (good > 0  &&  strchr (charset, data->m_Text[good-1]) != NULL)
			--good;
	}
	else {
		while (good > 0  &&  data->m_Text[good-1] == ' ')
			--good;
	}
	TruncateAt (good);		// Also works well with good == 0
}

