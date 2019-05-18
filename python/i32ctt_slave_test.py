import sys
import signal
import RPi.GPIO as gpio
from phy.at86rf233_rpi.driver import driver_at86rf233
from mac.ieee802154.driver import driver_ieee802154
from i32ctt.driver import driver_i32ctt
import time

#Ejemplo de driver de esclavo de I32CTT
class esclavo_prueba(driver_i32ctt.driver_esclavo):
  #Callback de lectura de registros. Recibe como argumento una lista con las direcciones de los
  #registros a leer. Debe retornar una lista de tuplas. Cada tupla contiene la direccion del
  #registro en el indice 0 y el valor del mismo en el indice 1.
  def callback_leer_registros(self, dir_registros):
    par_registros = []
    for d in dir_registros:
      #Se conforma la respuesta con tuplas que contienen la misma direccion de la llamada de lectura
      #y el dato, que es igual a la direccion incrementada en 1
      par = (d, d + 1)
      par_registros.append(par)
      print("Leido registro {:}, devolviendo 0x{:X}".format(par[0], par[1]))

    return par_registros

  #Callback de escritura de registros. Recibe como argumento una lista de tuplas. Cada tupla
  #contiene la direccion del registro en el indice 0 y el valor del mismo en el indice 1. Debe
  #retornar una lista con las direcciones de los registros modificados.
  def callback_escr_registros(self, par_registros):
    for par in par_registros:
      print("Registro {:} escrito con dato 0x{:X}".format(par[0], par[1]))

    #La respuesta se conforma usando las mismas direcciones que vienen en la llamada de escritura
    return map(lambda x: x[0], par_registros)

print("Listo para comenzar")

#En caso de usar un radio con PA/LNA, cuya direccion de comunicacion es controlada por el AT86RF233
#y cuyo control de pass through se controla por GPIO, se puede usar esta linea:
#phy = driver_at86rf233(gpio, FEM_TXRX = True, pin_FEM_CPS = 15)

#Para radios de OpenLabs (Raspberry Pi 802.15.4 radio) se usa esta linea:
phy = driver_at86rf233(gpio)
print("Physical interface started")

#Se instancia el driver de MAC, pasandole el driver de PHY
mac = driver_ieee802154(phy)
print("MAC driver initialized")

#Se instancia el driver de I32CTT, pasandole el driver de MAC
i32ctt = driver_i32ctt(mac)
print("I32CTT initialized")

#Configura el radio 802.15.4
mac.escr_config_red(canal=26, pan_id=0xCAFE, dir_corta=0x0202)

#Crea una instancia del esclavo y lo agrega a I32CTT
esclavo_ep0 = esclavo_prueba()
i32ctt.agregar_esclavo(esclavo_ep0, 0)

#Manejador de excepciones para SIGINT
def signal_handler(signal, frame):
  sys.exit(0)

#Se registra el manejador de exepciones
signal.signal(signal.SIGINT, signal_handler)

#El lazo principal solo necesita estar llamando la funcion actualizar periodicamente, para que
#I32CTT pueda procesar los paquetes recibidos y llamar el metodo actualizar de cada esclavo.
try:
  while True:
    i32ctt.actualizar()
    time.sleep(0.01)

finally:
  gpio.cleanup()
