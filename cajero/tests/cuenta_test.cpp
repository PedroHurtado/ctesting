#include <gtest/gtest.h>
#include "cuenta.h"

// ── Cuenta ──

TEST(Cuenta, IngresarAumentaElSaldo) {
    Cuenta c;

    c.ingresar(100_eur);

    EXPECT_EQ(100_eur, c.saldo());
}

TEST(Cuenta, NoSePuedeRetirarMasDeLoQueHay) {
    Cuenta c;
    c.ingresar(100_eur);

    EXPECT_THROW(c.retirar(100_eur + 1_cent), std::domain_error);

    EXPECT_EQ(100_eur, c.saldo());
}

TEST(Cuenta, SePuedeRetirarTodoElSaldo) {
    Cuenta c;
    c.ingresar(100_eur);

    c.retirar(100_eur);

    EXPECT_EQ(0_eur, c.saldo());
}

// ── Comisión en cajero ajeno (bloque 24, §7) ──

TEST(CajeroAjeno, CobraDosEurosDeComision) {
    Cuenta c;
    c.ingresar(100_eur);

    c.retirarEnCajeroAjeno(30_eur);

    EXPECT_EQ(68_eur, c.saldo());
}

TEST(CajeroAjeno, SiNoLlegaParaLaComisionNoRetiraNada) {
    Cuenta c;
    c.ingresar(100_eur);

    EXPECT_THROW(c.retirarEnCajeroAjeno(99_eur), std::domain_error);

    EXPECT_EQ(100_eur, c.saldo());
}

// ── Transferencias ──

TEST(Transferencia, MueveElSaldoEntreLasDosCuentas) {
    Cuenta origen, destino;
    origen.ingresar(100_eur);

    transferir(origen, destino, 40_eur);

    EXPECT_EQ(60_eur, origen.saldo());
    EXPECT_EQ(40_eur, destino.saldo());
}

TEST(Transferencia, SiFallaNoAlteraNingunaDeLasDosCuentas) {
    Cuenta origen, destino;
    origen.ingresar(100_eur);

    EXPECT_THROW(transferir(origen, destino, 500_eur), std::domain_error);

    EXPECT_EQ(100_eur, origen.saldo());
    EXPECT_EQ(0_eur,   destino.saldo());
}

TEST(Transferencia, NoSePuedeTransferirALaMismaCuenta) {
    Cuenta c;
    c.ingresar(100_eur);

    EXPECT_THROW(transferir(c, c, 10_eur), std::invalid_argument);

    EXPECT_EQ(100_eur, c.saldo());
}
