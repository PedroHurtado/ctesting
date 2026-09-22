# Día 2 — Bloque 6: Google Test desde cero

> *"Un test que no se ejecuta solo, no se ejecuta."*

---

## 1. El problema en una frase

Sin un framework, cada test es un `main` distinto con `if`s y `cout`: nadie los
lanza todos, nadie sabe cuáles fallan, y el primer fallo oculta los demás.

## 2. Qué es Google Test

Un framework **xUnit** para C++: registra los tests automáticamente, los
ejecuta de forma aislada, los protege de los fallos ajenos y devuelve un informe
y un **código de salida** que la integración continua entiende.

El paquete `googletest` trae dos bibliotecas:

| Biblioteca   | Para qué                         | Cuándo la usamos |
|--------------|----------------------------------|------------------|
| **gtest**    | Tests y assertions               | Hoy              |
| **gmock**    | Dobles de prueba y *matchers*    | *Matchers* hoy (bloque 7); dobles mañana |

Vinculamos `GTest::gmock_main` desde el principio: incluye a `gtest` y nos
evita tener que tocar el `CMakeLists.txt` mañana.

## 3. Bad: el "test" artesanal

```cpp
// main.cpp — lo que todos hemos escrito alguna vez
#include <iostream>
#include "calculadora.h"

int main() {
    Calculadora c;
    if (c.sumar(2, 3) != 5)                       // ← ¿y si falla? sigue ejecutando
        std::cout << "ERROR en sumar\n";

    if (c.dividir(10, 0) != 0)                    // ← esto revienta y se lleva
        std::cout << "ERROR en dividir\n";        //   por delante los tests siguientes

    std::cout << "Tests terminados\n";            // ← miente: no dice cuántos pasaron
    return 0;                                     // ← siempre 0: la CI cree que todo va bien
}
```

Cuatro defectos, todos graves:

- **No informa**: "ERROR en sumar" no dice qué esperaba ni qué obtuvo.
- **No aísla**: un `segfault` mata la ejecución completa.
- **Devuelve siempre 0**: ningún sistema de integración continua detectará nada.
- **No escala**: con 200 tests ese `main` es ilegible, y nadie añade el 201.

## 4. Good: el mismo test con Google Test

```cpp
// tests/calculadora_test.cpp
#include <gtest/gtest.h>
#include "calculadora.h"

TEST(Calculadora, SumaDosEnterosPositivos) {
    Calculadora c;                       // Arrange
    EXPECT_EQ(5, c.sumar(2, 3));         // Act + Assert
}

TEST(Calculadora, DividirPorCeroLanza) {
    Calculadora c;
    EXPECT_THROW(c.dividir(10, 0), std::invalid_argument);
}
```

Salida al fallar:

```
tests/calculadora_test.cpp:6: Failure
Expected equality of these values:
  5
  c.sumar(2, 3)
    Which is: 6
[  FAILED  ] Calculadora.SumaDosEnterosPositivos (0 ms)
```

Fichero, línea, valor esperado, valor obtenido y nombre del test. Y si esa
suite falla, el ejecutable devuelve **1**: la CI se entera.

### Anatomía de `TEST`

```cpp
TEST(NombreDeLaSuite, NombreDelCaso) { ... }
//   └── agrupa            └── describe el comportamiento concreto
```

- Son **identificadores C++**: sin espacios, sin acentos y **sin guiones bajos**
  (GoogleTest los usa internamente para componer nombres; está en la FAQ).
- `TEST` **registra el caso automáticamente**: no hay que apuntarlo en ninguna
  lista. Eso es exactamente lo que el `main` artesanal no tenía.
- La convención del curso, ya vista ayer: la suite es el **sujeto** (la clase)
  y el caso es el **comportamiento**, leído como una frase.

## 5. El proyecto: estructura y CMake

```
cuenta/
├── CMakeLists.txt
├── include/
│   └── cuenta.h
├── src/
│   └── cuenta.cpp
└── tests/
    ├── CMakeLists.txt
    └── cuenta_test.cpp
```

La regla: el código de producción **no sabe que existen los tests**. La
dependencia va en una sola dirección, `tests → src`, nunca al revés.

```cmake
# CMakeLists.txt (raíz)
cmake_minimum_required(VERSION 3.14)        # 3.14 = FetchContent_MakeAvailable
project(cuenta CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# La lógica, en una biblioteca: los tests enlazan contra ella.
add_library(cuenta_lib src/cuenta.cpp)
target_include_directories(cuenta_lib PUBLIC include)

enable_testing()                            # ← habilita ctest. Va en la raíz.
add_subdirectory(tests)
```

```cmake
# tests/CMakeLists.txt
include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.18.0          # ← etiqueta fija, nunca "main"
)
# Necesario en Windows: evita el choque de runtime entre gtest y tu proyecto.
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(cuenta_test cuenta_test.cpp)
target_link_libraries(cuenta_test PRIVATE cuenta_lib GTest::gmock_main)

include(GoogleTest)
gtest_discover_tests(cuenta_test)           # ← da de alta cada TEST en ctest
```

Y a compilar:

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

### Dos detalles que ahorran una tarde

- **`GIT_TAG v1.18.0`, no `main`.** Con `main`, el proyecto compila hoy y falla
  dentro de tres meses sin que nadie haya tocado una línea. Fijar la versión es
  parte de que el test sea **Repeatable** (FIRST, día 1).
- **`gtest_discover_tests` frente a `add_test`.** El primero ejecuta el binario
  al construir y registra **cada `TEST` por separado** en `ctest`; el segundo
  da de alta el ejecutable entero como un único test. Con el primero,
  `ctest -R Cuenta.Retirar` funciona.

## 6. Ejecutar y filtrar

Durante la kata del bloque 10 vamos a lanzar el binario directamente: es más
rápido y la salida es más rica.

```bash
./build/tests/cuenta_test                              # todos
./build/tests/cuenta_test --gtest_list_tests           # solo listarlos
./build/tests/cuenta_test --gtest_filter=Cuenta.*      # una suite
./build/tests/cuenta_test --gtest_filter=*Retirar*     # por patrón
./build/tests/cuenta_test --gtest_filter=-*Lento*      # excluir (el guion niega)
```

| Opción | Para qué sirve, en la práctica |
|---|---|
| `--gtest_filter=P` | El único que se usa cada minuto: aislar el test en rojo. |
| `--gtest_shuffle`  | Orden aleatorio: **delata tests que dependen unos de otros**. |
| `--gtest_repeat=N` | Repite N veces: caza los *flaky*. Combínalo con `shuffle`. |
| `--gtest_break_on_failure` | Rompe en el depurador justo en el fallo. |
| `--gtest_output=xml:res.xml` | Informe JUnit para Jenkins/GitLab/GitHub Actions. |
| `--gtest_brief=1` | Muestra solo los fallos. Útil con suites grandes. |

> **Truco de aula:** `--gtest_shuffle --gtest_repeat=3` sobre una suite que
> creéis sana. Si aparece un fallo, tenéis dependencia oculta entre tests.
> Volveremos a ello en el bloque 8.

## 7. Otras formas de instalarlo

| Vía | Cuándo tiene sentido | Coste |
|---|---|---|
| **`FetchContent`** | Proyecto nuevo, equipo pequeño, CI limpia. **La del curso.** | Compila gtest la primera vez; requiere red. |
| **[vcpkg](https://vcpkg.io/en/package/gtest)** / **[Conan](https://conan.io/center/recipes/gtest)** | Ya gestionáis así las dependencias. | Otra herramienta que mantener. |
| **Paquete del sistema** (`libgtest-dev`) | Máquina sin red. | Versión antigua e incontrolada: rompe *Repeatable*. |
| **Submódulo git** | Necesitáis parchear GoogleTest. | Casi nadie lo necesita. |

**Sin red en el aula:** clonad GoogleTest una vez en local y cambiad
`GIT_REPOSITORY` por la ruta del clon, o sustituid el bloque `FetchContent` por
`add_subdirectory(third_party/googletest)`. El resto del `CMakeLists.txt` no
cambia.

## 8. Qué ganamos y qué pagamos

**Ganamos:** registro automático de casos, aislamiento entre tests, mensajes de
fallo útiles, código de salida para la CI, y filtros que convierten "lanzar los
tests" en una operación de un segundo.

**Pagamos:** una dependencia externa, la primera compilación larga y un
`CMakeLists.txt` algo más largo. A cambio, es el mismo fichero durante los
cuatro días: hoy lo escribimos una vez y ya no se toca.

## 9. Mantra del bloque

> **"El test se escribe una vez y se ejecuta diez mil veces."**
> Todo lo que inviertas en que arranque con un solo comando, lo cobras
> multiplicado por diez mil.

---

## 10. Referencias

**Para montar el proyecto (gratuito):**

- **[GoogleTest — Quickstart: Building with CMake](https://google.github.io/googletest/quickstart-cmake.html)**
  — exactamente el `CMakeLists.txt` del apartado 5, explicado línea a línea.
  Si solo miráis un enlace hoy, este.
- **[GoogleTest — Primer](https://google.github.io/googletest/primer.html)**
  — qué es una suite, qué es un caso, cómo se nombran. Media hora bien gastada.
- **[CMake — módulo `FetchContent`](https://cmake.org/cmake/help/latest/module/FetchContent.html)**
  — documentación oficial de `FetchContent_Declare` y `MakeAvailable`, con las
  opciones para trabajar sin red.
- **[CMake — módulo `GoogleTest`](https://cmake.org/cmake/help/latest/module/GoogleTest.html)**
  — `gtest_discover_tests` y sus opciones (`TEST_PREFIX`, `DISCOVERY_TIMEOUT`).

**Cuando algo falle:**

- **[GoogleTest FAQ](https://google.github.io/googletest/faq.html)**
  — por qué no se pueden usar guiones bajos en los nombres, por qué un test no
  se ejecuta, qué hacer con las macros de Windows. Respuesta corta a casi todo.
- **[Repositorio google/googletest](https://github.com/google/googletest)**
  — las *releases* (de ahí sale el `GIT_TAG`) y la matriz de compiladores
  soportados. Desde v1.15 el mínimo es **C++17**, la base del curso.

**Contexto:**

- 📖 Jeff Langr, **[*Modern C++ Programming with TDD*](https://pragprog.com/titles/lotdd/modern-c-programming-with-test-driven-development/)**,
  cap. 2 — el mismo montaje visto por alguien que lo hace a diario en C++, con
  las variantes de organización de directorios que sobreviven al tiempo.
- **[*Software Engineering at Google*, cap. 11](https://abseil.io/resources/swe-book/html/ch11.html)**
  — **gratuito**. Por qué Google exige que toda la suite se lance con **un solo
  comando**, y qué pasaba cuando no era así.
