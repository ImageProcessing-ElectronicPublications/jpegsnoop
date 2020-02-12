#include "../source/DocLog.h"
#include "../source/JfifDecode.h"
#include "../source/WindowBuf.h"
#include "../source/DbSigs.h"
#include "WindowsClasses.h"

int main(int argc, char ** argv)
{
    char * filename;

    if (argc > 1)
        filename = argv[1];
    else
        filename = "file.jpg";

    CDbSigs * HashDB = new CDbSigs();

    CDocLog * glb_pDocLog = new CDocLog();

	// Allocate the file window buffer
	CwindowBuf * m_pWBuf = new CwindowBuf();

    CFile * jpegfile = new CFile(filename, 0);
    m_pWBuf->BufFileSet(jpegfile); /* TODO:" file here" */

	CimgDecode * m_pImgDec = new CimgDecode(glb_pDocLog,m_pWBuf);
    // m_pImgDec->

	CjfifDecode * m_pJfifDec = new CjfifDecode(glb_pDocLog,m_pWBuf,m_pImgDec, HashDB);

    m_pJfifDec->ProcessFile(jpegfile);

    delete glb_pDocLog;

    return 0;
}
