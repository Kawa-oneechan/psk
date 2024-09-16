#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBIW_CRC32 GetCRC
extern unsigned int GetCRC(unsigned char *ptr, int buf_len);
#define STBIW_WINDOWS_UTF8
//drop 90 K, we only ever save PNG files
#define STBIW_ONLY_PNG
#include "stb_image_write.h"
