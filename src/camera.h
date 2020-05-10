/*
 * Copyright (C) 2017-2020 Philip Jones
 *
 * Licensed under the MIT License.
 * See either the LICENSE file, or:
 *
 * https://opensource.org/licenses/MIT
 *
 */

#ifndef GBCC_CAMERA_H
#define GBCC_CAMERA_H

#include <stdint.h>

#ifdef __linux__
#include "camera_platform/null.h"
#else
#include "camera_platform/null.h"
#endif

struct gbcc;

#define GB_CAMERA_SENSOR_WIDTH 128
#define GB_CAMERA_SENSOR_HEIGHT 128
#define GB_CAMERA_SENSOR_SIZE (GB_CAMERA_SENSOR_HEIGHT * GB_CAMERA_SENSOR_WIDTH)

void gbcc_camera_initialise(struct gbcc *gbc);
void gbcc_camera_destroy(struct gbcc *gbc);
void gbcc_camera_clock(struct gbcc *gbc);
void gbcc_camera_capture_image(struct gbcc *gbc);
void gbcc_camera_platform_initialise(struct gbcc *gbc);
void gbcc_camera_platform_destroy(struct gbcc *gbc);
void gbcc_camera_platform_capture_image(struct gbcc *gbc, uint8_t image[GB_CAMERA_SENSOR_SIZE]);

#endif /* GBCC_CAMERA_H */
