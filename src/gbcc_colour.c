#include "gbcc_colour.h"
#include <stdbool.h>
#include <stdint.h>


static uint32_t lerp_colour(uint8_t r, uint8_t g, uint8_t b);
static bool lut_initialised = false;

/*
 * Dimensions are as follows:
 * 0 - which of R, G, B are we looking at?
 * 1 - R
 * 2 - G
 * 3 - B
 */
static const uint8_t lut[3][8][8][8] =
{
	{
		{
			{0x00, 0x01, 0x04, 0x09, 0x0F, 0x15, 0x1C, 0x22},
			{0x0D, 0x0E, 0x10, 0x14, 0x18, 0x1C, 0x22, 0x27},
			{0x1F, 0x1F, 0x20, 0x22, 0x25, 0x28, 0x2C, 0x30},
			{0x32, 0x32, 0x33, 0x34, 0x35, 0x38, 0x3B, 0x3E},
			{0x44, 0x44, 0x45, 0x45, 0x47, 0x48, 0x4A, 0x4D},
			{0x57, 0x57, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5E},
			{0x69, 0x69, 0x69, 0x6A, 0x6A, 0x6B, 0x6D, 0x6E},
			{0x7B, 0x7B, 0x7B, 0x7C, 0x7D, 0x7D, 0x7E, 0x80}
		},
		{
			{0x16, 0x16, 0x18, 0x1A, 0x1E, 0x21, 0x26, 0x2B},
			{0x1D, 0x1D, 0x1E, 0x20, 0x23, 0x26, 0x2B, 0x2F},
			{0x29, 0x29, 0x2A, 0x2B, 0x2D, 0x30, 0x33, 0x37},
			{0x38, 0x38, 0x39, 0x3A, 0x3B, 0x3D, 0x40, 0x43},
			{0x48, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4E, 0x51},
			{0x5A, 0x5A, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x60},
			{0x6B, 0x6B, 0x6C, 0x6C, 0x6D, 0x6E, 0x6F, 0x71},
			{0x7E, 0x7E, 0x7E, 0x7E, 0x7F, 0x80, 0x81, 0x82}
		},
		{
			{0x2C, 0x2D, 0x2D, 0x2F, 0x31, 0x33, 0x36, 0x3A},
			{0x30, 0x30, 0x31, 0x32, 0x34, 0x36, 0x39, 0x3C},
			{0x38, 0x38, 0x39, 0x39, 0x3B, 0x3D, 0x40, 0x43},
			{0x44, 0x44, 0x44, 0x45, 0x46, 0x48, 0x4A, 0x4C},
			{0x51, 0x52, 0x52, 0x53, 0x54, 0x55, 0x57, 0x59},
			{0x61, 0x61, 0x61, 0x62, 0x63, 0x64, 0x65, 0x67},
			{0x71, 0x71, 0x72, 0x72, 0x73, 0x74, 0x75, 0x76},
			{0x82, 0x82, 0x83, 0x83, 0x84, 0x85, 0x85, 0x86}
		},
		{
			{0x45, 0x45, 0x45, 0x46, 0x47, 0x49, 0x4B, 0x4D},
			{0x47, 0x47, 0x48, 0x48, 0x4A, 0x4B, 0x4D, 0x4F},
			{0x4C, 0x4C, 0x4D, 0x4D, 0x4E, 0x50, 0x52, 0x54},
			{0x55, 0x55, 0x55, 0x56, 0x57, 0x58, 0x5A, 0x5C},
			{0x60, 0x60, 0x60, 0x61, 0x62, 0x63, 0x64, 0x66},
			{0x6D, 0x6D, 0x6D, 0x6E, 0x6F, 0x6F, 0x71, 0x72},
			{0x7B, 0x7B, 0x7C, 0x7C, 0x7C, 0x7D, 0x7F, 0x80},
			{0x8B, 0x8B, 0x8B, 0x8C, 0x8C, 0x8D, 0x8E, 0x8F}
		},
		{
			{0x5D, 0x5D, 0x5D, 0x5E, 0x5E, 0x60, 0x61, 0x63},
			{0x5E, 0x5E, 0x5E, 0x5F, 0x60, 0x61, 0x63, 0x64},
			{0x62, 0x62, 0x62, 0x63, 0x64, 0x65, 0x66, 0x68},
			{0x68, 0x68, 0x69, 0x69, 0x6A, 0x6B, 0x6C, 0x6E},
			{0x71, 0x71, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76},
			{0x7C, 0x7C, 0x7C, 0x7D, 0x7E, 0x7E, 0x7F, 0x81},
			{0x88, 0x88, 0x89, 0x89, 0x89, 0x8A, 0x8B, 0x8C},
			{0x96, 0x96, 0x96, 0x97, 0x97, 0x98, 0x99, 0x9A}
		},
		{
			{0x75, 0x75, 0x75, 0x75, 0x76, 0x77, 0x78, 0x7A},
			{0x76, 0x76, 0x76, 0x77, 0x77, 0x78, 0x79, 0x7B},
			{0x79, 0x79, 0x79, 0x79, 0x7A, 0x7B, 0x7C, 0x7E},
			{0x7E, 0x7E, 0x7E, 0x7F, 0x7F, 0x80, 0x81, 0x82},
			{0x85, 0x85, 0x85, 0x86, 0x86, 0x87, 0x88, 0x89},
			{0x8E, 0x8E, 0x8E, 0x8F, 0x8F, 0x90, 0x91, 0x92},
			{0x98, 0x98, 0x99, 0x99, 0x99, 0x9A, 0x9B, 0x9C},
			{0xA5, 0xA5, 0xA5, 0xA6, 0xA6, 0xA7, 0xA7, 0xA8}
		},
		{
			{0x8C, 0x8C, 0x8D, 0x8D, 0x8E, 0x8E, 0x8F, 0x90},
			{0x8D, 0x8D, 0x8D, 0x8E, 0x8E, 0x8F, 0x90, 0x91},
			{0x90, 0x90, 0x90, 0x90, 0x91, 0x91, 0x92, 0x93},
			{0x94, 0x94, 0x94, 0x94, 0x95, 0x95, 0x96, 0x97},
			{0x9A, 0x9A, 0x9A, 0x9A, 0x9B, 0x9B, 0x9C, 0x9D},
			{0xA1, 0xA1, 0xA1, 0xA2, 0xA2, 0xA3, 0xA3, 0xA4},
			{0xAA, 0xAA, 0xAB, 0xAB, 0xAB, 0xAC, 0xAD, 0xAD},
			{0xB5, 0xB5, 0xB5, 0xB5, 0xB6, 0xB6, 0xB7, 0xB8}
		},
		{
			{0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA6, 0xA7, 0xA8},
			{0xA5, 0xA5, 0xA5, 0xA6, 0xA6, 0xA7, 0xA7, 0xA8},
			{0xA7, 0xA7, 0xA7, 0xA8, 0xA8, 0xA9, 0xA9, 0xAA},
			{0xAA, 0xAA, 0xAB, 0xAB, 0xAB, 0xAC, 0xAC, 0xAD},
			{0xAF, 0xAF, 0xAF, 0xB0, 0xB0, 0xB1, 0xB1, 0xB2},
			{0xB6, 0xB6, 0xB6, 0xB6, 0xB7, 0xB7, 0xB8, 0xB9},
			{0xBE, 0xBE, 0xBE, 0xBE, 0xBE, 0xBF, 0xC0, 0xC0},
			{0xC7, 0xC7, 0xC7, 0xC8, 0xC8, 0xC8, 0xC9, 0xCA}
		}
	},
	{
		{
			{0x00, 0x0F, 0x22, 0x36, 0x49, 0x5D, 0x70, 0x84},
			{0x10, 0x1A, 0x28, 0x39, 0x4C, 0x5F, 0x72, 0x85},
			{0x23, 0x29, 0x33, 0x41, 0x52, 0x64, 0x75, 0x89},
			{0x38, 0x3B, 0x42, 0x4E, 0x5C, 0x6C, 0x7C, 0x8E},
			{0x4C, 0x4F, 0x54, 0x5D, 0x68, 0x76, 0x85, 0x96},
			{0x61, 0x63, 0x67, 0x6E, 0x77, 0x83, 0x91, 0xA0},
			{0x75, 0x76, 0x79, 0x7F, 0x87, 0x92, 0x9D, 0xAB},
			{0x89, 0x8B, 0x8D, 0x92, 0x99, 0xA2, 0xAC, 0xB8}
		},
		{
			{0x04, 0x12, 0x24, 0x37, 0x4A, 0x5E, 0x71, 0x84},
			{0x13, 0x1C, 0x29, 0x3B, 0x4D, 0x5F, 0x72, 0x86},
			{0x25, 0x2A, 0x34, 0x42, 0x52, 0x64, 0x76, 0x89},
			{0x39, 0x3C, 0x43, 0x4E, 0x5C, 0x6C, 0x7D, 0x8E},
			{0x4D, 0x4F, 0x55, 0x5D, 0x69, 0x77, 0x86, 0x96},
			{0x61, 0x63, 0x67, 0x6E, 0x78, 0x84, 0x91, 0xA0},
			{0x75, 0x77, 0x7A, 0x80, 0x88, 0x92, 0x9E, 0xAB},
			{0x8A, 0x8B, 0x8D, 0x92, 0x99, 0xA2, 0xAD, 0xB8}
		},
		{
			{0x0E, 0x18, 0x27, 0x39, 0x4B, 0x5F, 0x71, 0x85},
			{0x19, 0x20, 0x2C, 0x3C, 0x4E, 0x61, 0x73, 0x86},
			{0x28, 0x2D, 0x36, 0x44, 0x54, 0x65, 0x77, 0x89},
			{0x3B, 0x3E, 0x45, 0x50, 0x5E, 0x6D, 0x7D, 0x8F},
			{0x4E, 0x51, 0x56, 0x5E, 0x6A, 0x77, 0x86, 0x96},
			{0x62, 0x64, 0x68, 0x6F, 0x79, 0x84, 0x92, 0xA0},
			{0x76, 0x77, 0x7B, 0x80, 0x88, 0x93, 0x9E, 0xAC},
			{0x8A, 0x8B, 0x8E, 0x93, 0x9A, 0xA2, 0xAD, 0xB9}
		},
		{
			{0x19, 0x20, 0x2C, 0x3D, 0x4E, 0x61, 0x73, 0x86},
			{0x21, 0x27, 0x31, 0x40, 0x51, 0x63, 0x75, 0x88},
			{0x2E, 0x32, 0x3A, 0x47, 0x56, 0x67, 0x78, 0x8B},
			{0x3F, 0x42, 0x48, 0x53, 0x60, 0x6F, 0x7F, 0x90},
			{0x51, 0x53, 0x58, 0x61, 0x6C, 0x79, 0x88, 0x98},
			{0x64, 0x66, 0x6A, 0x71, 0x7A, 0x86, 0x93, 0xA1},
			{0x78, 0x79, 0x7C, 0x82, 0x8A, 0x94, 0x9F, 0xAD},
			{0x8C, 0x8D, 0x8F, 0x94, 0x9B, 0xA3, 0xAE, 0xBA}
		},
		{
			{0x24, 0x2A, 0x33, 0x42, 0x52, 0x64, 0x76, 0x89},
			{0x2A, 0x2F, 0x38, 0x45, 0x54, 0x66, 0x77, 0x8A},
			{0x35, 0x38, 0x40, 0x4C, 0x5A, 0x6A, 0x7B, 0x8D},
			{0x44, 0x47, 0x4D, 0x56, 0x63, 0x71, 0x81, 0x92},
			{0x55, 0x57, 0x5C, 0x64, 0x6E, 0x7B, 0x8A, 0x9A},
			{0x67, 0x69, 0x6D, 0x74, 0x7D, 0x88, 0x95, 0xA3},
			{0x7A, 0x7B, 0x7F, 0x84, 0x8C, 0x96, 0xA1, 0xAE},
			{0x8E, 0x8F, 0x91, 0x96, 0x9D, 0xA5, 0xB0, 0xBB}
		},
		{
			{0x30, 0x34, 0x3C, 0x48, 0x57, 0x68, 0x79, 0x8B},
			{0x34, 0x38, 0x3F, 0x4B, 0x59, 0x6A, 0x7A, 0x8D},
			{0x3D, 0x40, 0x46, 0x51, 0x5E, 0x6E, 0x7E, 0x90},
			{0x4A, 0x4D, 0x52, 0x5B, 0x67, 0x75, 0x84, 0x95},
			{0x5A, 0x5C, 0x60, 0x68, 0x72, 0x7F, 0x8C, 0x9C},
			{0x6B, 0x6D, 0x71, 0x77, 0x80, 0x8B, 0x97, 0xA5},
			{0x7D, 0x7F, 0x82, 0x87, 0x8E, 0x98, 0xA3, 0xB0},
			{0x90, 0x92, 0x94, 0x99, 0x9F, 0xA8, 0xB2, 0xBD}
		},
		{
			{0x3B, 0x3E, 0x45, 0x50, 0x5D, 0x6D, 0x7D, 0x8F},
			{0x3E, 0x41, 0x48, 0x52, 0x60, 0x6F, 0x7F, 0x90},
			{0x46, 0x48, 0x4E, 0x58, 0x64, 0x72, 0x82, 0x93},
			{0x51, 0x54, 0x58, 0x61, 0x6C, 0x79, 0x88, 0x98},
			{0x5F, 0x61, 0x66, 0x6D, 0x76, 0x82, 0x90, 0x9F},
			{0x70, 0x71, 0x75, 0x7B, 0x84, 0x8E, 0x9A, 0xA8},
			{0x81, 0x82, 0x85, 0x8B, 0x92, 0x9B, 0xA6, 0xB3},
			{0x94, 0x95, 0x97, 0x9C, 0xA2, 0xAA, 0xB4, 0xBF}
		},
		{
			{0x46, 0x49, 0x4E, 0x58, 0x64, 0x73, 0x82, 0x93},
			{0x49, 0x4C, 0x51, 0x5A, 0x66, 0x74, 0x84, 0x94},
			{0x4F, 0x52, 0x57, 0x5F, 0x6B, 0x78, 0x87, 0x97},
			{0x59, 0x5C, 0x60, 0x68, 0x72, 0x7E, 0x8C, 0x9C},
			{0x66, 0x68, 0x6C, 0x73, 0x7C, 0x87, 0x94, 0xA2},
			{0x76, 0x77, 0x7B, 0x80, 0x88, 0x92, 0x9E, 0xAC},
			{0x86, 0x87, 0x8A, 0x8F, 0x96, 0x9F, 0xAA, 0xB6},
			{0x98, 0x99, 0x9B, 0xA0, 0xA6, 0xAE, 0xB7, 0xC2}
		}
	},
	{
		{
			{0x00, 0x16, 0x2C, 0x45, 0x5D, 0x75, 0x8C, 0xA5},
			{0x02, 0x17, 0x2D, 0x45, 0x5D, 0x75, 0x8C, 0xA5},
			{0x07, 0x19, 0x2E, 0x46, 0x5D, 0x75, 0x8D, 0xA5},
			{0x0F, 0x1E, 0x31, 0x47, 0x5E, 0x76, 0x8E, 0xA5},
			{0x17, 0x23, 0x34, 0x4A, 0x60, 0x77, 0x8E, 0xA6},
			{0x20, 0x29, 0x38, 0x4C, 0x62, 0x79, 0x90, 0xA7},
			{0x28, 0x30, 0x3D, 0x50, 0x65, 0x7B, 0x91, 0xA9},
			{0x30, 0x37, 0x42, 0x54, 0x68, 0x7D, 0x93, 0xAA}
		},
		{
			{0x02, 0x17, 0x2D, 0x45, 0x5D, 0x75, 0x8C, 0xA5},
			{0x04, 0x17, 0x2D, 0x45, 0x5D, 0x75, 0x8D, 0xA5},
			{0x08, 0x1A, 0x2F, 0x46, 0x5E, 0x75, 0x8D, 0xA5},
			{0x10, 0x1E, 0x31, 0x47, 0x5E, 0x76, 0x8E, 0xA5},
			{0x18, 0x23, 0x34, 0x4A, 0x60, 0x77, 0x8F, 0xA6},
			{0x20, 0x29, 0x38, 0x4D, 0x62, 0x79, 0x90, 0xA7},
			{0x28, 0x30, 0x3D, 0x50, 0x65, 0x7B, 0x91, 0xA9},
			{0x31, 0x37, 0x43, 0x54, 0x68, 0x7D, 0x94, 0xAA}
		},
		{
			{0x05, 0x18, 0x2E, 0x45, 0x5D, 0x75, 0x8D, 0xA5},
			{0x07, 0x19, 0x2E, 0x46, 0x5D, 0x75, 0x8D, 0xA5},
			{0x0C, 0x1C, 0x30, 0x47, 0x5E, 0x76, 0x8D, 0xA5},
			{0x13, 0x20, 0x32, 0x48, 0x5F, 0x77, 0x8E, 0xA6},
			{0x1A, 0x24, 0x35, 0x4A, 0x60, 0x78, 0x8F, 0xA6},
			{0x22, 0x2A, 0x39, 0x4D, 0x63, 0x79, 0x90, 0xA7},
			{0x2A, 0x31, 0x3E, 0x51, 0x65, 0x7C, 0x92, 0xA9},
			{0x32, 0x38, 0x43, 0x55, 0x68, 0x7E, 0x94, 0xAA}
		},
		{
			{0x0C, 0x1C, 0x30, 0x47, 0x5E, 0x76, 0x8D, 0xA5},
			{0x0D, 0x1D, 0x30, 0x47, 0x5E, 0x76, 0x8D, 0xA5},
			{0x11, 0x1F, 0x31, 0x48, 0x5F, 0x76, 0x8D, 0xA5},
			{0x17, 0x22, 0x34, 0x49, 0x60, 0x77, 0x8E, 0xA6},
			{0x1D, 0x27, 0x37, 0x4C, 0x61, 0x78, 0x8F, 0xA7},
			{0x24, 0x2C, 0x3B, 0x4E, 0x63, 0x7A, 0x90, 0xA8},
			{0x2B, 0x32, 0x3F, 0x52, 0x66, 0x7C, 0x92, 0xA9},
			{0x33, 0x39, 0x45, 0x56, 0x69, 0x7E, 0x94, 0xAB}
		},
		{
			{0x13, 0x20, 0x32, 0x48, 0x5F, 0x77, 0x8E, 0xA6},
			{0x14, 0x20, 0x33, 0x49, 0x5F, 0x77, 0x8E, 0xA6},
			{0x17, 0x22, 0x34, 0x49, 0x60, 0x77, 0x8E, 0xA6},
			{0x1B, 0x26, 0x36, 0x4B, 0x61, 0x78, 0x8F, 0xA7},
			{0x21, 0x2A, 0x39, 0x4D, 0x62, 0x79, 0x90, 0xA7},
			{0x27, 0x2F, 0x3D, 0x50, 0x65, 0x7B, 0x91, 0xA8},
			{0x2E, 0x35, 0x41, 0x53, 0x67, 0x7D, 0x93, 0xAA},
			{0x36, 0x3B, 0x46, 0x57, 0x6A, 0x7F, 0x95, 0xAB}
		},
		{
			{0x1A, 0x25, 0x35, 0x4A, 0x61, 0x78, 0x8F, 0xA6},
			{0x1B, 0x25, 0x36, 0x4B, 0x61, 0x78, 0x8F, 0xA7},
			{0x1D, 0x27, 0x37, 0x4B, 0x62, 0x78, 0x8F, 0xA7},
			{0x21, 0x2A, 0x39, 0x4D, 0x62, 0x79, 0x90, 0xA7},
			{0x26, 0x2E, 0x3B, 0x4F, 0x64, 0x7A, 0x91, 0xA8},
			{0x2C, 0x32, 0x3F, 0x52, 0x66, 0x7C, 0x92, 0xA9},
			{0x32, 0x38, 0x44, 0x55, 0x69, 0x7E, 0x93, 0xAB},
			{0x39, 0x3E, 0x48, 0x59, 0x6C, 0x80, 0x96, 0xAC}
		},
		{
			{0x21, 0x2B, 0x39, 0x4D, 0x63, 0x79, 0x90, 0xA8},
			{0x22, 0x2B, 0x3A, 0x4E, 0x63, 0x79, 0x90, 0xA8},
			{0x24, 0x2C, 0x3A, 0x4E, 0x63, 0x7A, 0x90, 0xA8},
			{0x27, 0x2F, 0x3C, 0x4F, 0x64, 0x7B, 0x91, 0xA8},
			{0x2B, 0x32, 0x3F, 0x52, 0x66, 0x7C, 0x92, 0xA9},
			{0x30, 0x37, 0x42, 0x54, 0x68, 0x7D, 0x93, 0xAA},
			{0x36, 0x3C, 0x47, 0x57, 0x6A, 0x7F, 0x95, 0xAC},
			{0x3C, 0x42, 0x4B, 0x5B, 0x6D, 0x82, 0x97, 0xAD}
		},
		{
			{0x29, 0x31, 0x3E, 0x50, 0x65, 0x7B, 0x91, 0xA9},
			{0x2A, 0x31, 0x3E, 0x51, 0x65, 0x7B, 0x92, 0xA9},
			{0x2B, 0x32, 0x3F, 0x51, 0x66, 0x7C, 0x92, 0xA9},
			{0x2E, 0x35, 0x41, 0x53, 0x67, 0x7D, 0x93, 0xAA},
			{0x31, 0x37, 0x43, 0x55, 0x68, 0x7E, 0x94, 0xAA},
			{0x36, 0x3B, 0x46, 0x57, 0x6A, 0x7F, 0x95, 0xAB},
			{0x3B, 0x40, 0x4A, 0x5A, 0x6D, 0x81, 0x96, 0xAD},
			{0x40, 0x45, 0x4F, 0x5E, 0x6F, 0x84, 0x98, 0xAE}
		}
	}
};

static float lerp1d(float a, float b, float t)
{
	return a * (1 - t) + b * t;
}

uint32_t gbcc_lerp_colour(uint8_t r, uint8_t g, uint8_t b)
{
	static uint32_t lut_calc[32][32][32];
	if (!lut_initialised) {
		for (uint8_t x = 0; x < 32; x++) {
			for (uint8_t y = 0; y < 32; y++) {
				for (uint8_t z = 0; z < 32; z++) {
					lut_calc[x][y][z] = lerp_colour(x, y, z);
				}
			}
		}
		lut_initialised = true;
	}
	return lut_calc[r][g][b];
}

/* Trilinearly interpolate from gameboy r,g,b values to hex code */
uint32_t lerp_colour(uint8_t r, uint8_t g, uint8_t b)
{
	uint8_t x0 = (r / 5);
	uint8_t x1 = x0 + 1;
	uint8_t y0 = (g / 5);
	uint8_t y1 = y0 + 1;
	uint8_t z0 = (b / 5);
	uint8_t z1 = z0 + 1;

	float xd = (float)r / 5 - x0;
	float yd = (float)g / 5 - y0;
	float zd = (float)b / 5 - z0;

	float c00r = lerp1d(lut[0][x0][y0][z0], lut[0][x1][y0][z0], xd);
	float c01r = lerp1d(lut[0][x0][y0][z1], lut[0][x1][y0][z1], xd);
	float c10r = lerp1d(lut[0][x0][y1][z0], lut[0][x1][y1][z1], xd);
	float c11r = lerp1d(lut[0][x0][y1][z1], lut[0][x1][y1][z1], xd);

	float c00g = lerp1d(lut[1][x0][y0][z0], lut[1][x1][y0][z0], xd);
	float c01g = lerp1d(lut[1][x0][y0][z1], lut[1][x1][y0][z1], xd);
	float c10g = lerp1d(lut[1][x0][y1][z0], lut[1][x1][y1][z1], xd);
	float c11g = lerp1d(lut[1][x0][y1][z1], lut[1][x1][y1][z1], xd);

	float c00b = lerp1d(lut[2][x0][y0][z0], lut[2][x1][y0][z0], xd);
	float c01b = lerp1d(lut[2][x0][y0][z1], lut[2][x1][y0][z1], xd);
	float c10b = lerp1d(lut[2][x0][y1][z0], lut[2][x1][y1][z1], xd);
	float c11b = lerp1d(lut[2][x0][y1][z1], lut[2][x1][y1][z1], xd);

	float c0r = lerp1d(c00r, c10r, yd);
	float c1r = lerp1d(c01r, c11r, yd);

	float c0g = lerp1d(c00g, c10g, yd);
	float c1g = lerp1d(c01g, c11g, yd);

	float c0b = lerp1d(c00b, c10b, yd);
	float c1b = lerp1d(c01b, c11b, yd);

	/* Factor on the end is to correct for restricted {x,y,z}0 value range */
	float cr = lerp1d(c0r, c1r, zd) * (float)(7.0 / (31.0 / 5.0));
	float cg = lerp1d(c0g, c1g, zd) * (float)(7.0 / (31.0 / 5.0));
	float cb = lerp1d(c0b, c1b, zd) * (float)(7.0 / (31.0 / 5.0));

	r = (uint8_t)cr;
	g = (uint8_t)cg;
	b = (uint8_t)cb;

	return (uint32_t)((uint32_t)(r << 16u) | (uint32_t)(g << 8u)) | (uint32_t)b;
}

uint32_t gbcc_colour_correct(uint32_t hex)
{
    uint8_t r = (uint8_t)((hex & 0xFF0000u) >> 19u);
    uint8_t g = (uint8_t)((hex & 0x00FF00u) >> 11u);
    uint8_t b = (uint8_t)((hex & 0x0000FFu) >> 3u);

    return gbcc_lerp_colour(r,g,b);
}

