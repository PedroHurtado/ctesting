# Día 5 — Bloque 20: Reutilizar y simplificar expresiones condicionales

> *"Cada `if` es una pregunta. Si la haces en cinco sitios, tienes cinco sitios
> donde equivocarte."*

---

## 1. El problema en una frase

Los condicionales crecen solos: cada requisito nuevo añade un `if`, y al año
nadie sabe qué combinación de ramas se ejecuta en cada caso.

## 2. El mapa del bloque

El bloque 17 ya enseñó dos técnicas (**cláusula de guarda** y **Decompose
Conditional**). Hoy completamos la caja de herramientas. **Una técnica por
síntoma:**

| Síntoma | Técnica | § |
|---|---|---|
| Varios `if` seguidos que devuelven **lo mismo** | *Consolidate Conditional Expression* | 4 |
| La **misma línea** en las dos ramas del `if` | *Consolidate Duplicate Conditional Fragments* | 5 |
| El **mismo `switch`** en varias funciones | *Replace Conditional with Polymorphism* (o `std::variant`) | 6 |
| `if (p == nullptr)` repetido en cada uso | *Introduce Special Case* (objeto nulo) | 7 |
| Cadena de `if/else if` que solo **elige un valor** | Condicional → **tabla** | 8 |

Todas las técnicas se aplican igual:

```
  1. Tests que fijan lo que HOY hace el código   (§3)
  2. Una técnica                                 (§4-§8)
  3. Tests en verde
  4. Commit
```

---

## 3. Antes de tocar: tests de caracterización

Un **test de caracterización** no comprueba lo que el código *debería* hacer.
Comprueba lo que **hace ahora**, sea correcto o no. Es la foto del "antes".

Para condicionales, la herramienta es `TEST_P` (bloque 14): **una fila por
combinación de ramas**.

```cpp
struct Caso { TipoEnvio tipo; int kg; Dinero coste; int dias; };

class EnvioTest : public ::testing::TestWithParam<Caso> {};

TEST_P(EnvioTest, CosteYPlazo) {
    const Caso c = GetParam();
    EXPECT_EQ(c.coste, coste(c.tipo, c.kg));
    EXPECT_EQ(c.dias,  plazoDias(c.tipo));
}

INSTANTIATE_TEST_SUITE_P(Tabla, EnvioTest, ::testing::Values(
    Caso{TipoEnvio::Estandar,  5,  4_eur, 3},    // frontera: 5 kg todavía es "ligero"
    Caso{TipoEnvio::Estandar,  6,  7_eur, 3},
    Caso{TipoEnvio::Urgente,   2, 14_eur, 1},
    Caso{TipoEnvio::Recogida, 30,  0_eur, 0}));
```

> **Cómo sé que la tabla está completa:** `gcovr --txt-metric branch` (bloque
> 15). Si una rama sale sin cubrir, falta una fila. Primero la fila, después el
> refactor.

---

## 4. Consolidar condiciones que dan lo mismo

### Bad

```cpp
Dinero descuento(const Pedido& p) {
    if (p.esEmpleado)       return 0_eur;
    if (p.total < 20_eur)   return 0_eur;
    if (p.enRebajas)        return 0_eur;       // ← tres preguntas, una sola respuesta
    return Dinero::centimos(p.total.enCentimos() / 10);
}
```

Parecen tres reglas. Es **una**: *"hay casos sin descuento"*. Y esa regla no
tiene nombre.

### Good

```cpp
bool noTieneDescuento(const Pedido& p) {
    return p.esEmpleado || p.total < 20_eur || p.enRebajas;
}

Dinero descuento(const Pedido& p) {
    if (noTieneDescuento(p)) return 0_eur;
    return Dinero::centimos(p.total.enCentimos() / 10);
}
```

**Aquí está la "reutilización" del temario.** `noTieneDescuento` es ahora una
función con nombre. El carrito, la factura y el correo de confirmación pueden
preguntar lo mismo **llamándola**, en vez de copiar las tres condiciones.

> ⚠️ Solo se consolida si es **la misma regla de negocio**. Si "empleado" y
> "rebajas" son motivos distintos que mañana tendrán resultados distintos,
> dejadlos separados.

---

## 5. Sacar lo repetido de las dos ramas

### Bad

```cpp
void cobrar(Ticket& t, Dinero base, bool urgente) {
    if (urgente) {
        t.total = base + 5_eur;
        t.emitidos++;                 // ← repetido
    } else {
        t.total = base;
        t.emitidos++;                 // ← repetido
    }
}
```

### Good

```cpp
void cobrar(Ticket& t, Dinero base, bool urgente) {
    t.total = urgente ? base + 5_eur : base;
    t.emitidos++;                     // ← una vez, fuera del if
}
```

Lo que se hace **siempre** va fuera del `if`. Dentro solo queda lo que
**cambia**. Parece poco, pero es el origen de muchos bugs: alguien añade una
tercera rama y se olvida de copiar el `emitidos++`.

---

## 6. El mismo `switch` en varios sitios

### Bad

```cpp
enum class TipoEnvio { Estandar, Urgente, Recogida };

Dinero coste(TipoEnvio tipo, int pesoKg) {
    switch (tipo) {
        case TipoEnvio::Estandar: return pesoKg > 5 ? 7_eur : 4_eur;
        case TipoEnvio::Urgente:  return 12_eur + Dinero::euros(pesoKg);
        case TipoEnvio::Recogida: return 0_eur;
    }
    throw std::logic_error{"tipo desconocido"};
}

int plazoDias(TipoEnvio tipo) {
    switch (tipo) { /* ... 3, 1, 0 ... */ }
}

std::string etiqueta(TipoEnvio tipo) {
    switch (tipo) { /* ... */ }                 // ← el tercer switch igual
}
```

Añadir el envío `Internacional` obliga a tocar **tres funciones** (o treinta,
en un proyecto real). Es la **cirugía a escopetazos** del bloque 17.

### Primera ayuda, gratis: `-Wall` y sin `default`

Antes de refactorizar, un truco de C++. Si el `switch` **no tiene `default`**,
GCC avisa de cada caso olvidado (salida real al añadir `Internacional`):

```
envio_bad2.h:9:12:  warning: enumeration value 'Internacional' not handled in switch [-Wswitch]
envio_bad2.h:18:12: warning: enumeration value 'Internacional' not handled in switch [-Wswitch]
envio_bad2.h:27:12: warning: enumeration value 'Internacional' not handled in switch [-Wswitch]
```

> **Regla:** en un `switch` sobre un `enum class`, **no pongáis `default`**.
> El `default` apaga este aviso.

### Good A — polimorfismo

Cada tipo de envío pasa a ser una clase. Cada rama del `switch`, un método.

```cpp
class Envio {
public:
    virtual ~Envio() = default;
    virtual Dinero      coste(int pesoKg) const = 0;
    virtual int         plazoDias()       const = 0;
    virtual std::string etiqueta()        const = 0;
};

class EnvioUrgente : public Envio {
public:
    Dinero coste(int pesoKg) const override { return 12_eur + Dinero::euros(pesoKg); }
    int plazoDias() const override { return 1; }
    std::string etiqueta() const override { return "Urgente (24 h)"; }
};
// EnvioEstandar y RecogidaEnTienda, igual
```

`Internacional` es ahora **una clase nueva**. No se toca nada de lo que ya
funciona: es el **OCP** del curso de patrones.

La red: la misma tabla de §3 se ejecuta contra las dos versiones a la vez
(salida real):

```cpp
TEST_P(EnvioTest, SwitchYPolimorfismoDanLoMismo) {
    const Caso c = GetParam();
    auto envio = crear(c.tipo);

    EXPECT_EQ(c.coste, coste(c.tipo, c.kg));      // el viejo
    EXPECT_EQ(c.coste, envio->coste(c.kg));       // el nuevo
    EXPECT_EQ(etiqueta(c.tipo), envio->etiqueta());
}
```

```
[==========] 4 tests from 1 test suite ran. (0 ms total)
[  PASSED  ] 4 tests.
```

### Good B — `std::variant` (C++17)

Si los tipos de envío son **un conjunto cerrado** (no va a añadir tipos un
cliente de vuestra librería), hay una alternativa sin herencia ni punteros:

```cpp
struct Estandar {};
struct Urgente {};
struct Recogida {};
using Envio = std::variant<Estandar, Urgente, Recogida>;

struct CosteEnvio {
    int pesoKg;
    Dinero operator()(Estandar) const { return pesoKg > 5 ? 7_eur : 4_eur; }
    Dinero operator()(Urgente)  const { return 12_eur + Dinero::euros(pesoKg); }
    Dinero operator()(Recogida) const { return 0_eur; }
};

Dinero c = std::visit(CosteEnvio{2}, Envio{Urgente{}});    // 14 EUR
```

Si os olvidáis de un caso, **no compila** (salida real, recortada):

```
error: no type named 'type' in 'struct std::invoke_result<CosteEnvio, Recogida&&>'
```

El mensaje es feo, pero es un error, no un aviso.

### ¿Cuál elijo?

| Lo que más va a cambiar... | Elige | Por qué |
|---|---|---|
| **Tipos nuevos** (Internacional, Dron...) | **Polimorfismo** | Tipo nuevo = clase nueva; no se toca lo existente |
| **Operaciones nuevas** (seguro, huella de CO₂...) | **`std::variant`** o `switch` | Operación nueva = un visitante nuevo; no se tocan los tipos |
| Nada, son 3 casos y un solo `switch` | **Dejarlo como está** | Un solo `switch` no es un olor. Tres, sí |

---

## 7. El `nullptr` que se pregunta en todas partes

### Bad

```cpp
const Cliente* buscar(int id);            // nullptr si no existe

std::string saludo(int id) {
    const Cliente* c = buscar(id);
    return "Hola, " + (c ? c->nombre() : std::string{"cliente"});   // ← if nulo nº 1
}

int descuento(int id) {
    const Cliente* c = buscar(id);
    return c ? c->porcentajeDescuento() : 0;                          // ← if nulo nº 2
}
```

Cada función que usa un cliente repite la pregunta. Y la que se olvide de
hacerla, desreferencia un `nullptr`.

### Good — un objeto para el caso especial

```cpp
class ClienteDesconocido : public Cliente {
public:
    std::string nombre() const override { return "cliente"; }
    int porcentajeDescuento() const override { return 0; }
};

const Cliente& buscar(int id) {
    static const ClienteDesconocido desconocido;
    auto it = clientes.find(id);
    if (it == clientes.end()) return desconocido;                    // ← el único if
    return it->second;
}

std::string saludo(int id)  { return "Hola, " + buscar(id).nombre(); }
int descuento(int id)       { return buscar(id).porcentajeDescuento(); }
```

La pregunta se hace **una vez**, en `buscar`. El resto del código ya no sabe
que existe el caso especial. Y devuelve una **referencia**: no hay puntero que
pueda ser nulo.

> Es el patrón *Null Object*. Solo sirve si el caso especial tiene un
> comportamiento **razonable** (nombre genérico, descuento 0). Si "cliente no
> encontrado" es un error, lanzad una excepción o devolved `std::optional`.

---

## 8. Una cadena de `if` que solo elige un valor

### Bad

```cpp
Dinero tarifa(Zona z) {
    if (z == Zona::Peninsula)     return 5_eur;
    else if (z == Zona::Baleares) return 9_eur;
    else if (z == Zona::Canarias) return 15_eur;
    else                          return 12_eur;    // ← ¿Ceuta? ¿o "cualquier otra"?
}
```

### Good

```cpp
Dinero tarifa(Zona z) {
    static const std::map<Zona, Dinero> tarifas{
        {Zona::Peninsula, 5_eur}, {Zona::Baleares, 9_eur},
        {Zona::Canarias, 15_eur}, {Zona::Ceuta,    12_eur},
    };
    return tarifas.at(z);       // zona sin tarifa → std::out_of_range, no un 12 inventado
}
```

Cuando las ramas **no hacen nada**, solo devuelven un dato, eso no es lógica:
son **datos**. Y los datos van en una tabla. Mañana esa tabla puede venir de
un fichero de configuración sin tocar el código.

---

## 9. Qué ganamos y qué pagamos

**Ganamos:**
- Reglas con nombre que se **reutilizan** en vez de copiarse.
- Añadir un caso nuevo sin tocar los que ya funcionan.
- Menos sitios donde olvidarse de una rama.

**Pagamos:**
- Más clases o más funciones pequeñas.
- La lógica ya no está "toda junta en un sitio": hay que saber navegar.
- Con polimorfismo, un `unique_ptr` y una llamada virtual donde antes había un `enum`.

## 10. Conexión con lo que viene

Todas estas técnicas son **mecánicas**: un nombre del catálogo y unos pasos
fijos. Eso es justo lo que un asistente de IA hace bien. El bloque 21 lo pone a
prueba... y enseña lo que se le escapa.

## 11. Mantra del bloque

> **"Primero la foto (caracterización), después el cambio, y el `if` que queda
> tiene nombre."**

---

## 12. Referencias

- **[refactoring.com — catálogo](https://refactoring.com/catalog/)**
  — **gratuito**. Busca *Consolidate Conditional Expression*, *Replace
  Conditional with Polymorphism* e *Introduce Special Case*: la mecánica exacta
  de §4, §6 y §7.
- **[Refactoring Guru — Simplificar expresiones condicionales](https://refactoring.guru/refactoring/techniques/simplifying-conditional-expressions)**
  — **gratuito**, con ejemplos visuales. Toda la familia de técnicas de este bloque.
- **[Wikipedia — *Characterization test*](https://en.wikipedia.org/wiki/Characterization_test)**
  — **gratuito**. Qué es un test de caracterización (§3) y por qué va antes de tocar nada.
- **[cppreference — `std::visit`](https://en.cppreference.com/w/cpp/utility/variant/visit)**
  — **gratuito**. La alternativa de §6 B, con el truco `overloaded` para escribir
  el visitante con lambdas.
- 📖 Martin Fowler,
  **[*Refactoring*](https://martinfowler.com/books/refactoring.html)** (2ª ed.),
  cap. 10 *Simplifying Conditional Logic* — el capítulo entero es este bloque.
- 📖 Michael Feathers,
  **[*Working Effectively with Legacy Code*](https://www.informit.com/store/working-effectively-with-legacy-code-9780131177055)**,
  cap. 13 — escribir tests de caracterización para código que no entiendes.
