//Headers for provided APIs.
#include "i32ctt-common.h"
#include "i32ctt-master.h"
#include "i32ctt-slave.h"

//Headers for depended APIs.
#include "mac-packet-driver.h"

//Implementation specific headers.
#include <stddef.h>
#include "i32ctt-framer.h"
#include "i32ctt-protocol.h"

//Local declarations.
//--------------------------------------------------------------------------------------------------

//Master configuration structure.
typedef struct {
  i32ctt_master_read_reg_callback_t read_reg_callback;
  i32ctt_master_write_reg_callback_t write_reg_callback;
} master_config_t;

//Master configuration instance.
static master_config_t master_config = {
  .read_reg_callback = NULL,
  .write_reg_callback = NULL,
};

//Slave state variables structure.
typedef struct {
  i32ctt_slave_instance_t *p_list_head;
  i32ctt_slave_instance_t *p_list_tail;
  uint8_t current_endpoint;
} slave_state_t;

//Slave state variables instance.
static slave_state_t slave_state = {
  .p_list_head = NULL,
  .p_list_tail = NULL,
  .current_endpoint = 0,
};

//I32CTT common functions.
//--------------------------------------------------------------------------------------------------

void i32ctt_update() {
  const uint8_t *p_rx_buffer;
  uint16_t packet_size;
  const i32ctt_frame_header_t *p_header;
  const void *p_payload;
  uint16_t payload_count;
  i32ctt_slave_instance_t *p_slave;

  //Check wether a packet is available.
  if (!mac_packet_driver.is_packet_available())
    return;

  //Try to retrieve the packet from the MAC packet driver.
  if (!mac_packet_driver.get_rx_packet(&p_rx_buffer, &packet_size))
    return;

  //Try to decapsulate the packet using the framer.
  if (!i32ctt_framer_decapsulate_packet(p_rx_buffer, packet_size, &p_header, &p_payload,
      &payload_count))
  {
    return;
  }

  //Direct the packet to a callback function, based on the command contained in it.
  switch (p_header->command) {
    case I32CTT_READ_CMD:
      //Direct READ_CMD packets to the slave that handles the endpoint. Run through the slave list.
      for (p_slave = slave_state.p_list_head; p_slave != NULL; p_slave = p_slave->p_list_next) {
        //Check wether this slave handles the endpoint (endpoint number matches).
        if (p_header->endpoint == p_slave->endpoint) {
          //Check wether the callback function is set.
          if (p_slave->read_reg_callback != NULL) {
            //Store the current endpoint for the i32ctt_slave_send_read_reg_ans function, which
            //should be called by the slave.
            slave_state.current_endpoint = p_header->endpoint;

            //Call the callback.
            p_slave->read_reg_callback(p_payload, payload_count);
          }
        }
      }
      break;
    case I32CTT_READ_ANS:
      //Direct READ_ANS packets to the master's read register callback function (if set).
      if (master_config.read_reg_callback != NULL)
        master_config.read_reg_callback(p_payload, payload_count);
      break;
    case I32CTT_WRITE_CMD:
      //Direct WRITE_CMD packets to the slave that handles the endpoint. Run through the slave list.
      for (p_slave = slave_state.p_list_head; p_slave != NULL; p_slave = p_slave->p_list_next) {
        //Check wether this slave handles the endpoint (endpoint number matches).
        if (p_header->endpoint == p_slave->endpoint) {
          //Check wether the callback function is set.
          if (p_slave->write_reg_callback != NULL) {
            //Store the current endpoint for the i32ctt_slave_send_write_reg_ans function, which
            //should be called by the slave.
            slave_state.current_endpoint = p_header->endpoint;

            //Call the callback.
            p_slave->write_reg_callback(p_payload, payload_count);
          }
        }
      }
      break;
    case I32CTT_WRITE_ANS:
      //Direct WRITE_ANS packets to the master's write register callback function (if set).
      if (master_config.write_reg_callback != NULL)
        master_config.write_reg_callback(p_payload, payload_count);
      break;
  }

  //Packet finished processing or got dropped. Dismiss it.
  mac_packet_driver.dismiss_rx_packet();
}

//I32CTT master functions.
//--------------------------------------------------------------------------------------------------

void i32ctt_master_set_read_callback(i32ctt_master_read_reg_callback_t callback_function) {
  //Store the read register callback function.
  master_config.read_reg_callback = callback_function;
}

void i32ctt_master_set_write_callback(i32ctt_master_write_reg_callback_t callback_function) {
  //Store the write register callback function.
  master_config.write_reg_callback = callback_function;
}

i32ctt_addr_t *i32ctt_master_get_read_cmd_buffer(uint16_t * max_reg_count) {
  uint8_t *p_mac_tx_buffer;

  //Check whether the maximum register count is requested (passed pointer is not NULL).
  if (max_reg_count != NULL) {
    uint16_t mac_mtu_size;

    //If requested, get the maximum transfer unit for MAC along with the pointer to the transmit
    //buffer, then call the framer to determine the maximum register count.
    p_mac_tx_buffer = mac_packet_driver.get_tx_packet(&mac_mtu_size);
    *max_reg_count = i32ctt_framer_get_max_payload_count(I32CTT_READ_CMD, mac_mtu_size);
  }
  else
    //If not requested, just get the pointer to the transmit buffer.
    p_mac_tx_buffer = mac_packet_driver.get_tx_packet(NULL);

  //Return the pointer to the transmit buffer for the payload. This is the pointer to the transmit
  //buffer from the layer below (MAC) moved forward to the frame payload offset.
  return (i32ctt_addr_t *) &p_mac_tx_buffer[i32ctt_framer_get_payload_offset(I32CTT_READ_CMD)];
}

i32ctt_reg_pair_t *i32ctt_master_get_write_cmd_buffer(uint16_t * max_reg_count) {
  uint8_t *p_mac_tx_buffer;

  //Check whether the maximum register count is requested (passed pointer is not NULL).
  if (max_reg_count != NULL) {
    uint16_t mac_mtu_size;

    //If requested, get the maximum transfer unit for MAC along with the pointer to the transmit
    //buffer, then call the framer to determine the maximum register count.
    p_mac_tx_buffer = mac_packet_driver.get_tx_packet(&mac_mtu_size);
    *max_reg_count = i32ctt_framer_get_max_payload_count(I32CTT_WRITE_CMD, mac_mtu_size);
  }
  else
    //If not requested, just get the pointer to the transmit buffer.
    p_mac_tx_buffer = mac_packet_driver.get_tx_packet(NULL);

  //Return the pointer to the transmit buffer for the payload. This is the pointer to the transmit
  //buffer from the layer below (MAC) moved forward to the frame payload offset.
  return (i32ctt_reg_pair_t *) &p_mac_tx_buffer[i32ctt_framer_get_payload_offset(I32CTT_WRITE_CMD)];
}

bool i32ctt_master_send_read_reg_cmd(uint8_t endpoint, uint16_t reg_count) {
  uint8_t *p_mac_tx_buffer;
  uint16_t mac_mtu_size;
  uint16_t packet_size;

  //Get the pointer to the transmit buffer and the MTU size from the MAC packet driver.
  p_mac_tx_buffer = mac_packet_driver.get_tx_packet(&mac_mtu_size);

  //Make sure that the payload isn't too large.
  if (reg_count > i32ctt_framer_get_max_payload_count(I32CTT_READ_CMD, mac_mtu_size))
    return false;

  //Encapsulate the packet using the I32CTT framer.
  packet_size = i32ctt_framer_encapsulate_packet(I32CTT_READ_CMD, endpoint, p_mac_tx_buffer,
                                                 reg_count);

  //Send the packet through the MAC packet driver.
  return mac_packet_driver.packet_send(packet_size);
}

bool i32ctt_master_send_write_reg_cmd(uint8_t endpoint, uint16_t reg_count) {
  uint8_t *p_mac_tx_buffer;
  uint16_t mac_mtu_size;
  uint16_t packet_size;

  //Get the pointer to the transmit buffer and the MTU size from the MAC packet driver.
  p_mac_tx_buffer = mac_packet_driver.get_tx_packet(&mac_mtu_size);

  //Make sure that the payload isn't too large.
  if (reg_count > i32ctt_framer_get_max_payload_count(I32CTT_WRITE_CMD, mac_mtu_size))
    return false;

  //Encapsulate the packet using the I32CTT framer.
  packet_size = i32ctt_framer_encapsulate_packet(I32CTT_WRITE_CMD, endpoint, p_mac_tx_buffer,
                                                 reg_count);

  //Send the packet through the MAC packet driver.
  return mac_packet_driver.packet_send(packet_size);  
}

//I32CTT slave functions.
//--------------------------------------------------------------------------------------------------

void i32ctt_slave_add_instance(i32ctt_slave_instance_t *p_slave) {
  //If the list is empty set this slave as the head. Append to the tail otherwise.
  if (slave_state.p_list_head == NULL)
    slave_state.p_list_head = p_slave;
  else
    slave_state.p_list_tail->p_list_next = p_slave;

  //Set the tail to this slave, then make sure NULL follows the tail.
  slave_state.p_list_tail = p_slave;
  p_slave->p_list_next = NULL;
}

i32ctt_reg_pair_t *i32ctt_slave_get_read_ans_buffer() {
  uint8_t *p_mac_tx_buffer;

  //Get the pointer to the transmit buffer.
  p_mac_tx_buffer = mac_packet_driver.get_tx_packet(NULL);

  //Return the pointer to the transmit buffer for the payload. This is the pointer to the transmit
  //buffer from the layer below (MAC) moved forward to the frame payload offset.
  return (i32ctt_reg_pair_t *) &p_mac_tx_buffer[i32ctt_framer_get_payload_offset(I32CTT_READ_ANS)];
}

i32ctt_addr_t *i32ctt_slave_get_write_ans_buffer() {
  uint8_t *p_mac_tx_buffer;

  //Get the pointer to the transmit buffer.
  p_mac_tx_buffer = mac_packet_driver.get_tx_packet(NULL);

  //Return the pointer to the transmit buffer for the payload. This is the pointer to the transmit
  //buffer from the layer below (MAC) moved forward to the frame payload offset.
  return (i32ctt_addr_t *) &p_mac_tx_buffer[i32ctt_framer_get_payload_offset(I32CTT_WRITE_ANS)];
}

bool i32ctt_slave_send_read_reg_ans(uint16_t reg_count) {
  uint8_t *p_mac_tx_buffer;
  uint16_t mac_mtu_size;
  uint16_t packet_size;

  //Get the pointer to the transmit buffer and the MTU size from the MAC packet driver.
  p_mac_tx_buffer = mac_packet_driver.get_tx_packet(&mac_mtu_size);

  //Make sure that the payload isn't too large.
  if (reg_count > i32ctt_framer_get_max_payload_count(I32CTT_READ_ANS, mac_mtu_size))
    return false;

  //Encapsulate the packet using the I32CTT framer. Use the currently stored endpoint.
  packet_size = i32ctt_framer_encapsulate_packet(I32CTT_READ_ANS, slave_state.current_endpoint,
                                                 p_mac_tx_buffer, reg_count);

  //Send the packet through the MAC packet driver.
  return mac_packet_driver.packet_send(packet_size);
}

bool i32ctt_slave_send_write_reg_ans(uint16_t reg_count) {
  uint8_t *p_mac_tx_buffer;
  uint16_t mac_mtu_size;
  uint16_t packet_size;

  //Get the pointer to the transmit buffer and the MTU size from the MAC packet driver.
  p_mac_tx_buffer = mac_packet_driver.get_tx_packet(&mac_mtu_size);

  //Make sure that the payload isn't too large.
  if (reg_count > i32ctt_framer_get_max_payload_count(I32CTT_WRITE_ANS, mac_mtu_size))
    return false;

  //Encapsulate the packet using the I32CTT framer. Use the currently stored endpoint.
  packet_size = i32ctt_framer_encapsulate_packet(I32CTT_WRITE_ANS, slave_state.current_endpoint,
                                                 p_mac_tx_buffer, reg_count);

  //Send the packet through the MAC packet driver.
  return mac_packet_driver.packet_send(packet_size);  
}
