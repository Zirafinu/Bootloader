#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>

#include <crc.h>
#include <flash_layout.h>
#include <gzip.h>

extern "C"
{
  int main() noexcept;
  void Reset_Handler() noexcept;

  bool application_is_valid() noexcept
  {
    return *reinterpret_cast<uint32_t *>(flash_layout::appl_end - 4) ==
           crc::compute(reinterpret_cast<uint8_t *>(flash_layout::appl_begin),
                        reinterpret_cast<uint8_t *>(flash_layout::appl_end - 4));
  }

  bool application_backup_is_valid() noexcept
  {
    return *reinterpret_cast<uint32_t *>(flash_layout::appl_backup_end - 4) ==
           crc::compute(reinterpret_cast<uint8_t *>(flash_layout::appl_backup_begin),
                        reinterpret_cast<uint8_t *>(flash_layout::appl_backup_end - 4));
  }

  static void erase_application() noexcept {}

  using flash_layout::appl_backup_begin;
  using flash_layout::appl_backup_end;
  using flash_layout::appl_begin;
  using flash_layout::appl_end;

  static uint8_t const *read_backup_it = nullptr;
  static uint8_t *write_application_it = nullptr;
  static std::array<uint8_t, 32> reab_buffer{};
  static uint16_t read_buffer_consumed = 0;
  static std::array<uint8_t, 1024> write_application_buffer{};
  static uint16_t write_application_buffer_used = 0;

  static bool can_read_more()
  {
    return read_backup_it <
           reinterpret_cast<uint8_t const *>(flash_layout::appl_backup_end);
  }
  static uint8_t read_appl_backup() { return *read_backup_it++; }
  static void write_application(uint8_t value)
  {
    write_application_buffer[write_application_buffer_used] = value;
    ++write_application_buffer_used;
    if (write_application_buffer_used == write_application_buffer.size())
    {
      // flush buffer
      write_application_buffer_used = 0;
      std::memcpy(write_application_it, write_application_buffer.data(),
                  write_application_buffer.size());
      write_application_it += write_application_buffer.size();
    }
  }
  static uint8_t read_application(std::size_t distance)
  {
    if (distance > write_application_buffer_used)
    {
      return *(write_application_it - (distance - write_application_buffer_used));
    }
    return write_application_buffer[write_application_buffer_used - distance];
  }

  void copy_backup_to_application() noexcept
  {
    erase_application();
    read_backup_it =
        reinterpret_cast<uint8_t const *>(&flash_layout::appl_backup_begin);
    write_application_it = reinterpret_cast<uint8_t *>(
        const_cast<size_t *>(&flash_layout::appl_begin));
    gzip::Inflate inflator{can_read_more, read_appl_backup, write_application,
                           read_application};
    inflator.decode();
    // flush buffer
    std::memcpy(write_application_it, write_application_buffer.data(),
                write_application_buffer_used);
  }

  // the applications reset handler address
  using entry_function_t = void (*)() noexcept;
  extern const entry_function_t application_entry_function
      __attribute__((noreturn));

  void jump_to_application() noexcept { application_entry_function(); }

  // the addresses to constant data
  extern size_t data_load_start;
  extern size_t data_load_end;
  extern size_t data_start;
  // the addresses to zero out
  extern size_t bss_start;
  extern size_t bss_end;
  // the initial_stack address
  extern const size_t estack;

  __attribute__((used)) void init_mem() noexcept
  {
    // set core in hsi clock mode, with pll enabled (just push out the
    // registers)

    // initialize the crc accelerator and set up a dma, to feed the application
    // into it

    std::memcpy(&data_start, &data_load_start, size_t(&data_load_end) - size_t(&data_load_start));
    std::memset(&bss_start, 0, size_t(&bss_end) - size_t(&bss_start));

    // __libc_init_array();
    main();
  }
}
