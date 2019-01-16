#ifndef __CRC32__H
#define __CRC32__H

typedef unsigned int crc32;

void crc32_init ();

crc32 crc32_get ( char * buffer );
 
#endif // __CRC32__H
