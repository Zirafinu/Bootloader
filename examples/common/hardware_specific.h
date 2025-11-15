#include <array>
#include <cstdint>

namespace bootloader::hardware_specific {
/**
 * (de)initialize the peripherals to update the application
 */
void initialize_core_for_update();
/**
 * (de)initialize the peripherals to launch the application
 */
void initialize_core_for_application_start();

/**
 * flush the write buffer to memory
 * @return true if write was success full
 */
[[nodiscard]]
bool flush_write_buffer(uint8_t *target, std::array<uint8_t, 1024> const &data, std::size_t valid_data);

/**
 * ersase the application memory
 * @return true if the application memory can be overwritten
 */
[[nodiscard]]
bool erase_application();
} // namespace bootloader::hardware_specific
