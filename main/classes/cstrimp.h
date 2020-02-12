// cstrimp.h [02 Oct 1998]
//


/*********************************************************************
* Very simple utility macros and inlines
*********************************************************************/

// Round up to next value divisible by 4, minus one.  Usually compiles to
//               or bx,3       :-))
inline void up_alloc (CPOS& value)
{
	value = CPOS(value | 3);
}

#define SET_data_EMPTY()		\
	data = &CStr::m_Null;		\
	data->m_Usage++;


/*********************************************************************
* Proc:		CStr::CStr()
* Purpose:	Constructs an empty instance
*********************************************************************/

inline CStr::CStr()
{
	SET_data_EMPTY();
}


/*********************************************************************
* Proc:		CStr::GetFirstChar
*********************************************************************/

inline char CStr::GetFirstChar() const
{
	return data->m_Text[0];
}


/*********************************************************************
* Proc:		CStr::IsEmpty and GetLength
*********************************************************************/

inline BOOL CStr::IsEmpty() const
{
	return data->m_Length == 0;
}

inline CPOS CStr::GetLength() const
{
	return data->m_Length;
}


/*********************************************************************
* Proc:		CStr::operator []
*********************************************************************/

inline char CStr::operator[](CPOS idx) const
{
#ifdef _DEBUG
	if (idx >= GetLength())
		ThrowBadIndex();
#endif
	return data->m_Text[idx];
}

inline char CStr::GetAt(CPOS idx) const
{
	return *this[idx];
}


/*********************************************************************
* Proc:		CStr::operator 'cast to const char*'
*********************************************************************/

inline CStr::operator const char* () const
{
	return data->m_Text;
}

inline const char* CStr::GetString() const
{
	return data->m_Text;
}


/*********************************************************************
* Proc:		CStr::[operator ==] and [operator !=] inlined forms
*********************************************************************/

inline BOOL operator ==(LPCTSTR s1, const CStr& s2)
{ return (s2 == s1); }

inline BOOL operator !=(const CStr& s1, const CStr& s2)
{ return !(s1 == s2); }

inline BOOL operator !=(const CStr& s1, LPCTSTR s2)
{ return !(s1 == s2); }

inline BOOL operator !=(LPCTSTR s1, const CStr& s2)
{ return !(s2 == s1); }


/*********************************************************************
* Proc:		CStr::AddString - synonyms for operators +=
*********************************************************************/

inline void CStr::AddString(const CStr& obj)
{ *this += obj; }

inline void CStr::AddString(const char* s)
{ *this += s; }

inline void CStr::AddStringAtLeft(const CStr& obj)
{
	AddStringAtLeft (obj.GetString());
}

inline void CStr::operator += (const char ch)
{
	AddChar (ch);
}


/*********************************************************************
* Proc:		CStr::AllTrim - a combination of Trim and LTrim
*********************************************************************/

inline void CStr::AllTrim(const char* charset /*= NULL*/)
{
	Trim(charset);
	LTrim(charset);
}

