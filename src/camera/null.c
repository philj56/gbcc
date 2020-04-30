#include "camera.h"
#include <stdint.h>

void gbcc_camera_platform_capture_image(uint8_t image[GB_CAMERA_SENSOR_SIZE]) {
	gbcc_camera_default_image(image);
}
