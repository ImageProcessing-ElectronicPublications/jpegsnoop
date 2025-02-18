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

// ==========================================================================
// CLASS DESCRIPTION:
// - Simple wrapper for document log routines
//
// ==========================================================================

#pragma once

#include "windowsclasses.h"
#include <stdio.h>

#ifndef _DOCLOG_H_
#define _DOCLOG_H_

class CDocLog
{
public:
    CDocLog(char * Output, uint64_t MaxLength);
    ~CDocLog(void);

    void        AddLine(const char * str);
    void        AddLineHdr(const char * str);
    void        AddLineHdrDesc(const char * str);
    void        AddLineWarn(const char * str);
    void        AddLineErr(const char * str);
    void        AddLineGood(const char * str);

    void        Enable();
    void        Disable();
    void        SetQuickMode(bool bQuick);
    bool        GetQuickMode();

    // void        SetDoc(CDocument *pDoc);
    void        Clear();

    unsigned    GetNumLinesLocal();
    // bool        GetLineLogLocal(unsigned nLine,char * &strOut,COLORREF &sCol);

    // void        DoLogSave(char * strLogName);

private:
    unsigned    AppendToLogLocal(const char * strTxt, COLORREF sColor);

private:

    uint64_t max_length;
    uint64_t written;
    char * log_out;
};

#endif
