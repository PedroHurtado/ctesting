# Día 1 — Bloque 5: El ciclo TDD (Rojo → Verde → Refactor)

> *"Nunca escribas una línea de código sin un test que falle."* — Kent Beck

---

## 1. El problema en una frase

Escribir primero el código y después el test **cambia lo que puedes descubrir**:
el test ya nace condicionado por la implementación que acabas de escribir.

## 2. Las tres leyes de TDD (Robert C. Martin)

1. No escribirás código de producción **salvo para hacer pasar un test que
   falla**.
2. No escribirás **más test** del necesario para fallar (no compilar **es**
   fallar).
3. No escribirás **más código de producción** del necesario para pasar ese test.

El resultado es un ciclo de **entre 30 segundos y 5 minutos**. Si llevas 20
minutos en rojo, has dado un paso demasiado grande: vuelve atrás y divídelo.

## 3. El ciclo

```
        ┌──────────────────────────────────────┐
        │                                      │
        ▼                                      │
   ┌─────────┐      ┌─────────┐      ┌──────────────┐
   │  ROJO   │ ───► │  VERDE  │ ───► │  REFACTOR    │
   │ escribe │      │ hazlo   │      │ mejora el    │
   │ un test │      │ pasar   │      │ diseño       │
   │ que falla│     │ ya      │      │ (sigue verde)│
   └─────────┘      └─────────┘      └──────────────┘
```

| Fase         | Objetivo                          | Qué está permitido                                |
|--------------|-----------------------------------|---------------------------------------------------|
| **Rojo**     | Definir el comportamiento deseado | Escribir test. **Nada** de código de producción.  |
| **Verde**    | Que pase, cuanto antes            | Lo mínimo. Incluso algo "feo" o hardcodeado.      |
| **Refactor** | Limpiar                           | Cambiar estructura. **Ningún** comportamiento nuevo.|

Las dos reglas que nunca se rompen:

- **En rojo no se refactoriza.** Refactorizar sobre código roto es
  depurar a ciegas.
- **En refactor no se añade funcionalidad.** Si aparece una idea nueva,
  se apunta en la lista y se vuelve a Rojo.

## 4. Por qué el test tiene que fallar primero

Es la parte que más se salta la gente, y la más importante. Ver el rojo prueba
tres cosas de golpe:

1. Que el test **realmente ejecuta** lo que crees (no un `TEST` mal nombrado,
   ni un fichero que no está en el `CMakeLists.txt`).
2. Que el `assert` **puede fallar** (no un `EXPECT_TRUE(true)` disfrazado).
3. Que el mensaje de fallo es **comprensible** — lo vas a leer dentro de seis
   meses a las 19:30.

> Un test que **nunca** has visto fallar no es un test: es una esperanza.

## 5. Ejemplo completo: FizzBuzz paso a paso

Especificación: para `n`, devuelve `"Fizz"` si es múltiplo de 3, `"Buzz"` si lo
es de 5, `"FizzBuzz"` si lo es de ambos, y el número en texto en otro caso.

### Ciclo 1 — Rojo

```cpp
#include <gtest/gtest.h>
#include "fizzbuzz.h"

TEST(FizzBuzz, DevuelveElNumeroCuandoNoEsMultiploDeTresNiDeCinco) {
    EXPECT_EQ("1", fizzbuzz(1));
}
```

No compila: `fizzbuzz` no existe. **Eso ya es rojo** (ley 2).

### Ciclo 1 — Verde (lo mínimo, aunque duela)

```cpp
std::string fizzbuzz(int n) {
    return "1";          // sí, hardcodeado. Es correcto en esta fase.
}
```

Esto se llama **"fake it till you make it"**. No es trampa: es la garantía de
que cada línea de producción está justificada por un test. Y obliga al
siguiente test a ser el que **triangule** la solución.

### Ciclo 2 — Rojo

```cpp
TEST(FizzBuzz, DevuelveElNumeroParaOtrosNoMultiplos) {
    EXPECT_EQ("2", fizzbuzz(2));     // falla: devuelve "1"
}
```

### Ciclo 2 — Verde

```cpp
std::string fizzbuzz(int n) {
    return std::to_string(n);
}
```

Dos ejemplos han forzado la generalización. Eso es **triangulación**.

### Ciclo 3 — Rojo, Verde

```cpp
TEST(FizzBuzz, DevuelveFizzParaLosMultiplosDeTres) {
    EXPECT_EQ("Fizz", fizzbuzz(3));
}
```

```cpp
std::string fizzbuzz(int n) {
    if (n % 3 == 0) return "Fizz";
    return std::to_string(n);
}
```

### Ciclo 4 y 5 — Buzz y FizzBuzz

```cpp
TEST(FizzBuzz, DevuelveBuzzParaLosMultiplosDeCinco) {
    EXPECT_EQ("Buzz", fizzbuzz(5));
}
TEST(FizzBuzz, DevuelveFizzBuzzParaLosMultiplosDeQuince) {
    EXPECT_EQ("FizzBuzz", fizzbuzz(15));
}
```

```cpp
std::string fizzbuzz(int n) {
    if (n % 15 == 0) return "FizzBuzz";   // ¡el orden importa!
    if (n % 3 == 0)  return "Fizz";
    if (n % 5 == 0)  return "Buzz";
    return std::to_string(n);
}
```

> Fíjate: si hubieras puesto `n % 15` al final, el test de 15 habría fallado.
> **El test te ha enseñado el bug antes de que llegara a existir.**

### Refactor (ahora sí, en verde)

```cpp
std::string fizzbuzz(int n) {
    std::string r;
    if (n % 3 == 0) r += "Fizz";
    if (n % 5 == 0) r += "Buzz";
    return r.empty() ? std::to_string(n) : r;
}
```

Cambia la estructura, desaparece el caso especial del 15, y **los cinco tests
siguen en verde**. Ese es el momento en que se nota para qué sirve la suite:
acabas de rehacer el algoritmo en 20 segundos sin un instante de duda.

## 6. La estructura de un test: AAA

Todos los tests que escribiremos tienen tres partes visualmente separadas:

```cpp
TEST(Cuenta, RetirarDisminuyeElSaldo) {
    // Arrange (Preparar): montar el escenario
    Cuenta cuenta{100.0};

    // Act (Actuar): UNA sola acción, la que se está probando
    cuenta.retirar(30.0);

    // Assert (Comprobar): el resultado esperado
    EXPECT_DOUBLE_EQ(70.0, cuenta.saldo());
}
```

En BDD (día 4) se llama **Given / When / Then**. Es lo mismo.

**Regla del único Act**: si tu test tiene dos acciones, probablemente son dos
tests. Y si tiene cinco `assert` sobre cosas distintas, cuando falle no sabrás
cuál era el problema.

### Cómo se nombra un test

El nombre es documentación. Debe leerse como una frase de especificación:

```cpp
// Bad
TEST(CuentaTest, test1) { ... }
TEST(CuentaTest, testRetirar) { ... }

// Good: sujeto + comportamiento + condición
TEST(Cuenta, NoPermiteRetirarMasDelSaldoDisponible) { ... }
TEST(Cuenta, RegistraLaOperacionCuandoLaRetiradaTieneExito) { ... }
```

Prueba de fuego: **leyendo solo los nombres de los tests, ¿se entiende qué hace
la clase?** Si sí, tienes la especificación viva del sistema — y eso es más de
lo que consigue el 95 % de la documentación escrita.

## 7. La lista de tests

TDD no empieza en el teclado, empieza en un papel. Antes de escribir nada,
haz la lista de comportamientos que quieres:

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

Coges **uno**, lo pones en rojo, lo pasas a verde, refactorizas, lo tachas.
Si a mitad se te ocurre otro caso, **no lo implementes**: añádelo a la lista y
sigue. Esa lista es el mecanismo que impide que TDD se convierta en vagabundeo.

## 8. Qué ganamos y qué pagamos

**Ganamos:**
- Cobertura alta **como efecto secundario**, no como objetivo.
- Diseño guiado por el uso real: la API la define su primer cliente.
- Feedback en segundos; depuración casi eliminada (el error está en lo último
  que has tocado, que son 5 líneas).
- Una red de seguridad que **convierte el refactor en una operación rutinaria**.
- Documentación ejecutable que no se desactualiza.

**Pagamos:**
- Es contraintuitivo al principio; la productividad cae unas semanas.
- Requiere disciplina sostenida, y se abandona bajo presión — que es justo
  cuando más falta hace.
- No todo se presta igual: UI, código altamente algorítmico, o integración con
  hardware requieren adaptaciones.
- **TDD no garantiza que el diseño sea bueno**: garantiza que puedes cambiarlo.
  Si no sabes SOLID, TDD te dará código malo con buenos tests.

## 9. Lo que NO es TDD

| Mito                                      | Realidad                                                  |
|-------------------------------------------|-----------------------------------------------------------|
| "TDD es escribir tests"                   | TDD es una técnica de **diseño**; los tests son el residuo |
| "TDD sustituye al QA"                     | Cubre lo unitario; no sustituye integración ni aceptación  |
| "Hay que llegar al 100 % de cobertura"    | La cobertura es un síntoma, no una meta                    |
| "Con TDD no hay bugs"                     | Hay menos, y los que hay se localizan mucho antes          |
| "Ralentiza el desarrollo"                 | Ralentiza teclear; acelera todo lo demás                   |

## 10. Mantra del bloque

> **"Rojo, Verde, Refactor. Sin saltarse ninguno, sin quedarse en ninguno."**
> El rojo prueba el test. El verde prueba el código.
> El refactor es donde está el valor — y solo es posible gracias a los dos anteriores.

---

## 11. Referencias

**El ciclo, de sus autores:**

- 📖 Kent Beck, **[*Test-Driven Development: By Example*](https://www.informit.com/store/test-driven-development-by-example-9780321146533)**
  — el libro fundacional. La parte I es un ejemplo seguido, ciclo a ciclo,
  exactamente como el FizzBuzz del apartado 5.
- Kent Beck, **[*Canon TDD*](https://tidyfirst.substack.com/p/canon-tdd)**
  (2023) — el propio autor reformula el ciclo en dos páginas, corrigiendo las
  deformaciones más extendidas. **Si solo lees una cosa, lee esta.**
- Robert C. Martin, **[*The Cycles of TDD*](https://blog.cleancoder.com/uncle-bob/2014/12/17/TheCyclesOfTDD.html)**
  — las tres leyes del apartado 2 y los ciclos que las envuelven.

**TDD en C++:**

- 📖 Jeff Langr, **[*Modern C++ Programming with Test-Driven Development*](https://pragprog.com/titles/lotdd/modern-c-programming-with-test-driven-development/)**
  — el único libro de TDD específico de C++, y además con Google Test.
  El acompañante natural de este curso.
- **[GoogleTest — Primer](https://google.github.io/googletest/primer.html)** y
  **[Quickstart con CMake](https://google.github.io/googletest/quickstart-cmake.html)**
  — media hora de lectura que **conviene traer hecha mañana**.

**Estructura y nombrado de los tests:**

- Martin Fowler, **[*Given-When-Then*](https://martinfowler.com/bliki/GivenWhenThen.html)**
  — el AAA del apartado 6, en su versión BDD (volveremos a esto el día 4).
- Martin Fowler, **[*Self Testing Code*](https://martinfowler.com/bliki/SelfTestingCode.html)**
  — por qué la suite es parte del producto, no un anexo.

**Práctica deliberada:**

- James Shore, **[*The Art of Agile Development* — capítulo de TDD](https://www.jamesshore.com/v2/books/aoad2/test-driven_development)**
  — **gratuito**. Un ciclo completo narrado paso a paso, con los errores
  típicos señalados.
- **[Wikipedia — Test-driven development](https://en.wikipedia.org/wiki/Test-driven_development)**
  — panorámica correcta y bien referenciada, útil para el resumen a un jefe.

---

## Mañana

Pasamos a la práctica con **Google Test**: instalación y CMake, `TEST` y
`TEST_F`, la familia de assertions (`ASSERT_*` vs `EXPECT_*`, matchers,
excepciones, coma flotante), fixtures y ciclo de vida, y el primer ciclo
Rojo-Verde-Refactor completo sobre un proyecto real. Y el diseño de casos que
hoy hemos visto en teoría — clases de equivalencia y valores frontera — aplicado
sobre código de verdad.
