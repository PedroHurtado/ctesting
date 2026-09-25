#define CUKE_OBJECT_PREFIX PasosCuenta          // ← obligatorio si hay más de un fichero de pasos
#include <gtest/gtest.h>
#include <cucumber-cpp/autodetect.hpp>
#include <string>
#include "cuenta.h"

using cucumber::ScenarioScope;

// El "mundo" del escenario: nace vacío en cada Escenario y muere al acabar.
struct Contexto {
    Cuenta      cuenta;
    std::string error;          // vacío = la última operación fue bien
};

GIVEN("^una cuenta con un saldo de (\\d+) euros$") {
    REGEX_PARAM(long long, euros);
    ScenarioScope<Contexto> ctx;

    if (euros > 0) ctx->cuenta.ingresar(Dinero::euros(euros));
}

WHEN("^retiro (-?\\d+) euros$") {
    REGEX_PARAM(long long, euros);
    ScenarioScope<Contexto> ctx;

    try {
        ctx->cuenta.retirar(Dinero::euros(euros));
    } catch (const std::exception& e) {
        ctx->error = e.what();                  // ← el Entonces decide si era lo esperado
    }
}

WHEN("^retiro (\\d+) euros en un cajero de otra entidad$") {
    REGEX_PARAM(long long, euros);
    ScenarioScope<Contexto> ctx;

    try {
        ctx->cuenta.retirarEnCajeroAjeno(Dinero::euros(euros));
    } catch (const std::exception& e) {
        ctx->error = e.what();
    }
}

THEN("^el saldo es de (\\d+) euros$") {
    REGEX_PARAM(long long, euros);
    ScenarioScope<Contexto> ctx;

    EXPECT_EQ(Dinero::euros(euros), ctx->cuenta.saldo());
}

THEN("^la operación se rechaza por \"([^\"]*)\"$") {
    REGEX_PARAM(std::string, motivo);
    ScenarioScope<Contexto> ctx;

    EXPECT_EQ(motivo, ctx->error);
}
