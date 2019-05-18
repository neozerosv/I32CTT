#Framer para el protocolo I32CTT

from framer import framer_i32ctt
import time

class driver_i32ctt:
  __timeout = 0.1
  __lista_esclavos = []

  #Clase vacia usada como contenedor de datos
  class clase_vacia:
    pass

  #Clase base para los esclavos de I32CTT. Provee el minimo de funcionalidad requerida.
  class driver_esclavo:
    def actualizar(self):
      pass

    def callback_leer_registros(self, dir_registros):
      return []

    def callback_escr_registros(self, par_registros):
      return []

  def __init__(self, mac):
    self.__mac = mac
    self.__framer = framer_i32ctt(self.__mac.leer_len_mtu())

  def max_registros_transferencia(self):
    #Retorna la cantidad maxima de registros que se pueden leer o escribir
    return self.__framer.leer_len_mtu(self.__framer.read_cmd)

  def leer_registros(self, dir_esclavo, num_endpoint, dir_registros):
    #Se verifica que la transaccion sea de una longitud adecuada
    if len(dir_registros) > self.__framer.leer_len_mtu(self.__framer.read_cmd):
      raise ValueError("Se requiere leer demasiados registros")

    #Se forma el paquete de I32CTT mediante el framer
    paquete = self.__framer.crear_paquete_read(num_endpoint, dir_registros)

    #Envia el paquete a la capa subyacente
    self.__mac.enviar_paquete(dir_esclavo, paquete)

    #Recibe la respuesta, la decodifica y la retorna
    paquete = self.__recibir_respuesta(dir_esclavo)
    return self.__framer.leer_paquete_read_ans(paquete, num_endpoint)

  def escr_registros(self, dir_esclavo, num_endpoint, par_registros):
    #Se verifica que la transaccion sea de una longitud adecuada
    if len(par_registros) > self.__framer.leer_len_mtu(self.__framer.write_cmd):
      raise ValueError("Se requiere escribir demasiados registros")

    #Se forma el paquete de I32CTT mediante el framer
    paquete = self.__framer.crear_paquete_write(num_endpoint, par_registros)

    #Envia el paquete a la capa subyacente
    self.__mac.enviar_paquete(dir_esclavo, paquete)

    #Recibe la respuesta, la decodifica y la retorna
    paquete = self.__recibir_respuesta(dir_esclavo)
    return self.__framer.leer_paquete_write_ans(paquete, num_endpoint)

  def agregar_esclavo(self, driver, num_endpoint):
    #Se asegura que no se agreguen esclavos con numeros de endpoint duplicados
    if num_endpoint in map(lambda x: x.num_endpoint, self.__lista_esclavos):
      raise ValueError("Numero de endpoint duplicado")

    #Agrega el esclavo a la lista
    esclavo = self.clase_vacia()
    esclavo.driver = driver
    esclavo.num_endpoint = num_endpoint
    self.__lista_esclavos.append(esclavo)

  def actualizar(self):
    #Se procesan todos los paquetes que lleguen
    self.__procesar_paquetes_llegada()

    #Llama el metodo de actualizacion para todos los endpoints
    for esclavo in self.__lista_esclavos:
      esclavo.driver.actualizar()

  def __recibir_respuesta(self, dir_esclavo):
    #Inicia el proceso de espera de respuesta, tomando el tiempo inicial
    t_ini = time.time()
    while True:
      #Verifica si hay un paquete presente
      if self.__mac.hay_paquete():
        #Hay paquete, se procede a recibirlo
        origen, paquete = self.__mac.recibir_paquete()

        #Se verifica que proceda del origen esperado
        if origen == dir_esclavo:
          #Si procede del origen correcto, 
          return paquete
      else:
        if time.time() - t_ini < self.__timeout:
          time.sleep(0.001)
        else:
          return []

  def __procesar_paquetes_llegada(self):
    #Verifica si hay un paquete recibido
    if not self.__mac.hay_paquete():
      return

    #Si lo hay, se toma el paquete de la capa MAC
    origen, paquete = self.__mac.recibir_paquete()

    #Verifica si se obtuvo un paquete valido
    if not paquete:
      return

    #Se descodifica el paquete de i32ctt con ayuda del framer
    comando, num_endpoint, payload = self.__framer.descodificar_paquete(paquete)

    #Verifica si se obtuvo un payload valido
    if not payload:
      return

    #Se determina que esclavo tiene el numero de endpoint que el paquete solicita
    esclavo = filter(lambda x: x.num_endpoint == num_endpoint, self.__lista_esclavos)

    #Verifica si se obtuvo un esclavo valido
    if not esclavo:
      return

    #Esclavo encontrado, se toma el primer elemento porque filter siempre retorna una lista
    esclavo = esclavo[0]

    #Actua en base al comando recibido
    if comando == self.__framer.read_cmd:
      #Comando de lectura, se llama la funcion de lectura del driver esclavo
      respuesta = esclavo.driver.callback_leer_registros(payload)

      #Se conforma un paquete de respuesta de lectura con los pares que retorna el callback
      paquete_resp = self.__framer.crear_paquete_read_ans(num_endpoint, respuesta)
    elif comando == self.__framer.write_cmd:
      #Comando de escritura, se llama la funcion de escritura del driver esclavo
      respuesta = esclavo.driver.callback_escr_registros(payload)

      #Se conforma un paquete de respuesta de escritura con las direcciones que retorna el callback
      paquete_resp = self.__framer.crear_paquete_write_ans(num_endpoint, respuesta)
    else:
      #Comando desconocido o no implementado
      return

    #Se envia la respuesta generada
    self.__mac.enviar_paquete(origen, paquete_resp)

  #TODO: Esta funcion es un feature de introspeccion, la cual aun no esta implementada
  #Esta funcion convierte una cadena de texto de 3 caracteres maximo a un ID numerico
  def __cadena_a_id_endpoint(self, cadena):
    id_endpoint = 0
    for i in range(len(cadena)):
      if i >= 3:
        break
      id_endpoint <<= 8
      id_endpoint |= ord(cadena[i])

    return id_endpoint
