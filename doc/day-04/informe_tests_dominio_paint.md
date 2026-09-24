# Día 4 — Informe: tests del dominio del Paint

> **Qué hicimos:** poner en código los tests del **módulo 1 sin `Canvas`**:
> `Point`, el contrato `IShape`, `Circle` y `Square`. Seguimos el plan del
> [ejercicio del día 3](../day-03/ejercicio_plan_de_pruebas_paint.md) (§6.1-6.4).
> Después medimos la cobertura de esa parte.

---

## 1. El resultado en una línea

**22 tests en verde, 2 desactivados a propósito, 100 % de líneas, funciones y
ramas.** Para llegar hubo que tocar **una** línea de diseño en producción.

---

## 2. El cambio que pidió el test: `Point::operator==`

Sin igualdad, `EXPECT_EQ(Point{7, 8}, shape.Position())` **no compila**. Es el
hallazgo 1 del §10 del ejercicio, ahora pasando de verdad.

### Bad — comparar campo a campo en cada test

```cpp
EXPECT_EQ(7, shape.Position().x);   // ← dos EXPECT para UN hecho
EXPECT_EQ(8, shape.Position().y);   // ← y el mensaje de fallo no dice "posición"
```

### Good — la igualdad vive en el tipo

```cpp
// include/paint/Point.h — producción
inline bool operator==(const Point& a, const Point& b) {
    return a.x == b.x && a.y == b.y;
}
inline bool operator!=(const Point& a, const Point& b) { return !(a == b); }

// tests/test_domain.cpp — solo del test: gtest imprime "(7, 8)" al fallar
namespace paint {
void PrintTo(const Point& p, std::ostream* os) { *os << "(" << p.x << ", " << p.y << ")"; }
}
```

El `operator==` es **código de producción nuevo**, así que lleva sus propios
tests: por eso `Point` pasa de 1 test en el plan a 4 en el código.

---

## 3. Qué tests hay

| Bloque | Funciones escritas | Ejecuciones | Qué prueba |
|---|---:|---:|---|
| `Point` | 4 | 4 | Por defecto `{0, 0}`; `==` si coinciden; `!=` si difiere la x; `!=` si difiere la y |
| Contrato `IShape` | 6 | **12** | C1-C6 como `TYPED_TEST` contra `Circle` y `Square` |
| `Circle` | 3 + 1 `DISABLED_` | 3 | Constructor (AAA con *Act* vacío), π·r² con `EXPECT_NEAR`, radio 0 |
| `Square` | 3 + 1 `DISABLED_` | 3 | Constructor, lado² con `EXPECT_DOUBLE_EQ`, lado 0 |
| **Total** | **18** | **22** | |

El contrato se escribe **una vez**. Añadir un `Triangle` cuesta dos cosas: su
`Maker<Triangle>` y una palabra en la lista de tipos.

```cpp
using ShapeTypes = ::testing::Types<Circle, Square>;   // ← + Triangle, y hereda los 6
TYPED_TEST_SUITE(ShapeContract, ShapeTypes);
```

### Los dos `DISABLED_`: los huecos del día 3

```cpp
// HUECO — hoy Circle(-2.0, ...) construye y devuelve área positiva.
TEST(Circle, DISABLED_RadioNegativoLanzaInvalidArgument) {
    EXPECT_THROW(Circle(-2.0, 0, Point{}), std::invalid_argument);
}
```

Fijan **una** decisión (lanzar) sin romper la suite. Si les quitáis el
`DISABLED_`, se ponen en rojo: **eso es TDD sobre código heredado**. Si se
elige otra política (saturar a 0 o documentarlo), se cambian dos líneas.

---

## 4. La ejecución

```
[==========] Running 22 tests from 5 test suites.
[----------] 6 tests from ShapeContract/0, where TypeParam = paint::Circle
[----------] 6 tests from ShapeContract/1, where TypeParam = paint::Square
...
[ DISABLED ] Circle.DISABLED_RadioNegativoLanzaInvalidArgument
[ DISABLED ] Square.DISABLED_LadoNegativoLanzaInvalidArgument
[==========] 22 tests from 5 test suites ran. (0 ms total)
[  PASSED  ] 22 tests.

  YOU HAVE 2 DISABLED TESTS
```

La suite antigua con `<cassert>` (`paint_tests`) **sigue pasando** con el
`operator==` nuevo.

---

## 5. La cobertura, y la trampa de las ramas

> ⚠️ **La lección más importante del informe.** Pasamos del 50 % al 100 % de
> ramas **sin escribir un solo test y sin tocar el código**. Solo cambió
> *cómo se mide*. Y hacerlo fue correcto.

### 5.1 Primera medición: algo no cuadra

```
gcovr -r . --object-directory build-cov --filter ... -s
```

| Fichero | Líneas | Ramas | Líneas "sin cubrir" |
|---|---:|---:|---|
| `Point.h` | 100 % | 4/4 | — |
| `Circle.cpp` | 100 % | **11/22 — 50 %** | 23, 27, 28, 29, 30 |
| `Square.cpp` | 100 % | **11/22 — 50 %** | 20, 24, 25, 26, 27 |

Todas las líneas se ejecutan, pero falta la mitad de las ramas. Miremos esas
líneas de `Circle.cpp`:

```cpp
23    return std::make_unique<Circle>(*this);              // Clone()

27    return "Circle | area: " + std::to_string(Area()) +  // ToString()
28           " | color: " + std::to_string(color_) +
29           " | pos: (" + std::to_string(position_.x) + ", " +
30           std::to_string(position_.y) + ")";
```

**No hay ni un `if`.** Ni `?:`, ni `&&`, ni `switch`, ni bucles. Entonces,
¿de dónde salen las ramas?

### 5.2 Las ramas las pone el compilador, no el programador

En C++, **cualquier llamada que pueda lanzar tiene dos salidas**: vuelve
normalmente o lanza una excepción. GCC compila las dos, y gcov cuenta cada
una como una **rama**.

| Llamada | ¿Por qué puede lanzar? |
|---|---|
| `std::make_unique<Circle>(...)` | Hace un `new` → `std::bad_alloc` si no hay memoria |
| `std::to_string(...)` | Crea un `std::string` → reserva memoria |
| cada `+` entre `std::string` | Crea una cadena nueva → reserva memoria |

Si una de esas llamadas lanza a mitad del `ToString`, GCC tiene que destruir
los trozos de cadena ya construidos antes de dejar salir la excepción. **Ese
código de limpieza es el camino "no tomado"** que ve gcov.

Nuestros tests nunca se quedan sin memoria, así que siempre van por el camino
normal: **una rama de cada dos, 11 de 22, el 50 %**. `Square.cpp` es idéntico
y da exactamente el mismo número.

### Bad — perseguir el número

```cpp
// Para "cubrir" la línea 28 habría que hacer fallar la memoria justo ahí
void* operator new(std::size_t n) {                 // ← reemplazar el new GLOBAL
    if (++g_llamadas == 3) throw std::bad_alloc{};  // ← ¿3? depende de la STL
    return std::malloc(n);
}

TEST(Circle, ToStringSinMemoriaLanza) {
    Circle c(2.0, 3, Point{1, 1});
    EXPECT_THROW(c.ToString(), std::bad_alloc);     // ← ¿qué decisión NUESTRA prueba esto?
}
```

Este test:

- **no prueba nuestro código**: prueba `std::string` y el compilador;
- **es frágil**: el `3` depende de cuántas reservas haga la librería estándar
  por dentro, que cambia entre versiones de GCC;
- **no verifica ninguna decisión**: `Circle` no hace nada especial si falta
  memoria; solo deja pasar la excepción.

Recordad la regla del día 3: *no se prueban los datos, se prueban las
decisiones*. En esas cinco líneas no hay ninguna decisión nuestra.

### Good — medir solo las ramas que son nuestras

```
gcovr -r . --object-directory build-cov --filter ... --exclude-throw-branches -s
```

`--exclude-throw-branches` le dice a gcovr: **no cuentes las ramas que solo
existen porque algo puede lanzar**. Las que quedan son ramas de verdad.

| Fichero | Líneas | Funciones | Ramas |
|---|---:|---:|---:|
| `Point.h` | 100 % | 100 % | 4/4 |
| `Circle.cpp` | 100 % | 100 % | 11/11 |
| `Square.cpp` | 100 % | 100 % | 11/11 |
| **Total** | **30/30** | **17/17** | **26/26** |

Las 11 ramas "que faltaban" en cada fichero eran exactamente las 11 de
excepción.

### 5.3 Cómo diagnosticarlo vosotros: tres pasos

```
1. ¿Líneas al 100 % y ramas muy por debajo?        → sospecha
2. ¿Las líneas señaladas tienen if / ?: / && / || /
   switch / bucle?
       NO  → son ramas de excepción, no tuyas
       SÍ  → hay una decisión sin test: escríbelo
3. Mide otra vez con --exclude-throw-branches
       sube al 100 %      → no faltaba ningún test
       sigue faltando algo → ESO sí es un test pendiente
```

### 5.4 Cuándo NO usar `--exclude-throw-branches`

La opción no es gratis en todos los proyectos. Si el código **gestiona
excepciones él mismo**, esas ramas son decisiones vuestras y hay que verlas:

```cpp
void Guardar(const Pedido& p) {
    try {
        db_.Insertar(p);
    } catch (const DbError&) {
        cola_reintentos_.push(p);   // ← DECISIÓN nuestra: esto SÍ necesita test
    }
}
```

Aquí el camino de la excepción **no** es ruido del compilador: es
comportamiento que el cliente espera. Se prueba con un doble de `db_` que
lance (bloque 12). En el dominio del Paint no hay ni un `try`, así que
excluirlas no esconde nada.

### 5.5 Y el reverso: el 100 % no cubre los huecos

El radio negativo **no es una rama del código**: es una decisión que el código
**no tomó**. No hay línea que cubrir, así que la cobertura no puede avisar.
Por eso siguen ahí los dos `DISABLED_`.

> **Un 50 % de ramas no siempre es un test que falta.
> Un 100 % no siempre es un código terminado.
> Primero se pregunta *qué rama*, luego se escribe el test.**

---

## 6. Cómo reproducirlo

Desde `02-Patrones c++/paint`, en **PowerShell** (desde Git Bash, las DLL de
MinGW de Git pueden hacer abortar el `.exe`):

```powershell
cmake -S . -B build-cov -G Ninja -DCMAKE_BUILD_TYPE=Debug -DPAINT_COVERAGE=ON
cmake --build build-cov
.\build-cov\tests\domain_tests.exe

gcovr -r . --object-directory build-cov `
      --filter 'include/paint/Point\.h' --filter 'src/Circle\.cpp' --filter 'src/Square\.cpp' `
      --exclude-throw-branches -s
```

| Pieza | Dónde |
|---|---|
| Tests | `tests/test_domain.cpp` |
| Target `domain_tests` (`find_package(GTest)` + `gtest_discover_tests`) | `tests/CMakeLists.txt` |
| Opción `PAINT_COVERAGE` (`--coverage -O0`) | `CMakeLists.txt` raíz |
| Informe HTML | `gcovr ... --html-details build-cov/coverage/index.html` |

### Cómo se ejecutan los tests

Todo desde `02-Patrones c++/paint`, en PowerShell y con el proyecto ya
compilado.

| Quiero… | Comando |
|---|---|
| Toda la suite | `.\build-cov\tests\domain_tests.exe` |
| Solo una suite | `.\build-cov\tests\domain_tests.exe --gtest_filter='Circle.*'` |
| El contrato contra **un** tipo (`/0` = `Circle`, `/1` = `Square`) | `... --gtest_filter='ShapeContract/1.*'` |
| Todo menos una suite | `... --gtest_filter='-Point.*'` |
| Ver qué tests hay, sin ejecutarlos | `... --gtest_list_tests` |
| Salida corta: solo los fallos y el resumen | `... --gtest_brief=1` |
| Incluir los `DISABLED_` | `... --gtest_also_run_disabled_tests` |
| Con CTest (cada `TEST` sale como un test aparte) | `cd build-cov; ctest -R "Point\|ShapeContract\|Circle\|Square"` |

**Los `DISABLED_` en rojo.** Así se ven los huecos sin tocar el código:

```
> .\build-cov\tests\domain_tests.exe --gtest_filter='Circle.*' --gtest_also_run_disabled_tests

test_domain.cpp:178: Failure
Expected: Circle(-2.0, 0, Point{}) throws an exception of type std::invalid_argument.
  Actual: it throws nothing.

[  FAILED  ] Circle.DISABLED_RadioNegativoLanzaInvalidArgument (0 ms)
[  PASSED  ] 3 tests.
 1 FAILED TEST
```

**Con CTest** los desactivados no se cuentan como fallos, pero aparecen
listados:

```
100% tests passed, 0 tests failed out of 22

The following tests did not run:
     20 - Circle.RadioNegativoLanzaInvalidArgument (Disabled)
     24 - Square.LadoNegativoLanzaInvalidArgument (Disabled)
```

> ⚠️ No uséis `ctest` sin `-R` para medir cobertura: también ejecuta la suite
> antigua `paint_tests`, que pasa por `Canvas`, los comandos y la `App`, y los
> números del dominio dejan de ser solo del dominio.

---

## 7. Qué ganamos y qué pagamos

| Ganamos | Pagamos |
|---|---|
| El contrato `IShape` escrito una vez y válido para cualquier figura futura | `TYPED_TEST` y el `Maker<T>` cuestan leerlos la primera vez |
| `Point` comparable: tests más cortos y mensajes de fallo legibles | Un cambio en producción pedido por el test |
| Los dos huecos quedan **escritos**, no olvidados | Siguen sin decidirse: `DISABLED_` es un recordatorio, no una solución |
| Una cifra de cobertura de la que nos podemos fiar | Hay que saber leer las ramas: el 50 % inicial era ruido |

---

## 8. Mantra

> ### **El test pidió `operator==`, y el código salió mejor.**
> ### **El 100 % mide lo que el código hace; los huecos son lo que no decidió.**

---

## Referencias

- **[GoogleTest — *Typed Tests*](https://google.github.io/googletest/advanced.html)**
  (gratuito) — el mecanismo del contrato `IShape`; explica `TYPED_TEST_SUITE`
  y cuándo conviene la variante *type-parameterized*.
- **[GoogleTest — *Assertions Reference*](https://google.github.io/googletest/reference/assertions.html)**
  (gratuito) — `EXPECT_NEAR` frente a `EXPECT_DOUBLE_EQ` y cómo usar `PrintTo`
  para imprimir tipos propios.
- **[gcovr — opciones de línea de comandos](https://gcovr.com/en/stable/manpage.html)**
  (gratuito) — `--exclude-throw-branches`, `--filter` y `--html-details`, las
  tres que usamos.
- 📖 Michael Feathers, **[*Working Effectively with Legacy Code*](https://www.informit.com/store/working-effectively-with-legacy-code-9780131177055)**,
  cap. 13 — tests de caracterización: qué hacer cuando el código no decide
  (nuestros `DISABLED_`).
- Bloques del curso: **[Ejercicio del plan de pruebas](../day-03/ejercicio_plan_de_pruebas_paint.md)**
  (de dónde salen estos tests), **[14 — Paramétricos](../day-03/14_tests_parametricos.md)**
  (`TYPED_TEST`), **[15 — Cobertura](../day-03/15_cobertura.md)** (gcovr).
