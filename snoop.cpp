#include "source/DocLog.h"
#include "source/JfifDecode.h"
#include "source/WindowBuf.h"
#include "source/DbSigs.h"
#include "source/WindowsClasses.h"

static CDbSigs * HashDB;

#include "snoop.h"

void InitJPEGSnoop()
{
    HashDB = new CDbSigs();
}

void RunJPEGSnoop( void * JPEG,
                   uint64_t JPEGSize,
                   char * LogOutput,
                   uint64_t LogMax )
{
    LogOutput[0] = 0;

    CDocLog * glb_pDocLog = new CDocLog(LogOutput, LogMax);
    CwindowBuf * m_pWBuf = new CwindowBuf();
    CFile * jpegfile = new CFile(JPEG, JPEGSize, "File.jpeg");

    m_pWBuf->BufFileSet(jpegfile); /* TODO:" file here" */

    CimgDecode * m_pImgDec = new CimgDecode(glb_pDocLog,m_pWBuf);
    CjfifDecode * m_pJfifDec = new CjfifDecode(glb_pDocLog,m_pWBuf,m_pImgDec, HashDB);

    m_pJfifDec->ProcessFile(jpegfile);

    delete glb_pDocLog;
    delete m_pWBuf;
    delete jpegfile;
    delete m_pImgDec;
    delete m_pJfifDec;
}


#ifdef SNOOP_TEST
void * file2mem(char * FilePath, uint64_t * SizeOutput)
{
    FILE * file = fopen(FilePath, "r");
    fseek(file, 0, SEEK_END);
    uint64_t file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    void * memory = malloc(file_size);
    fread(memory, file_size, 1, file);
    fclose(file);
    *SizeOutput = file_size;
    return memory;
}

int main(int argc, char ** argv)
{
    InitJPEGSnoop();

    uint64_t file_size;
    void * jpeg_file = file2mem(argv[1], &file_size);

    #define LOG_MAX 100000
    char * log_output = (char *)malloc(LOG_MAX);

    RunJPEGSnoop(jpeg_file, file_size, log_output, LOG_MAX);

    puts(log_output);

    free(jpeg_file);
    free(log_output);

    return 0;
}
#endif