# Día 2 — Google Test en las manos

Material de apoyo para la segunda jornada del curso **TDD con C++ (Google Test)**.

Ayer fue teoría. Hoy **todo el código compila**. Cada bloque trae fragmentos que
se pueden teclear tal cual en el proyecto que montamos en el bloque 6, y la
jornada termina con una kata completa de Rojo → Verde → Refactor.

## Estructura del día

| # | Bloque                                                        | Fichero                     |
|---|---------------------------------------------------------------|-----------------------------|
| 0 | Repaso del día 1 y mapa del día                               | `00_README_dia2.md`         |
| 6 | Google Test desde cero: CMake, primer test, ejecución          | `06_google_test_setup.md`   |
| 7 | Assertions: `ASSERT_*`, `EXPECT_*`, matchers, excepciones      | `07_assertions.md`          |
| 8 | Fixtures y ciclo de vida de un test                            | `08_fixtures.md`            |
| 9 | Qué probar: equivalencia, frontera, roles, MITs y MIMs         | `09_que_probar.md`          |
| 10| Kata completa: `Cuenta` en Rojo → Verde → Refactor             | `10_kata_rgr.md`            |
| 11| Caso práctico: hacer testeable el Paint del curso anterior      | `11_caso_paint.md`          |

La numeración continúa la del día 1 a propósito: el material de los cuatro días
es **un solo documento repartido en carpetas**.

## Lo que traemos del día 1

Hoy no se explican otra vez, pero se usan en cada línea:

| Del día 1                        | Dónde aparece hoy                                    |
|----------------------------------|------------------------------------------------------|
| **AAA** (Arrange-Act-Assert)     | La forma de todos los tests, bloques 6 a 10.         |
| **FIRST**                        | *Independent* justifica los fixtures (bloque 8).     |
| **Right-BICEPS**                 | La lista de casos de la kata (bloque 10).            |
| **CORRECT**                      | Los valores frontera del bloque 9.                   |
| **Rojo → Verde → Refactor**      | El bloque 10 entero.                                 |
| **DIP y costuras**               | El bloque 11, sobre código real: `ICanvas`, `IReader`. |
| **FIRST, la *S* de *Self-validating*** | El bloque 11: una suite que no podía fallar.   |
| **Nombrar el test como una frase**| Todos los `TEST(...)` de hoy.                       |

Y las tres promesas que dejamos escritas ayer y hoy se cumplen:
`EXPECT_DOUBLE_EQ` / `EXPECT_NEAR` (bloque 7), fixtures (bloque 8) y clases de
equivalencia + valores frontera aplicadas sobre código real (bloques 9 y 10).

El bloque 11 cierra el día con el único material que no se teclea: el **Paint
del curso anterior**, al que le aplicamos lo de ayer y lo de hoy para poder
probarlo. Es la primera vez que el curso mira código que ya existía, y trae la
sorpresa de la jornada: su suite de tests estaba en verde y **no comprobaba
nada**.

## Cómo enfocar cada bloque

El mismo esquema de siempre:

1. **El problema** que resuelve, en una frase.
2. **Definición** o enunciado formal.
3. **Bad**: código que duele.
4. **Good**: la alternativa, y por qué es mejor.
5. **Qué ganamos** y **qué pagamos**.
6. **Mantra** del bloque.

## Entorno de trabajo

- **C++17**, igual que ayer.
- **GoogleTest v1.18.0** traído con `FetchContent` (bloque 6). Esa versión
  exige C++17 como mínimo: encaja exactamente con la base del curso.
- **CMake ≥ 3.14** (necesario para `FetchContent_MakeAvailable`).
- Compilador: GCC, Clang o MSVC, indistintamente.
- Hace falta **red la primera vez** que se configura el proyecto: `FetchContent`
  clona GoogleTest. Si no hay red en el aula, el bloque 6 explica la alternativa.

## Lo que viene después

- **Día 3** — Dobles de prueba con **gMock** (mocks, fakes, stubs) sobre las
  costuras que diseñamos ayer, tests paramétricos, cobertura con `gcov`/`gcovr`,
  sanitizers, y refactorización guiada por *code smells* (con IA como apoyo).
- **Día 4** — Refactor avanzado, ATDD/BDD, historias de usuario, escenarios
  Gherkin y `cucumber-cpp`.

El puente es directo: hoy la kata del bloque 10 se queda deliberadamente **sin
persistencia ni notificaciones**. Mañana se las añadimos, y ahí es donde harán
falta los dobles.

Y del bloque 11 nos llevamos tres dobles escritos a mano —`FakeCanvas`,
`StringReader`, `RecordingWriter`— que mañana genera gMock, y un `FakeCanvas`
que cuenta llamadas a pelo: eso es un `EXPECT_CALL(...).Times(2)` esperando a
nacer.

## Bibliografía del día

Lista maestra de la jornada; cada bloque repite abajo solo lo que le afecta.

### Documentación de trabajo (gratuita, tenedla abierta)

| Enlace | Para qué |
|---|---|
| **[GoogleTest — Primer](https://google.github.io/googletest/primer.html)** | La media hora de lectura que resume el bloque 6. |
| **[Quickstart: CMake](https://google.github.io/googletest/quickstart-cmake.html)** | El `CMakeLists.txt` del bloque 6, paso a paso. |
| **[Assertions Reference](https://google.github.io/googletest/reference/assertions.html)** | La tabla completa del bloque 7. Consulta diaria. |
| **[Matchers Reference](https://google.github.io/googletest/reference/matchers.html)** | Todo lo que cabe dentro de un `EXPECT_THAT`. |
| **[Testing Reference](https://google.github.io/googletest/reference/testing.html)** | `TEST_F`, `SetUp`, `SetUpTestSuite`: el bloque 8. |
| **[Advanced Guide](https://google.github.io/googletest/advanced.html)** | Death tests, `SCOPED_TRACE`, entornos globales. |
| **[GoogleTest FAQ](https://google.github.io/googletest/faq.html)** | Responde antes de preguntar: casi todo está ahí. |

### Libros

- 📖 Jeff Langr, **[*Modern C++ Programming with TDD*](https://pragprog.com/titles/lotdd/modern-c-programming-with-test-driven-development/)**
  — el libro de hoy. Caps. 1-3 cubren prácticamente toda la jornada.
- 📖 Kent Beck, **[*Test-Driven Development: By Example*](https://www.informit.com/store/test-driven-development-by-example-9780321146533)**
  — la parte I es la misma kata del bloque 10 en otro lenguaje.
- 📖 Gerard Meszaros, **[*xUnit Test Patterns*](https://www.informit.com/store/xunit-test-patterns-refactoring-test-code-9780131495050)**
  — el catálogo del que salen los fixtures del bloque 8.
  **[Sitio gratuito](http://xunitpatterns.com/)** con los patrones resumidos.
