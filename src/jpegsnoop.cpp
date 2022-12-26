#include "include/doclog.h"
#include "include/jfifdecode.h"
#include "include/windowbuf.h"
#include "include/dbsigs.h"
#include "include/windowsclasses.h"

static CDbSigs * HashDB;

#include "jpegsnoop.h"

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

void UninitJPEGSnoop()
{
    delete HashDB;
}


/* The following is a usage example */
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
    if (argc > 1)
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

        void UninitJPEGSnoop();

        return 0;
    }
    else
    {
        printf("Usage: %s you.jpg", argv[0]);
    }
}
#endif
