# Día 4 — Ejercicio TDD: la cuenta bancaria

> **Qué vais a hacer:** construir una clase `Cuenta` **desde cero y con TDD**.
> Aquí no hay código. Solo están los **criterios de aceptación**: lo que la
> cuenta tiene que cumplir. El código saldrá de ponerlos en verde, uno a uno.
>
> Es justo lo contrario del Paint: allí teníamos el código y deducíamos los
> tests. Aquí los tests van **primero** y son la especificación.

---

## 1. Las reglas del juego

```
   ┌──────────┐     ┌──────────┐     ┌──────────────┐
   │  ROJO    │ ──► │  VERDE   │ ──► │  REFACTOR    │ ──┐
   │ un test  │     │ lo MÍNIMO│     │ limpio, y    │   │
   │ que falla│     │para pasar│     │ sigue verde  │   │
   └──────────┘     └──────────┘     └──────────────┘   │
        ▲                                               │
        └──────────── siguiente criterio ◄──────────────┘
```

1. **Un criterio cada vez, en orden.** No leáis el 5 hasta tener el 4 en verde.
2. **Primero el test, y verlo fallar.** Si no lo habéis visto en rojo, no
   sabéis si prueba algo.
3. **Lo mínimo para pasar.** Si devolver `0` a pelo pone el test en verde,
   devolved `0`. El siguiente test os obligará a generalizar.
4. **Nada de código sin un test que lo pida.** Ni un `if` "por si acaso".
5. **Refactorizad solo en verde**, y pasad todos los tests después.

---

## 2. Decisiones ya tomadas

Para que todos escribáis tests comparables:

| Decisión | Valor | Por qué |
|---|---|---|
| Nombre de la clase | `Cuenta` | — |
| Unidad de dinero | **céntimos, en `long long`** | Con `double`, `0.1 + 0.2 != 0.3`. El dinero no se guarda en coma flotante |
| Importe no válido | `std::invalid_argument` | Es un error de quien llama |
| Saldo insuficiente | `std::domain_error` | La petición es válida, pero la cuenta no puede atenderla |

Los nombres de los métodos (`Ingresar`, `Retirar`, `Saldo`…) **los decidís
vosotros al escribir el primer test que los necesite**. Eso también es TDD: el
test diseña la interfaz.

---

## 3. Criterios de aceptación

Formato de cada criterio:

- **Dado** (*Given*): cómo está la cuenta antes. Es el *Arrange*.
- **Cuando** (*When*): lo que se hace. Es el *Act*.
- **Entonces** (*Then*): lo que tiene que pasar. Es el *Assert*.

Cada criterio es **un test como mínimo**. Algunos piden más de uno.

### Bloque A — Abrir la cuenta

**CA-01. Una cuenta nueva tiene saldo 0.**
- **Dado** que abro una cuenta nueva
- **Cuando** consulto el saldo
- **Entonces** el saldo es 0

### Bloque B — Ingresar

**CA-02. Un ingreso aumenta el saldo.**
- **Dado** una cuenta nueva
- **Cuando** ingreso 100
- **Entonces** el saldo es 100

**CA-03. Los ingresos se acumulan.**
- **Dado** una cuenta nueva
- **Cuando** ingreso 100 y después 50
- **Entonces** el saldo es 150

**CA-04. No se puede ingresar 0.**
- **Dado** una cuenta con saldo 100
- **Cuando** intento ingresar 0
- **Entonces** se lanza `std::invalid_argument`
- **Y** el saldo sigue siendo 100

**CA-05. No se puede ingresar una cantidad negativa.**
- **Dado** una cuenta con saldo 100
- **Cuando** intento ingresar -50
- **Entonces** se lanza `std::invalid_argument`
- **Y** el saldo sigue siendo 100

### Bloque C — Retirar

**CA-06. Una retirada disminuye el saldo.**
- **Dado** una cuenta con saldo 100
- **Cuando** retiro 30
- **Entonces** el saldo es 70

**CA-07. Se puede retirar todo el saldo.** *(frontera)*
- **Dado** una cuenta con saldo 100
- **Cuando** retiro 100
- **Entonces** el saldo es 0

**CA-08. No se puede retirar más de lo que hay.** *(frontera + 1)*
- **Dado** una cuenta con saldo 100
- **Cuando** intento retirar 101
- **Entonces** se lanza `std::domain_error`
- **Y** el saldo sigue siendo 100

**CA-09. No se puede retirar de una cuenta vacía.**
- **Dado** una cuenta nueva
- **Cuando** intento retirar 1
- **Entonces** se lanza `std::domain_error`
- **Y** el saldo sigue siendo 0

**CA-10. No se puede retirar 0 ni una cantidad negativa.**
- **Dado** una cuenta con saldo 100
- **Cuando** intento retirar 0, o -50
- **Entonces** se lanza `std::invalid_argument`
- **Y** el saldo sigue siendo 100

> 💡 CA-10 son **dos** casos con el mismo resultado. Buen candidato para
> `TEST_P` (bloque 14).

### Bloque D — Movimientos

**CA-11. Una cuenta nueva no tiene movimientos.**
- **Dado** una cuenta nueva
- **Cuando** consulto los movimientos
- **Entonces** la lista está vacía

**CA-12. Cada operación correcta queda registrada, en orden.**
- **Dado** una cuenta nueva
- **Cuando** ingreso 100, retiro 30 e ingreso 20
- **Entonces** hay 3 movimientos, en ese orden: `+100`, `-30`, `+20`

**CA-13. Una operación rechazada no deja movimiento.**
- **Dado** una cuenta con saldo 100 y un movimiento
- **Cuando** intento retirar 500 y falla
- **Entonces** sigue habiendo un solo movimiento

### Bloque E — Transferencias *(para quien acabe)*

**CA-14. Una transferencia mueve el dinero de una cuenta a otra.**
- **Dado** una cuenta A con saldo 100 y una cuenta B con saldo 0
- **Cuando** transfiero 40 de A a B
- **Entonces** A tiene 60 y B tiene 40

**CA-15. Una transferencia sin fondos no toca ninguna de las dos cuentas.**
- **Dado** una cuenta A con saldo 100 y una cuenta B con saldo 0
- **Cuando** intento transferir 500 de A a B
- **Entonces** se lanza `std::domain_error`
- **Y** A sigue con 100 **y** B sigue con 0

**CA-16. No se puede transferir a la misma cuenta.**
- **Dado** una cuenta A con saldo 100
- **Cuando** intento transferir 10 de A a A
- **Entonces** se lanza `std::invalid_argument`
- **Y** A sigue con 100

---

## 4. Lo que tenéis que entregar

| Entregable | Cómo se comprueba |
|---|---|
| `Cuenta.h` / `Cuenta.cpp` y `test_cuenta.cpp` | Compila y todos los tests en verde |
| Un test por criterio, como mínimo, con AAA visible | Se lee el test y se reconoce el CA |
| Nombre de cada test en español y describiendo el criterio | `TEST(Cuenta, NoSePuedeRetirarMasDeLoQueHay)` |
| Cobertura de `Cuenta.cpp` | 100 % de líneas y ramas (gcovr con `--exclude-throw-branches`) |

Si os sale el 100 % **sin haberlo buscado**, es la mejor señal: en TDD cada
línea existe porque un test la pidió.

---

## 5. Preguntas para el final

1. ¿En qué criterio tuvisteis que añadir el primer `if`? ¿Cuál lo pidió?
2. ¿Qué nombre le pusisteis al método de ingresar y en qué test lo decidisteis?
3. En CA-04 y CA-08, ¿qué comprobasteis primero, la excepción o el saldo?
   ¿Os hizo falta un fixture?
4. ¿Algún refactor cambió un test? Si pasó, ¿era un test de comportamiento o de
   implementación?

---

## Mantra

> ### **El test va primero. Si no lo has visto en rojo, no sabes si prueba algo.**
> ### **Escribe lo mínimo para pasar: el siguiente test te obligará a hacerlo bien.**

---

## Referencias

- 📖 Kent Beck, **[*Test-Driven Development: By Example*](https://www.informit.com/store/test-driven-development-by-example-9780321146533)**
  — la parte I es un ejercicio con dinero muy parecido a este, contado paso a
  paso. Leedla después de hacerlo y comparad.
- **[Robert C. Martin — *The Three Rules of TDD*](http://butunclebob.com/ArticleS.UncleBob.TheThreeRulesOfTdd)**
  (gratuito) — las reglas 2, 3 y 4 de §1 en una página.
- **[Martin Fowler — *Given When Then*](https://martinfowler.com/bliki/GivenWhenThen.html)**
  (gratuito) — de dónde sale el formato de §3 y cómo se corresponde con AAA.
- **[Martin Fowler — *Test Driven Development*](https://martinfowler.com/bliki/TestDrivenDevelopment.html)**
  (gratuito) — el ciclo rojo-verde-refactor y el error más común: saltarse el
  refactor.
- Bloques del curso: **[5 — Ciclo TDD](../day-01/05_ciclo_tdd.md)**,
  **[10 — Kata rojo-verde-refactor](../day-02/10_kata_rgr.md)** y
  **[14 — Tests paramétricos](../day-03/14_tests_parametricos.md)** (para CA-10).
