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


#include "../main/WindowsClasses.h"

#include "DocLog.h"

// #include "JPEGsnoopDoc.h"

//
// Initialize the log
//
CDocLog::CDocLog(void)
{
	logFile = fopen("LogFile.txt", "w");
	return;
}

CDocLog::~CDocLog(void)
{
	fclose(logFile);
}


// Enable logging
void CDocLog::Enable() {
	// m_bEn = true;
};

// Disable logging
void CDocLog::Disable() {
	// m_bEn = false;
};

// Enable or disable the quick log mode
//
// INPUT:
// - bQuick			= If true, write to log buffer, if false, write to CDocument
//
void CDocLog::SetQuickMode(bool bQuick)
{
	return;
}

// Get the current quick log mode
//
// RETURN:
// - Are we in quick mode?
//
bool CDocLog::GetQuickMode()
{
	return false;
}

void CDocLog::Clear()
{

	return;
}


// Add a basic text line to the log
void CDocLog::AddLine(const char * strTxt)
{
	fprintf(logFile, "%s\n", strTxt);
	// COLORREF		sCol;
	// if (m_bEn) {
	// 	sCol = RGB(1, 1, 1);
	// 	// TODO: Do I really need newline in these line outputs?
	// 	if (m_bUseDoc) {
	// 		if (m_bLogQuickMode) {
	// 			AppendToLogLocal(strTxt+_T("\n"),sCol);
	// 		} else {
	// 			CJPEGsnoopDoc*	pSnoopDoc = (CJPEGsnoopDoc*)m_pDoc;
	// 			pSnoopDoc->AppendToLog(strTxt+_T("\n"),sCol);
	// 		}
	// 	} else {
	// 		AppendToLogLocal(strTxt+_T("\n"),sCol);
	// 	}
	// }
}

// Add a header text line to the log
void CDocLog::AddLineHdr(const char * strTxt)
{
	fprintf(logFile, "%s\n", strTxt);
	// COLORREF		sCol = RGB(1, 1, 255);
	// if (m_bEn) {
	// 	if (m_bUseDoc) {
	// 		if (m_bLogQuickMode) {
	// 			AppendToLogLocal(strTxt+_T("\n"),sCol);
	// 		} else {
	// 		CJPEGsnoopDoc*	pSnoopDoc = (CJPEGsnoopDoc*)m_pDoc;
	// 		pSnoopDoc->AppendToLog(strTxt+_T("\n"),sCol);
	// 		}
	// 	} else {
	// 		AppendToLogLocal(strTxt+_T("\n"),sCol);
	// 	}
	// }
}

// Add a header description text line to the log
void CDocLog::AddLineHdrDesc(const char * strTxt)
{
	fprintf(logFile, "%s\n", strTxt);
	// COLORREF		sCol = RGB(32, 32, 255);
	// if (m_bEn) {
	// 	if (m_bUseDoc) {
	// 		if (m_bLogQuickMode) {
	// 			AppendToLogLocal(strTxt+_T("\n"),sCol);
	// 		} else {
	// 		CJPEGsnoopDoc*	pSnoopDoc = (CJPEGsnoopDoc*)m_pDoc;
	// 		pSnoopDoc->AppendToLog(strTxt+_T("\n"),sCol);
	// 		}
	// 	} else {
	// 		AppendToLogLocal(strTxt+_T("\n"),sCol);
	// 	}
	// }
}

// Add a warning text line to the log
void CDocLog::AddLineWarn(const char * strTxt)
{
	fprintf(logFile, "%s\n", strTxt);
	// COLORREF		sCol = RGB(128, 1, 1);
	// if (m_bEn) {
	// 	if (m_bUseDoc) {
	// 		if (m_bLogQuickMode) {
	// 			AppendToLogLocal(strTxt+_T("\n"),sCol);
	// 		} else {
	// 			CJPEGsnoopDoc*	pSnoopDoc = (CJPEGsnoopDoc*)m_pDoc;
	// 			pSnoopDoc->AppendToLog(strTxt+_T("\n"),sCol);
	// 		}
	// 	} else {
	// 		AppendToLogLocal(strTxt+_T("\n"),sCol);
	// 	}
	// }
}

// Add an error text line to the log
void CDocLog::AddLineErr(const char * strTxt)
{
	fprintf(logFile, "%s\n", strTxt);
	// COLORREF		sCol = RGB(255, 1, 1);
	// if (m_bEn) {
	// 	if (m_bUseDoc) {
	// 		if (m_bLogQuickMode) {
	// 			AppendToLogLocal(strTxt+_T("\n"),sCol);
	// 		} else {
	// 			CJPEGsnoopDoc*	pSnoopDoc = (CJPEGsnoopDoc*)m_pDoc;
	// 			pSnoopDoc->AppendToLog(strTxt+_T("\n"),sCol);
	// 		}
	// 	} else {
	// 		AppendToLogLocal(strTxt+_T("\n"),sCol);
	// 	}
	// }
}

// Add a "good" indicator text line to the log
void CDocLog::AddLineGood(const char * strTxt)
{
	fprintf(logFile, "%s\n", strTxt);
	// COLORREF		sCol = RGB(16, 128, 16);
	// if (m_bEn) {
	// 	if (m_bUseDoc) {
	// 		if (m_bLogQuickMode) {
	// 			AppendToLogLocal(strTxt+_T("\n"),sCol);
	// 		} else {
	// 			CJPEGsnoopDoc*	pSnoopDoc = (CJPEGsnoopDoc*)m_pDoc;
	// 			pSnoopDoc->AppendToLog(strTxt+_T("\n"),sCol);
	// 		}
	// 	} else {
	// 		AppendToLogLocal(strTxt+_T("\n"),sCol);
	// 	}
	// }
}

// ======================================================================

unsigned CDocLog::AppendToLogLocal(const char * strTxt, COLORREF sColor)
{
	// Don't exceed a realistic maximum!
	// unsigned numLines = m_saLogQuickTxt.GetCount();
	// if (numLines == DOCLOG_MAX_LINES) {
	// 	m_saLogQuickTxt.Add(_T("*** TOO MANY LINES IN REPORT -- TRUNCATING ***"));
	// 	m_naLogQuickCol.Add((unsigned)sColor);
	// 	return 0;
	// } else if (numLines > DOCLOG_MAX_LINES) {
	// 	return 0;
	// }

	// m_saLogQuickTxt.Add(strTxt);
	// m_naLogQuickCol.Add((unsigned)sColor);

	// return 0;
	return 0;
}

// Get the number of lines in the local log or quick buffer
unsigned CDocLog::GetNumLinesLocal()
{
	return 10;
	// return m_saLogQuickTxt.GetCount();
}

// // Fetch a line from the local log
// // Returns false if line number is out of range
// bool CDocLog::GetLineLogLocal(unsigned nLine,const char * &strOut,COLORREF &sCol)
// {
// 	return true;
// }

// // Save the current log to text file with a simple implementation
// //
// // - This routine is implemented with a simple output mechanism rather
// //   than leveraging CRichEditCtrl::DoSave() so that we can perform
// //   this operation in command-line mode or batch mode operations.
// //
// void CDocLog::DoLogSave(const char * strLogName)
// {
// 	return;
// 	// CStdioFile*	pLog;

// 	// // Open the file for output
// 	// ASSERT(strLogName != _T(""));

// 	// // OLD COMMENTS FOLLOW
// 	// // This save method will only work if we were in Quick Log mode
// 	// // where we have recorded the log to a string buffer and not
// 	// // directly to the RichEdit
// 	// // Note that m_bLogQuickMode is only toggled on during processing
// 	// // so we can't check the status here (except seeing that there are no
// 	// // lines in the array)

// 	// // TODO: Ensure file doesn't exist and only overwrite if specified in command-line?

// 	// // TODO:
// 	// // Confirm that we are not writing to the same file we opened (m_strPathName)
// 	// // ASSERT(strLogName != m_strPathName);

// 	// try
// 	// {
// 	// 	// Open specified file
// 	// 	pLog = new CStdioFile(strLogName, CFile::modeCreate| CFile::modeWrite | CFile::typeText | CFile::shareDenyNone);
// 	// }
// 	// catch (CFileException* e)
// 	// {
// 	// 	TCHAR msg[MAX_BUF_EX_ERR_MSG];
// 	// 	const char * strError;
// 	// 	e->GetErrorMessage(msg,MAX_BUF_EX_ERR_MSG);
// 	// 	e->Delete();
// 	// 	strError.Format(_T("ERROR: Couldn't open file for write [%s]: [%s]"),
// 	// 		(LPCTSTR)strLogName, (LPCTSTR)msg);
// 	// 	// FIXME: Find an alternate method of signaling error in command-line mode
// 	// 	AfxMessageBox(strError);
// 	// 	pLog = NULL;

// 	// 	return;

// 	// }

// 	// // Step through the current log buffer
// 	// const char *		strLine;
// 	// COLORREF	sCol;
// 	// unsigned nQuickLines = GetNumLinesLocal();
// 	// for (unsigned nLine=0;nLine<nQuickLines;nLine++)
// 	// {
// 	// 	GetLineLogLocal(nLine,strLine,sCol);
// 	// 	pLog->WriteString(strLine);
// 	// }

// 	// // Close the file
// 	// if (pLog) {
// 	// 	pLog->Close();
// 	// 	delete pLog;
// 	// }

// }
