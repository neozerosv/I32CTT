#ifndef PACKET_DRIVER_H_
#define PACKET_DRIVER_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

//Packet driver structure.
typedef struct {
  uint8_t *(*get_tx_packet)(uint16_t *p_mtu_size);
  bool (*packet_send)(uint16_t payload_size);
  bool (*is_packet_available)();
  bool (*get_rx_packet)(const uint8_t **pp_rx_buffer, uint16_t *p_payload_size);
  void (*dismiss_rx_packet)();
} packet_driver_t;

#ifdef __cplusplus
}
#endif

#endif //PACKET_DRIVER_H_
