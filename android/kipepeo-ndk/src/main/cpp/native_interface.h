#ifndef KIPEPEO_NATIVE_INTERFACE_H
#define KIPEPEO_NATIVE_INTERFACE_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize the Kipepeo engine
 * 
 * @return true if successful, false otherwise
 */
bool kipepeo_init();

/**
 * Shutdown the Kipepeo engine
 */
void kipepeo_shutdown();

/**
 * Activate codec hooking
 * 
 * @return true if hooks activated, false otherwise
 */
bool kipepeo_activate_engine();

/**
 * Deactivate codec hooking
 */
void kipepeo_deactivate_engine();

/**
 * Check if running with root privileges
 * 
 * @return true if root available, false otherwise
 */
bool kipepeo_is_root_available();

/**
 * Get hook status as string
 * 
 * @return Status string (do not free)
 */
const char* kipepeo_get_hook_status();

/**
 * Get LLM inference tokens per second
 * 
 * @return Tokens per second (0.0 if not running)
 */
float kipepeo_get_tokens_per_second();

/**
 * Get total data saved in bytes
 * 
 * @return Bytes saved via compression
 */
uint64_t kipepeo_get_data_saved();

/**
 * Get compression ratio
 * 
 * @return Compression ratio (0.0-1.0)
 */
double kipepeo_get_compression_ratio();

/**
 * Reset all statistics
 */
void kipepeo_reset_stats();

#ifdef __cplusplus
}
#endif

#endif // KIPEPEO_NATIVE_INTERFACE_H
