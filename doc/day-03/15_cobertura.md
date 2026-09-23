# Día 3 — Bloque 15: Cobertura de código con gcov y gcovr

> *"La cobertura no mide lo que has probado. Mide lo que has ejecutado."*

---

## 1. El problema en una frase

La suite del Paint (bloque 11) estaba **en verde y no comprobaba nada**, y nadie
lo notó durante un curso entero.

## 2. Definición

La **cobertura de código** mide qué partes del código de producción se han
ejecutado mientras corrían los tests. No mide si estaban bien, ni si alguien las
comprobó: **solo si pasó el flujo por ahí**.

Hay varios tipos, y la diferencia entre los dos primeros es todo este bloque:

| Tipo | Qué cuenta | ¿Lo da gcov? |
|---|---|---|
| **De línea** | Líneas ejecutadas / líneas ejecutables | Sí (por defecto) |
| **De rama** | Salidas de cada `if`, `while`, `?:`, `&&` tomadas | Sí, con `--txt-metric branch` |
| **De función** | Funciones llamadas al menos una vez | Sí |
| **De decisión / MC-DC** | Cada condición individual de una decisión compuesta | Parcial (`--decisions`); exigido en avionica/automoción |
| **De camino** | Combinaciones de rutas por la función | No: crecen exponencialmente |

### La mecánica, en tres pasos

```
   g++ --coverage          ./mi_test              gcovr
 compila instrumentado ->  cuenta al           ->  agrega y formatea
   genera los .gcno        ejecutar: .gcda        (llama a gcov por dentro)
```

- **`.gcno`** — el mapa del código, lo crea el **compilador**.
- **`.gcda`** — los contadores, los escribe el **programa al terminar**.
- Si no aparece ningún `.gcda`, el binario no llegó a terminar bien, o
  `--coverage` faltaba **al enlazar**.

---

## 3. El montaje

### En crudo

```bash
g++ --coverage -O0 -g -c tarifa.cpp -o tarifa.o
g++ --coverage -O0 -g tarifa_test.cpp tarifa.o -lgtest -lgtest_main -o tarifa_test
./tarifa_test
gcovr --root . --txt
```

`-O0` importa: con optimizaciones, el compilador funde y reordena líneas y los
números dejan de corresponderse con el fichero que estáis leyendo.

### En CMake

```cmake
option(ENABLE_COVERAGE "Instrumentar para gcov" OFF)

if(ENABLE_COVERAGE AND NOT MSVC)
    target_compile_options(tarifa_lib PRIVATE --coverage -O0 -g)
    target_link_options(tarifa_lib    PUBLIC  --coverage)   # PUBLIC: se propaga al test
endif()
```

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON
cmake --build build && ctest --test-dir build
gcovr --root . --html-details build/cobertura.html
```

> **Por qué `option(...)` y no siempre.** El código instrumentado es entre dos y
> cuatro veces más lento y deja ficheros por todas partes. La cobertura se mide
> **en una pasada aparte**, no en el ciclo Rojo-Verde-Refactor. Si medirla os
> ralentiza el ciclo, dejaréis de medirla.

### Los comandos de gcovr que se usan de verdad

| Comando | Para qué |
|---|---|
| `gcovr --txt` | El resumen por consola. El del día a día |
| `gcovr --txt-metric branch --txt` | **Cobertura de ramas** (el apartado 5) |
| `gcovr --html-details informe.html` | Informe navegable, línea a línea, coloreado |
| `gcovr --xml-pretty -o cobertura.xml` | Formato Cobertura, para Jenkins/GitLab/SonarQube |
| `gcovr --filter 'src/.*' --exclude '.*_test\.cpp'` | Medir **solo producción**, nunca los tests |
| `gcovr --fail-under-branch 70` | Devuelve ≠ 0 y rompe la CI bajo ese umbral |

Y para no repetir banderas, un `gcovr.cfg` en la raíz:

```ini
filter = src/.*
exclude = .*_test\.cpp
html-details = build/cobertura.html
txt-metric = branch
```

---

## 4. Bad — el 100 % que no significa nada

`tarifa()`, con su bug de frontera del bloque 9 aún dentro:

```cpp
double tarifa(double peso) {
    if (peso <= 0.0)   throw PesoInvalido{};
    if (peso < 5.0)    return 3.00;
    if (peso < 20.0)   return 7.50;
    return 12.00;
}
```

Y un "test" que no comprueba absolutamente nada —la versión destilada de lo que
le pasaba al Paint—:

```cpp
TEST(Tarifa, RecorreTodosLosTramos) {
    tarifa(2.5);
    tarifa(12.0);
    tarifa(50.0);        // ← ni un solo EXPECT
}
```

```
[  PASSED  ] 1 test.

File          Lines    Exec   Cover   Missing
tarifa.cpp        5       5    100%
```

**Cien por cien de cobertura de líneas.** Un test que no afirma nada y una
métrica perfecta. Y ahora lo importante: con los **tres tests de verdad** del
bloque 9, con sus `EXPECT_DOUBLE_EQ`, el número es **exactamente el mismo**:

```
File          Lines    Exec   Cover   Missing
tarifa.cpp        5       5    100%
```

> **Ese es el hallazgo del bloque de hoy.** La cobertura de líneas **no
> distingue** un test que verifica de uno que solo pasea. Por eso no cazó el
> `NDEBUG` del Paint por sí sola: aquellos `assert` desactivados **se ejecutaban
> igual**, solo que no comprobaban nada.

## 5. Good — mirar las ramas, y mirar lo que falta

La misma ejecución, cambiando la métrica:

```bash
gcovr --root . --txt-metric branch --txt
```

```
File          Branches   Taken   Cover   Missing
tarifa.cpp           6       5     83%   4
```

**Seis ramas, cinco tomadas, y falta la de la línea 4.** La línea 4 es:

```cpp
    if (peso <= 0.0)   throw PesoInvalido{};    // ← la rama "sí" nunca se tomó
```

El informe ha dicho, con nombre y número de línea, **qué caso falta**: el peso
no positivo. Exactamente el hueco que el análisis de valores frontera del bloque
9 había predicho.

Eso es para lo que sirve la cobertura:

> **No para presumir de un número, sino para leer la lista de lo que ningún test
> ha tocado y preguntarse, caso por caso: "¿esto no hace falta probarlo, o es
> que se me ha olvidado?"**

Con `--html-details` lo veis en color sobre el propio código: verde ejecutado,
rojo nunca, y en cada `if` un contador de las dos salidas.

---

## 6. Cómo se lee un informe sin engañarse

| Lo que ves | Lo que suele significar |
|---|---|
| Línea roja en un `catch` | El camino de error no se prueba. Es **el sitio donde viven los bugs caros** |
| 100 % de línea y 60 % de rama | Se recorren los `if`, pero solo por un lado |
| Un fichero al 0 % | O está muerto (borradlo) o no lo prueba nadie (peor) |
| Cobertura que **baja** al añadir código | Normal: es la señal de que algo entró sin test |
| Un getter al 100 % | Ruido: infla la media y no prueba nada |

Dos trampas que conviene decir en voz alta:

- **Ley de Goodhart.** En cuanto la cobertura es un objetivo, deja de ser una
  medida. Un equipo al que le exigen el 80 % lo alcanza, y lo alcanza escribiendo
  los tests que suben el número —getters, constructores—, no los que encuentran
  fallos.
- **El umbral útil no es alto, es *no descendente*.** `--fail-under-branch` con
  el valor que ya tenéis hoy, y que no baje. Eso frena la erosión sin premiar el
  relleno.

---

## 7. Qué ganamos y qué pagamos

**Ganamos:** una lista objetiva de lo que ningún test ha tocado; la detección
inmediata de código muerto; una red contra la erosión en la CI; y, con
`--txt-metric branch`, la comprobación de si las fronteras del bloque 9 estaban
de verdad cubiertas.

**Pagamos:** compilación instrumentada y ejecución más lenta; ficheros
`.gcno`/`.gcda` que ensucian el árbol (a `.gitignore`); la falsa sensación de
seguridad que da un número alto; y la tentación, ya vista, de trabajar para la
métrica.

> **La frase que hay que llevarse:** la cobertura os dice, con certeza, **qué no
> está probado**. Sobre lo demás, no os dice nada.

## 8. Conexión con lo que viene

La cobertura señala el código que **ningún test ejecuta**. El **bloque 16** ataca
el problema simétrico: el código que los tests **sí ejecutan** y que está mal de
todas formas —un índice fuera de rango que hoy no revienta, un desbordamiento
con signo, un `use-after-free` que solo falla en producción—. Cobertura y
sanitizers son las dos mitades de la misma pregunta.

## 9. Mantra del bloque

> **"Cobertura alta no prueba que funcione; cobertura baja prueba que no lo
> sabes."**
> Usadla como lista de pendientes, nunca como nota del examen.

---

## 10. Referencias

**Las herramientas (gratuito):**

- **[GCC — `gcov`](https://gcc.gnu.org/onlinedocs/gcc/Gcov.html)** — qué son los
  `.gcno` y `.gcda`, por qué `--coverage` hace falta **también al enlazar** y por
  qué `-O0`. Explica la mitad de los problemas de montaje del apartado 3.
- **[gcovr — documentación](https://gcovr.com/en/stable/)** — todas las opciones
  del apartado 3, con ejemplos de los informes HTML. La página de
  **[configuración](https://gcovr.com/en/stable/guide/configuration.html)**
  documenta el `gcovr.cfg`.
- **[gcovr — instalación](https://gcovr.com/en/stable/installation.html)** — si
  no lo tenéis aún; y el
  [documento de instalación del curso](../00_instalacion.md) para el resto del
  montaje, incluidas las alternativas para MSVC.

**Para no dejarse engañar por el número:**

- **[Martin Fowler — *Test Coverage*](https://martinfowler.com/bliki/TestCoverage.html)**
  — **gratuito**, tres minutos. La tesis del apartado 6: la cobertura es útil
  para encontrar código no probado e inútil como objetivo de calidad. Si el
  equipo discute umbrales, mandad este enlace.
- **[Google Testing Blog — *Code Coverage Best Practices*](https://testing.googleblog.com/2020/08/code-coverage-best-practices.html)**
  — **gratuito**. Qué funcionó y qué no en una base de código enorme: por qué no
  fijan un umbral global, y por qué sí vigilan que no baje.
- **[*Software Engineering at Google*, cap. 11](https://abseil.io/resources/swe-book/html/ch11.html)**
  — **gratuito**. La sección sobre métricas de test y el coste real de
  perseguirlas.
- 📖 Andy Hunt y Dave Thomas,
  **[*Pragmatic Unit Testing*](https://pragprog.com/titles/utj2/pragmatic-unit-testing-in-java-8-with-junit/)**
  — el capítulo de cobertura conecta el informe con **CORRECT** y **BICEPS** del
  día 1: qué fila de la tabla falta cuando una rama sale en rojo.
