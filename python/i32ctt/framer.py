#Framer para el protocolo I32CTT

class framer_i32ctt:
  read_cmd    = 0x01
  __read_ans  = 0x02
  write_cmd   = 0x03
  __write_ans = 0x04
  list_cmd    = 0x05
  __list_ans  = 0x06
  find_cmd    = 0x07
  __find_ans  = 0x08

  def __init__(self, len_mtu_mac):
    self.len_mtu_mac = len_mtu_mac

  def leer_len_mtu(self, comando):
    #La longitud de la maxima unidad de transferencia (numero de elementos mas grande soportado como
    #payload del paquete) se determina segun el comando que contiene el paquete
    if comando == self.read_cmd or comando == self.write_cmd:
      #Nota: Si bien el comando "READ" ocupa menos espacio que el comando "WRITE", su respuesta
      #asociada ("READ ANS") ocupa el mismo espacio para la misma cantidad de registros, por lo que
      #se calcula el maximo numero de registros de la misma forma.
      return (self.len_mtu_mac - 2) // 6
    #TODO: Implementar el calculo de longitud para los comandos de introspeccion
    else:
      return 0

  def crear_paquete_read(self, num_endpoint, dir_registros):
    #Inicia el paquete con la cabecera, que contiene el comando y el numero de endpoint
    paquete = [self.read_cmd, num_endpoint & 0xFF]

    #Anexa la direccion al paquete, la cual es de 16 bits
    for i in dir_registros:
      paquete.extend(self.__descomponer_u16(i))

    return paquete

  def crear_paquete_write(self, num_endpoint, par_registros):
    #Inicia el paquete con la cabecera, que contiene el comando y el numero de endpoint
    paquete = [self.write_cmd, num_endpoint & 0xFF]

    for i in par_registros:
      #Verifica que cada pareja sea una tupla o lista con exactamente 2 elementos
      if len(i) != 2:
        raise ValueError('Tupla/lista mal formada')

      #Anexa la pareja al paquete, la direccion es de 16 bits y los datos de 32
      paquete.extend(self.__descomponer_u16(i[0]))
      paquete.extend(self.__descomponer_u32(i[1]))

    return paquete

  def crear_paquete_read_ans(self, num_endpoint, par_registros):
    #Inicia el paquete con la cabecera, que contiene la respuesta y el numero de endpoint
    paquete = [self.__read_ans, num_endpoint & 0xFF]

    for i in par_registros:
      #Verifica que cada pareja sea una tupla o lista con exactamente 2 elementos
      if len(i) != 2:
        raise ValueError('Tupla/lista mal formada')

      #Anexa la pareja al paquete, la direccion es de 16 bits y los datos de 32
      paquete.extend(self.__descomponer_u16(i[0]))
      paquete.extend(self.__descomponer_u32(i[1]))

    return paquete    

  def crear_paquete_write_ans(self, num_endpoint, dir_registros):
    #Inicia el paquete con la cabecera, que contiene la respuesta y el numero de endpoint
    paquete = [self.__write_ans, num_endpoint & 0xFF]

    #Anexa la direccion al paquete, la cual es de 16 bits
    for i in dir_registros:
      paquete.extend(self.__descomponer_u16(i))

    return paquete

  def leer_paquete_read_ans(self, paquete, num_endpoint):
    #Se descartan los paquetes demasiado cortos
    if len(paquete) < 2:
      return []

    #Se verifica que el tipo de paquete sea "READ ANS"
    if paquete[0] != self.__read_ans:
      return []

    #Se verifica que el numero de endpoint en el paquete sea el correcto
    if num_endpoint != paquete[1]:
      return []

    #Se extraen y retornan los pares de direccion/dato del paquete
    return self.__leer_dir_datos(paquete)

  def leer_paquete_write_ans(self, paquete, num_endpoint):
    #Se descartan los paquetes demasiado cortos
    if len(paquete) < 2:
      return []

    #Se verifica que el tipo de paquete sea "WRITE ANS"
    if paquete[0] != self.__write_ans:
      return []

    #Se verifica que el numero de endpoint en el paquete sea el correcto
    if num_endpoint != paquete[1]:
      return []

    #Se extraen y retornan las direcciones del paquete
    return self.__leer_direcciones(paquete)

  def descodificar_paquete(self, paquete):
    #Se descartan los paquetes demasiado cortos
    if len(paquete) < 2:
      return None, None, []

    #Se extraen los campos basicos del paquete
    comando = paquete[0]
    num_endpoint = paquete[1]

    #Se retorna el comando, numero de endpoint y contenido del paquete descodificado segun el
    #comando que contiene
    if comando == self.read_cmd:
      return comando, num_endpoint, self.__leer_direcciones(paquete)
    elif comando == self.write_cmd:
      return comando, num_endpoint, self.__leer_dir_datos(paquete)
    else:
      return None, None, []

  def __leer_dir_datos(self, paquete):
    #Se recorta el paquete, luego se determina que su nueva longitud sea consistente
    paquete = paquete[2:]
    if len(paquete) % 6 != 0:
      return []

    #Se extraen los pares de direccion/dato del paquete
    datos = []
    p = 0
    while p < len(paquete):
      direccion = self.__ensamblar_u16(paquete[p: p + 2])
      p += 2
      dato = self.__ensamblar_u32(paquete[p: p + 4])
      p += 4
      datos.append((direccion, dato))

    #Se retornan los pares extraidos
    return datos

  def __leer_direcciones(self, paquete):
    #Se recorta el paquete, luego se determina que su nueva longitud sea consistente
    paquete = paquete[2:]
    if len(paquete) % 2 != 0:
      return []

    #Se extraen las direcciones del paquete
    datos = []
    p = 0
    while p < len(paquete):
      direccion = self.__ensamblar_u16(paquete[p: p + 2])
      p += 2
      datos.append(direccion)

    #Se retornan las direcciones extraidas
    return datos

  #Ensambla un entero de 16 bits con 2 octetos consecutivos
  def __ensamblar_u16(self, octetos):
    return octetos[0] | (octetos[1] << 8)

  #Ensambla un entero de 32 bits con 4 octetos consecutivos
  def __ensamblar_u32(self, octetos):
    return octetos[0] | (octetos[1] << 8) | (octetos[2] << 16)  | (octetos[3] << 24)

  #Descompone un entero de 16 bits en una lista con 2 octetos
  def __descomponer_u16(self, entero):
    return [entero & 0xFF, (entero >> 8) & 0xFF]

  #Descompone un entero de 32 bits en una lista con 4 octetos
  def __descomponer_u32(self, entero):
    return [entero & 0xFF, (entero >> 8) & 0xFF, (entero >> 16) & 0xFF, (entero >> 24) & 0xFF]
