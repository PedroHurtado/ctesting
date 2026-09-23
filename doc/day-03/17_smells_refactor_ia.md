# Día 3 — Bloque 17: Malos olores, refactorización y la IA como apoyo

> *"Refactorizar sin tests no es refactorizar: es reescribir y cruzar los dedos."*

---

## 1. El problema en una frase

Todo el mundo sabe qué función habría que arreglar; nadie la toca, porque no hay
forma de demostrar que después sigue haciendo lo mismo.

## 2. Definición

**Refactorizar** es cambiar la estructura interna del código **sin cambiar su
comportamiento observable**. La definición es de Fowler, y la parte que cuenta
es la segunda: si el comportamiento cambia, no era un refactor, era un cambio
funcional disfrazado.

Y ahí está la conexión con los tres bloques anteriores:

```
   tests (12-14)  +  cobertura (15)  +  sanitizers (16)
              │
              └──►  ya podéis demostrar que el comportamiento no cambió
                            │
                            └──►  ya podéis refactorizar
```

Un **mal olor** (*code smell*) no es un bug: es una señal en la superficie de un
problema más profundo. No obliga a actuar; obliga a **mirar**.

---

## 3. Los olores que aparecen en C++ una y otra vez

| Olor | Cómo se reconoce | Refactor habitual |
|---|---|---|
| **Método largo** | No cabe en pantalla | *Extract Method* |
| **Condicional complejo** | `if` anidados de tres niveles | *Decompose Conditional*, cláusulas de guarda |
| **Sentencia `switch` repetida** | El mismo `switch` en cuatro sitios | *Replace Conditional with Polymorphism* |
| **Obsesión por primitivos** | `double importe`, `int idCliente` | *Replace Primitive with Object* |
| **Grupos de datos** | Los mismos 4 parámetros juntos siempre | *Introduce Parameter Object* |
| **Código duplicado** | Copia-pega con dos números cambiados | *Extract Function*, `TEST_P` en los tests |
| **Envidia de funcionalidad** | Un método usa más otra clase que la suya | *Move Method* |
| **Cirugía a escopetazos** | Un cambio obliga a tocar siete ficheros | Reagrupar responsabilidades |
| **Comentario que explica *qué* hace** | `// calcula el total con IVA` | Renombrar y extraer hasta que sobre |

> **El olor del día 2:** el texto de ayuda del Paint escrito a mano (bloque 11)
> era **código duplicado** de la sintaxis de cada figura. Y el `double` de la
> kata era **obsesión por primitivos** — lo arreglamos el día 4.

---

## 4. Bad — el condicional que crece

El punto que el temario llama *"simplificación de expresiones condicionales"*.
Empieza así de inocente:

```cpp
double precioFinal(const Cliente& c, double importe, bool cupon) {
    double resultado;
    if (importe > 0) {
        if (c.esVip()) {
            if (importe > 1000) {
                resultado = importe * 0.80;
            } else {
                resultado = importe * 0.90;
            }
        } else {
            if (cupon) {
                resultado = importe * 0.95;
            } else {
                resultado = importe;
            }
        }
    } else {
        throw ImporteInvalido{};
    }
    return resultado;                  // ← ¿seguro que siempre se asigna?
}
```

Qué duele, concretamente:

- **Cuatro niveles de anidamiento** para cuatro reglas de negocio.
- **El camino de error está al final**, a 14 líneas de la condición que lo
  provoca.
- **`resultado` sin inicializar**: el compilador no siempre avisa, y una rama
  nueva mal puesta devuelve basura.
- **Las reglas de negocio no se leen.** *"VIP con más de 1000 tiene un 20 %"* está
  ahí, pero hay que reconstruirlo mentalmente.

## 5. Good — guarda primero, reglas después

Dos refactorizaciones del catálogo, en este orden:

**Paso 1 — cláusula de guarda** (*Replace Nested Conditional with Guard
Clauses*): el caso excepcional sale por arriba y desaparece un nivel entero.

```cpp
if (importe <= 0) throw ImporteInvalido{};      // ← se acabó el else
```

**Paso 2 — *Decompose Conditional***: cada condición pasa a una función con
nombre, y el cuerpo queda plano.

```cpp
namespace {
    bool esGranCompraVip(const Cliente& c, double importe) {
        return c.esVip() && importe > 1000;
    }
}

double precioFinal(const Cliente& c, double importe, bool cupon) {
    if (importe <= 0) throw ImporteInvalido{};

    if (esGranCompraVip(c, importe)) return importe * 0.80;
    if (c.esVip())                   return importe * 0.90;
    if (cupon)                       return importe * 0.95;
    return importe;
}
```

Cuatro reglas, cuatro líneas, leídas de arriba abajo en el orden del contrato de
negocio. Sin variable temporal, sin anidamiento, sin rama que pueda quedar sin
asignar.

> **Cómo se hace esto sin miedo:** los pasos 1 y 2 se hacen **por separado**, y
> entre uno y otro **se lanzan los tests**. Si el paso 2 rompe algo, sabéis con
> certeza que fue el paso 2. Un refactor de diez cambios a la vez es una
> depuración de diez sospechosos.

Y aquí es donde el bloque 15 devuelve el favor: `gcovr --txt-metric branch`
sobre esta función os dice si las cuatro reglas tienen test. Si una sale en
rojo, **no la refactoricéis todavía**: escribid primero el test que la cubre.

---

## 6. La IA como apoyo, con una regla

Un modelo de lenguaje es **muy bueno** en la parte mecánica de esto —proponer
extracciones, renombrar con criterio, detectar duplicación, generar la tabla de
casos del `TEST_P` a partir de la especificación— y **es capaz de equivocarse
con total aplomo**: inventar una API que no existe, cambiar un `<=` por un `<`
al reescribir, o afirmar que un refactor "preserva el comportamiento" sin
haberlo comprobado.

De ahí la regla del curso, que es la misma que ya aplicamos en el bloque 11 del
día 2 —donde tres de las cinco afirmaciones "evidentes" resultaron falsas al
compilarlas—:

> ### **Una afirmación técnica que no has ejecutado es una hipótesis.**
>
> Da igual quién la haga: el compañero senior, la documentación, el modelo o
> vosotros mismos. Se ejecuta, y entonces es un hecho.

### El protocolo que hace segura la IA en refactor

| Paso | Por qué |
|---|---|
| 1. **Tests en verde antes de empezar** | Sin red, no hay refactor. Con IA tampoco |
| 2. **Cobertura de las ramas que vais a tocar** | Lo que no está cubierto, la IA lo puede romper en silencio |
| 3. **Pedid un refactor pequeño y con nombre** | *"aplica Decompose Conditional aquí"*, no *"mejora esto"* |
| 4. **Leed el diff, no el resumen** | La explicación puede ser correcta y el código no |
| 5. **Ejecutad los tests** | El único paso que convierte la hipótesis en hecho |
| 6. **Commit pequeño** | Para poder volver atrás sin perder el día |

**Lo que no se delega:** decidir **qué** hay que refactorizar y **por qué**. Eso
exige conocer el dominio y hacia dónde va el producto. La IA propone técnicas;
las prioridades las ponéis vosotros.

**El antipatrón:** pedirle que "refactorice el módulo entero" y aceptar un diff
de 400 líneas porque los tests pasan. Si la cobertura es del 40 %, que los tests
pasen no dice nada — lo acabáis de ver en el bloque 15.

---

## 7. Qué ganamos y qué pagamos

**Ganamos:** código que se lee en el orden del negocio; condicionales planos que
admiten una regla nueva sin tocar las anteriores; y una forma de trabajar en la
que un cambio estructural es aburrido en vez de arriesgado.

**Pagamos:** tiempo que no produce funcionalidad visible (y hay que saber
defenderlo); el riesgo de refactorizar por gusto en código que nadie va a volver
a tocar; y, con IA, la tentación de aceptar cambios que no habéis leído.

> **Cuándo NO refactorizar:** cuando el código funciona, nadie lo toca y no hay
> tests. Ahí el orden es el de Feathers: primero las **costuras** y los tests
> (día 1, bloque 3), después el refactor. Nunca al revés.

## 8. Conexión con lo que viene

Hoy hemos visto el catálogo y la mecánica segura. El **día 4** aplica
refactorizaciones mayores sobre el Paint —empezando por la que quedó prometida
en la kata, *Replace Primitive with Object*: un tipo `Dinero` que sustituya al
`double`— y después cambia de plano: de *"¿está bien construido?"* a *"¿es lo
que el cliente pidió?"*, con **ATDD/BDD**, historias de usuario, escenarios
Gherkin y `cucumber-cpp`.

## 9. Mantra del bloque

> **"Refactor pequeño, test verde, commit."**
> Tres pasos. Si os saltáis el segundo, los otros dos no valen nada.

---

## 10. Referencias

**El catálogo (gratuito):**

- **[refactoring.com — catálogo de refactorizaciones](https://refactoring.com/catalog/)**
  — **gratuito**. La lista completa de Fowler con la mecánica paso a paso de
  cada una, incluidas *Decompose Conditional* y *Replace Nested Conditional with
  Guard Clauses* del apartado 5. Tenedlo abierto mientras refactorizáis.
- **[Martin Fowler — *CodeSmell*](https://martinfowler.com/bliki/CodeSmell.html)**
  — **gratuito**, dos minutos. Qué es y qué no es un olor: por qué *"sugiere
  mirar"* y no *"obliga a cambiar"*.
- **[Refactoring Guru — catálogo de olores](https://refactoring.guru/refactoring/smells)**
  — **gratuito**. La tabla del apartado 3 con ejemplos y el refactor asociado a
  cada olor. Muy visual; útil para repartir en el equipo.

**Los libros de referencia:**

- 📖 Martin Fowler,
  **[*Refactoring: Improving the Design of Existing Code*](https://martinfowler.com/books/refactoring.html)**
  (2ª ed.) — el origen de todo lo de este bloque. Los capítulos 2 y 3 justifican
  cuándo merece la pena y cuándo no.
- 📖 Michael Feathers,
  **[*Working Effectively with Legacy Code*](https://www.informit.com/store/working-effectively-with-legacy-code-9780131177055)**
  — el libro para el caso del apartado 7: código sin tests que hay que cambiar
  igualmente. De aquí salen las **costuras** del día 1.
- 📖 Jeff Langr,
  **[*Modern C++ Programming with TDD*](https://pragprog.com/titles/lotdd/modern-c-programming-with-test-driven-development/)**,
  caps. 8-9 — refactorización y olores **en C++**, con los que el catálogo
  general no cubre: propiedad de la memoria, plantillas y compilación.

**Sobre trabajar con IA:**

- **[Martin Fowler — *Exploring Gen AI*](https://martinfowler.com/articles/exploring-gen-ai.html)**
  — **gratuito**. Experimentos de Thoughtworks usando LLM en refactorización y
  en código heredado, con lo que funcionó y lo que no. Es el apartado 6 con
  datos detrás.
