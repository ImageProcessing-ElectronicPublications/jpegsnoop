#ifndef _Snoop_h
#define _Snoop_h_

void InitJPEGSnoop();

void RunJPEGSnoop( void * JPEG,
                   uint64_t JPEGSize,
                   char * LogOutput,
                   uint64_t LogMax );

#endif