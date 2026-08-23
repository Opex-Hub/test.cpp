#ifndef CRASH_HANDLER_H
#define CRASH_HANDLER_H

namespace crash_handler {

/**
 * Initialize the crash handler.
 * @param log_path  Path to log file. If nullptr or empty, stderr is used.
 * @return true on success.
 */
bool install(const char* log_path = nullptr);

/**
 * Restore original signal handlers and close the log file.
 */
void uninstall();

} // namespace crash_handler

#endif // CRASH_HANDLER_H
