#ifndef INCLUDE_ARM_M_PROFILE_FLASH_LAYOUT_H
#define INCLUDE_ARM_M_PROFILE_FLASH_LAYOUT_H

#include <cstddef>
#include <cstdint>

namespace flash_layout {
using size_t = std::size_t;

// entire flash memory
constexpr size_t total_begin = ${total_begin};
constexpr size_t total_end = total_begin + ${total_size};

// bootloader flash memory
constexpr size_t bootloader_begin = ${bootl_begin};
constexpr size_t bootloader_end = bootloader_begin + ${bootl_size};

// application/application_update flash memory
constexpr size_t application_begin = ${appli_begin};
constexpr size_t application_end = application_begin + ${appli_size};
constexpr size_t update_begin = ${updat_begin};
constexpr size_t update_end = update_begin + ${updat_size};

// parameters flash memory
constexpr size_t param_begin = ${param_begin};
constexpr size_t param_end = param_begin + ${param_size};

// event_log flash memory
constexpr size_t event_log_begin = ${event_begin};
constexpr size_t event_log_end = event_log_begin + ${event_size};

// resources flash memory
constexpr size_t resources_begin = ${recrs_begin};
constexpr size_t resources_end = resources_begin + ${recrs_size};

static_assert(${total_size} == ${bootl_size} + ${appli_size} + ${updat_size} + ${param_size} + ${event_size} +
                                   ${recrs_size},
              "flash size mismatch");

struct Homogenous_Paged_Area {
    size_t begin;
    size_t bound;
    uint16_t first_page_index;
    uint16_t page_count;

    constexpr bool contains(size_t address) const { return begin <= address && address < bound; }
    constexpr size_t page_of(size_t address) const {
        return ((address - begin) / page_count) + first_page_index;
    }
    constexpr bool can_enumerate_page_of(size_t address) const {
        return begin <= address && address <= bound;
    }
    constexpr bool page_starts_at(size_t address) const { return ((address - begin) % page_count) == 0; }
};

constexpr Homogenous_Paged_Area flash_areas[] = {${HOMOGENOUS_PAGED_AREA_INITIALIZER}};

constexpr size_t page_of(size_t address) {
    for (const auto &area : flash_areas) {
        if (area.can_enumerate_page_of(address)) {
            return area.page_of(address);
        }
    }
    return ~size_t(0);
}

constexpr bool is_on_page_boundary(size_t address) {
    for (const auto &area : flash_areas) {
        if (area.can_enumerate_page_of(address) && area.page_starts_at(address)) {
            return true;
        }
    }
    return false;
}

constexpr size_t bootloader_begin_page = page_of(bootloader_begin);
constexpr size_t bootloader_end_page = page_of(bootloader_end);

constexpr size_t application_begin_page = page_of(application_begin);
constexpr size_t application_end_page = page_of(application_end);
constexpr size_t update_begin_page = page_of(update_begin);
constexpr size_t update_end_page = page_of(update_end);

constexpr size_t param_begin_page = page_of(param_begin);
constexpr size_t param_end_page = page_of(param_end);

constexpr size_t event_log_begin_page = page_of(event_log_begin);
constexpr size_t event_log_end_page = page_of(event_log_end);

constexpr size_t resources_begin_page = page_of(resources_begin);
constexpr size_t resources_end_page = page_of(resources_end);

static_assert(is_on_page_boundary(bootloader_begin));
static_assert(is_on_page_boundary(bootloader_end));
static_assert(is_on_page_boundary(application_begin));
static_assert(is_on_page_boundary(application_end));
static_assert(is_on_page_boundary(update_begin));
static_assert(is_on_page_boundary(update_end));
static_assert(is_on_page_boundary(param_begin));
static_assert(is_on_page_boundary(param_end));
static_assert(is_on_page_boundary(event_log_begin));
static_assert(is_on_page_boundary(event_log_end));
static_assert(is_on_page_boundary(resources_begin));
static_assert(is_on_page_boundary(resources_end));
} // namespace flash_layout

#endif
