# Día 1 — Fundamentos: agilidad, pruebas y diseño testable

Material de apoyo para la primera jornada del curso **TDD con C++ (Google Test)**.

Jornada **íntegramente teórica**. No escribimos aún tests con Google Test: hoy
construimos el vocabulario y los criterios que mañana aplicaremos con las manos
en el teclado. Todo el código que aparece hoy es **ilustrativo** y está pensado
para leerse en pantalla, no para compilarse.

## Estructura del día

| # | Bloque                                             | Fichero                       |
|---|----------------------------------------------------|-------------------------------|
| 0 | Presentación y mapa del curso                      | `00_README_dia1.md`           |
| 1 | Metodologías ágiles y XP                           | `01_agilidad_xp.md`           |
| 2 | Pruebas de software: tipos, límites y origen       | `02_pruebas_software.md`      |
| 3 | Diseño para poder testear                          | `03_diseno_testable.md`       |
| 4 | FIRST, BICEPS y CORRECT                            | `04_first_biceps_correct.md`  |
| 5 | El ciclo TDD: Rojo → Verde → Refactor              | `05_ciclo_tdd.md`             |

## Cómo enfocar cada bloque

Cada fichero sigue el mismo esquema que ya conocéis del curso de patrones:

1. **El problema** que resuelve, en una frase.
2. **Definición** o enunciado formal.
3. **Bad**: código o práctica que ilustra el problema.
4. **Good**: la alternativa, y por qué es mejor.
5. **Qué ganamos** y **qué pagamos**.
6. **Mantra** del bloque.

## Mapa de los cuatro días

- **Día 1 (hoy)** — Teoría: agilidad y XP, tipos de prueba, diseño testable,
  qué es un buen test (FIRST / BICEPS / CORRECT), el ciclo Rojo-Verde-Refactor.
- **Día 2** — Google Test en serio: assertions, fixtures, ciclo R-G-R práctico,
  diseño de casos de prueba (clases de equivalencia, valores frontera),
  roles en pruebas, MITs y MIMs.
- **Día 3** — Dobles de prueba con gMock (mocks, fakes, stubs), tests
  paramétricos, cobertura con `gcov`/`gcovr`, refactorización y *code smells*
  (incluyendo IA como apoyo al refactor).
- **Día 4** — Refactor avanzado, ATDD/BDD, historias de usuario, escenarios
  Gherkin, `cucumber-cpp` y cierre del curso.

## Conexión con el curso anterior (SOLID y patrones)

Este curso es la **continuación natural** del de patrones. Lo que allí era
"buen diseño por elegancia", aquí se convierte en "buen diseño **porque si no,
no puedes testear**":

| Lo que vimos en patrones           | Cómo reaparece en TDD                              |
|------------------------------------|----------------------------------------------------|
| **SRP**                            | Una clase con una responsabilidad → un test claro. |
| **OCP**                            | Añadir casos sin reescribir los tests existentes.  |
| **LSP**                            | Un mismo test debe pasar para todas las derivadas. |
| **ISP**                            | Interfaces pequeñas → mocks pequeños.              |
| **DIP**                            | Inyectar dependencias → poder sustituirlas en test.|
| **Strategy / Adapter / Facade**    | Costuras (*seams*) por donde entra el test.        |
| `unique_ptr<Base>` y composición   | El mecanismo real de inyección de dobles.          |

El mensaje de fondo: **un código difícil de testear es un código mal diseñado**.
El test no es un trámite posterior; es el primer cliente de tu diseño.

## Versión de C++ y herramientas

- **C++17** como base (usaremos `std::optional`, `if constexpr` puntualmente).
- **Google Test / Google Mock** a partir del día 2.
- **CMake** para la construcción, igual que en el curso de patrones.
- Compilador: GCC/Clang o MSVC, indistintamente.

---

## Bibliografía del curso

Lista maestra. Cada bloque repite abajo solo lo que le afecta.

### Libros de referencia

| 📖 Libro | Por qué |
|---|---|
| Kent Beck, **[*Test-Driven Development: By Example*](https://www.informit.com/store/test-driven-development-by-example-9780321146533)** (2002) | El libro fundacional. Corto y todo ejemplos. |
| Jeff Langr, **[*Modern C++ Programming with TDD*](https://pragprog.com/titles/lotdd/modern-c-programming-with-test-driven-development/)** (2013) | El único de TDD escrito para C++. Usa Google Test. |
| Michael Feathers, **[*Working Effectively with Legacy Code*](https://www.informit.com/store/working-effectively-with-legacy-code-9780131177055)** (2004) | Cómo meter tests en código que no los tiene. Muchos ejemplos en C++. |
| Gerard Meszaros, **[*xUnit Test Patterns*](https://www.informit.com/store/xunit-test-patterns-refactoring-test-code-9780131495050)** (2007) | Catálogo de patrones y antipatrones de test. [Sitio gratuito](http://xunitpatterns.com/). |
| Martin Fowler, **[*Refactoring*, 2ª ed.](https://martinfowler.com/books/refactoring.html)** (2018) | El catálogo de refactorizaciones ([resumen gratuito](https://refactoring.com/catalog/)). |
| Freeman & Pryce, **[*Growing OO Software, Guided by Tests*](http://www.growing-object-oriented-software.com/)** (2009) | TDD como técnica de diseño, no de verificación. |
| Hunt & Thomas, **[*Pragmatic Unit Testing*](https://pragprog.com/titles/utj2/pragmatic-unit-testing-in-java-8-with-junit/)** | Origen de **Right-BICEP** y **CORRECT** (bloque 4). |
| Robert C. Martin, **[*Clean Code*](https://www.informit.com/store/clean-code-a-handbook-of-agile-software-craftsmanship-9780132350884)** (2008) | Capítulo 9: origen de **FIRST**. |

### En línea (gratuito)

- **[GoogleTest — documentación oficial](https://google.github.io/googletest/)** — la usaremos a diario desde mañana.
- **[James Shore, *The Art of Agile Development*, 2ª ed.](https://www.jamesshore.com/v2/books/aoad2)** — libro completo y gratuito en la web.
- **[*Software Engineering at Google*, cap. 11 "Testing Overview"](https://abseil.io/resources/swe-book/html/ch11.html)** — cómo se prueba a escala real.
- **[C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines)** — referencia de estilo moderno para todo el código del curso.
