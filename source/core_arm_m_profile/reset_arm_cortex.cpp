#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>

#include <flash_layout.h>
#include <gzip.h>
#include "../bootloader.h"

extern "C"
{
  int main() noexcept;
  void Reset_Handler() noexcept;

  bool application_is_valid() noexcept
  {
    return *reinterpret_cast<uint32_t *>(flash_layout::appl_end - 4) ==
           crc(reinterpret_cast<uint8_t *>(flash_layout::appl_begin),
               reinterpret_cast<uint8_t *>(flash_layout::appl_end - 4));
  }

  bool application_backup_is_valid() noexcept
  {
    return *reinterpret_cast<uint32_t *>(flash_layout::appl_backup_end - 4) ==
           crc(reinterpret_cast<uint8_t *>(flash_layout::appl_backup_begin),
               reinterpret_cast<uint8_t *>(flash_layout::appl_backup_end - 4));
  }

  extern uint32_t application_skip_request;
  constexpr uint32_t application_skip_request_true = 0xC1AEE345UL;
  bool application_requested_skipping() noexcept
  {
    return application_skip_request == application_skip_request_true;
  }
  void reset_application_skipping_request() noexcept
  {
    application_skip_request = ~application_skip_request_true;
  }

  static void erase_application() noexcept {}

  using flash_layout::appl_backup_begin;
  using flash_layout::appl_backup_end;
  using flash_layout::appl_begin;
  using flash_layout::appl_end;

  static uint8_t const *read_backup_it = nullptr;
  static uint8_t *write_application_it = nullptr;
  static std::array<uint8_t, 1024> write_application_buffer{};
  static uint16_t write_application_buffer_used = 0;

  static bool can_read_more() { return read_backup_it < reinterpret_cast<uint8_t const *>(flash_layout::appl_backup_end); }
  static uint8_t read_appl_backup() { return *read_backup_it++; }
  static void write_application(uint8_t value)
  {
    write_application_buffer[write_application_buffer_used] = value;
    ++write_application_buffer_used;
    if (write_application_buffer_used == write_application_buffer.size())
    {
      // flush buffer
      write_application_buffer_used = 0;
      std::memcpy(write_application_it, write_application_buffer.data(), write_application_buffer.size());
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
    read_backup_it = reinterpret_cast<uint8_t const *>(&flash_layout::appl_backup_begin);
    write_application_it = reinterpret_cast<uint8_t *>(const_cast<size_t *>(&flash_layout::appl_begin));
    gzip::Inflate inflator{can_read_more, read_appl_backup, write_application, read_application};
    inflator.decode();
    // flush buffer
    std::memcpy(write_application_it, write_application_buffer.data(), write_application_buffer_used);
  }

  // the applications reset handler address
  using entry_function_t = void (*)() noexcept;
  extern const entry_function_t application_entry_function;

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

  void init_mem() noexcept
  {
    // set core in hsi clock mode, with pll enabled (just push out the
    // registers)

    // initialize the crc accelerator and set up a dma, to feed the application
    // into it
    for (auto *src = &data_load_start, *dst = &data_start; src != &data_load_end;
         ++src, ++dst)
    {
      *dst = *src;
    }
    for (auto *dst = &bss_start; dst != &bss_end; ++dst)
    {
      *dst = 0;
    }
    // __libc_init_array();
    main();
  }

  __attribute__((section(".isr_vector"))) const void *const isr_vector[] = {
      &estack,
      reinterpret_cast<void *const>(&Reset_Handler),
  };
}
