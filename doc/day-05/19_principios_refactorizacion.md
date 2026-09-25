# Día 5 — Bloque 19: Principios y técnicas de refactorización

> *"Haz que el cambio sea fácil (aviso: puede ser difícil), y después haz el
> cambio fácil."* — Kent Beck

---

## 1. El problema en una frase

El bloque 17 refactorizó **una función**. Hoy toca cambiar **un tipo que usan
cien sitios**, y hacerlo sin que el proyecto deje de compilar ni un solo minuto.

## 2. Definición, y los dos sombreros

**Refactorizar** es cambiar la estructura interna del código **sin cambiar su
comportamiento observable** (Fowler). Ya lo vimos en el bloque 17.

Lo nuevo de hoy es cómo se trabaja. Kent Beck lo explica con **dos sombreros**:

```
   🎩 Sombrero de AÑADIR              🎩 Sombrero de REFACTORIZAR
   ─────────────────────              ────────────────────────────
   Escribo un test nuevo.             No escribo tests nuevos.
   Cambio el comportamiento.          NO cambio el comportamiento.
   El test pasa de rojo a verde.      Todos los tests siguen en verde.
```

**Nunca los dos a la vez.** Si refactorizas y añades algo en el mismo paso y
un test falla, no sabes cuál de los dos cambios tiene la culpa.

> En el ciclo TDD ya lo hacíais sin nombrarlo: **Rojo y Verde** son el sombrero
> de añadir. **Refactor** es el otro sombrero.

---

## 3. Cuándo refactorizar

| Momento | Qué es | Ejemplo |
|---|---|---|
| **Preparatorio** | Antes de añadir algo, dejo el código listo para que añadirlo sea fácil | Antes de meter la comisión, saco el `Dinero` |
| **De comprensión** | He tardado diez minutos en entender algo; lo renombro para que el siguiente tarde uno | `int t` → `int plazoDias` |
| **Del campamento** | Dejo el código un poco mejor de lo que lo encontré | Quito un `else` que sobra mientras paso |
| **Regla de tres** | La primera vez lo escribo; la segunda, duplico con asco; la tercera, extraigo | Tres `switch` iguales → bloque 20 |
| **En la revisión** | El revisor propone un refactor; se hace en el mismo PR, con los tests | — |

**Cuándo NO:**

- Código que funciona, que nadie toca y que no tiene tests.
- Justo antes de una entrega, si no hay red de tests.
- Cuando es más barato reescribirlo (código pequeño y aislado).

---

## 4. Cuatro principios de técnica

| # | Principio | Qué significa en la práctica |
|---|---|---|
| 1 | **Pasos pequeños** | Un paso = un refactor con nombre del catálogo. Tests. Siguiente |
| 2 | **Verde entre pasos** | Si un paso deja rojo, se deshace (`git checkout .`), no se "arregla" |
| 3 | **Cambio en paralelo** | Lo nuevo convive con lo viejo mientras se migra (§7) |
| 4 | **El compilador también es un test** | En C++, un buen tipo convierte un bug en un error de compilación (§6) |

---

## 5. Bad — obsesión por primitivos

La kata del día 2 acabó con el saldo en céntimos, en un `long long`. Y el
ejercicio del día 4 también. Funciona, pero mirad la interfaz:

```cpp
class Cuenta {
public:
    void ingresar(long long centimos);
    void retirar(long long centimos);
    long long saldo() const;          // ¿céntimos? ¿euros?
};

void transferir(Cuenta& origen, Cuenta& destino, long long centimos);
```

Y ahora el código de quien la usa:

```cpp
Cuenta c;
c.ingresar(100);                      // ← quería ingresar 100 €. Ha ingresado 1 €.
EXPECT_EQ(100, c.saldo());            // ← y el test PASA. Está mal y está verde.
```

Qué duele, concretamente:

- **El tipo no dice la unidad.** `100` puede ser euros o céntimos. El
  compilador no lo sabe, y el test tampoco.
- **Cualquier número vale.** Un `id` de cliente, una fecha o un número de
  página se pueden pasar como dinero sin un solo aviso.
- **Las reglas del dinero están repartidas.** Cómo se imprime, cómo se suma, si
  puede ser negativo... cada función lo decide por su cuenta.

> Es el olor **obsesión por primitivos** (*Primitive Obsession*) del bloque 17.
> El refactor es **Replace Primitive with Object**, y es el que quedó prometido
> en la kata del día 2.

---

## 6. Good — un tipo `Dinero`

```cpp
// dinero.h
#pragma once
#include <ostream>
#include <iomanip>

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
```

Tres decisiones, y cada una tiene un porqué:

| Decisión | Por qué |
|---|---|
| **Constructor privado + `euros()` / `centimos()`** | Para crear dinero **hay que decir la unidad**. No hay forma de no decirla |
| **Literales `_eur` y `_cent`** | `100_eur` se lee como en el enunciado del cliente |
| **`operator<<`** | GoogleTest lo usa para escribir el mensaje de fallo (bloque 14) |

Y `Cuenta` pasa a hablar en `Dinero`:

```cpp
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
    Dinero saldo() const { return saldo_; }
private:
    static void exigirPositivo(Dinero d) {
        if (!d.esPositivo()) throw std::invalid_argument{"importe no valido"};
    }
    Dinero saldo_ = 0_eur;
};
```

### El bug de §5, ahora

```cpp
Cuenta c;
c.ingresar(100);
```

Salida real de GCC 14:

```
err.cpp:2:34: error: cannot convert 'int' to 'Dinero'
    2 | int main(){ Cuenta c; c.ingresar(100); }
      |                                  ^~~
      |                                  |
      |                                  int
cuenta.h:7:26: note:   initializing argument 1 of 'void Cuenta::ingresar(Dinero)'
```

**El bug ya no llega al test: se para en el compilador.** Es el principio 4 de
§4: el tipo es un test que se ejecuta cada vez que compiláis, gratis.

### Los tests, más claros

```cpp
TEST(Dinero, DiezIngresosDeDiezCentimosSonUnEuro) {
    Cuenta c;
    for (int i = 0; i < 10; ++i) c.ingresar(10_cent);
    EXPECT_EQ(1_eur, c.saldo());                  // ← el 0.9999999 del día 2, ya imposible
}

TEST(Cuenta, NoSePuedeRetirarMasDeLoQueHay) {
    Cuenta c;
    c.ingresar(100_eur);
    EXPECT_THROW(c.retirar(100_eur + 1_cent), std::domain_error);   // ← la frontera, legible
    EXPECT_EQ(100_eur, c.saldo());
}
```

Y cuando uno falla, gracias al `operator<<` (salida real):

```
test_dinero.cpp:30: Failure
Expected equality of these values:
  12_eur + 50_cent
    Which is: 12,50 EUR
  c.saldo()
    Which is: 12,05 EUR
```

Sin `operator<<`, GoogleTest habría escrito `8-byte object <E2-04 00-00 ...>`.

---

## 7. La mecánica: cambio en paralelo (expandir → migrar → contraer)

Cambiar la firma de `ingresar` rompe **todas** las llamadas a la vez. En un
proyecto real eso son horas en rojo. La técnica para no estar nunca en rojo:

```
  1. EXPANDIR            2. MIGRAR                    3. CONTRAER
  ───────────            ─────────                    ───────────
  Añado la firma         Cambio las llamadas          Borro la firma
  nueva. La vieja        una a una.                   vieja.
  sigue y delega.        Tests tras cada una.
        │                      │                            │
     verde                  verde                        verde
```

**Paso 1 — expandir.** Las dos firmas conviven. La vieja solo delega, y se
marca con `[[deprecated]]` (C++14):

```cpp
class Cuenta {
public:
    void ingresar(Dinero importe) {                 // NUEVA
        if (!importe.esPositivo()) throw std::invalid_argument{"importe no valido"};
        centimos_ += importe.enCentimos();
    }

    [[deprecated("usa ingresar(Dinero)")]]          // VIEJA: sigue, pero ya solo delega
    void ingresar(long long centimos) { ingresar(Dinero::centimos(centimos)); }
    // ...
};
```

Todo compila y todos los tests pasan. Y el compilador os da **la lista de lo
que falta por migrar** (salida real):

```
expand.cpp:23:15: warning: 'void Cuenta::ingresar(long long int)' is deprecated:
                  usa ingresar(Dinero) [-Wdeprecated-declarations]
   23 |     c.ingresar(10000);
      |     ~~~~~~~~~~^~~~~~~
```

**Paso 2 — migrar.** Cada aviso es una tarea. Se cambia una llamada, se pasan
los tests, se hace commit. Se puede parar a mitad y volver mañana: el proyecto
está en verde todo el rato.

**Paso 3 — contraer.** Cuando ya no quedan avisos, se borra la firma vieja.

> Esta técnica se llama *Parallel Change* o *Expand–Contract*. Es la misma que
> se usa para cambiar una API pública o el esquema de una base de datos sin
> parar el servicio.

---

## 8. Cuando un refactor rompe un test

Si refactorizáis y un test se pone en rojo, hay dos posibilidades:

| El test falla porque... | Qué significa | Qué hacer |
|---|---|---|
| El comportamiento cambió | El refactor tiene un bug | Deshacer el paso |
| El test miraba **cómo** se hace, no **qué** se hace | Test acoplado a la implementación | Reescribir el test para que mire el comportamiento |

Ejemplo del segundo caso: un test que comprobaba `EXPECT_EQ(10000, c.saldo())`
se rompe al migrar `saldo()` a `Dinero`. No es un bug: el test sabía que por
dentro había un `long long`. En el bloque 12 lo llamamos **test frágil**.

> Por eso el `EXPECT_CALL` se reserva para lo que **es** el comportamiento
> (enviar un aviso, cobrar), y no para cada llamada interna (bloque 13).

---

## 9. Qué ganamos y qué pagamos

**Ganamos:**
- Un tipo que dice la unidad y que el compilador vigila.
- Mensajes de fallo que se leen (`12,05 EUR`).
- Una forma de hacer cambios grandes **sin estar nunca en rojo**.

**Pagamos:**
- Una clase más, con sus operadores.
- Un periodo en el que conviven dos firmas (y hay que acordarse de contraer).
- Disciplina: pasos pequeños aunque "se vea" el cambio entero.

## 10. Conexión con lo que viene

El bloque 20 aplica la misma disciplina a la parte del temario que falta:
**reutilizar y simplificar expresiones condicionales**. El bloque 21 repite el
proceso con un asistente de IA haciendo el paso mecánico.

## 11. Mantra del bloque

> **"Un sombrero cada vez. Y que el compilador trabaje para ti."**

---

## 12. Referencias

- 📖 Martin Fowler,
  **[*Refactoring*](https://martinfowler.com/books/refactoring.html)** (2ª ed.)
  — cap. 2 (los dos sombreros, cuándo refactorizar) y la ficha de
  *Replace Primitive with Object*. El libro de hoy.
- **[refactoring.com — *Replace Primitive with Object*](https://refactoring.com/catalog/replacePrimitiveWithObject.html)**
  — **gratuito**. La mecánica paso a paso del §6.
- **[Danilo Sato — *Parallel Change*](https://martinfowler.com/bliki/ParallelChange.html)**
  — **gratuito**, cinco minutos. El §7 explicado por quien le puso nombre.
- **[Kent Beck — *Make the change easy*](https://x.com/KentBeck/status/250733358307500032)**
  — la frase del principio, en su tuit original. El refactor preparatorio en una línea.
- **[cppreference — `[[deprecated]]`](https://en.cppreference.com/w/cpp/language/attributes/deprecated)**
  — **gratuito**. El atributo que convierte la migración del §7 en una lista de
  avisos del compilador.
- **[C++ Core Guidelines — I.4: interfaces precisas y fuertemente tipadas](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Ri-typed)**
  — **gratuito**. La misma idea que el `Dinero`, con la autoridad de Stroustrup y Sutter.
- 📖 Jeff Langr,
  **[*Modern C++ Programming with TDD*](https://pragprog.com/titles/lotdd/modern-c-programming-with-test-driven-development/)**,
  cap. 6 — refactorización incremental en C++ con la suite en verde entre pasos.
