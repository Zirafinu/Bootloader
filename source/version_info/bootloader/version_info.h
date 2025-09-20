#ifndef BOOTLOADER_VERSION_INFO_VERSION_INFO_H_
#define BOOTLOADER_VERSION_INFO_VERSION_INFO_H_

#include <cstdint>

namespace version {
struct info {
    /** the software version */
    uint32_t version;
    /** the compatible hardware */
    uint32_t product_id;
    /** the time stamp of the build */
    uint32_t build_time;
};
} // namespace version

#endif
