#Driver para el protocolo de mac de IEEE 802.15.4

from framer import framer_ieee802154

class driver_ieee802154:
  def __init__(self, radio):
    self.__radio = radio
    self.__framer = framer_ieee802154(self.__radio.leer_len_mtu())

  def leer_len_mtu(self):
    #Retorna la longitud de la maxima unidad de transferencia (payload de paquete mas grande
    #soportado) mediante el framer
    return self.__framer.leer_len_mtu()

  def escr_config_red(self, canal, pan_id, dir_corta):
    #Traslada la configuracion al radio
    self.__radio.escr_canal(canal)
    self.__radio.escr_pan_id(pan_id)
    self.__radio.escr_dir_corta(dir_corta)

    #Traslada la configuracion al framer
    self.__framer.escr_pan_id(pan_id)
    self.__framer.escr_dir_corta(dir_corta)

  def enviar_paquete(self, destino, payload):
    #Se verifica que el paquete sea de una longitud adecuada
    if len(payload) > self.__framer.leer_len_mtu():
      raise ValueError("El paquete solicitado es demasiado largo")

    paquete = self.__framer.crear_mpdu(destino, payload)
    return self.__radio.enviar_paquete(paquete)

  def hay_paquete(self):
    return self.__radio.hay_paquete()

  def recibir_paquete(self):
    paquete = self.__radio.recibir_paquete()
    return self.__framer.extraer_payload(paquete)
