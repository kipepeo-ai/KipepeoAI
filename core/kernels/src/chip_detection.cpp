#include "kipepeo/kernels/chip_detection.h"
#include <cstring>
#include <cstdio>
#include <fstream>
#include <string>

#ifdef __ANDROID__
#include <sys/system_properties.h>
#endif

namespace kipepeo {
namespace kernels {

namespace {
    // Cache detected chip type
    ChipType g_detected_chip = ChipType::UNKNOWN;
    bool g_chip_detected = false;

    // Helper function to read /proc/cpuinfo
    std::string read_cpuinfo() {
        std::string result;
        std::ifstream file("/proc/cpuinfo");
        if (file.is_open()) {
            std::string line;
            while (std::getline(file, line)) {
                result += line + "\n";
            }
            file.close();
        }
        return result;
    }

    // Parse chip from /proc/cpuinfo
    ChipType detect_from_cpuinfo() {
        std::string cpuinfo = read_cpuinfo();
        if (cpuinfo.empty()) {
            return ChipType::UNKNOWN;
        }

        // MediaTek detection
        if (cpuinfo.find("MT6769") != std::string::npos || 
            cpuinfo.find("Helio G85") != std::string::npos ||
            cpuinfo.find("helio g85") != std::string::npos) {
            return ChipType::MEDIATEK_HELIO_G85;
        }
        
        if (cpuinfo.find("MT6789") != std::string::npos ||
            cpuinfo.find("Helio G99") != std::string::npos ||
            cpuinfo.find("helio g99") != std::string::npos) {
            return ChipType::MEDIATEK_HELIO_G99;
        }
        
        if (cpuinfo.find("MT6791") != std::string::npos ||
            cpuinfo.find("Helio G100") != std::string::npos ||
            cpuinfo.find("helio g100") != std::string::npos) {
            return ChipType::MEDIATEK_HELIO_G100;
        }

        // Qualcomm detection
        if (cpuinfo.find("SM7435") != std::string::npos ||
            cpuinfo.find("Snapdragon 7s Gen 2") != std::string::npos ||
            cpuinfo.find("snapdragon 7s gen 2") != std::string::npos ||
            cpuinfo.find("SM7435-AB") != std::string::npos) {
            return ChipType::QUALCOMM_SNAPDRAGON_7S_GEN2;
        }

        // Unisoc detection
        if (cpuinfo.find("T606") != std::string::npos ||
            cpuinfo.find("unisoc t606") != std::string::npos ||
            cpuinfo.find("UNISOC T606") != std::string::npos) {
            return ChipType::UNISOC_T606;
        }

        // Check for ARM Cortex cores to infer chip family
        if (cpuinfo.find("Cortex-A76") != std::string::npos) {
            // Could be Helio G99/G100
            if (cpuinfo.find("Cortex-A55") != std::string::npos) {
                // Likely Helio G99 or G100
                return ChipType::MEDIATEK_HELIO_G99; // Default to G99
            }
        }
        
        if (cpuinfo.find("Cortex-A78") != std::string::npos) {
            // Could be Snapdragon 7s Gen 2
            if (cpuinfo.find("Cortex-A55") != std::string::npos) {
                return ChipType::QUALCOMM_SNAPDRAGON_7S_GEN2;
            }
        }
        
        if (cpuinfo.find("Cortex-A75") != std::string::npos) {
            // Could be Unisoc T606
            if (cpuinfo.find("Cortex-A55") != std::string::npos) {
                return ChipType::UNISOC_T606;
            }
        }

        return ChipType::UNKNOWN;
    }

#ifdef __ANDROID__
    // Android-specific detection using system properties
    ChipType detect_from_android_props() {
        char prop_value[PROP_VALUE_MAX];
        
        // Try ro.board.platform
        if (__system_property_get("ro.board.platform", prop_value) > 0) {
            std::string platform = prop_value;
            if (platform.find("mt6769") != std::string::npos) {
                return ChipType::MEDIATEK_HELIO_G85;
            }
            if (platform.find("mt6789") != std::string::npos) {
                return ChipType::MEDIATEK_HELIO_G99;
            }
            if (platform.find("mt6791") != std::string::npos) {
                return ChipType::MEDIATEK_HELIO_G100;
            }
            if (platform.find("lahaina") != std::string::npos || 
                platform.find("taro") != std::string::npos) {
                // Could be Snapdragon 7s Gen 2 or similar
                return ChipType::QUALCOMM_SNAPDRAGON_7S_GEN2;
            }
        }

        // Try ro.chipname
        if (__system_property_get("ro.chipname", prop_value) > 0) {
            std::string chipname = prop_value;
            if (chipname.find("T606") != std::string::npos) {
                return ChipType::UNISOC_T606;
            }
        }

        // Try ro.hardware
        if (__system_property_get("ro.hardware", prop_value) > 0) {
            std::string hardware = prop_value;
            if (hardware.find("mt6769") != std::string::npos) {
                return ChipType::MEDIATEK_HELIO_G85;
            }
            if (hardware.find("mt6789") != std::string::npos) {
                return ChipType::MEDIATEK_HELIO_G99;
            }
            if (hardware.find("mt6791") != std::string::npos) {
                return ChipType::MEDIATEK_HELIO_G100;
            }
        }

        return ChipType::UNKNOWN;
    }
#endif

#ifdef __APPLE__
    // Apple-specific detection
    ChipType detect_apple_chip() {
        // On Apple platforms, we can use sysctlbyname or compile-time detection
        // For now, use compile-time detection based on architecture
        #if defined(__ARM_ARCH_8_5__) || defined(__ARM_ARCH_8_6__)
            // A17 Pro or M3/M4
            return ChipType::APPLE_A17; // Default to A17, could be refined
        #elif defined(__ARM_ARCH_8_4__)
            // A16 or M2
            return ChipType::APPLE_A16;
        #elif defined(__ARM_ARCH_8_3__)
            // A15 or M1
            return ChipType::APPLE_A15;
        #else
            // Older Apple Silicon
            return ChipType::APPLE_M1;
        #endif
    }
#endif
} // anonymous namespace

ChipType detect_chip() {
    if (g_chip_detected) {
        return g_detected_chip;
    }

#ifdef __APPLE__
    g_detected_chip = detect_apple_chip();
#elif defined(__ANDROID__)
    // Try Android system properties first
    g_detected_chip = detect_from_android_props();
    if (g_detected_chip == ChipType::UNKNOWN) {
        // Fallback to /proc/cpuinfo
        g_detected_chip = detect_from_cpuinfo();
    }
#else
    // Linux or other platforms
    g_detected_chip = detect_from_cpuinfo();
#endif

    g_chip_detected = true;
    return g_detected_chip;
}

const char* get_chip_name(ChipType chip) {
    switch (chip) {
        case ChipType::MEDIATEK_HELIO_G85:
            return "MediaTek Helio G85";
        case ChipType::MEDIATEK_HELIO_G99:
            return "MediaTek Helio G99";
        case ChipType::MEDIATEK_HELIO_G100:
            return "MediaTek Helio G100";
        case ChipType::QUALCOMM_SNAPDRAGON_7S_GEN2:
            return "Qualcomm Snapdragon 7s Gen 2";
        case ChipType::UNISOC_T606:
            return "Unisoc T606";
        case ChipType::APPLE_A13:
            return "Apple A13";
        case ChipType::APPLE_A14:
            return "Apple A14";
        case ChipType::APPLE_A15:
            return "Apple A15";
        case ChipType::APPLE_A16:
            return "Apple A16";
        case ChipType::APPLE_A17:
            return "Apple A17";
        case ChipType::APPLE_A18:
            return "Apple A18";
        case ChipType::APPLE_M1:
            return "Apple M1";
        case ChipType::APPLE_M2:
            return "Apple M2";
        case ChipType::APPLE_M3:
            return "Apple M3";
        case ChipType::APPLE_M4:
            return "Apple M4";
        default:
            return "Unknown";
    }
}

bool chip_supports_fp16(ChipType chip) {
    switch (chip) {
        case ChipType::MEDIATEK_HELIO_G99:
        case ChipType::MEDIATEK_HELIO_G100:
        case ChipType::QUALCOMM_SNAPDRAGON_7S_GEN2:
        case ChipType::APPLE_A13:
        case ChipType::APPLE_A14:
        case ChipType::APPLE_A15:
        case ChipType::APPLE_A16:
        case ChipType::APPLE_A17:
        case ChipType::APPLE_A18:
        case ChipType::APPLE_M1:
        case ChipType::APPLE_M2:
        case ChipType::APPLE_M3:
        case ChipType::APPLE_M4:
            return true;
        default:
            return false;
    }
}

void get_optimal_block_size(ChipType chip, bool is_big_core, size_t& MR, size_t& NR) {
    switch (chip) {
        case ChipType::MEDIATEK_HELIO_G85:
            // Cortex-A75/A55: smaller cache, use 4x4
            MR = is_big_core ? 4 : 4;
            NR = is_big_core ? 4 : 4;
            break;
            
        case ChipType::MEDIATEK_HELIO_G99:
        case ChipType::MEDIATEK_HELIO_G100:
            // Cortex-A76/A55: 64KB L1, 256KB L2
            MR = is_big_core ? 6 : 4;
            NR = is_big_core ? 6 : 4;
            break;
            
        case ChipType::QUALCOMM_SNAPDRAGON_7S_GEN2:
            // Cortex-A78/A55: 512KB L2, larger cache
            MR = is_big_core ? 8 : 4;
            NR = is_big_core ? 8 : 4;
            break;
            
        case ChipType::UNISOC_T606:
            // Cortex-A75/A55: smaller cache, conservative
            MR = is_big_core ? 4 : 4;
            NR = is_big_core ? 4 : 4;
            break;
            
        case ChipType::APPLE_M1:
        case ChipType::APPLE_M2:
        case ChipType::APPLE_M3:
        case ChipType::APPLE_M4:
        case ChipType::APPLE_A13:
        case ChipType::APPLE_A14:
        case ChipType::APPLE_A15:
        case ChipType::APPLE_A16:
        case ChipType::APPLE_A17:
        case ChipType::APPLE_A18:
            // Apple Silicon: large unified cache, aggressive blocking
            MR = is_big_core ? 16 : 8;
            NR = is_big_core ? 16 : 8;
            break;
            
        default:
            // Default: 4x4
            MR = 4;
            NR = 4;
            break;
    }
}

} // namespace kernels
} // namespace kipepeo

