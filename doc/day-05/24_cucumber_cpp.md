# Día 5 — Bloque 24: cucumber-cpp, los escenarios ejecutando C++

> *"Un escenario que no se ejecuta es un documento. Uno que se ejecuta es un
> test que lee tu cliente."*

---

## 1. El problema en una frase

El `.feature` del bloque 23 es texto. Para que sea un test, cada línea tiene
que acabar llamando a vuestra clase `Cuenta`.

## 2. Cómo funciona

**Cucumber** es la herramienta que lee los `.feature`. Está escrita en Ruby.
**cucumber-cpp** es el puente para que los pasos se escriban en C++.

Hablan por un socket TCP con el **protocolo *wire***:

```
  ┌───────────────┐   TCP :3902    ┌──────────────────────────────────────────┐
  │   cucumber    │ ─────────────► │   cajero_steps   (vuestro ejecutable)    │
  │   (Ruby)      │ "¿quién hace   │   ┌──────────────┐   ┌────────────────┐  │
  │               │  'retiro 30    │   │ cucumber-cpp │──►│  pasos en C++  │  │
  │ lee .feature  │  euros'?"      │   │ (servidor)   │   │  GIVEN / WHEN  │  │
  │ pinta         │ ◄───────────── │   └──────────────┘   └───────┬────────┘  │
  │ resultados    │ "yo, paso 21,  │                              ▼           │
  │               │  y ha ido bien"│                      ┌───────────────┐   │
  └───────────────┘                │                      │ Cuenta, Dinero│   │
                                   │                      └───────────────┘   │
                                   └──────────────────────────────────────────┘
```

Por eso se ejecuta en **dos pasos**: primero se arranca vuestro ejecutable
(se queda escuchando en el puerto 3902) y después `cucumber`.

---

## 3. Instalación: lo que hace falta

| Pieza | Versión probada | Para qué |
|---|---|---|
| Ruby | 3.2 | Para ejecutar `cucumber` |
| Gema `cucumber` | **7.1.0** (fija) | El lector de `.feature` |
| Gema `cucumber-wire` | **6.2.1** (fija) | El protocolo *wire* |
| CMake | ≥ 3.16 | — |
| Asio, nlohmann-json, TCLAP | las de conda-forge | Dependencias de cucumber-cpp |
| GoogleTest | 1.14 | Para usar `EXPECT_*` dentro de los pasos |
| cucumber-cpp | rama `main` | Se compila desde el código fuente |

> ⚠️ **Las versiones de las gemas van fijas.** Son las que usa el propio
> proyecto cucumber-cpp en su `Gemfile`. Con Cucumber 3.x no funciona. Y desde
> Cucumber-Ruby 8 el protocolo *wire* ya no viene dentro de Cucumber: solo
> funciona a través de la gema aparte `cucumber-wire`.

**En el aula lo hemos montado en Linux (WSL)**, sin permisos de administrador:

```bash
# 1. Herramientas en el espacio del usuario, con micromamba
curl -Ls https://micro.mamba.pm/api/micromamba/linux-64/latest | tar -xj bin/micromamba
./bin/micromamba create -y -p ~/cuke/env -c conda-forge \
    ruby=3.2 cmake make gxx_linux-64=13 asio nlohmann_json tclap gtest=1.14
export PATH=~/cuke/env/bin:$PATH
export CXX=x86_64-conda-linux-gnu-g++

# 2. Cucumber (Ruby)
gem install --no-document cucumber:7.1.0 cucumber-wire:6.2.1

# 3. cucumber-cpp
git clone --depth 1 https://github.com/cucumber/cucumber-cpp.git
cd cucumber-cpp
cmake -S . -B build -DCUKE_ENABLE_GTEST=on -DCUKE_ENABLE_BOOST_TEST=off \
      -DCUKE_ENABLE_QT_6=off -DCUKE_ENABLE_QT_5=off \
      -DCMAKE_PREFIX_PATH=$HOME/cuke/env -DCMAKE_INSTALL_PREFIX=$HOME/cuke/env
cmake --build build -j4
cmake --install build
```

Con `apt` y permisos de administrador es más corto: `ruby`, `cmake`,
`libasio-dev`, `nlohmann-json3-dev`, `libtclap-dev`, `libgtest-dev`, y los
pasos 2 y 3 igual.

> **Windows nativo (MinGW/MSVC):** cucumber-cpp tiene un *workflow* de Windows
> en su repositorio, pero **no lo hemos probado en la máquina del aula**.
> Para el curso, usad WSL.

---

## 4. El proyecto

El proyecto completo está en el repositorio: **[`03-test/cajero/`](../../cajero/)**
(con su [`README.md`](../../cajero/README.md)).

```
cajero/
├── CMakeLists.txt
├── aceptacion.sh                ← compila, arranca el servidor y lanza Cucumber
├── src/
│   ├── dinero.h                 ← el del bloque 19
│   └── cuenta.h
├── tests/                       ← bucle interno: GoogleTest
│   ├── dinero_test.cpp
│   └── cuenta_test.cpp
└── features/
    ├── retirar_efectivo.feature ← el del bloque 23
    ├── comision_cajero_ajeno.feature   ← §7
    ├── transferencias.feature
    ├── support/
    │   └── wire.rb              ← una línea: require 'cucumber/wire'
    └── step_definitions/
        ├── cucumber.wire        ← dónde está el servidor
        ├── cuenta_steps.cpp
        └── transferencia_steps.cpp
```

**`features/step_definitions/cucumber.wire`** — le dice a Cucumber dónde
escuchan los pasos en C++:

```yaml
host: localhost
port: 3902
```

**`features/support/wire.rb`** — activa el protocolo *wire* desde la gema
`cucumber-wire`. Sin este fichero funciona igual, pero cada ejecución empieza
con un aviso de obsolescencia (§8):

```ruby
require 'cucumber/wire'
```

**`CMakeLists.txt`** — dos ejecutables, uno por bucle:

```cmake
cmake_minimum_required(VERSION 3.16)
project(Cajero CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(GTest REQUIRED)

# El dominio: Dinero y Cuenta (solo cabeceras)
add_library(cajero INTERFACE)
target_include_directories(cajero INTERFACE src)

# ── Bucle interno: tests unitarios con GoogleTest (Windows y Linux) ──
enable_testing()
add_executable(cajero_tests
    tests/dinero_test.cpp
    tests/cuenta_test.cpp)
target_link_libraries(cajero_tests PRIVATE cajero GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(cajero_tests)

# ── Bucle externo: pasos de Cucumber (solo si cucumber-cpp está instalado) ──
find_package(nlohmann_json QUIET)   # cucumber-cpp lo necesita y su config no lo busca
find_package(CucumberCpp QUIET)

if(CucumberCpp_FOUND)
    add_executable(cajero_steps
        features/step_definitions/cuenta_steps.cpp
        features/step_definitions/transferencia_steps.cpp)
    target_link_libraries(cajero_steps PRIVATE cajero CucumberCpp::cucumber-cpp GTest::gtest)
else()
    message(STATUS "cucumber-cpp no encontrado: solo se compilan los tests unitarios")
endif()
```

- `CucumberCpp::cucumber-cpp` ya trae el `main()` que arranca el servidor. No
  hay que escribirlo.
- Si cucumber-cpp no está instalado (por ejemplo, en Windows con MinGW), el
  proyecto **compila igual** y se pueden pasar los tests unitarios con `ctest`.

---

## 5. Los pasos en C++

Cada línea del `.feature` se une a una función C++ mediante una **expresión
regular**. Lo que va entre paréntesis en la expresión se convierte en un
parámetro.

```cpp
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
```

Las cuatro piezas, una por una:

| Pieza | Qué hace | Lo conocéis como... |
|---|---|---|
| `GIVEN` / `WHEN` / `THEN` | Registra la función para las líneas que casan con la expresión | `TEST(...)` |
| `REGEX_PARAM(tipo, nombre)` | Saca el siguiente `(...)` de la expresión, ya convertido al tipo | El parámetro de `TEST_P` |
| `ScenarioScope<T>` | Un `T` **compartido por los pasos de un escenario**, nuevo en cada escenario | El fixture de `TEST_F` |
| `EXPECT_EQ` | El de siempre. Si falla, Cucumber marca el paso en rojo | — |

Dos detalles que importan:

- **El `WHEN` captura la excepción y no la comprueba.** Quien decide si el
  error era esperado es el `THEN`. Así el mismo `Cuando retiro X euros` sirve
  para los escenarios que pasan y para los que se rechazan.
- **Da igual la palabra clave.** `Y el saldo es de 100 euros` casa con el
  `THEN` porque la expresión solo mira el texto después de `Y`.

### Tablas de datos: `TABLE_PARAM`

Para el `Dadas las siguientes cuentas:` del bloque 23:

```cpp
struct Banco {
    std::map<std::string, Cuenta> cuentas;     // titular → cuenta
    std::string error;
};

GIVEN("^las siguientes cuentas:$") {
    TABLE_PARAM(tabla);
    ScenarioScope<Banco> banco;

    for (const auto& fila : tabla.hashes()) {             // una fila = un map columna→valor
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
```

---

## 6. Ejecutar

A mano, son dos pasos:

```bash
export PATH=~/cuke/env/bin:~/cuke/env/share/rubygems/bin:$PATH
export GEM_HOME=~/cuke/env/share/rubygems        # sin esto, Ruby no encuentra la gema

cmake -S . -B build -DCMAKE_PREFIX_PATH=$HOME/cuke/env -DCucumberCpp_DIR=$HOME/cuke/env/lib/cmake
cmake --build build

build/cajero_steps &          # 1. el servidor, en segundo plano
cucumber                      # 2. Cucumber lee features/ y se conecta
```

En el proyecto, `aceptacion.sh` hace todo esto. Además, si Cucumber falla,
cierra el servidor, que si no se queda esperando para siempre:

```bash
./aceptacion.sh                        # salida completa
./aceptacion.sh --format progress      # un punto por paso
ctest --test-dir build                 # y los tests unitarios
```

Salida real:

```
# language: es
Característica: Retirar efectivo
  Como titular de una cuenta
  Quiero retirar efectivo de mi cuenta
  Para disponer de mi dinero sin pasar por la oficina

  Escenario: Retirada con saldo suficiente    # features/retirar_efectivo.feature:9
    Dado una cuenta con un saldo de 100 euros # cuenta_steps.cpp:15
    Cuando retiro 30 euros                    # cuenta_steps.cpp:22
    Entonces el saldo es de 70 euros          # cuenta_steps.cpp:33

  [...]

  Esquema del escenario: Importe no válido                   # features/retirar_efectivo.feature:27
    Dado una cuenta con un saldo de 100 euros                # features/retirar_efectivo.feature:28
    Cuando retiro <importe> euros                            # features/retirar_efectivo.feature:29
    Entonces la operación se rechaza por "importe no valido" # features/retirar_efectivo.feature:30
    Y el saldo es de 100 euros                               # features/retirar_efectivo.feature:31

    Ejemplos:
      | importe |
      | 0       |
      | -50     |

5 scenarios (5 passed)
18 steps (18 passed)
0m0.028s
```

A la derecha de cada paso, **el fichero y la línea de C++** que lo ejecuta.

Opciones útiles:

| Opción | Para qué |
|---|---|
| `--format progress` | Un punto por paso (para CI) |
| `--tags "not @lento"` | Filtrar por etiquetas (bloque 23 §8) |
| `features/transferencias.feature:13` | Solo el escenario de esa línea |
| `--publish-quiet` | Quita el anuncio de *reports.cucumber.io* |

---

## 7. El doble bucle, en vivo

Llega la historia del bloque 22: **comisión de 2 € en cajeros de otra
entidad**. Seguimos el doble bucle paso a paso.

### 7.1 Escenario en rojo (bucle externo)

```gherkin
# language: es
Característica: Comisión en cajeros de otra entidad
  Como responsable de productos del banco
  Quiero cobrar 2 euros por cada retirada en un cajero de otra entidad
  Para cubrir lo que esa entidad nos cobra a nosotros

  Escenario: La comisión se suma al importe retirado
    Dado una cuenta con un saldo de 100 euros
    Cuando retiro 30 euros en un cajero de otra entidad
    Entonces el saldo es de 68 euros

  Escenario: La comisión también cuenta para el saldo disponible
    Dado una cuenta con un saldo de 100 euros
    Cuando retiro 99 euros en un cajero de otra entidad
    Entonces la operación se rechaza por "saldo insuficiente"
    Y el saldo es de 100 euros
```

Salida real. Los pasos `Dado` y `Entonces` ya existían; el `Cuando` es nuevo:

```
2 scenarios (2 undefined)
7 steps (3 skipped, 2 undefined, 2 passed)

You can implement step definitions for undefined steps with these snippets:

CUANDO("^retiro 30 euros en un cajero de otra entidad$") {
    pending();
}
```

> ⚠️ **El fragmento que propone no compila tal cual.** Escribe `CUANDO` porque
> traduce la palabra clave del `.feature`, pero en cucumber-cpp las macros son
> siempre `GIVEN`, `WHEN` y `THEN`. Y pone `30` a fuego: hay que cambiarlo
> por `(\\d+)`.

### 7.2 El paso, y los tests unitarios (bucle interno)

```cpp
WHEN("^retiro (\\d+) euros en un cajero de otra entidad$") {
    REGEX_PARAM(long long, euros);
    ScenarioScope<Contexto> ctx;

    try {
        ctx->cuenta.retirarEnCajeroAjeno(Dinero::euros(euros));
    } catch (const std::exception& e) {
        ctx->error = e.what();
    }
}
```

`retirarEnCajeroAjeno` no existe: **no compila**. Ahora entra TDD normal, con
GoogleTest. Primer test, primera implementación:

```cpp
TEST(CajeroAjeno, CobraDosEurosDeComision) {
    Cuenta c;
    c.ingresar(100_eur);

    c.retirarEnCajeroAjeno(30_eur);

    EXPECT_EQ(68_eur, c.saldo());
}
```

```cpp
void retirarEnCajeroAjeno(Dinero importe) {
    retirar(importe);
    retirar(comisionCajeroAjeno);           // ← dos operaciones: ¿y si falla la segunda?
}
```

El test unitario pasa. Volvemos al bucle externo y ejecutamos el escenario
(salida real):

```
  Escenario: La comisión también cuenta para el saldo disponible # features/comision_cajero_ajeno.feature:12
    Dado una cuenta con un saldo de 100 euros                    # cuenta_steps.cpp:14
    Cuando retiro 99 euros en un cajero de otra entidad          # cuenta_steps.cpp:32
    Entonces la operación se rechaza por "saldo insuficiente"    # cuenta_steps.cpp:50
    Y el saldo es de 100 euros                                   # cuenta_steps.cpp:43
      /home/pedro/cuke/cajero/features/step_definitions/cuenta_steps.cpp:47: Failure
      Expected equality of these values:
        Dinero::euros(euros)
          Which is: 100,00 EUR
        ctx->cuenta.saldo()
          Which is: 1,00 EUR
       (Cucumber::Wire::Exception)

2 scenarios (1 failed, 1 passed)
7 steps (1 failed, 6 passed)
```

**El escenario de negocio ha encontrado un bug que el test unitario no veía.**
Se retiraron los 99 €, la comisión falló después, y el cliente se queda con
1 €. Es la **atomicidad** de la kata del día 2, otra vez.

Se baja al bucle interno con un test que reproduce el fallo:

```cpp
TEST(CajeroAjeno, SiNoLlegaParaLaComisionNoRetiraNada) {
    Cuenta c;
    c.ingresar(100_eur);

    EXPECT_THROW(c.retirarEnCajeroAjeno(99_eur), std::domain_error);

    EXPECT_EQ(100_eur, c.saldo());
}
```

Rojo. Y la corrección:

```cpp
void retirarEnCajeroAjeno(Dinero importe) {
    retirar(importe + comisionCajeroAjeno); // ← una sola operación: o todo o nada
}
```

### 7.3 Todo en verde

```
[  PASSED  ] 2 tests.                  ← GoogleTest

.........................................
9 scenarios (9 passed)                 ← Cucumber, las tres características
34 steps (34 passed)
```

La historia está **hecha**: no porque lo diga el programador, sino porque
pasan los escenarios que escribió negocio.

---

## 8. Problemas reales que nos hemos encontrado

Todos salieron montando este bloque. Os los ahorramos:

| Síntoma | Causa | Solución |
|---|---|---|
| `Unable to contact the wire server at localhost:3902` | `cucumber` se ejecutó antes que el servidor, o el servidor no compiló | Arrancar primero `build/cajero_steps &` y comprobar que existe |
| `multiple definition of 'CukeObject0::bodyWithArgs()'` | Dos ficheros de pasos generan clases con el mismo nombre | `#define CUKE_OBJECT_PREFIX <nombre único>` antes del `#include` en cada fichero |
| `cannot find -lnlohmann_json::nlohmann_json` | La configuración CMake de cucumber-cpp no busca sus dependencias | `find_package(nlohmann_json REQUIRED)` antes de `find_package(CucumberCpp)` |
| `Could not find a package configuration file provided by "CucumberCpp"` | Se instala en `lib/cmake/`, no en `lib/cmake/CucumberCpp/` | `-DCucumberCpp_DIR=<prefijo>/lib/cmake` |
| El paso sale *undefined* aunque lo has escrito | La expresión regular no casa | Revisar `^` y `$`, espacios, y que `\\d` lleve **dos** barras en C++ |
| `\\w+` no casa con "José" | `std::regex` no entiende acentos en `\w` | Usar `([^ ]+)` o nombres sin acentos |
| `can't find gem cucumber (>= 0.a) with executable cucumber` | Ruby busca las gemas en otra carpeta | `export GEM_HOME=<prefijo>/share/rubygems` |
| El terminal se queda colgado tras un error de Cucumber | `cajero_steps` sigue esperando una conexión que no llega | `kill` al servidor; `aceptacion.sh` lo hace solo con un `trap` |
| Snippet con `CUANDO(...)` | Cucumber traduce la palabra clave | Usar `WHEN` |
| `WARNING: built-in usage of the wire protocol is deprecated` | Cucumber-Ruby 7.1 avisa de que el protocolo *wire* sale de Cucumber y pasa a la gema `cucumber-wire` | Crear `features/support/wire.rb` con `require 'cucumber/wire'` (§4). Comprobado: el aviso desaparece |

---

## 9. ¿Y si no puedo instalar cucumber-cpp?

Es una posibilidad real: depende de Ruby, de gemas con versión fija y de un
protocolo que Cucumber-Ruby ya ha sacado de su núcleo. **Lo que no se pierde es
la forma de trabajar.**

Sin Cucumber, el escenario se escribe igual en el `.feature` (o en la
historia), y el test de aceptación se escribe en GoogleTest con la misma
estructura:

```cpp
// Característica: Comisión en cajeros de otra entidad
// Escenario: La comisión también cuenta para el saldo disponible
TEST(ComisionCajeroAjeno, LaComisionTambienCuentaParaElSaldoDisponible) {
    // Dado una cuenta con un saldo de 100 euros
    Cuenta cuenta;
    cuenta.ingresar(100_eur);

    // Cuando retiro 99 euros en un cajero de otra entidad
    EXPECT_THROW(cuenta.retirarEnCajeroAjeno(99_eur), std::domain_error);

    // Entonces el saldo es de 100 euros
    EXPECT_EQ(100_eur, cuenta.saldo());
}
```

Se pierde que negocio lo lea ejecutándose. Se conserva lo más valioso: **la
conversación, los ejemplos concretos y el doble bucle**.

---

## 10. Ejercicio

Sobre el proyecto [`03-test/cajero/`](../../cajero/):

1. Añadid al `transferencias.feature` el criterio **CA-16** del día 4: *"No se
   puede transferir a la misma cuenta"*. Escribid primero el escenario y
   vedlo fallar.
2. Convertid los dos escenarios de transferencias que tienen la misma forma en
   un `Esquema del escenario` con `Ejemplos`.
3. Escribid una historia nueva con su *Example Mapping* (bloque 22 §6):
   **"límite diario de retirada de 600 €"**. ¿Cuántas tarjetas rojas os salen?
   Pista: *diario* significa **fecha**, y la fecha significa `Reloj`
   (bloque 13).

---

## 11. Qué ganamos y qué pagamos

**Ganamos:**
- Criterios de aceptación que **se ejecutan** en cada compilación.
- Un informe que entiende negocio: *"9 escenarios, 9 pasan"*.
- Una red a nivel de comportamiento que encuentra fallos entre piezas
  (§7.2) que los tests unitarios no ven.

**Pagamos:**
- Una cadena de herramientas larga: Ruby, gemas con versión fija, un socket.
- Una capa más que mantener (los pasos).
- Riesgo de futuro: el protocolo *wire* ya no forma parte del núcleo de
  Cucumber-Ruby; depende de una gema aparte.

## 12. Mantra del bloque

> **"El escenario dice cuándo has terminado. Los tests unitarios, cómo has
> llegado."**

---

## 13. Referencias

- **[cucumber-cpp en GitHub](https://github.com/cucumber/cucumber-cpp)**
  — **gratuito**. El README explica la instalación y el protocolo; la carpeta
  `examples/` tiene la calculadora y un catálogo de pasos
  (`FeatureShowcase`) con tablas, etiquetas y *hooks*.
- **[cucumber-cpp — Wiki](https://github.com/cucumber/cucumber-cpp/wiki/)**
  — **gratuito**. `REGEX_PARAM`, `TABLE_PARAM`, `ScenarioScope` y los *hooks*
  `BEFORE`/`AFTER`.
- **[Cucumber-Ruby — *Upgrading to 7.1.0*](https://github.com/cucumber/cucumber-ruby/blob/main/upgrading_notes/7.1.0.md)**
  — **gratuito**. El aviso del protocolo *wire* (§8) y el `wire.rb` que lo
  quita, en la fuente.
- **[Referencia de Gherkin](https://cucumber.io/docs/gherkin/reference/)**
  — **gratuito**. Para tener abierta mientras escribís los `.feature`.
- 📖 Steve Freeman y Nat Pryce,
  **[*Growing Object-Oriented Software, Guided by Tests*](https://www.informit.com/store/growing-object-oriented-software-guided-by-tests-9780321503626)**
  — el libro del doble bucle (§7). El cap. 1 tiene el diagrama original.
- 📖 Matt Wynne y Aslak Hellesøy,
  **[*The Cucumber Book* (2ª ed.)](https://pragprog.com/titles/hwcuc2/the-cucumber-book-second-edition/)**
  — pasos, contexto del escenario y cómo organizar muchos `.feature`.
