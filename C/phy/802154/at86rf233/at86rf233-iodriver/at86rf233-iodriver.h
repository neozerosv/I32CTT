#ifndef AT86RF233_IODRIVER_H_
#define AT86RF233_IODRIVER_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

//Function pointer for radio irq handler. Provided by PHY layer driver.
typedef void (*at86rf233_iodriver_irq_handler_t)(void);

//AT86RF233 I/O driver structure.
typedef struct {
  void (*spi_write)(const uint8_t *p_buffer, uint8_t len);
  void (*spi_read)(uint8_t *p_buffer, uint8_t len);
  void (*set_irq_handler)(at86rf233_iodriver_irq_handler_t irq_handler);
  void (*irq_enable)(bool state);
  void (*write_cs_pin)(bool state);
  void (*pulse_rst_pin)();
  void (*pulse_slp_tr_pin)();
  bool (*read_irq_pin)();
  uint32_t (*ms_count)();
} at86rf233_iodriver_t;

//AT86RF233 I/O driver instance.
extern const at86rf233_iodriver_t at86rf233_iodriver;

#ifdef __cplusplus
}
#endif

#endif //AT86RF233_IODRIVER_H_
