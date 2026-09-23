# Día 3 — Bloque 14: Tests paramétricos

> *"La tabla se escribe una vez; los casos se añaden en una línea."*

---

## 1. El problema en una frase

La suite del bloque 9 tenía **diez tests idénticos** salvo por dos números, y
añadir un tramo de tarifa obliga a copiar y pegar otros tres.

## 2. Definición

Un **test paramétrico** es un test escrito una sola vez que GoogleTest ejecuta
con *N* juegos de datos, **registrando cada juego como un caso independiente**.
Esa última parte es la que importa: si fallan tres, os enteráis de los tres.

Tres piezas:

```cpp
class Tarifa : public TestWithParam<Caso> { };   // 1. fixture con parámetro
TEST_P(Tarifa, CobraLoQueDiceLaTabla) { ... }    // 2. el test, una vez
INSTANTIATE_TEST_SUITE_P(Tramos, Tarifa, ValuesIn(casos));  // 3. los datos
```

---

## 3. Bad — el bucle dentro del test

La tentación evidente cuando uno ve diez tests casi iguales:

```cpp
TEST(Tarifa, TodosLosTramos) {
    struct { double peso; double esperado; } casos[] = {
        {0.01, 3.00}, {5.0, 3.00}, {5.01, 7.50},
        {20.0, 7.50}, {20.01, 12.00}, {50.0, 12.00},
    };

    for (const auto& c : casos)
        EXPECT_DOUBLE_EQ(c.esperado, tarifa(c.peso));   // ← ¿cuál ha fallado?
}
```

Parece que resuelve el problema. No lo resuelve:

- **Un solo caso en el informe.** `[ FAILED ] Tarifa.TodosLosTramos`. ¿Cuál de
  los seis? A mirar el `stdout`.
- **Con `ASSERT_*` en vez de `EXPECT_*`, el primer fallo aborta** y los cinco
  restantes no se ejecutan: perdéis información justo cuando más la necesitáis.
- **No se puede filtrar.** `--gtest_filter=*20*` no aísla el caso del límite.
- **Salta la *S* de FIRST**: el test deja de ser *Self-validating* de un vistazo.

> El parche clásico es `SCOPED_TRACE` dentro del bucle (bloque 7), que sí dice
> qué iteración falló. Es una mejora real, pero sigue siendo **un solo caso**
> para `ctest`.

## 4. Good — `TEST_P`

```cpp
#include <gmock/gmock.h>
using ::testing::TestWithParam;
using ::testing::ValuesIn;

struct Caso {                       // la struct que el bloque 9 dejó escrita
    double      peso;
    double      esperado;
    const char* porque;             // ← el nombre del caso en el informe
};

class TarifaTest : public TestWithParam<Caso> {};

TEST_P(TarifaTest, CobraSegunElTramo) {
    const Caso& c = GetParam();                     // ← el juego de datos actual
    EXPECT_DOUBLE_EQ(c.esperado, tarifa(c.peso));
}

const Caso kCasos[] = {
    {0.01,  3.00, "MinimoPesoValido"},
    {2.50,  3.00, "DentroDelPrimerTramo"},
    {5.00,  3.00, "CincoExactosSigueEnElPrimerTramo"},   // ← la frontera que mata bugs
    {5.01,  7.50, "PasadosLosCincoSaltaAlSegundo"},
    {20.00, 7.50, "VeinteExactosSigueEnElSegundo"},
    {20.01, 12.00, "PasadosLosVeinteSaltaAlTercero"},
    {50.00, 12.00, "MuyPorEncima"},
};

INSTANTIATE_TEST_SUITE_P(
    Tramos, TarifaTest, ValuesIn(kCasos),
    [](const auto& info) { return info.param.porque; });   // ← los nombres
```

Y el informe, ejecutado contra la implementación con el **bug de frontera del
bloque 9** (`if (peso < 5.0)` donde debía decir `<=`):

```
[       OK ] Tramos/TarifaTest.CobraSegunElTramo/MinimoPesoValido
[  FAILED  ] Tramos/TarifaTest.CobraSegunElTramo/CincoExactosSigueEnElPrimerTramo
[       OK ] Tramos/TarifaTest.CobraSegunElTramo/PasadosLosCincoSaltaAlSegundo
[  FAILED  ] Tramos/TarifaTest.CobraSegunElTramo/VeinteExactosSigueEnElSegundo
[       OK ] Tramos/TarifaTest.CobraSegunElTramo/PasadosLosVeinteSaltaAlTercero

 2 FAILED TESTS
```

**Los dos que fallan son exactamente las dos fronteras.** No "algo de la tarifa
está mal": los dos casos concretos, con nombre, y los otros cinco confirmando
que el resto sigue bien. Eso es lo que el bucle del apartado 3 no puede daros.

Siete casos independientes, cada uno con nombre propio, filtrables uno a uno:

```bash
./tarifa_test --gtest_filter=*CincoExactos*
```

**Añadir un tramo nuevo es ahora una línea en `kCasos`.** Ese es todo el
beneficio, y es enorme cuando la tabla tiene treinta filas.

> ⚠️ **El generador de nombres no es opcional en la práctica.** Sin la lambda,
> GoogleTest numera: `Tramos/TarifaTest.CobraSegunElTramo/0`, `/1`, `/2`… Un
> informe de CI lleno de `/4` no le dice nada a nadie. Y ojo: el nombre solo
> puede llevar **letras, dígitos y `_`**; ni espacios, ni acentos, ni puntos.

### El detalle que afea el informe

Al fallar, GoogleTest intenta imprimir el parámetro. Con una `struct` propia no
sabe cómo, y suelta esto:

```
[  FAILED  ] .../CincoExactosSigueEnElPrimerTramo, where GetParam() =
             24-byte object <00-00 00-00 00-00 14-40 00-00 ...>
```

Se arregla enseñándole a imprimirla. Una función y desaparece el volcado:

```cpp
std::ostream& operator<<(std::ostream& os, const Caso& c) {
    return os << c.porque << " (peso=" << c.peso << ", esperado=" << c.esperado << ")";
}
```

```
[  FAILED  ] .../CincoExactosSigueEnElPrimerTramo, where GetParam() =
             CincoExactosSigueEnElPrimerTramo (peso=5, esperado=3)
```

---

## 5. De dónde salen los parámetros

| Generador | Produce | Ejemplo |
|---|---|---|
| `Values(a, b, c)` | esos valores | `Values(1, 2, 3)` |
| `ValuesIn(contenedor)` | un array, vector o rango | `ValuesIn(kCasos)` |
| `Range(inicio, fin[, paso])` | `[inicio, fin)` | `Range(0, 10)` → 0…9 |
| `Bool()` | `false`, `true` | banderas |
| `Combine(g1, g2)` | **el producto cartesiano** | `Combine(Bool(), Range(1, 4))` → 6 casos |

`Combine` es la herramienta del **análisis por pares** del bloque 9… y también
su trampa: `Combine` de cuatro generadores de cinco valores son **625 casos**.
La técnica pairwise existía precisamente para no ejecutarlos todos. Usad
`Combine` cuando el producto sea pequeño; con más, generad la tabla reducida y
pasadla con `ValuesIn`.

```cpp
class DescuentoTest : public TestWithParam<std::tuple<bool, int>> {};

TEST_P(DescuentoTest, NuncaSuperaElCienPorCien) {
    const auto [esVip, cupones] = GetParam();
    EXPECT_LE(descuento(esVip, cupones), 100);
}

INSTANTIATE_TEST_SUITE_P(Matriz, DescuentoTest,
                         Combine(Bool(), Range(0, 4)));   // 2 x 4 = 8 casos
```

---

## 6. Cuándo **no** usar `TEST_P`

No es gratis, y no todo test tabular quiere ser paramétrico:

| Situación | Qué usar |
|---|---|
| Mismos *asserts*, solo cambian los datos | **`TEST_P`** |
| Cada caso afirma **algo distinto** (uno lanza, otro devuelve, otro no toca el mock) | `TEST` separados |
| Dos o tres casos en total | `TEST` separados: más legibles, cero andamio |
| El nombre del test es la mejor documentación del requisito | `TEST` separados |

> La regla práctica: **si el cuerpo del `TEST_P` necesita un `if` sobre el
> parámetro, el test paramétrico era el instrumento equivocado.** Estáis
> metiendo dos tests distintos en uno.

En el ejemplo del bloque 9, los casos de excepción (`tarifa(0.0)`,
`tarifa(-1.0)`) se quedan como `TEST` propios: afirman `EXPECT_THROW`, no un
valor. Mezclarlos en la tabla obligaría a ese `if`.

---

## 7. El primo hermano: `TYPED_TEST`

`TEST_P` varía **los datos**. `TYPED_TEST` varía **el tipo**: escribís el test
una vez y GoogleTest lo ejecuta contra varias implementaciones.

Es la forma de probar que **todas las implementaciones de una interfaz cumplen
el mismo contrato** —incluido el fake—:

```cpp
template <typename T> class ContratoCanvas : public Test {};
using Implementaciones = ::testing::Types<Canvas, FakeCanvas>;
TYPED_TEST_SUITE(ContratoCanvas, Implementaciones);

TYPED_TEST(ContratoCanvas, AlAnadirCreceElRecuento) {
    TypeParam canvas;                          // ← Canvas real y FakeCanvas
    canvas.Add(std::make_unique<Circle>());
    EXPECT_EQ(1u, canvas.Count());
}
```

Esto responde a la pregunta incómoda del bloque 12: *"¿y quién prueba el
fake?"*. **Este test.** El mismo contrato, verificado contra los dos.

---

## 8. Qué ganamos y qué pagamos

**Ganamos:** la tabla de equivalencia y frontera del bloque 9 deja de ser
copia-pega; añadir un caso cuesta una línea; cada caso sigue siendo
independiente y filtrable; y la tabla de datos, aislada, se lee como una
especificación.

**Pagamos:** más ceremonia (fixture, generador, nombres); mensajes de error de
plantilla bastante feos cuando algo no encaja; y la tentación de meter en la
tabla casos que no pertenecen a ella, que acaba en el `if` del apartado 6.

## 9. Conexión con lo que viene

Ya sabemos escribir muchos casos baratos. Falta la pregunta que nadie ha hecho
todavía: **¿qué partes del código no ha tocado ni uno solo de ellos?** El
**bloque 15** lo responde con números —y de paso mide si las fronteras de esta
tabla estaban realmente cubiertas, o solo lo parecía—.

## 10. Mantra del bloque

> **"Un caso, una línea."**
> Si añadir un caso de prueba cuesta más de una línea, el test acabará sin
> cubrir el caso que importaba.

---

## 11. Referencias

**Para escribirlos (gratuito):**

- **[GoogleTest — *Value-Parameterized Tests*](https://google.github.io/googletest/advanced.html#value-parameterized-tests)**
  — la guía oficial completa: `TEST_P`, todos los generadores del apartado 5 y
  el generador de nombres. Es la fuente directa de este bloque.
- **[GoogleTest — *Typed Tests*](https://google.github.io/googletest/advanced.html#typed-tests)**
  — `TYPED_TEST` y `TYPED_TEST_P` (la variante para contratos reutilizables
  entre ficheros), que es el apartado 7 en detalle.
- **[Testing Reference](https://google.github.io/googletest/reference/testing.html)**
  — la referencia formal de `TestWithParam`, `GetParam`,
  `INSTANTIATE_TEST_SUITE_P` y sus argumentos. Consulta rápida.
- **[Matchers Reference](https://google.github.io/googletest/reference/matchers.html)**
  — porque un `TEST_P` con `EXPECT_THAT` y un matcher en la tabla suele ser más
  expresivo que un `EXPECT_EQ` por columna.

**Para decidir qué poner en la tabla:**

- 📖 Andy Hunt y Dave Thomas,
  **[*Pragmatic Unit Testing*](https://pragprog.com/titles/utj2/pragmatic-unit-testing-in-java-8-with-junit/)**
  — de aquí salen **Right-BICEPS** y **CORRECT** (día 1), que son exactamente el
  criterio para decidir qué filas merecen estar en `kCasos`. El lenguaje da
  igual: las listas son universales.
- 📖 Jeff Langr,
  **[*Modern C++ Programming with TDD*](https://pragprog.com/titles/lotdd/modern-c-programming-with-test-driven-development/)**,
  cap. 7 — tests dirigidos por datos en C++ y cuándo dejan de compensar; cubre
  la tabla del apartado 6.
