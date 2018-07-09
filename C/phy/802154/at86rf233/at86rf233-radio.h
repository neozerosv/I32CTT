#ifndef AT86RF233_RADIO_H_
#define AT86RF233_RADIO_H_

//SPI bus commands for AT86RF233 radio.
#define SPI_CMD_REG_READ    0x80
#define SPI_CMD_REG_WRITE   0xC0
#define SPI_CMD_FB_READ     0x20
#define SPI_CMD_FB_WRITE    0x60

//AT86RF233 register addresses.
#define TRX_STATUS_ADDR     0x01
#define TRX_STATE_ADDR      0x02
#define TRX_CTRL_1_ADDR     0x04
#define PHY_CC_CCA_ADDR     0x08
#define TRX_CTRL_2_ADDR     0x0C
#define IRQ_MASK_ADDR       0x0E
#define IRQ_STATUS_ADDR     0x0F
#define SHORT_ADDR_0_ADDR   0x20
#define SHORT_ADDR_1_ADDR   0x21
#define PAN_ID_0_ADDR       0x22
#define PAN_ID_1_ADDR       0x23

//TRX_STATUS register bitfields.
#define TRX_STATUS_TRX_STATUS_MASK                          0x1F
#define TRX_STATUS_TRX_STATUS_POS                           0
#define TRX_STATUS_TRX_STATUS_P_ON                          0x00
#define TRX_STATUS_TRX_STATUS_BUSY_RX                       0x01
#define TRX_STATUS_TRX_STATUS_BUSY_TX                       0x02
#define TRX_STATUS_TRX_STATUS_RX_ON                         0x06
#define TRX_STATUS_TRX_STATUS_TRX_OFF                       0x08
#define TRX_STATUS_TRX_STATUS_PLL_ON                        0x09
#define TRX_STATUS_TRX_STATUS_SLEEP                         0x0F
#define TRX_STATUS_TRX_STATUS_PREP_DEEP_SLEEP               0x10
#define TRX_STATUS_TRX_STATUS_BUSY_RX_AACK                  0x11
#define TRX_STATUS_TRX_STATUS_BUSY_TX_ARET                  0x12
#define TRX_STATUS_TRX_STATUS_RX_AACK_ON                    0x16
#define TRX_STATUS_TRX_STATUS_TX_ARET_ON                    0x19
#define TRX_STATUS_TRX_STATUS_STATE_TRANSITION_IN_PROGRESS  0x1F

//TRX_STATE register bitfields.
#define TRX_STATE_TRX_CMD_NOP                         0x00
#define TRX_STATE_TRX_CMD_TX_START                    0x02
#define TRX_STATE_TRX_CMD_FORCE_TRX_OFF               0x03
#define TRX_STATE_TRX_CMD_FORCE_PLL_ON                0x04
#define TRX_STATE_TRX_CMD_RX_ON                       0x06
#define TRX_STATE_TRX_CMD_TRX_OFF                     0x08
#define TRX_STATE_TRX_CMD_PLL_ON                      0x09
#define TRX_STATE_TRX_CMD_PREP_DEEP_SLEEP             0x10
#define TRX_STATE_TRX_CMD_RX_AACK_ON                  0x16
#define TRX_STATE_TRX_CMD_TX_ARET_ON                  0x19
#define TRX_STATE_TRAC_STATUS_MASK                    0xE0
#define TRX_STATE_TRAC_STATUS_POS                     5
#define TRX_STATE_TRAC_STATUS_SUCCESS                 0x00
#define TRX_STATE_TRAC_STATUS_SUCCESS_DATA_PENDING    0x01
#define TRX_STATE_TRAC_STATUS_SUCCESS_WAIT_FOR_ACK    0x02
#define TRX_STATE_TRAC_STATUS_CHANNEL_ACCESS_FAILURE  0x03
#define TRX_STATE_TRAC_STATUS_NO_ACK                  0x05
#define TRX_STATE_TRAC_STATUS_INVALID                 0x07

//TRX_CTRL_1 register bitfields.
#define TRX_CTRL_1_PA_EXT_EN_MASK   0x80
#define TRX_CTRL_1_PA_EXT_EN_POS    7

//PHY_CC_CCA register bitfields.
#define PHY_CC_CCA_CHANNEL_MASK       0x1F
#define PHY_CC_CCA_CHANNEL_POS        0

//TRX_CTRL_2 register bitfields.
#define TRX_CTRL_2_RX_SAFE_MODE_MASK      0x80
#define TRX_CTRL_2_RX_SAFE_MODE_POS       7

//IRQ_MASK register bitfields.
#define IRQ_MASK_IRQ_3_TRX_END_MASK       0x08
#define IRQ_MASK_IRQ_3_TRX_END_POS        3

//IRQ_STATUS register bitfields.
#define IRQ_STATUS_IRQ_3_TRX_END_MASK   0x08
#define IRQ_STATUS_IRQ_3_TRX_END_POS    3

#endif //AT86RF233_RADIO_H_
