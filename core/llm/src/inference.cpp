#include "kipepeo/inference.h"
#include "kipepeo/kernels/chip_detection.h"

using namespace kipepeo::kernels;

extern "C" {

const char* kipepeo_get_soc_type(void) {
    ChipType chip = detect_chip();
    return get_chip_name(chip);
}

} // extern "C"
