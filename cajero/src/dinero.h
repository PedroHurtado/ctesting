#pragma once
#include <ostream>
#include <iomanip>
#include <stdexcept>

class Dinero {
public:
    static Dinero centimos(long long c) { return Dinero{c}; }
    static Dinero euros(long long e)    { return Dinero{e * 100}; }

    long long enCentimos() const { return centimos_; }
    bool esPositivo()      const { return centimos_ > 0; }

    Dinero operator+(Dinero otro) const { return Dinero{centimos_ + otro.centimos_}; }
    Dinero operator-(Dinero otro) const { return Dinero{centimos_ - otro.centimos_}; }

    bool operator==(Dinero o) const { return centimos_ == o.centimos_; }
    bool operator!=(Dinero o) const { return !(*this == o); }
    bool operator< (Dinero o) const { return centimos_ <  o.centimos_; }
    bool operator> (Dinero o) const { return o < *this; }

    friend std::ostream& operator<<(std::ostream& os, Dinero d) {
        return os << d.centimos_ / 100 << ','
                  << std::setw(2) << std::setfill('0') << d.centimos_ % 100 << " EUR";
    }

private:
    explicit Dinero(long long c) : centimos_{c} {}   // ← explicit: un número suelto NO es dinero
    long long centimos_;
};

inline Dinero operator""_eur(unsigned long long e)  { return Dinero::euros(static_cast<long long>(e)); }
inline Dinero operator""_cent(unsigned long long c) { return Dinero::centimos(static_cast<long long>(c)); }
