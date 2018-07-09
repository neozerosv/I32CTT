//Headers for provided APIs.
#include "mac-802154-driver.h"
#include "mac-packet-driver.h"

//Headers for depended APIs.
#include "phy-802154-driver.h"
#include "phy-packet-driver.h"

//Implementation specific headers.
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "mac-802154-framer.h"

//Local declarations.
//--------------------------------------------------------------------------------------------------

//State variables structure.
typedef struct {
  uint16_t rx_src_addr;
} driver_state_t;

//State variables instance.
static driver_state_t driver_state;

//MAC packet driver API declarations.
//--------------------------------------------------------------------------------------------------

//MAC packet driver API functions.
static uint8_t *get_tx_packet(uint16_t *p_mtu_size);
static bool packet_send(uint16_t payload_size);
static bool is_packet_available();
static bool get_rx_packet(const uint8_t **pp_rx_buffer, uint16_t *p_payload_size);
static void dismiss_rx_packet();

//MAC packet driver instance.
const packet_driver_t mac_packet_driver = {
  .get_tx_packet       = get_tx_packet,
  .packet_send         = packet_send,
  .is_packet_available = is_packet_available,
  .get_rx_packet       = get_rx_packet,
  .dismiss_rx_packet   = dismiss_rx_packet,
};

//MAC 802154 driver functions.
//--------------------------------------------------------------------------------------------------

void mac_802154_driver_init(uint16_t pan_id, uint16_t local_addr, uint8_t random_seq_num) {
  //Initialize the framer.
  mac_802154_framer_init(pan_id, local_addr, random_seq_num);

  //Configure the phy driver.
  phy_802154_driver_set_pan_id(pan_id);
  phy_802154_driver_set_short_addr(local_addr);
}

void mac_802154_driver_set_tx_destination_addr(uint16_t dst_addr) {
  //Set the destination address to the framer.
  mac_802154_framer_set_destination(dst_addr);
}

uint16_t mac_802154_driver_get_rx_source_addr() {
  //Return the source address of last received packet.
  return driver_state.rx_src_addr;
}

//MAC packet driver API functions.
//--------------------------------------------------------------------------------------------------

static uint8_t *get_tx_packet(uint16_t *p_mtu_size) {
  uint8_t *p_phy_tx_buffer;

  //Check whether the maximum transfer unit size is requested (passed pointer is not NULL).
  if (p_mtu_size != NULL) {
    uint16_t phy_mtu_size;

    //If requested, get the maximum transfer unit for PHY along with the pointer to the transmit
    //buffer, then call the framer to determine the maximum transfer unit size for MAC.
    p_phy_tx_buffer = phy_packet_driver.get_tx_packet(&phy_mtu_size);
    *p_mtu_size = mac_802154_framer_get_max_payload_size(phy_mtu_size);
  }
  else
    //If not requested, just get the pointer to the transmit buffer.
    p_phy_tx_buffer = phy_packet_driver.get_tx_packet(NULL);

  //Return the pointer to the transmit buffer for the payload. This is the pointer to the transmit
  //buffer from the layer below (PHY) moved forward to the frame payload offset.
  return &p_phy_tx_buffer[mac_802154_framer_get_payload_offset()];
}

static bool packet_send(uint16_t payload_size) {
  uint8_t *p_phy_tx_buffer;
  uint16_t phy_mtu_size;
  uint16_t packet_size;

  //Get the pointer to the transmit buffer and the MTU size from the PHY packet driver.
  p_phy_tx_buffer = phy_packet_driver.get_tx_packet(&phy_mtu_size);

  //Make sure that the payload isn't too large.
  if (payload_size > mac_802154_framer_get_max_payload_size(phy_mtu_size))
    return false;

  //Encapsulate the packet using the MAC framer.
  packet_size = mac_802154_framer_encapsulate_packet(p_phy_tx_buffer, payload_size);

  //Send the packet through the PHY packet driver.
  return phy_packet_driver.packet_send(packet_size);
}

static bool is_packet_available() {
  //Simply translate the call to the PHY packet driver.
  return phy_packet_driver.is_packet_available();
}

static bool get_rx_packet(const uint8_t **pp_rx_buffer, uint16_t *p_payload_size) {
  const uint8_t *p_phy_rx_buffer;
  uint16_t phy_packet_size;

  //Get the packet from the PHY packet driver.
  if (!phy_packet_driver.get_rx_packet(&p_phy_rx_buffer, &phy_packet_size))
    return false;

  //Decapsulate the packet. Store the source address and return the payload pointer and the payload
  //size in the same call.
  if (!mac_802154_framer_decapsulate_packet(p_phy_rx_buffer, phy_packet_size,
                                            &driver_state.rx_src_addr, pp_rx_buffer,
                                            p_payload_size))
  {
    return false;
  }

  //Packet processed succesfully.
  return true;
}

static void dismiss_rx_packet() {
  //Simply translate the call to the PHY packet driver.
  phy_packet_driver.dismiss_rx_packet();
}
