#ifndef MAC_802154_PROTOCOL_H_
#define MAC_802154_PROTOCOL_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//Frame control field (FCF) bitfields.
//------------------------------------

//Frame type.
#define FRAME_TYPE_MASK         (0b111 << 0)
#define FRAME_TYPE_BEACON       (0b000 << 0)
#define FRAME_TYPE_DATA         (0b001 << 0)
#define FRAME_TYPE_ACKNOWLEDGE  (0b010 << 0)
#define FRAME_TYPE_MAC_COMMAND  (0b011 << 0)

//Security enabled.
#define SECURITY_ENABLED  (0b1 << 3)

//Frame pending.
#define FRAME_PENDING   (0b1 << 4)

//Acknowledgement request.
#define ACK_REQUEST   (0b1 << 5)

//PAN ID compression.
#define PAN_ID_COMPRESSION  (0b1 << 6)

//Destination addressing mode.
#define DST_ADDR_MODE_MASK        (0b11 << 10)
#define DST_ADDR_MODE_NONE        (0b00 << 10)
#define DST_ADDR_MODE_SHORT_ADDR  (0b10 << 10)
#define DST_ADDR_MODE_LONG_ADDR   (0b11 << 10)

//Frame version.
#define FRAME_VERSION_MASK        (0b11 << 12)
#define FRAME_VERSION_2003        (0b00 << 12)
#define FRAME_VERSION_2006_2011   (0b01 << 12)

//Source addressing mode.
#define SRC_ADDR_MODE_MASK        (0b11 << 14)
#define SRC_ADDR_MODE_NONE        (0b00 << 14)
#define SRC_ADDR_MODE_SHORT_ADDR  (0b10 << 14)
#define SRC_ADDR_MODE_LONG_ADDR   (0b11 << 14)

//Type definitions.
//-----------------

//MAC frame header. PAN ID compression and short addresses are always used in this implementation in
//order to save memory. Frame structure is fixed so design is simplified.
typedef struct __attribute__((__packed__)) {
  uint16_t fcf;
  uint8_t seq_num;
  uint16_t pan_id;
  uint16_t dst_addr;
  uint16_t src_addr;
} mac_frame_header_t;

//MAC frame trailer.
typedef struct __attribute__((__packed__)) {
  uint16_t fcs;
} mac_frame_trailer_t;

#ifdef __cplusplus
}
#endif

#endif //MAC_802154_PROTOCOL_H_
