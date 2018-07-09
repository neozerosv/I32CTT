#ifndef AT86RF233_IODRIVER_ARDUINO_H_
#define AT86RF233_IODRIVER_ARDUINO_H_

#ifdef __cplusplus
extern "C" {
#endif

//Arduino AT86RF233 I/O driver functions.
void at86rf233_iodriver_arduino_init(int cs_pin, int rst_pin, int slp_tr_pin, int irq_pin);
void at86rf233_iodriver_arduino_set_fem_cps_pin(int fem_cps_pin);

#ifdef __cplusplus
}
#endif

#endif //AT86RF233_IODRIVER_ARDUINO_H_
