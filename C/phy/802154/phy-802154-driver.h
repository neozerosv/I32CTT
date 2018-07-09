#ifndef PHY_802154_DRIVER_H_
#define PHY_802154_DRIVER_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//PHY 802.15.4 driver functions.
void phy_802154_driver_set_channel(uint8_t channel);
void phy_802154_driver_set_pan_id(uint16_t pan_id);
void phy_802154_driver_set_short_addr(uint16_t short_addr);

#ifdef __cplusplus
}
#endif

#endif //PHY_802154_DRIVER_H_
