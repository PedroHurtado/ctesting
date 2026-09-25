#include <gtest/gtest.h>
#include <sstream>
#include "cuenta.h"

TEST(Dinero, EurosYCentimosSonLaMismaCantidad) {
    EXPECT_EQ(Dinero::centimos(1250), 12_eur + 50_cent);
}

TEST(Dinero, DiezIngresosDeDiezCentimosSonUnEuro) {
    Cuenta c;
    for (int i = 0; i < 10; ++i) c.ingresar(10_cent);
    EXPECT_EQ(1_eur, c.saldo());
}

TEST(Dinero, SeImprimeConDosDecimales) {
    std::ostringstream os;
    os << 12_eur + 5_cent;
    EXPECT_EQ("12,05 EUR", os.str());
}
