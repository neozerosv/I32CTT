import RPi.GPIO as gpio
from phy.at86rf233_rpi.driver import driver_at86rf233
from mac.ieee802154.driver import driver_ieee802154
from i32ctt.driver import driver_i32ctt

#En caso de usar un radio con PA/LNA, cuya direccion de comunicacion es controlada por el AT86RF233
#y cuyo control de pass through se controla por GPIO, se puede usar esta linea:
#phy = driver_at86rf233(gpio, FEM_TXRX = True, pin_FEM_CPS = 15)

#Para radios de OpenLabs (Raspberry Pi 802.15.4 radio) se usa esta linea:
phy = driver_at86rf233(gpio)

#Se instancia el driver de MAC, pasandole el driver de PHY
mac = driver_ieee802154(phy)

#Se instancia el driver de I32CTT, pasandole el driver de MAC
i32ctt = driver_i32ctt(mac)

#Configura el radio 802.15.4
mac.escr_config_red(canal=26, pan_id=0xCAFE, dir_corta=0x0200)

try:
  #Ejemplo de escritura de registro. Escribe al nodo con direccion MAC 0x0100, endpoint 0 los
  #siguentes registros, los cuales se pasan como una lista de tuplas
  #Registro 2: 0x0
  #Registro 3: 0x00FFFFFF
  registros = i32ctt.escr_registros(0x0100, 0, [(2, 0x0), (3, 0x00FFFFFF)])
  #Se imprime la lista de registros retornada por el otro nodo
  if registros:
    for i in registros:
      print("Registro escrito: {}".format(i))
  else:
    print("No hubo respuesta al escribir")
    exit()

  #Ejemplo de lectura de registro. Lee del nodo con direccion MAC 0x0100, endpoint 0, los siguentes
  #registros, los cuales deben se pasados como una lista
  #Registro 0
  #Registro 1
  pares = i32ctt.leer_registros(0x0100, 0, [0, 1])
  #Se imprime la lista de tuplas retornada por el otro nodo (poseen la misma estructura que se le
  #pasa a la funcion de escritura)
  if pares:
    for i in pares:
      print("Registro {}: 0x{:08X}".format(i[0], i[1]))
  else:
    print("No hubo respuesta al leer")

finally:
  gpio.cleanup()
