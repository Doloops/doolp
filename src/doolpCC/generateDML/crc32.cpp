#include <crc32.h>

crc32 crc32_table[0xFF];


// Reflection is a requirement for the official CRC-32 standard.
// You can create CRCs without it, but they won't conform to the standard.
crc32 crc32_reflect(crc32 ref, char ch)
{// Used only by crc32_init()

     crc32 value = 0;

     // Swap bit 0 for bit 7
     // bit 1 for bit 6, etc.
     for(int i = 1; i < (ch + 1); i++)
     {
          if(ref & 1)
               value |= 1 << (ch - i);
          ref >>= 1;
     }
     return value;
}

void crc32_init ()
{
  crc32 base = 0xabdebb1e;
  for ( int i = 0 ; i <= 0xFF; i++)
    {
      crc32_table[i] = crc32_reflect(i, 8) << 24;
      for (int j = 0; j < 8; j++)
	crc32_table[i] = (crc32_table[i] << 1) ^ (crc32_table[i] & (1 << 31) ? base : 0);
      crc32_table[i] = crc32_reflect(crc32_table[i], 32);

    }
}

crc32 crc32_get ( char * buffer )
{
  crc32 key = 0xffffffff;
  while(*buffer != '\0')
    key = (key >> 8) ^ crc32_table[(key & 0xFF) ^ *buffer++];
  return key ^ 0xffffffff;
}
