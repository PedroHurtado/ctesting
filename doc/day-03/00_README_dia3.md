# Día 3 — Dobles, medida y confianza para cambiar el código

Material de apoyo para la tercera jornada del curso **TDD con C++ (Google Test)**.

El día 2 terminó con tres dobles escritos a mano, una costura sin implementar y
una suite que estaba en verde sin comprobar nada. Hoy se cierran las tres cosas:
gMock genera los dobles, el `Reloj` de la kata por fin funciona, y la cobertura
pone un número al agujero del Paint. Y al final del día, con esa red montada,
tocamos el código a propósito.

## Estructura del día

| #  | Bloque                                                         | Fichero                        |
|----|----------------------------------------------------------------|--------------------------------|
| 0  | Repaso del día 2 y corrección del ejercicio                     | `00_README_dia3.md`            |
| 12 | Dobles de prueba: dummy, stub, spy, mock y fake                 | `12_dobles_de_prueba.md`       |
| 13 | gMock en la práctica: `MOCK_METHOD`, `EXPECT_CALL`, `ON_CALL`   | `13_gmock.md`                  |
| 14 | Tests paramétricos: `TEST_P` y `TYPED_TEST`                     | `14_tests_parametricos.md`     |
| 15 | Cobertura de código con `gcov` y `gcovr`                        | `15_cobertura.md`              |
| 16 | Sanitizers: ASan, UBSan y TSan                                  | `16_sanitizers.md`             |
| 17 | Malos olores, refactorización y la IA como apoyo                | `17_smells_refactor_ia.md`     |

La numeración continúa la del día 2 a propósito: el material de los cuatro días
es **un solo documento repartido en carpetas**.

Fuera de la carpeta, transversal a todo el curso:
**[`../00_instalacion.md`](../00_instalacion.md)** — cómo se instala cada
herramienta, por sistema operativo, y qué hacer cuando algo no compila.

## El ejercicio del día 2, corregido

Quedó propuesto al final de la kata. La solución, en el orden en que salía sola:

```
[x] Una cuenta nueva tiene el historial vacío       ← el trivial: arranca el motor
[x] historial() devuelve los movimientos en orden cronológico
[x] Transferir mueve saldo de una cuenta a otra
[x] Una transferencia fallida no altera NINGUNA de las dos cuentas
```

```cpp
TEST(Cuenta, UnaCuentaNuevaTieneElHistorialVacio) {
    Cuenta c;
    EXPECT_THAT(c.historial(), IsEmpty());
}

TEST(Cuenta, ElHistorialConservaElOrdenCronologico) {
    Cuenta c;
    c.ingresar(100_eur);
    c.retirar(30_eur);
    EXPECT_THAT(c.historial(), ElementsAre(Ingreso{100_eur}, Retirada{30_eur}));
}

TEST(Transferencia, MueveElSaldoEntreLasDosCuentas) {
    Cuenta origen, destino;
    origen.ingresar(100_eur);

    transferir(origen, destino, 40_eur);

    EXPECT_EQ(60_eur,  origen.saldo());
    EXPECT_EQ(40_eur, destino.saldo());
}

TEST(Transferencia, SiFallaNoAlteraNingunaDeLasDosCuentas) {
    Cuenta origen, destino;
    origen.ingresar(50_eur);

    EXPECT_THROW(transferir(origen, destino, 80_eur), SaldoInsuficiente);

    EXPECT_EQ(50_eur, origen.saldo());      // ← las DOS, no solo la de origen
    EXPECT_EQ(0_eur,  destino.saldo());
}
```

Los dos puntos donde todo el mundo tropieza, y por qué importan:

- **La atomicidad se prueba desde fuera.** El último test no mira cómo está
  implementada `transferir`; mira que, tras el fallo, **los dos saldos** siguen
  donde estaban. Esa es la diferencia entre probar comportamiento y probar
  implementación, y hoy la vais a necesitar entera en el bloque 13.
- **`ElementsAre` en vez de un bucle.** Un `for` con `EXPECT_EQ` dentro dice
  *"algo del historial está mal"*; `ElementsAre` dice *qué* elemento y *en qué
  posición*. Volvemos a ello en el bloque 14, donde el mismo argumento justifica
  `TEST_P`.

## Lo que traemos del día 2

Hoy no se explican otra vez, pero se usan en cada línea:

| Del día 1 y 2                        | Dónde aparece hoy                                        |
|--------------------------------------|----------------------------------------------------------|
| **DIP y costuras**                   | Todo el bloque 12: la costura es donde va el doble.       |
| **FIRST** (*Fast*, *Independent*, *Repeatable*) | La justificación de doblar el reloj y la red (12). |
| **Matchers** de `EXPECT_THAT` (bloque 7) | Los mismos, dentro de `EXPECT_CALL` (13).             |
| **Clases de equivalencia y frontera** (bloque 9) | La tabla que el bloque 14 escribe una sola vez. |
| **El `NDEBUG` del Paint** (bloque 11) | El caso que abre el bloque 15: verde y sin comprobar nada.|
| **Los tres dobles a mano** (bloque 11)| Rehechos con `MOCK_METHOD` en el bloque 13.               |
| **El séptimo caso de la kata** (bloque 10) | Implementado con gMock en el bloque 13.              |
| **"Verificar en vez de razonar"** (bloque 11) | La regla que enmarca el uso de IA en el bloque 17. |

Y se cumplen las promesas que quedaron escritas:

| Promesa | Dónde se cumple |
|---|---|
| *"gMock genera mañana estos dobles"* | Bloque 13 |
| *"`adds` es un `EXPECT_CALL(...).Times(2)` esperando a nacer"* | Bloque 13 |
| *"el `Reloj` es el bloque con el que abre mañana"* | Bloque 13 |
| *"esta tabla, escrita una vez"* (bloque 9) | Bloque 14 |
| *"el día 3 la medimos de verdad, incluida la de ramas"* | Bloque 15 |
| *"sanitizers el día 3"* | Bloque 16 |

## Cómo enfocar cada bloque

El mismo esquema de siempre:

1. **El problema** que resuelve, en una frase.
2. **Definición** o enunciado formal.
3. **Bad**: código que duele.
4. **Good**: la alternativa, y por qué es mejor.
5. **Qué ganamos** y **qué pagamos**.
6. **Mantra** del bloque.

## Entorno de trabajo

- **C++17** y **GoogleTest v1.18.0**, igual que ayer. `GTest::gmock_main` ya
  estaba enlazado desde el bloque 6: **el `CMakeLists.txt` no se toca** para usar
  gMock.
- **`gcovr`** (bloque 15): `pip install gcovr`. `gcov` ya viene con GCC.
- ⚠️ **Sanitizers (bloque 16): MinGW no los trae.** Está comprobado en la máquina
  del aula — `cannot find -lasan`. El bloque 16 los enseña sobre Linux/WSL y
  explica las alternativas en Windows. No perdáis la mañana peleándoos con el
  enlazador.
- Todo el montaje, paso a paso y por sistema operativo, en
  **[`../00_instalacion.md`](../00_instalacion.md)**.

## Lo que viene después

- **Día 4** — Refactorización avanzada sobre el Paint, empezando por *Replace
  Primitive with Object* (el tipo `Dinero` prometido en la kata); después
  **ATDD/BDD**: historias de usuario, criterios de aceptación, escenarios
  **Gherkin** y **cucumber-cpp**. Y el cierre del curso.

El puente es directo: hoy hemos aprendido a demostrar que un cambio no rompe
nada. Mañana usamos esa red para cambios grandes de verdad, y luego subimos un
nivel — de *"¿está bien construido?"* a *"¿es lo que el cliente pidió?"*.

## Bibliografía del día

Lista maestra de la jornada; cada bloque repite abajo solo lo que le afecta.

### Documentación de trabajo (gratuita, tenedla abierta)

| Enlace | Para qué |
|---|---|
| **[gMock for Dummies](https://google.github.io/googletest/gmock_for_dummies.html)** | El tutorial del bloque 13, de principio a fin. |
| **[gMock Cheat Sheet](https://google.github.io/googletest/gmock_cheat_sheet.html)** | Matchers, cardinalidades y acciones en una página. Consulta diaria. |
| **[gMock Cookbook](https://google.github.io/googletest/gmock_cook_book.html)** | *Move-only types*, métodos no virtuales, plantillas: el apartado 8 del bloque 13. |
| **[Value-Parameterized Tests](https://google.github.io/googletest/advanced.html#value-parameterized-tests)** | `TEST_P` y todos los generadores del bloque 14. |
| **[gcovr](https://gcovr.com/en/stable/)** | Todas las opciones del bloque 15, con ejemplos de informe. |
| **[GCC — `gcov`](https://gcc.gnu.org/onlinedocs/gcc/Gcov.html)** | Qué son los `.gcno`/`.gcda` y por qué `--coverage` va también al enlazar. |
| **[AddressSanitizer](https://github.com/google/sanitizers/wiki/AddressSanitizer)** | Cómo se lee el informe del bloque 16. |
| **[refactoring.com — catálogo](https://refactoring.com/catalog/)** | La mecánica paso a paso de cada refactorización del bloque 17. |

### Lecturas que cambian el criterio (gratuitas)

- **[Martin Fowler — *Mocks Aren't Stubs*](https://martinfowler.com/articles/mocksArentStubs.html)**
  — la distinción que ordena el bloque 12 entero. Si solo leéis una cosa hoy.
- **[Martin Fowler — *Test Coverage*](https://martinfowler.com/bliki/TestCoverage.html)**
  — tres minutos que evitan una discusión de umbrales de dos horas.
- **[*Software Engineering at Google*, cap. 13 — *Test Doubles*](https://abseil.io/resources/swe-book/html/ch13.html)**
  — por qué a gran escala prefieren fakes a mocks. El contrapeso al entusiasmo
  por `EXPECT_CALL`.

### Libros

- 📖 Jeff Langr, **[*Modern C++ Programming with TDD*](https://pragprog.com/titles/lotdd/modern-c-programming-with-test-driven-development/)**
  — el libro de hoy. El cap. 5 es el bloque 13; los caps. 8-9, el bloque 17.
- 📖 Gerard Meszaros, **[*xUnit Test Patterns*](https://www.informit.com/store/xunit-test-patterns-refactoring-test-code-9780131495050)**
  — el catálogo del que salen los cinco dobles del bloque 12.
  **[Sitio gratuito](http://xunitpatterns.com/)** con las fichas resumidas.
- 📖 Martin Fowler, **[*Refactoring*](https://martinfowler.com/books/refactoring.html)** (2ª ed.)
  — el origen del bloque 17 y del día 4.
- 📖 Michael Feathers, **[*Working Effectively with Legacy Code*](https://www.informit.com/store/working-effectively-with-legacy-code-9780131177055)**
  — qué hacer cuando hay que cambiar código que no tiene tests. La respuesta
  empieza por las costuras del día 1.
