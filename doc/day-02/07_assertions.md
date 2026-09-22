# Día 2 — Bloque 7: Assertions

> *"Cuando el test falla a las 19:30, lo único que tienes es el mensaje."*

---

## 1. El problema en una frase

Un test que solo sabe decir "falso" te obliga a abrir el depurador; un test que
dice *qué esperaba, qué obtuvo y en qué elemento* te ahorra media hora.

## 2. La distinción de base: `ASSERT_*` frente a `EXPECT_*`

| Macro | Si falla | Cuándo usarla |
|---|---|---|
| `EXPECT_*` | Marca el fallo y **sigue** ejecutando el test | Por defecto, **siempre** |
| `ASSERT_*` | Marca el fallo y **sale de la función** (`return`) | Cuando seguir no tiene sentido o es peligroso |

La regla práctica: **`EXPECT_*` salvo que continuar vaya a provocar un
`segfault`**. Si compruebas que un puntero no es nulo y luego lo desreferencias,
esa primera comprobación es `ASSERT_*`.

```cpp
TEST(Repositorio, DevuelveElClienteBuscado) {
    auto* cliente = repo.buscar(42);
    ASSERT_NE(nullptr, cliente);            // ← si es nulo, la línea siguiente revienta
    EXPECT_EQ("Ana", cliente->nombre());    // ← a partir de aquí, EXPECT
}
```

> **Trampa clásica:** `ASSERT_*` hace `return;`, así que **no compila** dentro
> de una función que devuelva algo distinto de `void`. Si tienes un *helper*
> que comprueba cosas y devuelve `bool`, o lo conviertes en `void`, o usas
> `EXPECT_*`. Es el error de compilación más habitual de la primera semana.

## 3. El catálogo que se usa el 95 % del tiempo

```cpp
// Igualdad y comparación
EXPECT_EQ(esperado, real);        EXPECT_NE(a, b);
EXPECT_LT(a, b);  EXPECT_LE(a, b);  EXPECT_GT(a, b);  EXPECT_GE(a, b);

// Booleanos
EXPECT_TRUE(cuenta.estaBloqueada());
EXPECT_FALSE(lista.empty());

// Cadenas C (const char*): comparan CONTENIDO, no punteros
EXPECT_STREQ("hola", cadena);     EXPECT_STRCASEEQ("HOLA", cadena);

// Coma flotante (apartado 5)
EXPECT_DOUBLE_EQ(70.0, cuenta.saldo());
EXPECT_NEAR(3.1416, pi, 0.0001);

// Excepciones (apartado 6)
EXPECT_THROW(cuenta.retirar(1000), SaldoInsuficiente);
EXPECT_NO_THROW(cuenta.retirar(10));

// Fallo y éxito explícitos
FAIL()  << "no deberíamos haber llegado aquí";
ADD_FAILURE() << "marca el fallo pero continúa";
SUCCEED();
```

**El orden es `(esperado, real)`.** GoogleTest no lo impone, pero el mensaje de
fallo se lee mucho mejor y toda la suite debe seguir el mismo criterio. En este
curso: **esperado primero**, igual que ayer.

Con `std::string` usa `EXPECT_EQ`, no `EXPECT_STREQ`: `STREQ` es para
`const char*`. Y ojo con el fallo silencioso inverso:

```cpp
// Bad — compara DIRECCIONES de memoria, no contenido. Puede pasar por azar.
const char* nombre = obtenerNombre();
EXPECT_EQ("Ana", nombre);

// Good
EXPECT_STREQ("Ana", nombre);
```

## 4. Bad / Good: el mensaje de fallo importa

### Bad: todo metido en un `EXPECT_TRUE`

```cpp
TEST(Carrito, CalculaElTotalConDescuento) {
    Carrito c;
    c.anadir({"libro", 20.0});
    c.anadir({"taza",  10.0});
    c.aplicarCupon("VERANO");

    EXPECT_TRUE(c.total() == 27.0);            // ← 1
    EXPECT_TRUE(c.lineas().size() == 2);       // ← 2
    EXPECT_TRUE(c.lineas()[0].nombre == "libro" &&
                c.lineas()[1].nombre == "taza");// ← 3
}
```

Salida cuando falla:

```
Value of: c.total() == 27.0
  Actual: false
Expected: true
```

**No dice cuánto valía el total.** Y la tercera línea, con su `&&`, no dice
siquiera cuál de los dos artículos estaba mal.

### Good: la macro adecuada para cada comprobación

```cpp
#include <gmock/gmock.h>          // ← los matchers viven en gmock
using ::testing::ElementsAre;
using ::testing::Field;

TEST(Carrito, ElCuponVeranoAplicaUnDiezPorCientoAlTotal) {
    Carrito c;
    c.anadir({"libro", 20.0});
    c.anadir({"taza",  10.0});

    c.aplicarCupon("VERANO");

    EXPECT_DOUBLE_EQ(27.0, c.total());
    EXPECT_THAT(c.lineas(), ElementsAre(Field(&Linea::nombre, "libro"),
                                        Field(&Linea::nombre, "taza")));
}
```

Salida cuando falla:

```
Expected equality of these values:
  27.0
  c.total()
    Which is: 30
```

Y si el segundo artículo estuviera mal:

```
Value of: c.lineas()
Expected: has 2 elements where element #0 has field `nombre` that is equal to "libro",
          element #1 has field `nombre` that is equal to "taza"
  Actual: { { nombre: "libro" }, { nombre: "tazon" } }, whose element #1 doesn't match
```

Misma cantidad de código. Muchísima más información.

> **Regla:** en cuanto escribas `EXPECT_TRUE(a == b)` o `EXPECT_TRUE(x && y)`,
> existe una macro mejor. `EXPECT_TRUE` es solo para funciones que **ya**
> devuelven `bool` con un nombre que se lee como una afirmación
> (`EXPECT_TRUE(cuenta.estaBloqueada())`).

## 5. Coma flotante: la trampa favorita

```cpp
TEST(Suma, DosDecimales) {
    EXPECT_EQ(0.3, 0.1 + 0.2);      // ← FALLA. Y no es culpa de GoogleTest.
}
```

`0.1` y `0.2` no son representables exactamente en binario. La suma da
`0.30000000000000004`. **Nunca compares flotantes con `EXPECT_EQ`.**

| Macro | Qué hace | Cuándo |
|---|---|---|
| `EXPECT_FLOAT_EQ(a, b)` | Tolera 4 ULP (`float`) | Comparar `float` sin criterio propio |
| `EXPECT_DOUBLE_EQ(a, b)` | Tolera 4 ULP (`double`) | El caso normal con `double` |
| `EXPECT_NEAR(a, b, tol)` | Tolera `tol` absoluto | Cuando **el dominio** fija la tolerancia |

*ULP* = *Unit in the Last Place*: el salto mínimo entre dos flotantes
consecutivos. "4 ULP" significa "hasta cuatro flotantes de distancia", una
tolerancia **relativa** al tamaño del número: razonable para valores normales,
inútil para valores acumulados a lo largo de un millón de operaciones.

```cpp
// Good: la tolerancia la decide el dominio, no la máquina
EXPECT_NEAR(1234.56, factura.total(), 0.005);   // medio céntimo
EXPECT_NEAR(0.0, error, 1e-9);                  // ← con valores cerca de 0,
                                                //   DOUBLE_EQ es demasiado estricto
```

### La solución de verdad para dinero

```cpp
// Bad
class Cuenta { double saldo_; };        // ← 0.1 + 0.2 != 0.3, y Hacienda no perdona

// Good
class Cuenta { long long centimos_; };  // ← entero: EXPECT_EQ vuelve a valer
```

El test te ha llevado a un mejor diseño. Otra vez. Lo veremos en el refactor de
la kata (bloque 10).

## 6. Excepciones

```cpp
EXPECT_THROW(cuenta.retirar(1000.0), SaldoInsuficiente);  // esa excepción concreta
EXPECT_ANY_THROW(parsear("<<<"));                         // cualquiera — evítalo
EXPECT_NO_THROW(cuenta.retirar(10.0));                    // afirma que NO lanza
```

`EXPECT_ANY_THROW` es casi siempre demasiado laxo: pasa igual si lanzas la
excepción correcta que si te explota un `bad_alloc` por un bug. **Sé concreto.**

### Cuando además quieres comprobar el mensaje

```cpp
// Opción A: a mano. Fíjate en el FAIL(): sin él, el test pasa si NO lanza.
TEST(Cuenta, LaExcepcionDiceCuantoFaltaba) {
    Cuenta c{100.0};
    try {
        c.retirar(150.0);
        FAIL() << "se esperaba SaldoInsuficiente";   // ← imprescindible
    } catch (const SaldoInsuficiente& e) {
        EXPECT_STREQ("faltan 50.00 EUR", e.what());
    }
}

// Opción B: con matcher de gmock, una sola línea
using ::testing::ThrowsMessage;
using ::testing::HasSubstr;

EXPECT_THAT([&]{ c.retirar(150.0); },
            ThrowsMessage<SaldoInsuficiente>(HasSubstr("faltan 50")));
```

> El `FAIL()` de la opción A es el error más repetido del bloque: sin esa línea,
> si el código deja de lanzar, el `try` termina limpiamente y **el test pasa en
> verde mintiendo**. Es justo lo que el día 1 llamábamos un test que nunca has
> visto fallar.

## 7. `EXPECT_THAT` y los matchers

Es la assertion más expresiva: `EXPECT_THAT(valor, matcher)`. Necesita
`#include <gmock/gmock.h>` (por eso enlazamos `gmock_main` en el bloque 6).

```cpp
using namespace ::testing;

EXPECT_THAT(nombre,     StartsWith("Sr."));
EXPECT_THAT(mensaje,    HasSubstr("timeout"));
EXPECT_THAT(mensaje,    MatchesRegex("Error [0-9]+: .*"));

EXPECT_THAT(ids,        ElementsAre(1, 2, 3));            // exacto y en orden
EXPECT_THAT(ids,        UnorderedElementsAre(3, 1, 2));   // exacto, sin orden
EXPECT_THAT(ids,        Contains(7));
EXPECT_THAT(ids,        SizeIs(3));
EXPECT_THAT(ids,        IsEmpty());
EXPECT_THAT(ids,        Each(Gt(0)));                     // todos positivos

EXPECT_THAT(edad,       AllOf(Ge(18), Lt(65)));
EXPECT_THAT(estado,     Not(Eq(Estado::Error)));
EXPECT_THAT(total,      DoubleNear(27.0, 0.001));
EXPECT_THAT(opcional,   Optional(42));                    // std::optional
EXPECT_THAT(puntero,    Pointee(Field(&Cliente::nombre, "Ana")));
```

El valor no está en la brevedad, está en **el mensaje de fallo**: el matcher
sabe describirse a sí mismo y describir por qué el valor no encaja.

```cpp
// Bad: ¿qué sobraba? ¿qué faltaba? ¿en qué posición?
EXPECT_TRUE(std::equal(ids.begin(), ids.end(), esperados.begin()));

// Good
EXPECT_THAT(ids, ElementsAreArray(esperados));
```

## 8. Dos herramientas para cuando falla y no sabes cuál

```cpp
// Mensaje propio: se concatena al informe de fallo
EXPECT_EQ(esperado, real) << "fallo procesando el fichero " << ruta;

// SCOPED_TRACE: añade contexto a TODOS los fallos que ocurran dentro del ámbito
void compruebaTarifa(int minutos, double esperado) {
    SCOPED_TRACE("minutos = " + std::to_string(minutos));   // ← sin esto, el fallo
    EXPECT_DOUBLE_EQ(esperado, tarifa(minutos));            //   apunta siempre a
}                                                           //   esta misma línea

TEST(Tarifa, EscalaPorTramos) {
    compruebaTarifa(0,   0.0);
    compruebaTarifa(59,  0.5);
    compruebaTarifa(60,  1.0);
}
```

Sin `SCOPED_TRACE`, los tres fallos posibles señalan la misma línea del
*helper* y no sabes cuál de las tres llamadas falló. (Para este patrón concreto
—el mismo test con muchos datos— la herramienta definitiva son los **tests
paramétricos**, que veremos mañana.)

## 9. Qué ganamos y qué pagamos

**Ganamos:** diagnósticos que se leen sin abrir el depurador, tests que
documentan la intención (`ElementsAre` dice más que un `std::equal`), y la
seguridad de que las comparaciones de flotantes y cadenas no fallan por sorpresa.

**Pagamos:** hay que conocer el catálogo. Merece la pena tener abierta la
página de referencia las dos primeras semanas; después salen solas cinco o seis
y el resto se busca.

## 10. Mantra del bloque

> **"La assertion no se elige por lo que comprueba, sino por lo que dirá cuando
> falle."**
> Si el mensaje no te basta para arreglarlo, has elegido mal la macro.

---

## 11. Referencias

**Referencia de consulta diaria (gratuita):**

- **[GoogleTest — Assertions Reference](https://google.github.io/googletest/reference/assertions.html)**
  — el catálogo completo, con las diferencias exactas entre `ASSERT` y `EXPECT`
  y las variantes de flotantes y cadenas. La página que más veces vais a abrir.
- **[GoogleTest — Matchers Reference](https://google.github.io/googletest/reference/matchers.html)**
  — todo lo que cabe en un `EXPECT_THAT`, por categorías. Vale la pena leerla
  entera una vez: hay matchers que ni imaginabais que existían.
- **[GoogleTest — Advanced Guide](https://google.github.io/googletest/advanced.html)**
  — `SCOPED_TRACE`, predicados (`EXPECT_PRED2`), assertions propias y
  **[death tests](https://google.github.io/googletest/advanced.html#death-tests)**
  (comprobar que el código aborta: útil con `assert()` y precondiciones duras).

**Coma flotante:**

- Bruce Dawson, **[*Comparing Floating Point Numbers, 2012 Edition*](https://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/)**
  — **gratuito**. De dónde sale lo de los 4 ULP, y por qué cerca de cero hay
  que usar tolerancia absoluta. Es *el* artículo sobre el tema.
- **[`std::numeric_limits`](https://en.cppreference.com/w/cpp/types/numeric_limits)**
  — `epsilon()`, `infinity()`, `quiet_NaN()`: los valores frontera de los tipos
  numéricos, que reaparecen en el bloque 9 con CORRECT.

**Calidad del test:**

- Google Testing Blog, **[*Test Behavior, Not Implementation*](https://testing.googleblog.com/2013/08/testing-on-toilet-test-behavior-not.html)**
  — **gratuito**, dos minutos. Por qué el `EXPECT_TRUE` gigante del apartado 4
  es además un test frágil.
- Google Testing Blog, **[*Writing Descriptive Test Names*](https://testing.googleblog.com/2014/10/testing-on-toilet-writing-descriptive.html)**
  — **gratuito**. Complementa el nombrado del día 1 con el mensaje de fallo.
- 📖 Jeff Langr, **[*Modern C++ Programming with TDD*](https://pragprog.com/titles/lotdd/modern-c-programming-with-test-driven-development/)**,
  cap. 3 — las assertions vistas desde la práctica en C++, incluidos los
  problemas de comparar objetos propios y cómo enseñarle a GoogleTest a
  imprimirlos (`PrintTo`).
