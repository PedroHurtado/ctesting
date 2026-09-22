# Día 2 — Bloque 10: Kata completa — `Cuenta` en Rojo, Verde, Refactor

> *"Nunca escribas una línea de código sin un test que falle."* — Kent Beck

---

## 1. El problema en una frase

Todo lo anterior se entiende leyéndolo; el ciclo TDD solo se aprende
**haciéndolo**, y con pasos lo bastante pequeños como para que incomoden.

## 2. El punto de partida: la lista de tests

La misma lista con la que cerramos el día 1. TDD empieza aquí, no en el teclado:

```
Cuenta bancaria
[ ] Cuenta nueva tiene saldo 0
[ ] Ingresar aumenta el saldo
[ ] Retirar disminuye el saldo
[ ] No permite retirar más del saldo
[ ] No permite ingresar importes negativos
[ ] Cuenta bloqueada rechaza cualquier operación
[ ] Retirada por encima del límite diario se rechaza
```

Están ordenados a propósito: **de lo trivial a lo interesante**. El primero
existe para arrancar el motor —crear el fichero, el `CMakeLists.txt`, ver el
primer verde—, no porque aporte valor.

El proyecto es el del bloque 6, con `cuenta_lib`, `include/cuenta.h`,
`src/cuenta.cpp` y `tests/cuenta_test.cpp`.

## 3. Ciclo 1 — Cuenta nueva tiene saldo 0

### Rojo

```cpp
// tests/cuenta_test.cpp
#include <gtest/gtest.h>
#include "cuenta.h"

TEST(Cuenta, UnaCuentaNuevaTieneSaldoCero) {
    Cuenta cuenta;
    EXPECT_DOUBLE_EQ(0.0, cuenta.saldo());
}
```

```
fatal error: cuenta.h: No such file or directory
```

**Eso ya es rojo.** No compilar *es* fallar (ley 2 de las tres leyes del día 1).

### Verde

```cpp
// include/cuenta.h
#pragma once

class Cuenta {
public:
    double saldo() const { return 0.0; }    // ← hardcodeado. Correcto en esta fase.
};
```

```
[  PASSED  ] 1 test.
```

Sí, `return 0.0;` está cableado. Es *fake it till you make it*: la garantía de
que cada línea de producción nace de un test. El siguiente test la obligará a
generalizarse.

### Refactor

Nada que limpiar todavía. Se tacha el caso y se sigue.

## 4. Ciclo 2 — Ingresar aumenta el saldo

### Rojo

```cpp
TEST(Cuenta, IngresarAumentaElSaldo) {
    Cuenta cuenta;

    cuenta.ingresar(50.0);

    EXPECT_DOUBLE_EQ(50.0, cuenta.saldo());
}
```

Falla al compilar: `ingresar` no existe.

### Verde

```cpp
class Cuenta {
public:
    void   ingresar(double importe) { saldo_ += importe; }
    double saldo() const            { return saldo_; }
private:
    double saldo_ = 0.0;            // ← el cableado del ciclo 1 desaparece solo
};
```

Los **dos** tests en verde. Fíjate en lo que acaba de pasar: el segundo test
forzó la generalización del primero. Eso es **triangulación**.

## 5. Ciclo 3 — Retirar disminuye el saldo

### Rojo

```cpp
TEST(Cuenta, RetirarDisminuyeElSaldo) {
    Cuenta cuenta;
    cuenta.ingresar(100.0);

    cuenta.retirar(30.0);

    EXPECT_DOUBLE_EQ(70.0, cuenta.saldo());
}
```

### Verde

```cpp
void retirar(double importe) { saldo_ -= importe; }
```

### Refactor: el primero de verdad, y es en los tests

Tres tests, y en dos de ellos ya se repite `Cuenta cuenta; cuenta.ingresar(...)`.
Es el momento del fixture del bloque 8 —**ahora**, cuando la duplicación ya
existe, no antes por si acaso:

```cpp
class CuentaTest : public ::testing::Test {
protected:
    Cuenta cuentaCon(double saldoInicial) {
        Cuenta c;
        c.ingresar(saldoInicial);
        return c;
    }
    Cuenta cuenta;                     // instancia nueva en CADA TEST_F
};

TEST_F(CuentaTest, UnaCuentaNuevaTieneSaldoCero) {
    EXPECT_DOUBLE_EQ(0.0, cuenta.saldo());
}

TEST_F(CuentaTest, IngresarAumentaElSaldo) {
    cuenta.ingresar(50.0);
    EXPECT_DOUBLE_EQ(50.0, cuenta.saldo());
}

TEST_F(CuentaTest, RetirarDisminuyeElSaldo) {
    auto cuenta = cuentaCon(100.0);
    cuenta.retirar(30.0);
    EXPECT_DOUBLE_EQ(70.0, cuenta.saldo());
}
```

**El código de test se refactoriza igual que el de producción.** Es la mitad de
la base de código y la que más se lee.

## 6. Ciclo 4 — No permite retirar más del saldo

El primer test que expresa una **regla de negocio**. Y la primera decisión de
diseño: ¿qué hace la clase cuando no puede?

Tres opciones, y el test es quien la elige: devolver `bool`, devolver
`std::optional`, o lanzar. Aquí lanzamos: es un error de uso, no un resultado
esperado.

### Rojo

```cpp
TEST_F(CuentaTest, NoPermiteRetirarMasDelSaldoDisponible) {
    auto cuenta = cuentaCon(100.0);

    EXPECT_THROW(cuenta.retirar(150.0), SaldoInsuficiente);
}

TEST_F(CuentaTest, UnaRetiradaRechazadaNoAlteraElSaldo) {
    auto cuenta = cuentaCon(100.0);

    EXPECT_THROW(cuenta.retirar(150.0), SaldoInsuficiente);

    EXPECT_DOUBLE_EQ(100.0, cuenta.saldo());   // ← la postcondición, que se olvida siempre
}
```

Dos tests, no uno. El primero comprueba que **avisa**; el segundo, que **no
hace nada a medias**. Un `retirar` que lanza *después* de haber restado el saldo
pasaría el primero y fallaría el segundo.

### Verde

```cpp
#include <stdexcept>

class SaldoInsuficiente : public std::runtime_error {
public:
    SaldoInsuficiente() : std::runtime_error{"saldo insuficiente"} {}
};

void retirar(double importe) {
    if (importe > saldo_) throw SaldoInsuficiente{};
    saldo_ -= importe;
}
```

### Y aquí entra el bloque 9

`importe > saldo_` es una **frontera**. Tres casos, no uno:

```cpp
TEST_F(CuentaTest, PermiteRetirarExactamenteTodoElSaldo) {
    auto cuenta = cuentaCon(100.0);

    cuenta.retirar(100.0);                       // ← el caso que caza el >= por >

    EXPECT_DOUBLE_EQ(0.0, cuenta.saldo());
}
```

Si alguien hubiera escrito `importe >= saldo_`, los tests anteriores seguirían
verdes y este lo destaparía. **Ese es el test que vale por diez.**

## 7. Ciclo 5 — No permite importes negativos

### Rojo

```cpp
TEST_F(CuentaTest, NoPermiteIngresarImportesNegativos) {
    EXPECT_THROW(cuenta.ingresar(-10.0), ImporteInvalido);
}

TEST_F(CuentaTest, NoPermiteRetirarImportesNegativos) {
    auto cuenta = cuentaCon(100.0);
    EXPECT_THROW(cuenta.retirar(-10.0), ImporteInvalido);   // ← sin esto, retirar(-10)
}                                                           //   INGRESA 10 euros
```

Ese segundo test es un agujero de seguridad real, y sale de recorrer la **R de
*Range*** de CORRECT (bloque 9), no de la especificación.

¿Y el cero? Es otra frontera, y **la especificación no dice nada**. En TDD esto
no se resuelve adivinando: se pregunta al cliente (el *Cliente en el equipo* de
XP, día 1). Supongamos que responde "ingresar 0 no es un error, simplemente no
hace nada":

```cpp
TEST_F(CuentaTest, IngresarCeroNoEsUnErrorYDejaElSaldoIgual) {
    auto cuenta = cuentaCon(100.0);
    EXPECT_NO_THROW(cuenta.ingresar(0.0));
    EXPECT_DOUBLE_EQ(100.0, cuenta.saldo());
}
```

**El test acaba de convertir una ambigüedad del requisito en una decisión
documentada y ejecutable.** Ese es el valor de TDD que no aparece en la
cobertura.

### Verde

```cpp
void ingresar(double importe) {
    if (importe < 0.0) throw ImporteInvalido{};
    saldo_ += importe;
}

void retirar(double importe) {
    if (importe < 0.0)    throw ImporteInvalido{};
    if (importe > saldo_) throw SaldoInsuficiente{};
    saldo_ -= importe;
}
```

### Refactor: extraer la validación

```cpp
class Cuenta {
public:
    void ingresar(double importe) {
        exigirImporteValido(importe);
        saldo_ += importe;
    }

    void retirar(double importe) {
        exigirImporteValido(importe);
        if (importe > saldo_) throw SaldoInsuficiente{};
        saldo_ -= importe;
    }

private:
    static void exigirImporteValido(double importe) {
        if (importe < 0.0) throw ImporteInvalido{};
    }

    double saldo_ = 0.0;
};
```

Los ocho tests siguen verdes. Duración del refactor: veinte segundos, cero
dudas. **Eso** es lo que compra la suite.

## 8. Ciclo 6 — Cuenta bloqueada rechaza cualquier operación

### Rojo

```cpp
TEST_F(CuentaTest, UnaCuentaBloqueadaRechazaLosIngresos) {
    cuenta.bloquear();
    EXPECT_THROW(cuenta.ingresar(10.0), CuentaBloqueada);
}

TEST_F(CuentaTest, UnaCuentaBloqueadaRechazaLasRetiradas) {
    auto cuenta = cuentaCon(100.0);
    cuenta.bloquear();
    EXPECT_THROW(cuenta.retirar(10.0), CuentaBloqueada);
}

TEST_F(CuentaTest, UnaCuentaBloqueadaSigueMostrandoElSaldo) {
    auto cuenta = cuentaCon(100.0);
    cuenta.bloquear();
    EXPECT_DOUBLE_EQ(100.0, cuenta.saldo());    // ← consultar NO es operar
}
```

El tercero delimita el alcance de "cualquier operación". Sin él, "cualquiera"
es ambiguo y el siguiente que toque la clase lo interpretará a su manera.

### Verde y refactor

```cpp
void ingresar(double importe) {
    exigirOperativa();
    exigirImporteValido(importe);
    saldo_ += importe;
}

void retirar(double importe) {
    exigirOperativa();
    exigirImporteValido(importe);
    if (importe > saldo_) throw SaldoInsuficiente{};
    saldo_ -= importe;
}

void bloquear() { bloqueada_ = true; }

private:
    void exigirOperativa() const {
        if (bloqueada_) throw CuentaBloqueada{};
    }
    bool bloqueada_ = false;
```

Nótese el **orden** de las guardas: bloqueada primero. Es una decisión de
negocio (¿qué error se reporta si la cuenta está bloqueada *y* el importe es
negativo?) y, como toda decisión, merece su test:

```cpp
TEST_F(CuentaTest, ElBloqueoTienePrioridadSobreLaValidacionDelImporte) {
    cuenta.bloquear();
    EXPECT_THROW(cuenta.ingresar(-10.0), CuentaBloqueada);
}
```

## 9. El refactor grande: adiós a los `double`

Once tests en verde. Ahora se puede hacer algo que sin ellos daría miedo.

El problema del bloque 7, ahora en el dominio:

```cpp
TEST_F(CuentaTest, LosCentimosNoSePierdenAlAcumularIngresos) {
    for (int i = 0; i < 10; ++i) cuenta.ingresar(0.1);

    EXPECT_DOUBLE_EQ(1.0, cuenta.saldo());   // ← FALLA: 0.9999999999999999
}
```

Rojo. Y no es un bug de la validación ni del bloqueo: es que **`double` es el
tipo equivocado para dinero**.

Refactor: la representación interna pasa a céntimos, la interfaz pública no
cambia. Los once tests anteriores son la red que garantiza que el
comportamiento se conserva.

```cpp
class Cuenta {
public:
    void ingresar(double importe) {
        exigirOperativa();
        exigirImporteValido(importe);
        centimos_ += aCentimos(importe);
    }

    void retirar(double importe) {
        exigirOperativa();
        exigirImporteValido(importe);
        const long long c = aCentimos(importe);
        if (c > centimos_) throw SaldoInsuficiente{};
        centimos_ -= c;
    }

    double saldo() const { return centimos_ / 100.0; }

private:
    static long long aCentimos(double importe) {
        return std::llround(importe * 100.0);       // ← redondeo explícito, no truncado
    }

    long long centimos_ = 0;        // ← entero: la aritmética vuelve a ser exacta
    bool      bloqueada_ = false;
};
```

Doce tests en verde, incluido el de los céntimos. Se ha cambiado el tipo del
estado interno de una clase de dominio **sin un instante de duda**, porque el
significado de "sigue funcionando" estaba escrito y era ejecutable.

> El paso siguiente natural sería un tipo `Dinero` propio y que la interfaz
> dejara de hablar en `double`. Es un buen ejercicio, y el día 4 lo veremos
> como refactorización *Replace Primitive with Object*.

## 10. El séptimo caso, y por qué se queda para mañana

```
[ ] Retirada por encima del límite diario se rechaza
```

"Diario" implica **fecha**, y la fecha implica reloj:

```cpp
void retirar(double importe) {
    auto hoy = std::chrono::system_clock::now();    // ← ¿test? el reloj no se puede fijar
    // ...
}
```

Este test sería **lento**, **no repetible** y dependería del día en que se
ejecute: rompe la I, la R y la F de FIRST. La solución es exactamente la del
bloque 3 del día 1: **una costura**.

```cpp
class Reloj {                                  // la costura
public:
    virtual ~Reloj() = default;
    virtual std::chrono::system_clock::time_point ahora() const = 0;
};

class Cuenta {
public:
    explicit Cuenta(const Reloj& reloj) : reloj_{reloj} {}   // inyección
    // ...
private:
    const Reloj& reloj_;
};
```

En el test se inyecta un reloj falso que devuelve la fecha que nos convenga.
**Eso es un doble de prueba, y es el bloque con el que abre mañana.** Lo
dejamos escrito en la lista, sin implementar: es el puente al día 3.

## 11. Los errores típicos de la kata

Los que salen siempre, y conviene reconocerlos en el momento:

| Error | Síntoma | Corrección |
|---|---|---|
| **Saltarse el rojo** | "Ya sé que va a fallar" | No lo sabes: la mitad de las veces el test no ejecuta lo que crees |
| **Pasos demasiado grandes** | 15 minutos en rojo, 40 líneas escritas | `git checkout .` y partirlo en tres |
| **Implementar de más** | Código para casos que ningún test pide | Es código sin especificar: sobra |
| **Refactorizar en rojo** | Retocar el diseño mientras algo falla | Verde primero. Siempre |
| **Añadir función en el refactor** | "Ya que estoy…" | A la lista. Y se vuelve a rojo |
| **Tests que dependen del orden** | Falla con `--gtest_shuffle` | Fixture limpio, nada de estado compartido |
| **No comprobar la postcondición** | El error se lanza, pero el estado quedó a medias | Un `EXPECT` del estado tras cada excepción |

## 12. Qué ganamos y qué pagamos

**Ganamos:** doce tests que son la especificación viva de `Cuenta`, dos
decisiones de negocio ambiguas resueltas y documentadas, un cambio de
representación interna hecho sin miedo, y una costura identificada para mañana.

**Pagamos:** más tiempo del que habría costado escribir la clase de un tirón.
La cuenta sale al primer cambio de requisito —y a la clase `Cuenta` le van a
cambiar los requisitos.

## 13. Ejercicio propuesto

Sobre el mismo proyecto, añadid a la lista y resolvedlo con el ciclo completo:

```
[ ] Transferir mueve saldo de una cuenta a otra
[ ] Una transferencia fallida no altera NINGUNA de las dos cuentas
[ ] historial() devuelve los movimientos en orden cronológico
[ ] Una cuenta nueva tiene el historial vacío
```

Tres pistas, ninguna gratuita:

1. Empezad por el más trivial (`historial` vacío). El motor arranca solo.
2. Para el historial usad `EXPECT_THAT(..., ElementsAre(...))` del bloque 7,
   no un bucle con `EXPECT_EQ`.
3. La transferencia fallida es la **atomicidad** del ciclo 4 aplicada a dos
   objetos: escribid **primero** el test que comprueba los dos saldos.

## 14. Mantra del bloque

> **"Un test en rojo, el código mínimo, y limpiar. Nada más, nada menos, en ese
> orden."**
> El rojo especifica. El verde implementa. El refactor es donde está el valor —y
> solo es posible gracias a los dos anteriores.

---

## 15. Referencias

**La kata, hecha por sus autores:**

- 📖 Kent Beck, **[*Test-Driven Development: By Example*](https://www.informit.com/store/test-driven-development-by-example-9780321146533)**
  — la parte I es esta misma kata (dinero multidivisa) ciclo a ciclo, incluido
  el refactor del apartado 9. Sesenta páginas y se lee en una tarde.
- Kent Beck, **[*Canon TDD*](https://tidyfirst.substack.com/p/canon-tdd)**
  — **gratuito**, dos páginas. El ciclo descrito sin las deformaciones
  habituales, por el autor, veinte años después.
- James Shore, **[*The Art of Agile Development* — capítulo de TDD](https://www.jamesshore.com/v2/books/aoad2/test-driven_development)**
  — **gratuito**. Un ciclo completo narrado con los errores del apartado 11
  señalados sobre la marcha.

**En C++:**

- 📖 Jeff Langr, **[*Modern C++ Programming with TDD*](https://pragprog.com/titles/lotdd/modern-c-programming-with-test-driven-development/)**,
  caps. 4-5 — una kata larga en C++ con GoogleTest, con las decisiones de
  diseño (excepciones frente a códigos de error, valores frente a primitivas)
  discutidas según van apareciendo.
- 📖 Michael Feathers, **[*Working Effectively with Legacy Code*](https://www.informit.com/store/working-effectively-with-legacy-code-9780131177055)**
  — la costura del `Reloj` del apartado 10 es su técnica de *Extract Interface*.
  El libro para cuando la clase ya existe y no tiene tests.

**Práctica deliberada:**

- **[GoogleTest — Assertions Reference](https://google.github.io/googletest/reference/assertions.html)**
  — **gratuito**. Tenedla abierta durante la kata: `EXPECT_THROW`,
  `EXPECT_DOUBLE_EQ` y `EXPECT_THAT` se usan en cada ciclo.

---

## Mañana

Los **dobles de prueba con gMock**: el `Reloj` del apartado 10 generado
automáticamente, mocks frente a fakes frente a stubs, y qué comprobar con cada
uno. Después, **tests paramétricos** (`TEST_P`) para las tablas del bloque 9,
**cobertura** real con `gcov`/`gcovr`, **sanitizers**, y refactorización guiada
por *code smells* con la IA como apoyo.
