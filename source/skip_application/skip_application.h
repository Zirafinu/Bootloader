/**
 *
 */
namespace skip_application {
bool skip_is_requested() noexcept;
void skip_reset_request() noexcept;
void skip_request() noexcept;

bool update_is_requested() noexcept;
void update_reset_request() noexcept;
void update_request() noexcept;
} // namespace skip_application
