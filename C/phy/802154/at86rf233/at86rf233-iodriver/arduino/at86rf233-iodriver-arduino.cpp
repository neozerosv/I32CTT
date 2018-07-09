//Headers for provided APIs.
#include "at86rf233-iodriver-arduino.h"
#include "at86rf233-iodriver.h"

//Implementation specific headers.
#include "Arduino.h"
#include "SPI.h"

extern "C" {

//Local declarations.
//--------------------------------------------------------------------------------------------------

//Driver configuration structure.
typedef struct {
  int cs_pin;
  int rst_pin;
  int slp_tr_pin;
  int irq_pin;
  at86rf233_iodriver_irq_handler_t irq_handler;
} config_t;

//Driver configuration instance.
static config_t config {
  .cs_pin = 0,
  .rst_pin = 0,
  .slp_tr_pin = 0,
  .irq_pin = 0,
  .irq_handler = NULL,
};

//AT86RF233 I/O driver API declarations.
//--------------------------------------------------------------------------------------------------

//AT86RF233 I/O driver API functions.
static void spi_write(const uint8_t *p_buffer, uint8_t len);
static void spi_read(uint8_t *p_buffer, uint8_t len);
static void set_irq_handler(at86rf233_iodriver_irq_handler_t irq_handler);
static void irq_enable(bool state);
static void write_cs_pin(bool state);
static void pulse_rst_pin();
static void pulse_slp_tr_pin();
static bool read_irq_pin();
static uint32_t ms_count();

//AT86RF233 I/O driver instance.
const at86rf233_iodriver_t at86rf233_iodriver = {
  .spi_write        = spi_write,
  .spi_read         = spi_read,
  .set_irq_handler  = set_irq_handler,
  .irq_enable       = irq_enable,
  .write_cs_pin     = write_cs_pin,
  .pulse_rst_pin    = pulse_rst_pin,
  .pulse_slp_tr_pin = pulse_slp_tr_pin,
  .read_irq_pin     = read_irq_pin,
  .ms_count         = ms_count,
};

//Arduino AT86RF233 I/O driver functions.
//--------------------------------------------------------------------------------------------------

void at86rf233_iodriver_arduino_init(int cs_pin, int rst_pin, int slp_tr_pin, int irq_pin) {
  //Store driver configurations.
  config.cs_pin = cs_pin;
  config.rst_pin = rst_pin;
  config.slp_tr_pin = slp_tr_pin;
  config.irq_pin = irq_pin;

  //Initialize SPI peripheral.
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE0);

  //Initialize output pins.
  pinMode(config.cs_pin, OUTPUT);
  digitalWrite(config.cs_pin, HIGH);
  pinMode(config.rst_pin, OUTPUT);
  digitalWrite(config.rst_pin, HIGH);
  pinMode(config.slp_tr_pin, OUTPUT);
  digitalWrite(config.slp_tr_pin, LOW);

  //Initialize input pins.
  pinMode(config.irq_pin, INPUT);
}

void at86rf233_iodriver_arduino_set_fem_cps_pin(int fem_cps_pin) {
  //Configures the CPS pin for the radio frontend module and sets is to high so PA/LNA mode is used.
  pinMode(fem_cps_pin, OUTPUT);
  digitalWrite(fem_cps_pin, HIGH);
}

//AT86RF233 I/O driver API functions.
//--------------------------------------------------------------------------------------------------

static void spi_write(const uint8_t *p_buffer, uint8_t len) {
  //Write as many bytes as requested from the buffer.
  while (len--)
    SPI.transfer(*p_buffer++);
}

static void spi_read(uint8_t *p_buffer, uint8_t len) {
  //Read as many bytes as requested to the buffer.
  while (len--)
    *p_buffer++ = SPI.transfer(0x00);
}

static void set_irq_handler(at86rf233_iodriver_irq_handler_t irq_handler) {
  //Store the irq handler pointer.
  config.irq_handler = irq_handler;
}

static void irq_enable(bool state) {
  //If enabling, attach the interrupt pin to the stored irq handler. Otherwise detach it.
  if (state && config.irq_handler != NULL)
    attachInterrupt(digitalPinToInterrupt(config.irq_pin), config.irq_handler, RISING);
  else
    detachInterrupt(digitalPinToInterrupt(config.irq_pin));
}

static void write_cs_pin(bool state) {
  //Update the pin state.
  digitalWrite(config.cs_pin, state);
}

static void pulse_rst_pin() {
  //Pulse the reset pin.
  digitalWrite(config.rst_pin, LOW);
  delay(100);
  digitalWrite(config.rst_pin, HIGH);
  delay(100);
}

static void pulse_slp_tr_pin() {
  //Update the pin state.
  digitalWrite(config.slp_tr_pin, HIGH);
  delayMicroseconds(1);
  digitalWrite(config.slp_tr_pin, LOW);
}

static bool read_irq_pin() {
  //Return the pin state.
  return digitalRead(config.irq_pin);
}

static uint32_t ms_count() {
  //Return the milliseconds count.
  return millis();
}

} //extern "C"
