# Día 5 — Refactorizar sin miedo y probar lo que pidió el cliente

Material de apoyo para la quinta y última jornada del curso **TDD con C++
(Google Test)**.

Hoy se cierran los dos temas que faltaban del temario:

- **Tema 6 — Refactorización de código.** El bloque 17 dio el catálogo y la
  regla. Hoy: los principios, el tipo `Dinero` que quedó prometido desde la
  kata del día 2, la simplificación de condicionales y un refactor completo con
  IA en el que los tests "de siempre" no bastan.
- **Tema 7 — Pruebas de aceptación (ATDD/BDD).** Subimos un nivel: de
  *"¿está bien construido?"* a *"¿es lo que pidió el cliente?"*. Historias de
  usuario, escenarios Gherkin y `cucumber-cpp` ejecutando C++ de verdad.

## Estructura del día

| #  | Bloque                                                      | Fichero                              |
|----|-------------------------------------------------------------|--------------------------------------|
| 0  | Mapa del día                                                | `00_README_dia5.md`                  |
| 19 | Principios y técnicas de refactorización: el tipo `Dinero`  | `19_principios_refactorizacion.md`   |
| 20 | Reutilizar y simplificar expresiones condicionales          | `20_condicionales.md`                |
| 21 | Refactorización con IA, paso a paso                         | `21_refactor_con_ia.md`              |
| 22 | ATDD, BDD e historias de usuario                            | `22_atdd_historias_usuario.md`       |
| 23 | Gherkin: escenarios en lenguaje de negocio                  | `23_gherkin.md`                      |
| 24 | cucumber-cpp: los escenarios ejecutando C++                 | `24_cucumber_cpp.md`                 |
| 25 | Cierre del curso                                            | `25_cierre_curso.md`                 |

La numeración sigue la de los días anteriores: el material es **un solo
documento repartido en carpetas**.

## Un solo hilo conductor: la `Cuenta`

Todo el día trabaja sobre la cuenta bancaria de la kata (día 2) y del
ejercicio del día 4. Así lo que cambia es el concepto, no el decorado:

```
  día 2: Cuenta con double  ──►  día 4: céntimos en long long
                                          │
               bloque 19: tipo Dinero  ◄──┘
                          │
               bloque 22: historia "comisión en cajero ajeno"
                          │
               bloque 23: escrita en Gherkin
                          │
               bloque 24: ejecutada con cucumber-cpp contra la Cuenta
```

## Lo que traemos de días anteriores

| De... | Dónde aparece hoy |
|---|---|
| **Los dos sombreros** del ciclo TDD (día 1) | Bloque 19 §2 |
| **Obsesión por primitivos** (bloque 17) | El `Dinero` del bloque 19 |
| **`TEST_P`** (bloque 14) | Tests de caracterización (20) y el `Esquema del escenario` (23) |
| **Cobertura de ramas** (bloque 15) | La prueba de que no basta (21) |
| **"Una afirmación no ejecutada es una hipótesis"** (bloque 17) | Todo el bloque 21 |
| **Criterios Dado/Cuando/Entonces** (ejercicio del día 4) | Convertidos en Gherkin (23) |
| **Atomicidad** de la kata (día 2) | El bug que caza el escenario (24) |

## El proyecto del día

**[`03-test/cajero/`](../../cajero/)** tiene todo el código del bloque 24 listo
para compilar: `Dinero`, `Cuenta`, los tests unitarios de GoogleTest, los tres
`.feature` y los pasos en C++. Cómo se compila y se ejecuta está en su
[`README.md`](../../cajero/README.md).

- **Windows (MinGW):** compila y pasa los 11 tests unitarios con `ctest`.
- **Linux / WSL:** además, `./aceptacion.sh` ejecuta los 9 escenarios.

## Todo el código se ha ejecutado

Cada fragmento de código de hoy **se ha compilado y ejecutado**, y las salidas
que aparecen en los documentos son reales:

- Bloques 19-21: C++17 con MinGW GCC 14.2 y GoogleTest 1.18 (Windows), y
  `gcovr` para la cobertura del bloque 21.
- Bloque 24: `cucumber-cpp` (rama `main`) + Cucumber-Ruby 7.1.0 en Linux (WSL),
  con GCC 13 y GoogleTest 1.14.

## Entorno de trabajo

- **Bloques 19-21:** el mismo de siempre. Nada nuevo que instalar.
- **Bloque 24:** ⚠️ `cucumber-cpp` necesita Ruby, dos gemas con versión fija y
  varias librerías. **En Windows usad WSL.** La instalación completa, sin
  permisos de administrador, está en el bloque 24 §3. Si no da tiempo a
  instalarlo, el bloque 24 §9 explica cómo trabajar igual solo con GoogleTest.

## Bibliografía del día

### Documentación de trabajo (gratuita, tenedla abierta)

| Enlace | Para qué |
|---|---|
| **[refactoring.com — catálogo](https://refactoring.com/catalog/)** | La mecánica de cada refactor de los bloques 19 y 20 |
| **[Referencia de Gherkin](https://cucumber.io/docs/gherkin/reference/)** | Todas las palabras clave del bloque 23 |
| **[cucumber-cpp](https://github.com/cucumber/cucumber-cpp)** | Instalación y ejemplos del bloque 24 |

### Lecturas que cambian el criterio (gratuitas)

- **[Dan North — *Introducing BDD*](https://dannorth.net/introducing-bdd/)**
  — el artículo que dio origen a BDD. Si solo leéis una cosa hoy.
- **[Wikipedia — *Characterization test*](https://en.wikipedia.org/wiki/Characterization_test)**
  — la idea que salva el refactor del bloque 21.
- **[Matt Wynne — *Introducing Example Mapping*](https://cucumber.io/blog/bdd/example-mapping-introduction/)**
  — la técnica del bloque 22 en cinco minutos.

### Libros

- 📖 Martin Fowler, **[*Refactoring*](https://martinfowler.com/books/refactoring.html)** (2ª ed.)
  — bloques 19 y 20.
- 📖 Gojko Adzic, **[*Specification by Example*](https://gojko.net/books/specification-by-example/)**
  — bloques 22 y 23.
- 📖 Steve Freeman y Nat Pryce,
  **[*Growing Object-Oriented Software, Guided by Tests*](https://www.informit.com/store/growing-object-oriented-software-guided-by-tests-9780321503626)**
  — el doble bucle del bloque 24.
