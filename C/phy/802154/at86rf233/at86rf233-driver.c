//Headers for provided APIs.
#include "at86rf233-driver.h"
#include "phy-802154-driver.h"
#include "phy-packet-driver.h"

//Headers for depended APIs.
#include "at86rf233-iodriver.h"

//Implementation specific headers.
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "at86rf233-radio.h"

//Local declarations.
//--------------------------------------------------------------------------------------------------

//Time out in ms for packet transmission.
#define RADIO_TRANSMIT_TIMEOUT_MS 50

//Macro used to perform a read-modify-write operation on a register bitfield. Note that the full
//name of the register address, the field mask and field position are inferred from the parameters.
#define REGISTER_WRITE_FIELD(REG, FIELD, VALUE)\
  register_write(REG##_ADDR,\
                 (register_read(REG##_ADDR) & ~REG##_##FIELD##_MASK) |\
                 (((VALUE) << REG##_##FIELD##_POS) & (REG##_##FIELD##_MASK)))

//Macro used to perform a read access on a register bitfield.
#define REGISTER_READ_FIELD(REG, FIELD)\
  ((register_read(REG##_ADDR) & REG##_##FIELD##_MASK) >> REG##_##FIELD##_POS)

//Macros used to test individual bits in a register.
#define REGISTER_BIT_IS_SET(REG, FIELD) ((register_read(REG##_ADDR) & REG##_##FIELD##_MASK) != 0)
#define REGISTER_BIT_IS_CLEAR(REG, FIELD) ((register_read(REG##_ADDR) & REG##_##FIELD##_MASK) == 0)

//Enumeration of relevant radio target states (not related to actual state codes used by radio).
//Used during radio state changes.
typedef enum {
  TARGET_STATE_RX_AACK = 0, TARGET_STATE_TX_ARET = 1,
} target_state_t;

//Statically allocated transmit and receive buffers. All protocol layers encapsulate/decapsulate
//their frames inside these.
static uint8_t radio_tx_buffer[127];
static uint8_t radio_rx_buffer[127];

//State variables structure.
typedef struct {
  uint16_t rx_payload_size;   //Payload size of packet in receive buffer
  bool rx_packet_waiting;     //Packet is ready in the receive buffer and waiting to be processed
  bool rx_packet_queued;      //Packet is stored in the radio buffer and waiting to be retrieved
} driver_state_t;

//State variables instance.
static driver_state_t driver_state = {
  .rx_payload_size = 0,
  .rx_packet_waiting = false,
  .rx_packet_queued = false,
};

//Local functions.
static uint8_t register_read(uint8_t addr);
static void register_write(uint8_t addr, uint8_t value);
static void set_radio_state(target_state_t target_state);
static void rx_packet_handler();
static bool rx_packet_read();

//PHY packet driver API declarations.
//--------------------------------------------------------------------------------------------------

//PHY packet driver API functions.
static uint8_t *get_tx_packet(uint16_t *p_mtu_size);
static bool packet_send(uint16_t payload_size);
static bool is_packet_available();
static bool get_rx_packet(const uint8_t **pp_rx_buffer, uint16_t *p_payload_size);
static void dismiss_rx_packet();

//PHY packet driver instance.
const packet_driver_t phy_packet_driver = {
  .get_tx_packet       = get_tx_packet,
  .packet_send         = packet_send,
  .is_packet_available = is_packet_available,
  .get_rx_packet       = get_rx_packet,
  .dismiss_rx_packet   = dismiss_rx_packet,
};

//Local functions.
//--------------------------------------------------------------------------------------------------

static uint8_t register_read(uint8_t addr) {
  const uint8_t cmd_byte = SPI_CMD_REG_READ | addr;
  uint8_t rx_byte;

  //Perform the register access mode sequence.
  at86rf233_iodriver.write_cs_pin(false);       //Assert chip select
  at86rf233_iodriver.spi_write(&cmd_byte, 1);   //Write command byte
  at86rf233_iodriver.spi_read(&rx_byte, 1);     //Read register value
  at86rf233_iodriver.write_cs_pin(true);        //Deassert chip select

  return rx_byte;
}

static void register_write(uint8_t addr, uint8_t value) {
  const uint8_t spi_tx_buffer[2] = { SPI_CMD_REG_WRITE | addr, value };

  //Perform the register access mode sequence.
  at86rf233_iodriver.write_cs_pin(false);           //Assert chip select
  at86rf233_iodriver.spi_write(spi_tx_buffer, 2);   //Write command and register value
  at86rf233_iodriver.write_cs_pin(true);            //Deassert chip select
}

static void set_radio_state(target_state_t target_state) {
  uint8_t state;

  //Loop repeats undefinitely until reaching the desired state.
  for (;;) {
    //Get the current radio state.
    state = REGISTER_READ_FIELD(TRX_STATUS, TRX_STATUS);

    //Determine next action based on current state.
    switch (state) {
      case TRX_STATUS_TRX_STATUS_TRX_OFF:
        //Current state is clock state. Transition directly to desired state.
        switch (target_state) {
          case TARGET_STATE_RX_AACK:
            register_write(TRX_STATE_ADDR, TRX_STATE_TRX_CMD_RX_AACK_ON);
            break;
          case TARGET_STATE_TX_ARET:
            register_write(TRX_STATE_ADDR, TRX_STATE_TRX_CMD_TX_ARET_ON);
            break;
          default:
            return;
        }
        break;
      case TRX_STATUS_TRX_STATUS_BUSY_RX_AACK:
      case TRX_STATUS_TRX_STATUS_BUSY_TX_ARET:
      case TRX_STATUS_TRX_STATUS_STATE_TRANSITION_IN_PROGRESS:
        //Busy states are transitional and end eventually by themselves.
        break;
      case TRX_STATUS_TRX_STATUS_RX_AACK_ON:
        //Current state is RX listen state with automatic acknoledge.
        switch (target_state) {
          case TARGET_STATE_RX_AACK:
            //Already on desired state.
            return;
          case TARGET_STATE_TX_ARET:
            register_write(TRX_STATE_ADDR, TRX_STATE_TRX_CMD_TX_ARET_ON);
            break;
          default:
            return;
        }
        break;
      case TRX_STATUS_TRX_STATUS_TX_ARET_ON:
        //Current state is PLL state with automatic retransmission enabled.
        switch (target_state) {
          case TARGET_STATE_RX_AACK:
            register_write(TRX_STATE_ADDR, TRX_STATE_TRX_CMD_RX_AACK_ON);
            break;
          case TARGET_STATE_TX_ARET:
            //Already on desired state.
            return;
          default:
            return;
        }
        break;
      case TRX_STATUS_TRX_STATUS_P_ON:
      case TRX_STATUS_TRX_STATUS_RX_ON:
      case TRX_STATUS_TRX_STATUS_PLL_ON:
      case TRX_STATUS_TRX_STATUS_PREP_DEEP_SLEEP:
        //These states are not used. Transition to clock state to abandon them.
        register_write(TRX_STATE_ADDR, TRX_STATE_TRX_CMD_TRX_OFF);
        break;
      default:
        //For any other state (which is considered invalid), try to recover the radio by forcing a
        //transition to clock state. This includes the simpler (non automatic) transmit and receive
        //states.
        register_write(TRX_STATE_ADDR, TRX_STATE_TRX_CMD_FORCE_TRX_OFF);
        break;
    }
  }
}

static void rx_packet_handler() {
  //Check wether the transfer complete interrupt in the radio is set. Since the IRQ_STATUS register
  //is read, all interrupts will be cleared. If not set, the interrupt is discarded.
  if (REGISTER_BIT_IS_CLEAR(IRQ_STATUS, IRQ_3_TRX_END))
    return;

  //Check wether the previous packet has been processed. If not processed set the packet queued flag
  //and leave it in the radio buffer so it can be retrieved later.
  if (driver_state.rx_packet_waiting) {
    driver_state.rx_packet_queued = true;
    return;
  }

  //Retrieve the packet from the radio. If successful, set the packet waiting flag.
  if (rx_packet_read())
    driver_state.rx_packet_waiting = true;
}

static bool rx_packet_read() {
  uint8_t trac_status;
  const uint8_t cmd_byte = SPI_CMD_FB_READ;
  uint8_t PHR_byte;

  //Check wether the last transaction status is successful. If not, drop the packet.
  trac_status = REGISTER_READ_FIELD(TRX_STATE, TRAC_STATUS);
  if (trac_status != TRX_STATE_TRAC_STATUS_SUCCESS &&
      trac_status != TRX_STATE_TRAC_STATUS_SUCCESS_WAIT_FOR_ACK)
  {
    return false;
  }

  //Perform the frame buffer access mode sequence.
  at86rf233_iodriver.write_cs_pin(false);                     //Assert chip select
  at86rf233_iodriver.spi_write(&cmd_byte, 1);                 //Write command byte
  at86rf233_iodriver.spi_read(&PHR_byte, 1);                  //Read PHY header
  driver_state.rx_payload_size = PHR_byte & 0x7F;             //Determine payload size
  at86rf233_iodriver.spi_read(radio_rx_buffer,
                              driver_state.rx_payload_size);  //Read the payload
  at86rf233_iodriver.write_cs_pin(true);                      //Deassert chip select

  //Packet has been read. Return successfully.
  return true;
}

//AT86RF233 driver functions.
//--------------------------------------------------------------------------------------------------

void at86rf233_driver_init(bool enable_fem_txrx) {
  //Reset the AT86RF233 radio.
  at86rf233_iodriver.pulse_rst_pin();

  //Enable dynamic frame buffer protection so received packets aren't overwritten by new ones until
  //read.
  REGISTER_WRITE_FIELD(TRX_CTRL_2, RX_SAFE_MODE, 1);

  //Read the interrupt status register to clear their state before enabling them.
  register_read(IRQ_STATUS_ADDR);

  //Set the received packet handler function as the irq handler for the radio.
  at86rf233_iodriver.set_irq_handler(rx_packet_handler);

  //Enable the interrupt from this side.
  at86rf233_iodriver.irq_enable(true);

  //Enable the radio transfer complete interrupt in the radio.
  register_write(IRQ_MASK_ADDR, IRQ_MASK_IRQ_3_TRX_END_MASK);

  //Enable automatic control of the external RF front end module through pin DIG3 and DIG4 if
  //requested.
  if (enable_fem_txrx)
    REGISTER_WRITE_FIELD(TRX_CTRL_1, PA_EXT_EN, 1);

  //All configured. Set the radio in the receive with automatic acknoledge state.
  set_radio_state(TARGET_STATE_RX_AACK);
}

//PHY 802.15.4 driver functions.
//--------------------------------------------------------------------------------------------------

void phy_802154_driver_set_channel(uint8_t channel) {
  //Write the requested channel in the PHY_CC_CCA register.
  REGISTER_WRITE_FIELD(PHY_CC_CCA, CHANNEL, channel);
}

void phy_802154_driver_set_pan_id(uint16_t pan_id) {
  //Store the requested PAN ID in the corresponding register pair.
  register_write(PAN_ID_0_ADDR, pan_id & 0xFF);
  register_write(PAN_ID_1_ADDR, pan_id >> 8);
}

void phy_802154_driver_set_short_addr(uint16_t short_addr) {
  //Store the requested short address in the corresponding register pair.
  register_write(SHORT_ADDR_0_ADDR, short_addr & 0xFF);
  register_write(SHORT_ADDR_1_ADDR, short_addr >> 8);
}

//PHY packet driver API functions.
//--------------------------------------------------------------------------------------------------

static uint8_t *get_tx_packet(uint16_t *p_mtu_size) {
  //Return the maximum transfer unit size (largest supported packet), if requested.
  if (p_mtu_size != NULL)
    *p_mtu_size = 127;

  //Return the pointer to the transmit buffer.
  return radio_tx_buffer;
}

static bool packet_send(uint16_t payload_size) {
  const uint8_t cmd_byte = SPI_CMD_FB_WRITE;
  uint8_t PHR_byte;
  uint32_t t_start;
  bool retval = false;

  //Make sure that the packet isn't too large.
  if (payload_size > 127)
    return false;

  //Set the radio in the transmit with automatic retry state.
  set_radio_state(TARGET_STATE_TX_ARET);

  //Perform the frame buffer access mode sequence.
  at86rf233_iodriver.write_cs_pin(false);                     //Assert chip select
  at86rf233_iodriver.spi_write(&cmd_byte, 1);                 //Write command byte
  PHR_byte = payload_size & 0x7F;                             //Determine PHY header
  at86rf233_iodriver.spi_write(&PHR_byte, 1);                 //Write PHY header
  at86rf233_iodriver.spi_write(radio_tx_buffer, PHR_byte);    //Write the payload
  at86rf233_iodriver.write_cs_pin(true);                      //Deassert chip select

  //Temporarily disable the radio interrupt so the packet receive handler doesn't trigger.
  at86rf233_iodriver.irq_enable(false);

  //Pulse the SLP_TR pin to start packet transmission.
  at86rf233_iodriver.pulse_slp_tr_pin();

  //Wait for the radio to activate its IRQ pin after sending the packet.
  t_start = at86rf233_iodriver.ms_count();
  for (;;) {
    //Check wether the IRQ pin is active.
    if (at86rf233_iodriver.read_irq_pin()) {
      //Assert that the interrupt is caused by packet transmission end (also clears the interrupt).
      if (REGISTER_BIT_IS_SET(IRQ_STATUS, IRQ_3_TRX_END)) {
        //Check wether the transaction was successful.
        if (REGISTER_READ_FIELD(TRX_STATE, TRAC_STATUS) == TRX_STATE_TRAC_STATUS_SUCCESS)
          //Set the return value flag if successful.
          retval = true;

        //Radio transaction finished. Abandon the loop.
        break;
      }
    }

    //Check wether the timeout period has elapsed. Abandon the loop if so.
    if (at86rf233_iodriver.ms_count() - t_start >= RADIO_TRANSMIT_TIMEOUT_MS)
      break;
  }

  //Set the radio in the receive with automatic acknoledge state again.
  set_radio_state(TARGET_STATE_RX_AACK);

  //Enable the radio interrupt again.
  at86rf233_iodriver.irq_enable(true);

  return retval;
}

static bool is_packet_available() {
  //A packet is available if the packet waiting flag is set.
  return driver_state.rx_packet_waiting;
}

static bool get_rx_packet(const uint8_t **pp_rx_buffer, uint16_t *p_payload_size) {
  //Assert that a packet is waiting.
  if (!driver_state.rx_packet_waiting)
    return false;

  //Return the pointer to the receive buffer and the size of last received packet.
  *pp_rx_buffer = radio_rx_buffer;
  *p_payload_size = driver_state.rx_payload_size;
  return true;
}

static void dismiss_rx_packet() {
  //Packet processed by caller. Clear the packet waiting flag.
  driver_state.rx_packet_waiting = false;

  //Check wether a packet is queued. If so, it'll have to be retrieved.
  if (!driver_state.rx_packet_queued)
    return;

  //Disable IRQs before reading the frame buffer.
  at86rf233_iodriver.irq_enable(false);

  //Retrieve the packet from the radio. If successful, set the packet waiting flag.
  if (rx_packet_read())
    driver_state.rx_packet_waiting = true;

  //Packet has been retrieved. Clear the packet queued flag.
  driver_state.rx_packet_queued = false;

  //Enable the radio interrupt again.
  at86rf233_iodriver.irq_enable(true);
}
