# Día 5 — Bloque 25: Cierre del curso

> *"No soy un gran programador; soy un buen programador con grandes
> costumbres."* — Kent Beck

---

## 1. El curso en un dibujo

Cinco días, una sola idea: **poder cambiar el código sin miedo**.

```
                          ¿ES LO QUE PIDIÓ EL CLIENTE?
                    ┌───────────────────────────────────────┐
   Día 5            │  Historias · Gherkin · cucumber-cpp   │  ← ATDD / BDD
                    └───────────────────┬───────────────────┘
                                        │ el escenario dice cuándo has terminado
                    ┌───────────────────▼───────────────────┐
   Días 3 y 5       │  Refactor · olores · condicionales ·  │  ← cambiar sin miedo
                    │  IA con red                           │
                    └───────────────────┬───────────────────┘
                                        │ solo es posible con...
                    ┌───────────────────▼───────────────────┐
   Días 3 y 4       │  gMock · TEST_P · cobertura ·         │  ← medir la red
                    │  sanitizers                           │
                    └───────────────────┬───────────────────┘
                    ┌───────────────────▼───────────────────┐
   Día 2            │  GoogleTest · fixtures · fronteras ·  │  ← tejer la red
                    │  kata Rojo-Verde-Refactor             │
                    └───────────────────┬───────────────────┘
                    ┌───────────────────▼───────────────────┐
   Día 1            │  XP · tipos de prueba · diseño        │  ← los cimientos
                    │  testable · FIRST · ciclo TDD         │
                    └───────────────────────────────────────┘
```

---

## 2. El temario, cumplido

| Tema del temario | Dónde lo vimos |
|---|---|
| **1. Fundamentos y metodologías ágiles** | Bloque 1 |
| **2. Pruebas de software** | Bloque 2 |
| **3. Principios de diseño** (SOLID, Tell-Don't-Ask, FIRST/BICEPS/CORRECT) | Bloques 3 y 4 |
| **4. Gestión y ejecución de pruebas** (roles, MITs, MIMs, fronteras) | Bloque 9 |
| **5. Metodología TDD** (ciclo, GoogleTest, gMock, `TEST_P`, cobertura) | Bloques 5-8, 10-16, 18 |
| **6. Refactorización de código** (principios, olores, condicionales, IA) | Bloques 17, 19, 20 y 21 |
| **7. Pruebas de aceptación** (historias, Gherkin, cucumber-cpp) | Bloques 22, 23 y 24 |

Y las promesas que fueron quedando escritas en el material:

| Promesa | Dónde se cumplió |
|---|---|
| gMock genera los dobles escritos a mano | Bloque 13 |
| El `Reloj` de la kata | Bloque 13 |
| Cobertura de ramas con `gcov`/`gcovr` | Bloque 15 |
| El tipo `Dinero` (*Replace Primitive with Object*) | Bloque 19 |
| BDD con Cucumber y `cucumber-cpp` | Bloques 22-24 |

---

## 3. Los mantras, juntos

Una línea por bloque. Si os lleváis una hoja del curso, que sea esta.

| # | Mantra |
|---|---|
| 1 | *Si duele, hazlo más a menudo.* |
| 2 | *No se prueba para demostrar que funciona; se prueba para descubrir dónde falla.* |
| 3 | *El test es el primer cliente de tu API.* |
| 4 | *FIRST te dice cómo escribirlo, BICEPS qué mirar, CORRECT dónde mirar.* |
| 5 | *Rojo, Verde, Refactor. Sin saltarse ninguno, sin quedarse en ninguno.* |
| 7 | *La assertion no se elige por lo que comprueba, sino por lo que dirá cuando falle.* |
| 9 | *Los bugs no viven en el centro de los rangos; viven en los bordes.* |
| 12 | *Stub para entrar, mock para salir.* |
| 13 | *`ON_CALL` para que funcione, `EXPECT_CALL` para afirmar.* |
| 14 | *Un caso, una línea.* |
| 15 | *Cobertura alta no prueba que funcione; cobertura baja prueba que no lo sabes.* |
| 17 | *Refactor pequeño, test verde, commit.* |
| 19 | *Un sombrero cada vez. Y que el compilador trabaje para ti.* |
| 20 | *Primero la foto, después el cambio, y el `if` que queda tiene nombre.* |
| 21 | *La IA propone, los tests deciden.* |
| 22 | *Primero la conversación, después el ejemplo, y por último el código.* |
| 23 | *Escribe el escenario para que lo lea tu cliente, no tu compilador.* |
| 24 | *El escenario dice cuándo has terminado. Los tests unitarios, cómo has llegado.* |

---

## 4. El lunes por la mañana

Cinco cosas concretas para empezar en vuestro proyecto. **Una por semana**, no
todas a la vez.

| Semana | Qué hacer | Cómo sabréis que funciona |
|---|---|---|
| 1 | Montar GoogleTest en el CMake del proyecto y escribir **un** test | `ctest` en verde en la máquina de un compañero |
| 2 | El próximo bug: **primero un test que lo reproduzca**, después el arreglo | El test estaba en rojo antes del arreglo |
| 3 | Añadir `gcovr` al CI. **Solo mirar**, sin umbral | Tenéis un número y sabéis qué ficheros están a cero |
| 4 | Una función nueva, entera con TDD | Cada línea tiene un test que la pidió |
| 5 | Una historia con *Example Mapping* y los tres amigos | Negocio ha escrito o leído los ejemplos |

> **El error típico:** intentar poner tests a todo el código heredado de golpe.
> No. Tests **a lo que tocáis**: cada bug, cada funcionalidad nueva, cada
> refactor. En seis meses, lo que más se toca estará cubierto.

---

## 5. Para seguir practicando

**Katas** (ejercicios cortos para repetir):

| Kata | Qué practica |
|---|---|
| *String Calculator* (Roy Osherove) | Ciclo TDD con pasos muy pequeños |
| *Bowling Game* (Robert C. Martin) | Triangulación y refactor |
| *Gilded Rose* (Emily Bache) | Tests de caracterización y condicionales (bloques 20 y 21) |
| *Mars Rover* | Diseño emergente y dobles de prueba |

*Gilded Rose* es la mejor continuación de hoy: código heredado lleno de `if`
anidados y versión en C++ lista para empezar.

---

## 6. Retrospectiva

Diez minutos, tres columnas:

```
   ┌──────────────────┬──────────────────┬──────────────────┐
   │   EMPEZAR A...   │   DEJAR DE...    │   SEGUIR...      │
   ├──────────────────┼──────────────────┼──────────────────┤
   │                  │                  │                  │
   │                  │                  │                  │
   └──────────────────┴──────────────────┴──────────────────┘
```

Una pregunta para cada uno, en voz alta:

> **¿Qué vas a hacer distinto el lunes?**

---

## 7. Mantra del curso

> **"El test va primero. El refactor, siempre. Y el cliente decide cuándo has
> terminado."**

---

## 8. Referencias

- **[Emily Bache — *Gilded Rose Refactoring Kata*](https://github.com/emilybache/GildedRose-Refactoring-Kata)**
  — **gratuito**. La kata de §5, con versión en C++ y GoogleTest. El mejor
  ejercicio para repetir los bloques 20 y 21.
- **[Emily Bache — *Samman Coaching*](https://sammancoaching.org/)**
  — **gratuito**. Material para practicar TDD en equipo, con sesiones de una hora.
- **[Kata-Log](https://kata-log.rocks/)**
  — **gratuito**. Un catálogo de katas clasificadas por lo que practican.
- 📖 Kent Beck,
  **[*Test-Driven Development: By Example*](https://www.informit.com/store/test-driven-development-by-example-9780321146533)**
  — el libro con el que empezó todo. Corto y se relee con otros ojos después
  del curso.
- 📖 Jeff Langr,
  **[*Modern C++ Programming with TDD*](https://pragprog.com/titles/lotdd/modern-c-programming-with-test-driven-development/)**
  — el libro del curso, de principio a fin, en C++.
- 📖 Michael Feathers,
  **[*Working Effectively with Legacy Code*](https://www.informit.com/store/working-effectively-with-legacy-code-9780131177055)**
  — para el §4: cuando el código que tenéis que cambiar no tiene tests.
