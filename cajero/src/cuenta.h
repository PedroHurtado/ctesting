#pragma once
#include "dinero.h"
#include <stdexcept>

class Cuenta {
public:
    void ingresar(Dinero importe) {
        exigirPositivo(importe);
        saldo_ = saldo_ + importe;
    }
    void retirar(Dinero importe) {
        exigirPositivo(importe);
        if (importe > saldo_) throw std::domain_error{"saldo insuficiente"};
        saldo_ = saldo_ - importe;
    }
    void retirarEnCajeroAjeno(Dinero importe) {
        retirar(importe + comisionCajeroAjeno); // ← una sola operación: o todo o nada
    }
    Dinero saldo() const { return saldo_; }
    static inline const Dinero comisionCajeroAjeno = 2_eur;
private:
    static void exigirPositivo(Dinero d) {
        if (!d.esPositivo()) throw std::invalid_argument{"importe no valido"};
    }
    Dinero saldo_ = 0_eur;
};

inline void transferir(Cuenta& origen, Cuenta& destino, Dinero importe) {
    if (&origen == &destino) throw std::invalid_argument{"misma cuenta"};
    origen.retirar(importe);        // si lanza, destino no se ha tocado
    destino.ingresar(importe);
}
