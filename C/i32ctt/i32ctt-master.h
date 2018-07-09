#ifndef I32CTT_MASTER_H_
#define I32CTT_MASTER_H_

#include <stdint.h>
#include <stdbool.h>
#include "i32ctt-common.h"

#ifdef __cplusplus
extern "C" {
#endif

//Function pointer for the callback that handles READ_ANS packets. Provided by the master.
typedef void (*i32ctt_master_read_reg_callback_t)(const i32ctt_reg_pair_t *reg_pair_array,
                                                  uint16_t reg_count);

//Function pointer for the callback that handles WRITE_ANS packets. Provided by the master.
typedef void (*i32ctt_master_write_reg_callback_t)(const i32ctt_addr_t *reg_addr_array,
                                                   uint16_t reg_count);

//I32CTT master functions.
void i32ctt_master_set_read_callback(i32ctt_master_read_reg_callback_t callback_function);
void i32ctt_master_set_write_callback(i32ctt_master_write_reg_callback_t callback_function);
i32ctt_addr_t *i32ctt_master_get_read_cmd_buffer(uint16_t * max_reg_count);
i32ctt_reg_pair_t *i32ctt_master_get_write_cmd_buffer(uint16_t * max_reg_count);
bool i32ctt_master_send_read_reg_cmd(uint8_t endpoint, uint16_t reg_count);
bool i32ctt_master_send_write_reg_cmd(uint8_t endpoint, uint16_t reg_count);

#ifdef __cplusplus
}
#endif

#endif //I32CTT_MASTER_H_
