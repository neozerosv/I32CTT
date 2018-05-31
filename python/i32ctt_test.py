import signal
import sys
import RPi.GPIO as gpio
from phy.at86rf233_rpi.driver import driver_at86rf233
from mac.ieee802154.driver import driver_ieee802154
from i32ctt.driver import driver_i32ctt

print("Ready to start")
phy = driver_at86rf233(gpio)
print("Physical interface started")
mac = driver_ieee802154(phy)
print("MAC driver initialized")
i32ctt = driver_i32ctt(mac)
print("I32CTT initialized")

mac.escr_config_red(canal=26, pan_id=0xCAFE, dir_corta=0x0201)

def signal_handler(signal, frame):
  gpio.cleanup()
  sys.exit(0)

signal.signal(signal.SIGINT, signal_handler)

print("Trying to get packets")
try:
  #registros = i32ctt.escr_registro(0x0100, 0, ((2, 0x0),(3, 0x00FFFFFF)))
  #if registros:
  #  for i in registros:
  #    print("Registro escrito: {}".format(i))
  #else:
  #  print("No hubo respuesta al escribir")
  #  exit()

  #pares = i32ctt.leer_registro(0x0100, 0, (2,))
  #if pares:
  #  for i in pares:
  #    print("Registro {}: 0x{:08X}".format(i[0], i[1]))
  #else:
    #print("No hubo respuesta al leer")
  while True:
    i32ctt.esperar_paquete()

finally:
  gpio.cleanup()
