#include <algorithm>
#include <cstddef>
#include <cstdint>

extern "C" {
int main() noexcept;
void Reset_Handler() noexcept;

extern uint32_t application_skip_request;
constexpr uint32_t application_skip_request_true = 0xC1AEE345UL;
bool application_requested_skipping() noexcept {
  return application_skip_request == application_skip_request_true;
}
void reset_application_skipping_request() noexcept {
  application_skip_request = ~application_skip_request_true;
}

void copy_backup_to_application() noexcept {}

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

void init_mem() noexcept {
  // set core in hsi clock mode, with pll enabled (just punsh out the
  // registers)

  // initialize the crc accalerator and set up a dma, to feed the application
  // into it
  for (auto *src = &data_load_start, *dst = &data_start; src != &data_load_end;
       ++src, ++dst) {
    *dst = *src;
  }
  for (auto *dst = &bss_start; dst != &bss_end; ++dst) {
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
