# language: es
Característica: Comisión en cajeros de otra entidad
  Como responsable de productos del banco
  Quiero cobrar 2 euros por cada retirada en un cajero de otra entidad
  Para cubrir lo que esa entidad nos cobra a nosotros

  Escenario: La comisión se suma al importe retirado
    Dado una cuenta con un saldo de 100 euros
    Cuando retiro 30 euros en un cajero de otra entidad
    Entonces el saldo es de 68 euros

  Escenario: La comisión también cuenta para el saldo disponible
    Dado una cuenta con un saldo de 100 euros
    Cuando retiro 99 euros en un cajero de otra entidad
    Entonces la operación se rechaza por "saldo insuficiente"
    Y el saldo es de 100 euros
