# language: es
Característica: Retirar efectivo
  Como titular de una cuenta
  Quiero retirar efectivo de mi cuenta
  Para disponer de mi dinero sin pasar por la oficina

  Regla: No se puede retirar más de lo que hay

    Escenario: Retirada con saldo suficiente
      Dado una cuenta con un saldo de 100 euros
      Cuando retiro 30 euros
      Entonces el saldo es de 70 euros

    Escenario: Se puede retirar todo el saldo
      Dado una cuenta con un saldo de 100 euros
      Cuando retiro 100 euros
      Entonces el saldo es de 0 euros

    Escenario: Retirada sin saldo suficiente
      Dado una cuenta con un saldo de 100 euros
      Cuando retiro 101 euros
      Entonces la operación se rechaza por "saldo insuficiente"
      Y el saldo es de 100 euros

  Regla: Solo se pueden retirar importes positivos

    Esquema del escenario: Importe no válido
      Dado una cuenta con un saldo de 100 euros
      Cuando retiro <importe> euros
      Entonces la operación se rechaza por "importe no valido"
      Y el saldo es de 100 euros

      Ejemplos:
        | importe |
        | 0       |
        | -50     |
