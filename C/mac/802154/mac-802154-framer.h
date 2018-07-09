#ifndef MAC_802154_FRAMER_H_
#define MAC_802154_FRAMER_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

//MAC 802.15.4 framer functions.
void mac_802154_framer_init(uint16_t pan_id, uint16_t local_addr, uint8_t random_seq_num);
void mac_802154_framer_set_destination(uint16_t dst_addr);
uint16_t mac_802154_framer_get_max_payload_size(uint16_t packet_mtu_size);
uint16_t mac_802154_framer_get_payload_offset();
uint16_t mac_802154_framer_encapsulate_packet(uint8_t *p_tx_buffer, uint16_t payload_size);
bool mac_802154_framer_decapsulate_packet(const uint8_t *p_rx_buffer, uint16_t packet_size,
                                          uint16_t *p_src_addr, const uint8_t **pp_payload,
                                          uint16_t *p_payload_size);

#ifdef __cplusplus
}
#endif

#endif //MAC_802154_FRAMER_H_
