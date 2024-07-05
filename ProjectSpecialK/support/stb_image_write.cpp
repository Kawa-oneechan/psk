#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBIW_CRC32 stb_crc32
extern "C" { extern unsigned int stb_crc32(unsigned char *ptr, int buf_len); }
#include "stb_image_write.h"
