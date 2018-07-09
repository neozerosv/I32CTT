#ifndef AT86RF233_DRIVER_H_
#define AT86RF233_DRIVER_H_

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

//AT86RF233 driver functions.
void at86rf233_driver_init(bool enable_fem_txrx);

#ifdef __cplusplus
}
#endif

#endif //AT86RF233_DRIVER_H_
