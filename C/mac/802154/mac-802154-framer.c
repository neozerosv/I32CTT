//Headers for provided APIs.
#include "mac-802154-framer.h"

//Implementation specific headers.
#include <string.h>
#include "mac-802154-protocol.h"

//Local declarations.
//--------------------------------------------------------------------------------------------------

//Framer configuration structure.
typedef struct {
  uint16_t pan_id;
  uint16_t local_addr;
} config_t;

//Framer configuration instance.
static config_t config;

//State variables structure.
typedef struct {
  uint8_t seq_num;
  uint16_t dst_addr;
} framer_state_t;

//State variables instance.
static framer_state_t framer_state;

//MAC 802.15.4 framer functions.
//--------------------------------------------------------------------------------------------------

void mac_802154_framer_init(uint16_t pan_id, uint16_t local_addr, uint8_t random_seq_num) {
  //Initialize framer configuration and state.
  config.pan_id = pan_id;
  config.local_addr = local_addr;
  framer_state.seq_num = random_seq_num;
}

void mac_802154_framer_set_destination(uint16_t dst_addr) {
  //Store the destination address for future packets.
  framer_state.dst_addr = dst_addr;
}

uint16_t mac_802154_framer_get_max_payload_size(uint16_t packet_mtu_size) {
  //The maximum payload size is the packet maximum transfer unit size minus the sizes of the header
  //and trailer.
  return packet_mtu_size - sizeof(mac_frame_header_t) - sizeof(mac_frame_trailer_t);
}

uint16_t mac_802154_framer_get_payload_offset() {
  //The MAC payload offset equals the size of the header.
  return sizeof(mac_frame_header_t);
}

uint16_t mac_802154_framer_encapsulate_packet(uint8_t *p_tx_buffer, uint16_t payload_size) {
  //Packet header.
  const mac_frame_header_t header = {
    .fcf = FRAME_TYPE_DATA | ACK_REQUEST | PAN_ID_COMPRESSION | DST_ADDR_MODE_SHORT_ADDR |
           FRAME_VERSION_2006_2011 | SRC_ADDR_MODE_SHORT_ADDR,  //Append frame control field
    .seq_num = framer_state.seq_num++,                          //Append seq number and increment it
    .pan_id = config.pan_id,                                    //Append local PAN ID
    .dst_addr = framer_state.dst_addr,                          //Append short destination address
    .src_addr = config.local_addr,                              //Append short source address
  };

  //Packet trailer.
  const mac_frame_trailer_t trailer = {
    .fcs = 0,   //Initialized to 0. Radio is expected to fill this during transmission.
  };

  //Copy the packet header and trailer.
  memcpy(p_tx_buffer, &header, sizeof(header));
  memcpy(&p_tx_buffer[sizeof(header) + payload_size], &trailer, sizeof(trailer));

  //Return the packet size, which is the total combined size of the header, payload and trailer.
  return sizeof(header) + payload_size + sizeof(trailer);
}

bool mac_802154_framer_decapsulate_packet(const uint8_t *p_rx_buffer, uint16_t packet_size,
                                          uint16_t *p_src_addr, const uint8_t **pp_payload,
                                          uint16_t *p_payload_size)
{
  const mac_frame_header_t *p_header = (mac_frame_header_t *) p_rx_buffer;
  const uint16_t fcf_filter = FRAME_TYPE_MASK | SECURITY_ENABLED | PAN_ID_COMPRESSION |
                              SRC_ADDR_MODE_MASK | DST_ADDR_MODE_MASK;
  const uint16_t fcf_pattern = FRAME_TYPE_DATA | PAN_ID_COMPRESSION | DST_ADDR_MODE_SHORT_ADDR |
                               SRC_ADDR_MODE_SHORT_ADDR;

  //Filter data packets and accept those which have the security bit off, pan compresion on and
  //short destination and source addresses.
  if ((p_header->fcf & fcf_filter) != fcf_pattern)
    return false;

  //Notes:
  //- Destination address and PAN ID are not filtered. It's assumed that the radio is filtering
  //  addresses automatically.
  //- FCS is not checked by software. Again, it's assumed that the radio verifies this.

  //Extract and return the source address.
  *p_src_addr = p_header->src_addr;

  //Return the payload pointer, which is right after the frame header.
  *pp_payload = &p_rx_buffer[sizeof(mac_frame_header_t)];

  //Return the payload size, which is the packet size minus the sizes of header and trailer.
  *p_payload_size = packet_size - sizeof(mac_frame_header_t) - sizeof(mac_frame_trailer_t);

  //Packet decoded successfully.
  return true;
}
