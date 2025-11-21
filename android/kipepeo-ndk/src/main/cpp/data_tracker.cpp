#include "data_tracker.h"
#include <android/log.h>
#include <string.h>
#include <stdio.h>
#include<stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define TAG "KipepeoData"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)

// Global statistics
static DataStats g_data_stats = {0};
static pthread_mutex_t g_data_mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * Try to read network stats from /proc/net/xt_qtaguid/stats
 * This requires root access
 */
static bool read_qtaguid_stats(uint64_t* rx_bytes, uint64_t* tx_bytes) {
    FILE* fp = fopen("/proc/net/xt_qtaguid/stats", "r");
    if (!fp) {
        LOGD("Cannot open /proc/net/xt_qtaguid/stats (requires root)");
        return false;
    }
    
    char line[256];
    uint64_t total_rx = 0;
    uint64_t total_tx = 0;
    
    // Skip header line
    if (fgets(line, sizeof(line), fp) == NULL) {
        fclose(fp);
        return false;
    }
    
    // Read data lines
    while (fgets(line, sizeof(line), fp)) {
        int idx;
        char iface[32];
        int acct_tag_hex;
        int uid_tag_int;
        int cnt_set;
        uint64_t rx, tx;
        
        int parsed = sscanf(line, "%d %s %x %d %d %*d %llu %*d %*d %*d %llu",
                           &idx, iface, &acct_tag_hex, &uid_tag_int, &cnt_set,
                           &rx, &tx);
        
        if (parsed >= 7) {
            total_rx += rx;
            total_tx += tx;
        }
    }
    
    fclose(fp);
    
    *rx_bytes = total_rx;
    *tx_bytes = total_tx;
    
    LOGD("Read qtaguid stats: RX=%llu, TX=%llu", total_rx, total_tx);
    return true;
}

bool data_tracker_init() {
    LOGI("Initializing data tracker...");
    
    pthread_mutex_lock(&g_data_mutex);
    memset(&g_data_stats, 0, sizeof(DataStats));
    pthread_mutex_unlock(&g_data_mutex);
    
    // Try to read initial network stats
    uint64_t rx, tx;
    if (read_qtaguid_stats(&rx, &tx)) {
        LOGI("Data tracker initialized with qtaguid (root mode)");
    } else {
        LOGI("Data tracker initialized in app-level mode (non-root)");
    }
    
    return true;
}

void data_tracker_cleanup() {
    LOGI("Cleaning up data tracker...");
    
    pthread_mutex_lock(&g_data_mutex);
    memset(&g_data_stats, 0, sizeof(DataStats));
    pthread_mutex_unlock(&g_data_mutex);
}

void data_tracker_get_stats(DataStats* stats) {
    if (!stats) return;
    
    pthread_mutex_lock(&g_data_mutex);
    
    // Try to update with latest network stats
    uint64_t rx, tx;
    if (read_qtaguid_stats(&rx, &tx)) {
        // Use network stats if available
        g_data_stats.bytes_received = rx;
        g_data_stats.bytes_sent = tx;
    }
    
    // Calculate compression ratio
    if (g_data_stats.bytes_original > 0) {
        g_data_stats.compression_ratio = 
            (double)g_data_stats.bytes_compressed / (double)g_data_stats.bytes_original;
    } else {
        g_data_stats.compression_ratio = 1.0;
    }
    
    memcpy(stats, &g_data_stats, sizeof(DataStats));
    pthread_mutex_unlock(&g_data_mutex);
}

void data_tracker_reset_stats() {
    LOGI("Resetting data statistics...");
    
    pthread_mutex_lock(&g_data_mutex);
    memset(&g_data_stats, 0, sizeof(DataStats));
    pthread_mutex_unlock(&g_data_mutex);
}

void data_tracker_update(uint64_t original_size, uint64_t compressed_size, bool is_encode) {
    pthread_mutex_lock(&g_data_mutex);
    
    g_data_stats.bytes_original += original_size;
    g_data_stats.bytes_compressed += compressed_size;
    
    if (original_size > compressed_size) {
        g_data_stats.bytes_saved += (original_size - compressed_size);
    }
    
    LOGD("Data update: original=%llu, compressed=%llu, saved=%llu (encode=%d)",
         original_size, compressed_size, g_data_stats.bytes_saved, is_encode);
    
    pthread_mutex_unlock(&g_data_mutex);
}
