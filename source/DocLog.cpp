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


#include "WindowsClasses.h"

#include "DocLog.h"

// #include "JPEGsnoopDoc.h"

//
// Initialize the log
//
CDocLog::CDocLog(char * Output, uint64_t MaxLength)
{
	this->log_out = Output;
	this->written = 0;
	this->max_length = MaxLength-1;
	return;
}

CDocLog::~CDocLog(void)
{
	return;
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
	/* +1 for the newline character */
	uint64_t length_text = strlen(strTxt)+1;

	/* Only write if it fits */
	if (written+length_text < max_length)
	{
		sprintf(log_out+written, "%s\n", strTxt);
		written += length_text;
	}
}

// Add a header text line to the log
void CDocLog::AddLineHdr(const char * strTxt)
{
	AddLine(strTxt);
}

// Add a header description text line to the log
void CDocLog::AddLineHdrDesc(const char * strTxt)
{
	AddLine(strTxt);
}

// Add a warning text line to the log
void CDocLog::AddLineWarn(const char * strTxt)
{
	AddLine(strTxt);
}

// Add an error text line to the log
void CDocLog::AddLineErr(const char * strTxt)
{
	AddLine(strTxt);
}

// Add a "good" indicator text line to the log
void CDocLog::AddLineGood(const char * strTxt)
{
	AddLine(strTxt);
}

// ======================================================================

unsigned CDocLog::AppendToLogLocal(const char * strTxt, COLORREF sColor)
{
	AddLine(strTxt);
	return 0;
}

// Get the number of lines in the local log or quick buffer
unsigned CDocLog::GetNumLinesLocal()
{
	return 10;
	// return m_saLogQuickTxt.GetCount();
}