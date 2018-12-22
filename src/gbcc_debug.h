#ifndef GBCC_DEBUG_H
#define GBCC_DEBUG_H

#include "gbcc.h"

enum GBCC_LOG_LEVEL {
  GBCC_LOG_INFO,
  GBCC_LOG_DEBUG,
  GBCC_LOG_ERROR
};

void gbcc_print_registers(struct gbc *gbc);
void gbcc_print_op(struct gbc *gbc);
void gbcc_log_error(const char *fmt, ...);
void gbcc_log_debug(const char *fmt, ...);
void gbcc_log_info(const char *fmt, ...);
void gbcc_log_append_error(const char *fmt, ...);
void gbcc_log_append_debug(const char *fmt, ...);
void gbcc_log_append_info(const char *fmt, ...);
//void gbcc_log(enum GBCC_LOG_LEVEL level, const char *fmt, ...);
//void gbcc_log_append(enum GBCC_LOG_LEVEL level, const char *fmt, ...);
void gbcc_vram_dump(struct gbc *gbc, const char *filename);
void gbcc_sram_dump(struct gbc *gbc, const char* filename);
void gbcc_print_state(struct gbc *gbc);

#endif /* GBCC_DEBUG_H */
