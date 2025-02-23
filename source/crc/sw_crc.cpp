#include <cstdint>

const uint32_t crc32_tab[16] = {
    0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, //
    0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3, //
    0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988, //
    0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, //
};

//
uint32_t crc(const void *mem_begin, const void *mem_end) noexcept {
  const uint8_t *p = reinterpret_cast<const uint8_t *>(mem_begin);
  uint32_t crc;

  crc = ~0U;
  while (p != mem_end) {
    const uint8_t byte = *p++;
    crc = crc32_tab[(crc ^ (byte >> 4)) & 0xF] ^ (crc >> 4);
    crc = crc32_tab[(crc ^ byte) & 0xF] ^ (crc >> 4);
  }
  return crc ^ ~0U;
}
