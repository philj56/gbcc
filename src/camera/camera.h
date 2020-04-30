#include "../gbcc.h"

#define GB_CAMERA_SENSOR_WIDTH 128
#define GB_CAMERA_SENSOR_HEIGHT 128
#define GB_CAMERA_SENSOR_SIZE (GB_CAMERA_SENSOR_HEIGHT * GB_CAMERA_SENSOR_WIDTH)

void gbcc_camera_capture_image(struct gbcc_core *gbc);
void gbcc_camera_platform_capture_image(uint8_t image[GB_CAMERA_SENSOR_SIZE]);
void gbcc_camera_default_image(uint8_t *image);
