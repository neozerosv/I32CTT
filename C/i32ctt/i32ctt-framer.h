#ifndef I32CTT_FRAMER_H_
#define I32CTT_FRAMER_H_

#include <stdint.h>
#include <stdbool.h>
#include "i32ctt-protocol.h"

#ifdef __cplusplus
extern "C" {
#endif

//I32CTT framer functions.
uint16_t i32ctt_framer_get_max_payload_count(uint8_t command, uint16_t packet_mtu_size);
uint16_t i32ctt_framer_get_payload_offset(uint8_t command);
uint16_t i32ctt_framer_encapsulate_packet(uint8_t command, uint8_t endpoint, uint8_t *p_tx_buffer,
                                          uint16_t reg_count);
bool i32ctt_framer_decapsulate_packet(const uint8_t *p_rx_buffer, uint16_t packet_size,
                                      const i32ctt_frame_header_t **pp_header,
                                      const void **pp_payload, uint16_t *p_payload_count);

#ifdef __cplusplus
}
#endif

#endif //I32CTT_FRAMER_H_
