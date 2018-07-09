#ifndef MAC_802154_DRIVER_H_
#define MAC_802154_DRIVER_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//MAC 802154 driver functions.
void mac_802154_driver_init(uint16_t pan_id, uint16_t short_addr, uint8_t random_seq_num);
void mac_802154_driver_set_tx_destination_addr(uint16_t dst_addr);
uint16_t mac_802154_driver_get_rx_source_addr();

#ifdef __cplusplus
}
#endif

#endif //MAC_802154_DRIVER_H_
