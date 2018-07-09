#ifndef I32CTT_COMMON_H_
#define I32CTT_COMMON_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//Register address-data pair structure. Used to exchange WRITE_CMD and READ_ANS packets.
typedef struct __attribute__((packed)) {
  uint16_t address;
  uint32_t data;
} i32ctt_reg_pair_t;

//Register address structure. Used to exchange READ_CMD and WRITE_ANS packets.
typedef struct __attribute__((packed)) {
  uint16_t address;
} i32ctt_addr_t;

//I32CTT common functions.
void i32ctt_update();

#ifdef __cplusplus
}
#endif

#endif //I32CTT_COMMON_H_
