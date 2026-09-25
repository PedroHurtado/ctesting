#define CUKE_OBJECT_PREFIX PasosTransferencia   // ← obligatorio si hay más de un fichero de pasos
#include <gtest/gtest.h>
#include <cucumber-cpp/autodetect.hpp>
#include <map>
#include <string>
#include "cuenta.h"

using cucumber::ScenarioScope;

struct Banco {
    std::map<std::string, Cuenta> cuentas;     // titular → cuenta
    std::string error;
};

GIVEN("^las siguientes cuentas:$") {
    TABLE_PARAM(tabla);
    ScenarioScope<Banco> banco;

    for (const auto& fila : tabla.hashes()) {
        Cuenta& cuenta = banco->cuentas[fila.at("titular")];
        const long long euros = std::stoll(fila.at("saldo"));
        if (euros > 0) cuenta.ingresar(Dinero::euros(euros));
    }
}

WHEN("^(\\w+) transfiere (\\d+) euros a (\\w+)$") {
    REGEX_PARAM(std::string, origen);
    REGEX_PARAM(long long, euros);
    REGEX_PARAM(std::string, destino);
    ScenarioScope<Banco> banco;

    try {
        transferir(banco->cuentas.at(origen), banco->cuentas.at(destino),
                   Dinero::euros(euros));
    } catch (const std::exception& e) {
        banco->error = e.what();
    }
}

THEN("^(\\w+) tiene (\\d+) euros$") {
    REGEX_PARAM(std::string, titular);
    REGEX_PARAM(long long, euros);
    ScenarioScope<Banco> banco;

    EXPECT_EQ(Dinero::euros(euros), banco->cuentas.at(titular).saldo());
}

THEN("^la transferencia se rechaza por \"([^\"]*)\"$") {
    REGEX_PARAM(std::string, motivo);
    ScenarioScope<Banco> banco;

    EXPECT_EQ(motivo, banco->error);
}
