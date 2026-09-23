# Instalación de herramientas — guía detallada

> *"La herramienta que no está instalada no existe."*

Documento **transversal** del curso: se usa el día 2 (Google Test), el día 3
(gMock, gcov/gcovr, sanitizers) y el día 4 (cucumber-cpp). Está fuera de
`day-0X/` a propósito, porque no es un bloque de clase: es la referencia que
abrís cuando algo no compila.

---

## 1. Qué hace falta y para qué

| Herramienta | Para qué | ¿Imprescindible? |
|---|---|---|
| **Compilador C++17** (GCC ≥ 7, Clang ≥ 5, MSVC ≥ 2017) | Compilar todo | Sí |
| **CMake ≥ 3.14** | `FetchContent_MakeAvailable` | Sí |
| **Generador**: Ninja, Make o MSBuild | Construir | Sí (uno) |
| **GoogleTest + gMock** | Tests y dobles | Sí |
| **gcov** | Instrumentación de cobertura | Día 3 (viene con GCC/Clang) |
| **gcovr** | Informe legible a partir de gcov | Día 3 |
| **Python ≥ 3.9** | Solo para instalar `gcovr` | Día 3 |
| **Sanitizers** (ASan/UBSan/TSan) | Detectar UB y errores de memoria | Día 3 (ver §7: **no en MinGW**) |

**Regla que recorre todo el documento:** fija las versiones. Una dependencia sin
versión fijada rompe la *R* de **FIRST** (*Repeatable*, día 1): el proyecto
compila hoy y falla dentro de tres meses sin que nadie haya tocado una línea.

---

## 2. Antes de instalar nada: qué tienes ya

Lanzad esto en vuestra terminal. Lo que responda con una versión, ya está.

```bash
g++ --version          # o: clang++ --version
cmake --version        # necesitamos >= 3.14
ninja --version        # opcional pero recomendado
gcov --version         # viene con GCC; debe coincidir en versión con g++
python --version       # >= 3.9
gcovr --version
```

> **Detalle que muerde:** `gcov` y `g++` **deben ser de la misma versión**. Si
> compiláis con `g++-14` y analizáis con un `gcov` 11 del sistema, gcovr falla
> con *"version mismatch"*. En Windows con un único MinGW no pasa; en Linux con
> varios GCC instalados, sí (ver §9).

---

## 3. Windows

Dos caminos. **No son compatibles entre sí**: elegid uno y no mezcléis binarios.

### 3.1 Opción A — MinGW-w64 (la del aula)

Es la más ligera y la que usa la máquina del curso.

**Paso 1 — Descargar el toolchain.** Dos fuentes fiables:

- **[WinLibs](https://winlibs.com/)** — un `.zip`, sin instalador ni permisos de
  administrador. Descargad la variante **UCRT runtime · POSIX threads · x86_64**.
- **[MSYS2](https://www.msys2.org/)** — gestor de paquetes completo, si además
  queréis Clang o actualizar con `pacman`.

**Paso 2 — Descomprimir** en una ruta **sin espacios ni acentos**:

```
C:\mingw64\
```

`C:\Program Files\...` y `C:\Users\José\...` dan problemas con CMake y con
scripts. Es un consejo barato que ahorra una tarde.

**Paso 3 — Añadir `C:\mingw64\bin` al PATH**:

```powershell
# PowerShell, permanente y solo para tu usuario
[Environment]::SetEnvironmentVariable(
    "Path",
    [Environment]::GetEnvironmentVariable("Path", "User") + ";C:\mingw64\bin",
    "User")
```

Cerrad y abrid la terminal. Comprobad:

```powershell
g++ --version     # -> g++ (MinGW-W64 x86_64-ucrt-posix-seh ...) 14.2.0
gcov --version    # -> gcov (MinGW-W64 ...) 14.2.0   <- misma versión: bien
```

**Paso 4 — CMake y Ninja.** Instalador de
**[cmake.org/download](https://cmake.org/download/)**, marcando *"Add CMake to
the system PATH"*. Ninja suele venir dentro de MinGW; si no,
`winget install Ninja-build.Ninja`.

> ⚠️ **El fallo más desconcertante de Windows.** Si ejecutáis los tests desde
> **Git Bash**, el binario puede morir con `0xc0000139`
> (*STATUS_ENTRYPOINT_NOT_FOUND*) sin imprimir nada. No es vuestro código: Git
> trae sus propias `libstdc++-6.dll` y `libgcc_s_seh-1.dll`, y ensombrecen a las
> de `C:\mingw64`. Dos soluciones, las dos válidas:
>
> 1. Ejecutad los tests desde **PowerShell** o **cmd**, no desde Git Bash.
> 2. Enlazad el runtime estáticamente y el `.exe` queda autónomo:
>    ```cmake
>    if(MINGW)
>        add_link_options(-static -static-libgcc -static-libstdc++)
>    endif()
>    ```
>    Es lo que hace el Paint del curso anterior, y por eso a él no le pasa.

### 3.2 Opción B — MSVC (Visual Studio)

Si vuestra empresa ya compila con MSVC, usad esto y no MinGW.

**Paso 1** — Instalad **[Visual Studio Build Tools](https://learn.microsoft.com/en-us/cpp/build/vscpp-step-0-installation)**
(gratuito) marcando la carga de trabajo *"Desarrollo para el escritorio con
C++"*. Incluye MSVC, CMake y Ninja.

**Paso 2** — Trabajad siempre desde el *"Developer PowerShell for VS"*: es el
que pone `cl.exe` en el PATH.

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
```

**Con MSVC, el `--coverage` de gcov no existe.** Las alternativas, en §6.3.

### 3.3 vcpkg (opcional, para MSVC o MinGW)

```powershell
git clone https://github.com/microsoft/vcpkg C:\vcpkg
C:\vcpkg\bootstrap-vcpkg.bat
C:\vcpkg\vcpkg install gtest
```

Y al configurar el proyecto:

```powershell
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
```

Ficha del paquete: **[vcpkg.io/en/package/gtest](https://vcpkg.io/en/package/gtest)**.

---

## 4. Linux

Todo está en los repositorios oficiales:

```bash
# Debian / Ubuntu
sudo apt update
sudo apt install build-essential cmake ninja-build python3-pip gcovr

# Fedora / RHEL
sudo dnf install gcc-c++ cmake ninja-build python3-pip gcovr

# Arch
sudo pacman -S base-devel cmake ninja python-pip gcovr
```

> **No instaléis `libgtest-dev` del sistema.** Trae una versión antigua e
> incontrolada y, en Debian, ni siquiera la biblioteca compilada: solo las
> fuentes. Usad `FetchContent` (§5.1). Es exactamente el aviso de la tabla del
> bloque 6.

Los **sanitizers** vienen con GCC y Clang en Linux, sin instalar nada más.

---

## 5. macOS

```bash
xcode-select --install                  # Clang, make y las cabeceras del sistema
brew install cmake ninja gcovr
```

Clang de Apple trae **ASan y UBSan**, pero **no TSan** en todas las versiones.
`gcov` de Apple es un alias de `llvm-cov gcov`; funciona con gcovr, pero si os
da guerra, instalad GCC real con `brew install gcc` y usad `g++-14`.

---

## 6. GoogleTest + gMock: las cuatro vías

`gtest` y `gmock` **son el mismo paquete**. Instalando uno, tenéis los dos.

### 6.1 `FetchContent` — la del curso

No instala nada: CMake descarga y compila GoogleTest dentro de vuestro `build/`.

```cmake
include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.18.0          # etiqueta fija, nunca "main"
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)   # necesario en MSVC
FetchContent_MakeAvailable(googletest)

target_link_libraries(mi_test PRIVATE mi_lib GTest::gmock_main)
```

| Ventaja | Coste |
|---|---|
| Cero instalación; misma versión para todo el equipo y la CI | Requiere **red** la primera vez; compila GoogleTest una vez por proyecto |

Las etiquetas disponibles están en
**[github.com/google/googletest/releases](https://github.com/google/googletest/releases)**.
Desde la v1.15 el mínimo es **C++17**, que es la base del curso.

### 6.2 Instalarlo en el sistema — la del aula sin red

Compila GoogleTest **una sola vez** y lo deja disponible para todos los
proyectos. Es lo que tiene la máquina del curso.

```bash
git clone --depth 1 --branch v1.18.0 https://github.com/google/googletest.git C:/dev/googletest
cd C:/dev/googletest

cmake -S . -B build -G Ninja \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX=C:/mingw64 \
      -DBUILD_GMOCK=ON -DINSTALL_GTEST=ON

cmake --build build --parallel
cmake --install build
```

En Linux/macOS, cambiad el prefijo por `/usr/local` y añadid `sudo` al
`--install`.

Y el `CMakeLists.txt` del proyecto se queda en dos líneas:

```cmake
find_package(GTest REQUIRED)
target_link_libraries(mi_test PRIVATE mi_lib GTest::gmock_main)
```

> **Por qué el prefijo es el del compilador.** Instalando en `C:\mingw64`,
> `find_package(GTest)` lo encuentra **sin** `CMAKE_PREFIX_PATH` ni variables de
> entorno: CMake busca ahí por defecto. Si preferís no tocar el toolchain,
> instalad en `C:/dev/gtest` y configurad con
> `-DCMAKE_PREFIX_PATH=C:/dev/gtest`.

### 6.3 `FetchContent` contra un clon local

Lo mejor de los dos mundos cuando el aula se queda sin red: mantenéis el
`CMakeLists.txt` del curso y solo cambiáis la URL por una ruta.

```cmake
FetchContent_Declare(
  googletest
  GIT_REPOSITORY file:///C:/dev/googletest    # el clon de §6.2
  GIT_TAG        v1.18.0
)
```

### 6.4 Resumen de decisión

| Situación | Vía |
|---|---|
| Proyecto nuevo, hay red, equipo pequeño | **`FetchContent`** (§6.1) |
| Aula o CI sin red | **Instalado** (§6.2) o **clon local** (§6.3) |
| Ya gestionáis dependencias con vcpkg/Conan | **vcpkg** (§3.3) |
| Necesitáis parchear GoogleTest | Submódulo git + `add_subdirectory` |

---

## 7. Cobertura: gcov y gcovr

Son **dos piezas distintas** y conviene no confundirlas:

```
  g++ --coverage          ./mi_test              gcovr
 compila instrumentado -> genera .gcda   ->  lee .gcno/.gcda  ->  informe
    (crea los .gcno)      al ejecutarse        llamando a gcov     HTML/XML
```

- **`gcov`** es el programa de GCC que traduce los ficheros binarios de
  contadores a texto. **Ya lo tenéis**: viene con GCC, no se instala aparte.
- **`gcovr`** es un programa Python que llama a `gcov` por vosotros, agrega los
  resultados de todo el proyecto y saca HTML, XML (Cobertura), JSON o texto.

### 7.1 Instalar gcovr

```bash
pip install gcovr
gcovr --version        # -> gcovr 8.6
```

En Linux podéis usar el paquete del sistema (§4). En Windows, `pip` deja el
ejecutable en `...\Python3XX\Scripts\gcovr.exe`, que ya está en el PATH si
marcasteis *"Add Python to PATH"* al instalar Python.

Si `gcovr` no se encuentra tras instalarlo, invocadlo por módulo:

```bash
python -m gcovr --version
```

### 7.2 Comprobar que la cadena funciona

```bash
g++ --coverage -O0 -g -c calc.cpp -o calc.o   # -> aparece calc.gcno
# ... enlazar, ejecutar los tests ...         # -> aparece calc.gcda
gcovr --root . --txt
```

Si sale una tabla con porcentajes, la cadena entera está bien.

### 7.3 Cobertura con MSVC y con Clang

`--coverage` es de GCC. Los equivalentes:

| Compilador | Herramienta |
|---|---|
| GCC | `--coverage` + `gcov` + **gcovr** |
| Clang | `--coverage` (compatible con gcov), o `-fprofile-instr-generate -fcoverage-mapping` + `llvm-cov` |
| MSVC | **OpenCppCoverage** (gratuito) o *Code Coverage* de Visual Studio Enterprise |

---

## 8. Sanitizers: leed esto antes de intentarlo en Windows

Los sanitizers no se instalan: son **opciones del compilador** que enlazan una
biblioteca de runtime. El problema es que **esa biblioteca no siempre existe**.

```bash
g++ -fsanitize=address -g mi_test.cpp -o mi_test
```

### ⚠️ MinGW no trae sanitizers

Comprobado en la máquina del aula (GCC 14.2, MinGW-w64 UCRT):

```
ld.exe: cannot find -lasan: No such file or directory
ld.exe: cannot find -lubsan: No such file or directory
ld.exe: cannot find -ltsan: No such file or directory
```

No es un error de configuración: `libasan`, `libubsan` y `libtsan` **no se
distribuyen** en las builds de MinGW-w64. Alternativas reales en Windows:

| Alternativa | Qué da | Coste |
|---|---|---|
| **MSVC** `/fsanitize=address` | ASan completo (VS 2019 16.9+) | Cambiar de compilador |
| **MSYS2 `clang64`** (`pacman -S mingw-w64-clang-x86_64-clang`) | ASan y UBSan | Otro toolchain más |
| **WSL / Linux / Docker** | Los tres sanitizers, sin sorpresas | Una VM |
| **`-D_GLIBCXX_ASSERTIONS`** | Comprueba índices y precondiciones de la STL | Gratis, pero cubre mucho menos |

La última funciona en MinGW hoy mismo y es el mínimo decente si no podéis
cambiar de toolchain:

```cmake
target_compile_options(mi_lib PRIVATE $<$<CONFIG:Debug>:-D_GLIBCXX_ASSERTIONS>)
```

### En Linux y macOS

Funcionan sin instalar nada. La receta que usa el bloque 16:

```cmake
# ASan + UBSan juntos: se llevan bien. TSan va aparte.
target_compile_options(mi_test PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer -g)
target_link_options(mi_test    PRIVATE -fsanitize=address,undefined)
```

---

## 9. Verificación final: el proyecto de humo

Si esto compila, pasa y saca cobertura, tenéis el día 3 entero resuelto.

```
humo/
├── CMakeLists.txt
├── calc.h
├── calc.cpp
└── calc_test.cpp
```

```cpp
// calc.h
#pragma once
struct Reloj { virtual ~Reloj() = default; virtual int hora() const = 0; };
int  suma(int a, int b);
bool esDeManana(const Reloj& r);
```

```cpp
// calc.cpp
#include "calc.h"
int  suma(int a, int b) { return a + b; }
bool esDeManana(const Reloj& r) { return r.hora() < 12; }
```

```cpp
// calc_test.cpp  -- prueba gtest Y gmock a la vez
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "calc.h"

TEST(Suma, DosPositivos) { EXPECT_EQ(suma(2, 3), 5); }

class RelojMock : public Reloj {
public:
    MOCK_METHOD(int, hora, (), (const, override));
};

TEST(EsDeManana, AntesDeLasDoce) {
    RelojMock reloj;
    EXPECT_CALL(reloj, hora()).WillOnce(testing::Return(9));
    EXPECT_TRUE(esDeManana(reloj));
}
```

```cmake
cmake_minimum_required(VERSION 3.16)
project(humo LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(GTest REQUIRED)          # o el bloque FetchContent de §6.1
enable_testing()

add_library(calc calc.cpp)
target_include_directories(calc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(calc PRIVATE --coverage -O0 -g)
target_link_options(calc PUBLIC --coverage)      # PUBLIC: se propaga al test

add_executable(humo_test calc_test.cpp)
target_link_libraries(humo_test PRIVATE calc GTest::gmock_main)

if(MINGW)
    target_link_options(humo_test PRIVATE -static -static-libgcc -static-libstdc++)
endif()

include(GoogleTest)
gtest_discover_tests(humo_test)
```

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
gcovr --root . --filter '.*calc\.cpp' --txt --html-details build/cobertura.html
```

Salida esperada:

```
100% tests passed, 0 tests failed out of 2
------------------------------------------------------------
File          Lines    Exec   Cover   Missing
calc.cpp          4       3     75%   5
```

Ese 75 % no es un fallo del montaje: es que `esDeManana` solo se ha probado por
una de sus dos ramas. De eso trata el bloque 15.

---

## 10. Problemas frecuentes

| Síntoma | Causa | Solución |
|---|---|---|
| El test muere con `0xc0000139` y no imprime nada | DLL de MinGW ajenas en el PATH (típico: Git Bash) | Ejecutar desde PowerShell, o enlazar con `-static -static-libgcc -static-libstdc++` |
| `Could NOT find GTest` | No está instalado, o está en otro prefijo | `FetchContent` (§6.1), o `-DCMAKE_PREFIX_PATH=<prefijo>` |
| MSVC: `mismatch detected for 'RuntimeLibrary'` | gtest y vuestro proyecto con runtimes distintos | `set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)` **antes** de `FetchContent_MakeAvailable` |
| `gcovr`: `FileNotFoundError [WinError 2]` | Ruta a gcov con `\` en `--gcov-executable` | Barras normales: `C:/mingw64/bin/gcov.exe` |
| `gcovr`: *version mismatch* | `gcov` de distinta versión que `g++` | `gcovr --gcov-executable gcov-14` |
| Cobertura del 100 % sospechosa, o ningún `.gcda` | Compilado en Release, o `--coverage` solo al compilar | `-DCMAKE_BUILD_TYPE=Debug`, y `--coverage` **también al enlazar** |
| Los `assert` no saltan en los tests | `NDEBUG` lo define el modo Release | Compilar los tests en Debug, o `-UNDEBUG` en ese target |
| `cannot find -lasan` | MinGW no trae sanitizers | Ver §8 |
| Un test pasa solo y falla en la suite | Estado compartido entre tests | `--gtest_shuffle --gtest_repeat=3` y revisad el fixture (bloque 8) |

---

## 11. Referencias

**Instalación y toolchain (todo gratuito):**

- **[MSYS2](https://www.msys2.org/)** — la vía mantenida para tener GCC, Clang y
  `pacman` en Windows. Si vais a actualizar el compilador alguna vez, empezad
  aquí en lugar de por un `.zip`.
- **[WinLibs](https://winlibs.com/)** — MinGW-w64 en un zip, sin instalador ni
  permisos de administrador. Es lo que usa la máquina del aula.
- **[CMake — descargas](https://cmake.org/download/)** y
  **[módulo `FetchContent`](https://cmake.org/cmake/help/latest/module/FetchContent.html)**
  — la documentación de `FetchContent_Declare`, incluidas las opciones para
  trabajar sin red que usa §6.3.
- **[Visual Studio Build Tools](https://learn.microsoft.com/en-us/cpp/build/vscpp-step-0-installation)**
  — MSVC sin instalar el IDE completo. La alternativa de §3.2.

**Las herramientas del día 3:**

- **[GoogleTest — releases](https://github.com/google/googletest/releases)** — de
  aquí sale el `GIT_TAG`, y la matriz de compiladores soportados por versión.
  Consultadlo antes de subir de versión.
- **[gcovr — instalación](https://gcovr.com/en/stable/installation.html)** y
  **[configuración](https://gcovr.com/en/stable/guide/configuration.html)** —
  todas las opciones de §7, incluido el fichero `gcovr.cfg` para no repetir
  banderas en cada invocación.
- **[GCC — `gcov`](https://gcc.gnu.org/onlinedocs/gcc/Gcov.html)** — qué son
  exactamente los `.gcno` y `.gcda`, y por qué hay que pasar `--coverage` también
  al enlazar. Responde la mitad de la tabla de §10.
- **[GCC — opciones de instrumentación](https://gcc.gnu.org/onlinedocs/gcc/Instrumentation-Options.html)**
  — la lista oficial de `-fsanitize=`, útil para comprobar qué soporta vuestra
  build concreta antes de pelearos con el enlazador.
- **[vcpkg — paquete gtest](https://vcpkg.io/en/package/gtest)** — si vuestra
  empresa ya gestiona las dependencias así, esta es la ficha con las *triplets*
  disponibles.
