#ifndef I32CTT_SLAVE_H_
#define I32CTT_SLAVE_H_

#include <stdint.h>
#include <stddef.h>
#include "i32ctt-common.h"

#ifdef __cplusplus
extern "C" {
#endif

//Function pointer for the callback that handles READ_CMD packets. Provided by the slave.
typedef void (*i32ctt_slave_read_reg_callback_t)(const i32ctt_addr_t *reg_addr_array,
                                                 uint16_t reg_count);

//Function pointer for the callback that handles WRITE_CMD packets. Provided by the slave.
typedef void (*i32ctt_slave_write_reg_callback_t)(const i32ctt_reg_pair_t *reg_pair_array,
                                                  uint16_t reg_count);

//I32CTT slave instance structure.
typedef struct i32ctt_slave_instance i32ctt_slave_instance_t;
struct i32ctt_slave_instance {
  i32ctt_slave_read_reg_callback_t read_reg_callback;
  i32ctt_slave_write_reg_callback_t write_reg_callback;
  i32ctt_slave_instance_t *p_list_next;
  uint8_t endpoint;
};

//Macro used to declare an I32CTT slave instance. Note that a callback can be NULL if not required.
#define I32CTT_SLAVE_INSTANCE(name, endpoint_num, read_callback, write_callback)\
i32ctt_slave_instance_t name {\
  .read_reg_callback = read_callback,\
  .write_reg_callback = write_callback,\
  .p_list_next = NULL,\
  .endpoint = endpoint_num,\
};

//I32CTT slave functions.
void i32ctt_slave_add_instance(i32ctt_slave_instance_t *p_slave);
i32ctt_reg_pair_t *i32ctt_slave_get_read_ans_buffer();
i32ctt_addr_t *i32ctt_slave_get_write_ans_buffer();
bool i32ctt_slave_send_read_reg_ans(uint16_t reg_count);
bool i32ctt_slave_send_write_reg_ans(uint16_t reg_count);

#ifdef __cplusplus
}
#endif

#endif //I32CTT_SLAVE_H_
