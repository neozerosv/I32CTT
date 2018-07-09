//Headers for provided APIs.
#include "i32ctt-framer.h"

//Headers for depended APIs.
#include "i32ctt-common.h"

//Implementation specific headers.
#include <string.h>
#include "i32ctt-protocol.h"

//I32CTT framer functions.
//--------------------------------------------------------------------------------------------------

uint16_t i32ctt_framer_get_max_payload_count(uint8_t command, uint16_t packet_mtu_size) {
  switch (command) {
    case I32CTT_READ_CMD:
    case I32CTT_READ_ANS:
    case I32CTT_WRITE_CMD:
    case I32CTT_WRITE_ANS:
      //For all these commands and answers, the maximum payload/register count is always the amount
      //of register pair structures that can fit in a packet after subtracting the header size.
      //Note: In the case of I32CTT_READ_CMD, even if only addresses are contained in the packet,
      //the related answer I32CTT_READ_ANS packet contains register pairs, which are bigger and
      //therefore constrain the limit. A reversed but similar situation happens for the
      //I32CTT_WRITE_CMD and I32CTT_WRITE_ANS packets.
      return (packet_mtu_size - sizeof(i32ctt_frame_header_t)) / sizeof(i32ctt_reg_pair_t);
    default:
      //Unknown/unimplemented command.
      return 0;
  }
}

uint16_t i32ctt_framer_get_payload_offset(uint8_t command) {
  switch (command) {
    case I32CTT_READ_CMD:
    case I32CTT_READ_ANS:
    case I32CTT_WRITE_CMD:
    case I32CTT_WRITE_ANS:
      //For all these commands and answers, the payload offset is the position after the header.
      return sizeof(i32ctt_frame_header_t);
    default:
      //Unknown/unimplemented command.
      return 0;
  }
}

uint16_t i32ctt_framer_encapsulate_packet(uint8_t command, uint8_t endpoint, uint8_t *p_tx_buffer,
                                          uint16_t reg_count)
{
  //Packet header.
  const i32ctt_frame_header_t header = {
    .command = command,
    .endpoint = endpoint,
  };

  //Store the packet header and return the packet size according to the command.
  switch (command) {
    case I32CTT_READ_CMD:
    case I32CTT_WRITE_ANS:
      //Copy the packet header.
      memcpy(p_tx_buffer, &header, sizeof(header));

      //Return the packet size, which is the size of the header plus the total combined size of all
      //register addresses.
      return sizeof(i32ctt_frame_header_t) + sizeof(i32ctt_addr_t) * reg_count;
    case I32CTT_READ_ANS:
    case I32CTT_WRITE_CMD:
      //Copy the packet header.
      memcpy(p_tx_buffer, &header, sizeof(header));

      //Return the packet size, which is the size of the header plus the total combined size of all
      //register pairs.
      return sizeof(i32ctt_frame_header_t) + sizeof(i32ctt_reg_pair_t) * reg_count;
    default:
      //Unknown/unimplemented command.
      return 0;
  }
}

bool i32ctt_framer_decapsulate_packet(const uint8_t *p_rx_buffer, uint16_t packet_size,
                                      const i32ctt_frame_header_t **pp_header,
                                      const void **pp_payload, uint16_t *p_payload_count)
{
  i32ctt_frame_header_t *p_header = (i32ctt_frame_header_t *) p_rx_buffer;
  uint16_t payload_count;

  //Reject packets that are too short or contain no payload.
  if (packet_size <= sizeof(i32ctt_frame_header_t))
    return false;

  //Proceed according to the command stored in the packet.
  switch (p_header->command) {
    case I32CTT_READ_CMD:
    case I32CTT_WRITE_ANS:
      //Packets with these commands have an array of addresses. Assert that their size is a multiple
      //of the address structure.
      if ((packet_size - sizeof(i32ctt_frame_header_t)) % sizeof(i32ctt_addr_t) != 0)
        return false;

      //Calculate how many addresses are contained in the packet.
      payload_count = (packet_size - sizeof(i32ctt_frame_header_t)) / sizeof(i32ctt_addr_t);
      break;
    case I32CTT_WRITE_CMD:
    case I32CTT_READ_ANS:
      //Packets with these commands have an array of register pairs. Assert that their size is a
      //multiple of the register pair structure.
      if ((packet_size - sizeof(i32ctt_frame_header_t)) % sizeof(i32ctt_reg_pair_t) != 0)
        return false;

      //Calculate how many register pairs are contained in the packet.
      payload_count = (packet_size - sizeof(i32ctt_frame_header_t)) / sizeof(i32ctt_reg_pair_t);
      break;
    default:
      //Unknown/unimplemented command. Reject the packet.
      return false;
  }

  //Return the decapsulated data.
  *pp_header = p_header;
  *pp_payload = &p_rx_buffer[sizeof(i32ctt_frame_header_t)];
  *p_payload_count = payload_count;

  //Packet decoded successfully.
  return true;
}
