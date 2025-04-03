#include <cstdint>

const uint32_t crc32_bzip2_tab[16] = {
    0x00000000, 0x04C11DB7, 0x09823B6E, 0x0D4326D9, //
    0x130476DC, 0x17C56B6B, 0x1A864DB2, 0x1E475005, //
    0x2608EDB8, 0x22C9F00F, 0x2F8AD6D6, 0x2B4BCB61, //
    0x350C9B64, 0x31CD86D3, 0x3C8EA00A, 0x384FBDBD, //
};

//
uint32_t crc(const void *mem_begin, const void *mem_end) noexcept
{
  const uint8_t *p = reinterpret_cast<const uint8_t *>(mem_begin);
  uint32_t crc;

  crc = ~0U;
  while (p != mem_end)
  {
    const uint8_t byte = *p++;
    crc = (crc << 4) ^ crc32_bzip2_tab[((crc >> 28) ^ (byte >> 4)) & 0xF];
    crc = (crc << 4) ^ crc32_bzip2_tab[((crc >> 28) ^ byte) & 0xF];
  }
  return crc ^ ~0U;
}
