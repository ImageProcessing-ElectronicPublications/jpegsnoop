#ifndef _JPEGSNOOP_H_
#define _JPEGSNOOP_H_

void InitJPEGSnoop();

void RunJPEGSnoop( void * JPEG,
                   uint64_t JPEGSize,
                   char * LogOutput,
                   uint64_t LogMax );

void UninitJPEGSnoop();

#endif
