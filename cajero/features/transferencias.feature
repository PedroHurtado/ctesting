# language: es
Característica: Transferencias entre cuentas
  Como titular de una cuenta
  Quiero transferir dinero a otra cuenta
  Para pagar a otras personas sin usar efectivo

  Antecedentes:
    Dadas las siguientes cuentas:
      | titular | saldo |
      | Ana     | 100   |
      | Luis    | 0     |

  Escenario: Transferencia con saldo suficiente
    Cuando Ana transfiere 40 euros a Luis
    Entonces Ana tiene 60 euros
    Y Luis tiene 40 euros

  Escenario: Una transferencia sin fondos no toca ninguna cuenta
    Cuando Ana transfiere 500 euros a Luis
    Entonces la transferencia se rechaza por "saldo insuficiente"
    Y Ana tiene 100 euros
    Y Luis tiene 0 euros
