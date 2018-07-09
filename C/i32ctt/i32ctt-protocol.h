#ifndef I32CTT_PROTOCOL_H_
#define I32CTT_PROTOCOL_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//I32CTT Commands.
//----------------

#define I32CTT_READ_CMD   0x01
#define I32CTT_READ_ANS   0x02
#define I32CTT_WRITE_CMD  0x03
#define I32CTT_WRITE_ANS  0x04
#define I32CTT_LIST_CMD   0x05
#define I32CTT_LIST_ANS   0x06
#define I32CTT_FIND_CMD   0x07
#define I32CTT_FIND_ANS   0x08

//Type definitions.
//-----------------

//I32CTT frame header.
typedef struct __attribute__((__packed__)) {
  uint8_t command;
  uint8_t endpoint;
} i32ctt_frame_header_t;

#ifdef __cplusplus
}
#endif

#endif //I32CTT_PROTOCOL_H_
