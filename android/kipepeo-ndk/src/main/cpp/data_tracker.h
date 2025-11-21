#ifndef KIPEPEO_DATA_TRACKER_H
#define KIPEPEO_DATA_TRACKER_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Data tracking statistics
 */
typedef struct {
    uint64_t bytes_received;      // Total bytes received
    uint64_t bytes_sent;           // Total bytes sent
    uint64_t bytes_original;       // Estimated original size (before compression)
    uint64_t bytes_compressed;     // Actual compressed size
    uint64_t bytes_saved;          // Bytes saved via compression
    double compression_ratio;      // Compression ratio (0.0-1.0)
} DataStats;

/**
 * Initialize data tracking system
 * 
 * @return true if successful, false otherwise
 */
bool data_tracker_init();

/**
 * Cleanup data tracking
 */
void data_tracker_cleanup();

/**
 * Get current data statistics
 * 
 * @param stats Pointer to DataStats structure to fill
 */
void data_tracker_get_stats(DataStats* stats);

/**
 * Reset all statistics
 */
void data_tracker_reset_stats();

/**
 * Update statistics with new encode/decode event
 * 
 * @param original_size Original uncompressed size in bytes
 * @param compressed_size Compressed size in bytes
 * @param is_encode true for encode, false for decode
 */
void data_tracker_update(uint64_t original_size, uint64_t compressed_size, bool is_encode);

#ifdef __cplusplus
}
#endif

#endif // KIPEPEO_DATA_TRACKER_H
