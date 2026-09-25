# Día 5 — Bloque 22: ATDD, BDD e historias de usuario

> *"TDD te dice si has construido bien la cosa. ATDD te dice si has construido
> la cosa buena."*

---

## 1. El problema en una frase

Una clase puede tener 100 % de cobertura, todos los tests en verde... y hacer
algo que el cliente **no pidió**.

## 2. Tres siglas, una idea

| | **TDD** | **ATDD** | **BDD** |
|---|---|---|---|
| Nombre | *Test-Driven Development* | *Acceptance Test-Driven Development* | *Behaviour-Driven Development* |
| Pregunta | ¿El código hace lo que **el programador** cree? | ¿El sistema hace lo que **el negocio** pidió? | Igual que ATDD, **en un lenguaje que todos entienden** |
| Lo escribe | El programador | Negocio + QA + programador | Negocio + QA + programador |
| Unidad | Una clase, una función | Una historia de usuario | Un comportamiento (escenario) |
| Herramienta | GoogleTest | Cualquiera | Gherkin + Cucumber (bloques 23 y 24) |

**ATDD y BDD son casi lo mismo.** BDD es ATDD con dos añadidos: se habla de
*comportamiento* en vez de *test*, y los ejemplos se escriben en un formato
fijo (Dado / Cuando / Entonces) que lee cualquiera.

---

## 3. El doble bucle

```
   ┌──────────────── BUCLE EXTERNO (ATDD) — horas o días ────────────────┐
   │                                                                     │
   │   1. Escenario de aceptación en ROJO                                │
   │            │                                                        │
   │            ▼                                                        │
   │   ┌──── BUCLE INTERNO (TDD) — minutos ─────┐                        │
   │   │  Rojo ──► Verde ──► Refactor ──┐       │                        │
   │   │    ▲                           │       │  se repite hasta que...│
   │   │    └───────────────────────────┘       │                        │
   │   └────────────────────────────────────────┘                        │
   │            │                                                        │
   │            ▼                                                        │
   │   2. ...el escenario de aceptación pasa a VERDE                     │
   │            │                                                        │
   │            ▼                                                        │
   │   3. Refactor, y siguiente escenario                                │
   └─────────────────────────────────────────────────────────────────────┘
```

- El **escenario** dice **qué** hay que construir y **cuándo** has terminado.
- Los **tests unitarios** dicen **cómo** está construido cada pieza.

> En el bloque 24 lo veréis funcionando de verdad: un escenario en rojo, dos
> tests unitarios, y el escenario en verde.

---

## 4. Historias de usuario

Una **historia de usuario** es una necesidad contada desde el punto de vista de
quien la tiene. El formato clásico (Connextra):

```
Como   <rol>
Quiero <qué>
Para   <para qué / valor>
```

### Bad

```
Como desarrollador
Quiero añadir un campo comision a la tabla MOVIMIENTOS
Para guardar la comisión
```

- El rol es **el programador**, no quien obtiene el valor.
- El "qué" es **una solución técnica**, no una necesidad.
- El "para qué" repite el "qué". **No hay valor de negocio.**

### Good

```
Como   responsable de productos del banco
Quiero cobrar 2 euros por cada retirada en un cajero de otra entidad
Para   cubrir lo que esa entidad nos cobra a nosotros
```

- El rol es **una persona real** del negocio.
- El "qué" dice **qué**, no **cómo** (nada de tablas ni campos).
- El "para" explica el **porqué**: si mañana esa entidad deja de cobrar, la
  historia deja de tener sentido. Eso ayuda a decidir.

### Las 3 C (Ron Jeffries)

Una historia no es solo la frase de la tarjeta:

| C | Qué es |
|---|---|
| **Card** (tarjeta) | La frase. Es un **recordatorio** de una conversación, no la especificación |
| **Conversation** (conversación) | Negocio, QA y desarrollo hablan de ella. Aquí salen los detalles |
| **Confirmation** (confirmación) | Los **criterios de aceptación**. Cómo sabremos que está hecha |

La tercera C es la que se convierte en tests. Es donde conectan las historias
y los tests.

### INVEST: ¿está bien escrita?

| Letra | Significa | Pregunta |
|---|---|---|
| **I** | *Independent* | ¿Se puede hacer sin esperar a otra historia? |
| **N** | *Negotiable* | ¿El detalle se puede discutir, o ya viene cerrado como un contrato? |
| **V** | *Valuable* | ¿Aporta valor a alguien que no sea el equipo? |
| **E** | *Estimable* | ¿El equipo sabe más o menos cuánto cuesta? |
| **S** | *Small* | ¿Cabe en un sprint (mejor, en pocos días)? |
| **T** | *Testable* | ¿Se puede escribir un criterio de aceptación? |

La historia *Bad* de arriba falla la **V**. Una historia como *"el sistema debe
ser rápido"* falla la **T**.

---

## 5. Criterios de aceptación: reglas y ejemplos

Ya los habéis usado: el ejercicio de la cuenta bancaria del día 4 era **una
lista de criterios de aceptación** en formato Dado / Cuando / Entonces.

Un criterio de aceptación tiene dos partes:

- Una **regla**: *"no se puede retirar más de lo que hay"*.
- Uno o varios **ejemplos** concretos: *"con 100 €, retirar 101 € se rechaza"*.

Las reglas solas son ambiguas. Los ejemplos, no:

| Regla | Pregunta que la regla no contesta | El ejemplo la contesta |
|---|---|---|
| Se cobran 2 € por retirar en un cajero ajeno | ¿Y si con la comisión no me llega el saldo? | Con 100 €, retirar 99 € se **rechaza** |
| No se puede retirar más de lo que hay | ¿Se puede dejar la cuenta a cero? | Con 100 €, retirar 100 € **se permite** |

**Los ejemplos concretos son el test.** Por eso en BDD se dice *specification
by example*.

---

## 6. Cómo salen los ejemplos: *Example Mapping*

Una técnica de 25 minutos para una historia, con tarjetas de cuatro colores:

```
                ┌───────────────────────────────────────────┐
  AMARILLA      │  Comisión en cajeros de otra entidad      │   ← la historia
                └───────────────────────────────────────────┘
        ┌─────────────────────────┐   ┌─────────────────────────┐
  AZUL  │ Se cobran 2 € por       │   │ La comisión cuenta para │   ← las reglas
        │ retirada                │   │ el saldo disponible     │
        └─────────────────────────┘   └─────────────────────────┘
        ┌─────────────────────────┐   ┌─────────────────────────┐
  VERDE │ 100 €, retiro 30        │   │ 100 €, retiro 99        │   ← los ejemplos
        │ → quedan 68             │   │ → rechazada, quedan 100 │
        └─────────────────────────┘   └─────────────────────────┘
        ┌─────────────────────────────────────────────────────┐
  ROJA  │ ¿Se cobra también si la retirada falla?             │   ← las dudas
        └─────────────────────────────────────────────────────┘
```

Cómo se lee el resultado:

- **Muchas rojas** → la historia no está lista. Hay que preguntar antes de programar.
- **Muchas azules** → la historia es demasiado grande. Hay que partirla.
- **Pocas de todo** → está lista. Las verdes se convierten en escenarios (bloque 23).

### Los tres amigos

Esa sesión la hacen tres perfiles, no uno:

| Amigo | Aporta |
|---|---|
| **Negocio** (*Product Owner*, analista) | Qué problema se resuelve y qué reglas hay |
| **Desarrollo** | Qué es posible y qué casos técnicos pueden fallar |
| **Pruebas** (QA) | Los casos raros: fronteras, errores, "¿y si...?" |

> Es el *Cliente en el equipo* de XP (día 1) llevado a una reunión con formato.

---

## 7. Qué ganamos y qué pagamos

**Ganamos:**
- Las dudas salen **antes** de programar, no en la demo.
- Una definición de "hecho" que no depende de opiniones.
- Documentación que se ejecuta y, por tanto, no se queda vieja.

**Pagamos:**
- Tiempo de reunión de tres perfiles por historia.
- Negocio tiene que implicarse. Si no se implica, BDD se convierte en
  "tests con otra sintaxis" y pierde casi todo su valor.

## 8. Conexión con lo que viene

Los ejemplos verdes de §6 hay que escribirlos en un formato que lea el negocio
y que ejecute la máquina. Ese formato es **Gherkin** (bloque 23), y la
herramienta que lo ejecuta contra C++ es **cucumber-cpp** (bloque 24).

## 9. Mantra del bloque

> **"Primero la conversación, después el ejemplo, y por último el código."**

---

## 10. Referencias

- **[Dan North — *Introducing BDD*](https://dannorth.net/introducing-bdd/)**
  — **gratuito**. El artículo que inventó BDD. Explica por qué cambió "test"
  por "comportamiento".
- **[Ron Jeffries — *Essential XP: Card, Conversation, Confirmation*](https://ronjeffries.com/xprog/articles/expcardconversationconfirmation/)**
  — **gratuito**. Las 3 C de §4, contadas por su autor.
- **[Bill Wake — *INVEST in Good Stories*](https://xp123.com/articles/invest-in-good-stories-and-smart-tasks/)**
  — **gratuito**. El artículo original de INVEST.
- **[Matt Wynne — *Introducing Example Mapping*](https://cucumber.io/blog/bdd/example-mapping-introduction/)**
  — **gratuito**. La técnica de §6 con fotos de tarjetas reales.
- **[Martin Fowler — *Specification By Example*](https://martinfowler.com/bliki/SpecificationByExample.html)**
  — **gratuito**. Por qué los ejemplos concretos son mejor especificación que las reglas.
- 📖 Mike Cohn,
  **[*User Stories Applied*](https://www.informit.com/store/user-stories-applied-for-agile-software-development-9780321205681)**
  — el libro de referencia sobre historias de usuario.
- 📖 Gojko Adzic,
  **[*Specification by Example*](https://gojko.net/books/specification-by-example/)**
  — cómo lo hacen equipos reales, con casos de estudio.
