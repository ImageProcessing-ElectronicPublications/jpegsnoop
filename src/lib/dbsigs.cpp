// JPEGsnoop - JPEG Image Decoder & Analysis Utility
// Copyright (C) 2017 - Calvin Hass
// http://www.impulseadventure.com/photo/jpeg-snoop.html
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.
//


#include "windowsclasses.h"

#include "dbsigs.h"
#include "general.h"

#include "snoop.h"

#include "signatures.inl"

#define    MAX_BUF_SET_FILE    131072


// TODO:
// - Convert m_sSigListExtra[] to use CObArray instead of managing it
//   directly. Otherwise we are inefficient with memory and could potentially
//   not allocate enough space.


// Initialize the signature database array sizes
//
// PRE:
// - m_sSigList[]
// - m_sExcMmNoMkrList[]
// - m_sExcMmIsEditList[]
// - m_sSwIjgList[]
// - m_sXComSwList[]
//
// POST:
// - m_nSigListNum
// - m_nSigListExtraNum
// - m_nExcMmNoMkrListNum
// - m_nExcMmIsEditListNum
// - m_nSwIjgListNum
// - m_nXcomSwListNum
// - m_strDbDir
//
CDbSigs::CDbSigs()
{
    // Count the built-in database
    bool bDone;

    m_bFirstRun = false;

    bDone = false;
    m_nSigListNum = 0;
    while (!bDone)
    {
        if (!strcmp(m_sSigList[m_nSigListNum].strXMake,_T("*")))
            bDone = true;
        else
            m_nSigListNum++;
    }

    // Count number of exceptions in signatures.inl
    bDone = false;
    m_nExcMmNoMkrListNum = 0;
    while (!bDone)
    {
        if (!_tcscmp(m_sExcMmNoMkrList[m_nExcMmNoMkrListNum].strXMake,_T("*")))
            bDone = true;
        else
            m_nExcMmNoMkrListNum++;
    }

    bDone = false;
    m_nExcMmIsEditListNum = 0;
    while (!bDone)
    {
        if (!_tcscmp(m_sExcMmIsEditList[m_nExcMmIsEditListNum].strXMake,_T("*")))
            bDone = true;
        else
            m_nExcMmIsEditListNum++;
    }


    bDone = false;
    m_nSwIjgListNum = 0;
    while (!bDone)
    {
        if (!_tcscmp(m_sSwIjgList[m_nSwIjgListNum],_T("*")))
            bDone = true;
        else
            m_nSwIjgListNum++;
    }

    bDone = false;
    m_nXcomSwListNum = 0;
    while (!bDone)
    {
        if (!_tcscmp(m_sXComSwList[m_nXcomSwListNum],_T("*")))
            bDone = true;
        else
            m_nXcomSwListNum++;
    }


    // Reset extra database
    m_nSigListExtraNum = 0;

    // Default to user database dir not set yet
    // This will cause a fail if database load/store
    // functions are called before SetDbDir()
    m_strDbDir = _T("");

}

CDbSigs::~CDbSigs()
{
}

// Is this the first time running the application?
// If so, we skip certain warning messages (such as lack of existing user DB file)
void CDbSigs::SetFirstRun(bool bFirstRun)
{
    m_bFirstRun = bFirstRun;
}

unsigned CDbSigs::GetNumSigsInternal()
{
    return m_nSigListNum;
}

unsigned CDbSigs::GetNumSigsExtra()
{
    return m_nSigListExtraNum;
}


// Read an unsigned integer (4B) from the buffer
bool CDbSigs::BufReadNum(PBYTE pBuf,unsigned &nOut,unsigned nMaxBytes,unsigned &nOffsetBytes)
{
    nMaxBytes;    // Unreferenced param

    ASSERT(pBuf);
    // TODO: check for buffer bounds
    nOut = (unsigned)pBuf[nOffsetBytes];
    nOffsetBytes += sizeof(unsigned);
    return true;
}

// Write an unsigned integer (4B) to the buffer
bool CDbSigs::BufWriteNum(PBYTE pBuf,unsigned nIn,unsigned nMaxBytes,unsigned &nOffsetBytes)
{
    nMaxBytes;    // Unreferenced param

    ASSERT(pBuf);
    // TODO: check for buffer bounds
    PBYTE        pBufBase;
    unsigned*    pBufInt;
    pBufBase = &pBuf[nOffsetBytes];
    pBufInt = (unsigned*)pBufBase;
    pBufInt[0] = nIn;
    nOffsetBytes += sizeof(unsigned);
    return true;
}


// Attempt to read a line from the buffer
// This is a replacement for CStdioFile::ReadLine()
// Both 16-bit unicode and 8-bit SBCS encoding modes are supported (via bUni)
// Offset parameter is incremented accordingly
// Supports both newline and NULL for string termination
bool CDbSigs::BufReadStr(PBYTE pBuf,CString &strOut,unsigned nMaxBytes,bool bUni,unsigned &nOffsetBytes)
{
    ASSERT(pBuf);
    CString        strOutTmp;
    bool        bDone;

    char        chAsc;
    wchar_t        chUni;
    unsigned    nCharSz = ((bUni)?sizeof(wchar_t):sizeof(char));

    bDone = false;
    strOut = _T("");
    // Ensure we don't overrun the buffer by calculating the last
    // byte index required for each iteration.
    for (unsigned nInd=nOffsetBytes; (!bDone)&&(nInd+nCharSz-1<nMaxBytes); nInd+=nCharSz)
    {
        if (bUni)
        {
            chUni = pBuf[nInd];
            if ( (chUni != '\n') && (chUni != 0) )
            {
                strOut += chUni;
            }
            else
            {
                bDone = true;
                nOffsetBytes = nInd+nCharSz;
            }
        }
        else
        {
            chAsc = pBuf[nInd];
            if ( (chAsc != '\n') && (chAsc != 0) )
            {
                strOut += chAsc;
            }
            else
            {
                bDone = true;
                nOffsetBytes = nInd+nCharSz;
            }
        }
    }

    if (!bDone)
    {
        nOffsetBytes = nMaxBytes;
        // The input was not terminated, so we're still going to return what we got so far
        return false;
    }

    return true;
}

// Return true if we managed to write entire string including terminator
// without overrunning nMaxBytes
bool CDbSigs::BufWriteStr(PBYTE pBuf,CString strIn,unsigned nMaxBytes,bool bUni,unsigned &nOffsetBytes)
{
    ASSERT(pBuf);

    bool        bRet = false;
    char        chAsc;
    wchar_t        chUni;
    unsigned    nCharSz = ((bUni)?sizeof(wchar_t):sizeof(char));
    PBYTE        pBufBase;
    LPWSTR        pBufUni;
    LPSTR        pBufAsc;

    pBufBase = pBuf + nOffsetBytes;
    pBufUni = (LPWSTR)pBufBase;
    pBufAsc = (LPSTR)pBufBase;

#ifdef UNICODE
    // Create non-Unicode version of string
    // Ref: http://social.msdn.microsoft.com/Forums/vstudio/en-US/85f02321-de88-47d2-98c8-87daa839a98e/how-to-convert-cstring-to-const-char-?forum=vclanguage
    // Added constant specifier
    LPCSTR        pszNonUnicode;
    USES_CONVERSION;
    // Not specifying code page explicitly but assume content
    // should be ASCII. Default code page is probably Windows-1252.
    pszNonUnicode = CW2A( strIn.LockBuffer( ) );
    strIn.UnlockBuffer( );
#endif


    unsigned    nStrLen;
    unsigned    nChInd;
    nStrLen = strIn.GetLength();
    for (nChInd=0; (nChInd<nStrLen)&&(nOffsetBytes+nCharSz-1<nMaxBytes); nChInd++)
    {
        if (bUni)
        {
            // Normal handling for unicode
            chUni = strIn.GetAt(nChInd);
            pBufUni[nChInd] = chUni;
        }
        else
        {

#ifdef UNICODE
            // To avoid Warning C4244: Conversion from 'wchar_t' to 'char' possible loss of data
            // We need to implement conversion here
            // Ref: http://stackoverflow.com/questions/4786292/converting-unicode-strings-and-vice-versa

            // Since we have compiled for unicode, the CString character fetch
            // will be unicode char. Therefore we need to use ANSI-converted form.
            chAsc = pszNonUnicode[nChInd];
#else
            // Since we have compiled for non-Unicode, the CString character fetch
            // will be single byte char
            chAsc = strIn.GetAt(nChInd);
#endif
            pBufAsc[nChInd] = chAsc;
        }
        // Advance pointers
        nOffsetBytes += nCharSz;
    }

    // Now terminate if we have space
    if ((nOffsetBytes + nCharSz-1) < nMaxBytes)
    {
        if (bUni)
        {
            chUni = wchar_t(0);
            pBufUni[nChInd] = chUni;
        }
        else
        {
            chAsc = char(0);
            pBufAsc[nChInd] = chAsc;
        }
        // Advance pointers
        nOffsetBytes += nCharSz;

        // Since we managed to include terminator, return is successful
        bRet = true;
    }

    // Return true if we finished the string write (without exceeding nMaxBytes)
    // or false otherwise
    return bRet;
}


// TODO: Should we include editors in this search?
bool CDbSigs::SearchSignatureExactInternal(CString strMake, CString strModel, CString strSig)
{
    bool        bFoundExact = false;
    bool        bDone = false;
    unsigned    nInd = 0;
    while (!bDone)
    {
        if (nInd >= m_nSigListNum)
        {
            bDone = true;
        }
        else
        {

            if ( (m_sSigList[nInd].strXMake  == strMake) &&
                    (m_sSigList[nInd].strXModel == strModel) &&
                    ((m_sSigList[nInd].strCSig  == strSig) || (m_sSigList[nInd].strCSigRot == strSig)) )
            {
                bFoundExact = true;
                bDone = true;
            }
            nInd++;
        }
    }

    return bFoundExact;
}

bool CDbSigs::SearchCom(CString strCom)
{
    bool bFound = false;
    bool bDone = false;
    unsigned nInd = 0;
    if (strCom.GetLength() == 0)
    {
        bDone = true;
    }
    while (!bDone)
    {
        if (nInd >= m_nXcomSwListNum)
        {
            bDone = true;
        }
        else
        {
            if (strCom.Find(m_sXComSwList[nInd]) != -1)
            {
                bFound = true;
                bDone = true;
            }
            nInd++;
        }
    }
    return bFound;
}

// Returns total of built-in plus local DB
unsigned CDbSigs::GetDBNumEntries()
{
    return (m_nSigListNum + m_nSigListExtraNum);
}

// Returns total of built-in plus local DB
unsigned CDbSigs::IsDBEntryUser(unsigned nInd)
{
    if (nInd < m_nSigListNum)
    {
        return false;
    }
    else
    {
        return true;
    }
}

// Return a ptr to the struct containing the indexed entry
bool CDbSigs::GetDBEntry(unsigned nInd,CompSig* pEntry)
{
    unsigned nIndOffset;
    unsigned nIndMax = GetDBNumEntries();
    ASSERT(pEntry);
    ASSERT(nInd<nIndMax);
    if (nInd < m_nSigListNum)
    {
        pEntry->eEditor  = m_sSigList[nInd].eEditor;
        pEntry->strXMake    = m_sSigList[nInd].strXMake;
        pEntry->strXModel   = m_sSigList[nInd].strXModel;
        pEntry->strUmQual   = m_sSigList[nInd].strUmQual;
        pEntry->strCSig     = m_sSigList[nInd].strCSig;
        pEntry->strCSigRot  = m_sSigList[nInd].strCSigRot;
        pEntry->strXSubsamp = m_sSigList[nInd].strXSubsamp;
        pEntry->strMSwTrim  = m_sSigList[nInd].strMSwTrim;
        pEntry->strMSwDisp  = m_sSigList[nInd].strMSwDisp;
        return true;
    }
    else
    {
        nIndOffset = nInd-m_nSigListNum;
        pEntry->eEditor  = m_sSigListExtra[nIndOffset].eEditor;
        pEntry->strXMake    = m_sSigListExtra[nIndOffset].strXMake;
        pEntry->strXModel   = m_sSigListExtra[nIndOffset].strXModel;
        pEntry->strUmQual   = m_sSigListExtra[nIndOffset].strUmQual;
        pEntry->strCSig     = m_sSigListExtra[nIndOffset].strCSig;
        pEntry->strCSigRot  = m_sSigListExtra[nIndOffset].strCSigRot;
        pEntry->strXSubsamp = m_sSigListExtra[nIndOffset].strXSubsamp;
        pEntry->strMSwTrim  = m_sSigListExtra[nIndOffset].strMSwTrim;
        pEntry->strMSwDisp  = m_sSigListExtra[nIndOffset].strMSwDisp;
        return true;
    }
}

void CDbSigs::SetEntryValid(unsigned nInd,bool bValid)
{
    // TODO: Bounds check
    ASSERT(nInd < m_nSigListExtraNum);
    m_sSigListExtra[nInd].bValid = bValid;
}


unsigned CDbSigs::GetIjgNum()
{
    return m_nSwIjgListNum;
}

LPTSTR CDbSigs::GetIjgEntry(unsigned nInd)
{
    return m_sSwIjgList[nInd];
}

// Update the directory used for user database
void CDbSigs::SetDbDir(CString strDbDir)
{
    m_strDbDir = strDbDir;
}


char * _tcschr(char * str, char C)
{
    uint64_t index = 0;
    while (str[index] && str[index] != C)
    {
        if (str[index] == C) return str + index;
        index++;
    }

    return NULL;
}

// Search exceptions for Make/Model in list of ones that don't have Makernotes
bool CDbSigs::LookupExcMmNoMkr(CString strMake,CString strModel)
{
    bool bFound = false;
    bool bDone = false;
    unsigned nInd = 0;
    if (strMake.GetLength() == 0)
    {
        bDone = true;
    }
    while (!bDone)
    {
        if (nInd >= m_nExcMmNoMkrListNum)
        {
            bDone = true;
        }
        else
        {
            // Perform exact match on Make, case sensitive
            // Check Make field and possibly Model field (if non-empty)
            if (_tcscmp(m_sExcMmNoMkrList[nInd].strXMake,strMake) != 0)
            {
                // Make did not match
            }
            else
            {
                // Make matched, now check to see if we need
                // to compare the Model string
                if (_tcslen(m_sExcMmNoMkrList[nInd].strXModel) == 0)
                {
                    // No need to compare, we're bDone
                    bFound = true;
                    bDone = true;
                }
                else
                {
                    // Need to check model as well
                    // Since we may like to do a substring match, support wildcards

                    // FnInd position of "*" if it exists in DB entry
                    LPTSTR pWildcard;
                    unsigned nCompareLen;
                    pWildcard = _tcschr(m_sExcMmNoMkrList[nInd].strXModel,'*');
                    if (pWildcard != NULL)
                    {
                        // Wildcard present
                        nCompareLen = pWildcard - (m_sExcMmNoMkrList[nInd].strXModel);
                    }
                    else
                    {
                        // No wildcard, do full match
                        nCompareLen = _tcslen(m_sExcMmNoMkrList[nInd].strXModel);
                    }

                    if (strncmp(m_sExcMmNoMkrList[nInd].strXModel,strModel,nCompareLen) != 0)
                    {
                        // No match
                    }
                    else
                    {
                        // Matched as well, we're bDone
                        bFound = true;
                        bDone = true;
                    }
                }
            }

            nInd++;
        }
    }

    return bFound;
}

// Search exceptions for Make/Model in list of ones that are always edited
bool CDbSigs::LookupExcMmIsEdit(CString strMake,CString strModel)
{
    bool bFound = false;
    bool bDone = false;
    unsigned nInd = 0;
    if (strMake.GetLength() == 0)
    {
        bDone = true;
    }
    while (!bDone)
    {
        if (nInd >= m_nExcMmIsEditListNum)
        {
            bDone = true;
        }
        else
        {
            // Perform exact match, case sensitive
            // Check Make field and possibly Model field (if non-empty)
            if (_tcscmp(m_sExcMmIsEditList[nInd].strXMake,strMake) != 0)
            {
                // Make did not match
            }
            else
            {
                // Make matched, now check to see if we need
                // to compare the Model string
                if (_tcslen(m_sExcMmIsEditList[nInd].strXModel) == 0)
                {
                    // No need to compare, we're bDone
                    bFound = true;
                    bDone = true;
                }
                else
                {
                    // Need to check model as well
                    if (_tcscmp(m_sExcMmIsEditList[nInd].strXModel,strModel) != 0)
                    {
                        // No match
                    }
                    else
                    {
                        // Matched as well, we're bDone
                        bFound = true;
                        bDone = true;
                    }
                }
            }

            nInd++;
        }
    }

    return bFound;
}


// -----------------------------------------------------------------------
// Sample string indicator database
// -----------------------------------------------------------------------


// Sample list of software programs that also use the IJG encoder
LPTSTR CDbSigs::m_sSwIjgList[] =
{
    _T("GIMP"),
    _T("IrfanView"),
    _T("idImager"),
    _T("FastStone Image Viewer"),
    _T("NeatImage"),
    _T("Paint.NET"),
    _T("Photomatix"),
    _T("XnView"),
    _T("*"),
};


// Sample list of software programs marked by EXIF.COMMENT field
//
// NOTE: Have not included the following indicators as they
//       also appear in some digicams in addition to software encoders.
//   "LEAD Technologies"
//   "U-Lead Systems"
//   "Intel(R) JPEG Library" (unsure if ever in hardware)
//
LPTSTR CDbSigs::m_sXComSwList[] =
{
    _T("gd-jpeg"),
    _T("Photoshop"),
    _T("ACD Systems"),
    _T("AppleMark"),
    _T("PICResize"),
    _T("NeatImage"),
    _T("*"),
};


// Software signature list (m_sSigList) is located in "signatures.inl"

