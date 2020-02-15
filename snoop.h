#ifndef _JPEGSnoop_h
#define _JPEGSnoop_h_

void InitJPEGSnoop();

void RunJPEGSnoop( void * JPEG,
                   uint64_t JPEGSize,
                   char * LogOutput,
                   uint64_t LogMax );

#endif