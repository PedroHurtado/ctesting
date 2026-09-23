# Día 3 — Bloque 13: gMock en la práctica

> *"El doble que escribes a mano envejece; el que declaras, no."*

---

## 1. El problema en una frase

Los tres dobles del bloque 11 —`FakeCanvas`, `StringReader`, `RecordingWriter`—
son unas cuarenta líneas escritas a mano, y **cada vez que alguien añade un
método a la interfaz, dejan de compilar**.

## 2. Qué es gMock

La otra mitad del paquete GoogleTest. Le declaráis qué métodos tiene la
interfaz y **genera el doble**: contadores, registro de argumentos, mensajes de
fallo y verificación automática incluidos.

```cpp
#include <gmock/gmock.h>
using ::testing::_;          // el comodín
using ::testing::Return;
```

Ya está enlazado desde el día 2: `GTest::gmock_main` incluye a gtest. No hay que
tocar el `CMakeLists.txt`.

> **gMock genera stubs, spies y mocks. No genera fakes.** Un fake tiene lógica
> propia (el `std::map` del bloque 12) y eso hay que escribirlo. Es la razón
> práctica de haber aprendido a distinguirlos.

---

## 3. Bad — el doble a mano

El `FakeCanvas` del bloque 11, tal cual lo dejamos:

```cpp
class FakeCanvas : public ICanvas {
public:
    void Add(std::unique_ptr<IShape> shape) override {
        ++adds;                                     // ← contador a mano
        shapes_.push_back(std::move(shape));
    }
    std::size_t Count() const override { return shapes_.size(); }
    /* ...y los demás métodos, obligatorios aunque el test no los use... */
    int adds = 0;
    int removes = 0;
private:
    std::vector<std::unique_ptr<IShape>> shapes_;
};
```

```cpp
EXPECT_EQ(2, canvas.adds);      // ← si falla: "Expected equality of 2 and 1"
```

Lo que duele:

- **El contador es código vuestro**, y puede tener bugs. Un `++adds` en el sitio
  equivocado convierte el test en una mentira.
- **No dice con qué se llamó.** `adds == 2` no distingue dos círculos de dos
  cuadrados.
- **Rompe al crecer la interfaz.** Añadid un método virtual puro a `ICanvas` y
  los tres dobles dejan de compilar a la vez.
- **El mensaje de fallo es pobre**: `2` contra `1`, sin decir qué llamada faltó.

## 4. Good — el mismo doble, declarado

```cpp
class MockCanvas : public ICanvas {
public:
    MOCK_METHOD(void, Add, (std::unique_ptr<IShape> shape), (override));
    MOCK_METHOD(std::unique_ptr<IShape>, Remove, (IShape* handle), (override));
    MOCK_METHOD(IShape&, At, (std::size_t index), (override));
    MOCK_METHOD(std::size_t, Count, (), (const, override));
};
```

```cpp
MockCanvas canvas;
EXPECT_CALL(canvas, Add(_)).Times(2);      // ← el contador del bloque 11, declarado

App app{canvas, reader, writer};
app.Run();
```

Y si solo llega una llamada, gMock os dice exactamente qué pasó:

```
Actual function call count doesn't match EXPECT_CALL(canvas, Add(_))...
         Expected: to be called twice
           Actual: called once - unsatisfied and active
```

### La anatomía de `MOCK_METHOD`

```cpp
MOCK_METHOD(std::size_t, Count, (), (const, override));
//          └ retorno    └nombre └args  └ cualificadores
```

Los cualificadores son los de la función real: `const`, `override`, `noexcept`,
`ref`, `Calltype(...)`. **Tienen que coincidir con la firma de la interfaz**, o
`override` no compila —que es justo lo que queréis que pase—.

> ⚠️ **El error que sale siempre: las comas.** El preprocesador no sabe de
> plantillas y parte `std::map<int, Pedido>` en dos argumentos. Hay dos salidas:
> ```cpp
> MOCK_METHOD(void, Guardar, ((const std::map<int, Pedido>&)), (override));  // paréntesis extra
> using Catalogo = std::map<int, Pedido>;                                    // o un alias (mejor)
> MOCK_METHOD(void, Guardar, (const Catalogo&), (override));
> ```

---

## 5. La anatomía de `EXPECT_CALL`

```cpp
EXPECT_CALL(mock, Metodo(matchers...))
    .Times(cardinalidad)
    .WillOnce(accion)
    .WillRepeatedly(accion);
```

Tres partes, tres preguntas: **con qué** se llama, **cuántas veces**, y **qué
devuelve**.

### 5.1 Matchers — *con qué* se llama

| Matcher | Casa con |
|---|---|
| `_` | cualquier cosa |
| `42`, `"hola"` | ese valor exacto (usa `operator==`) |
| `Eq(x)`, `Ne(x)`, `Gt(x)`, `Ge(x)`, `Lt(x)`, `Le(x)` | comparaciones |
| `NotNull()`, `IsNull()` | punteros |
| `HasSubstr("error")`, `StartsWith("GET")`, `MatchesRegex(...)` | cadenas |
| `ElementsAre(1, 2, 3)`, `Contains(x)`, `SizeIs(3)` | contenedores |
| `Field(&Pedido::total, Gt(100))` | un campo del argumento |
| `Truly(pred)` | un predicado vuestro |
| `AllOf(Ge(0), Le(10))`, `AnyOf(...)`, `Not(...)` | combinaciones |

Son **los mismos matchers de `EXPECT_THAT`** que ya usasteis en el bloque 7. No
hay nada nuevo que aprender: se reutiliza el vocabulario.

```cpp
EXPECT_CALL(pasarela, Cobrar(Gt(0), Field(&Tarjeta::caducada, false)));
```

### 5.2 Cardinalidad — *cuántas* veces

| Expresión | Significado |
|---|---|
| `.Times(2)` | exactamente dos |
| `.Times(0)` | **nunca** — "esto no debe ocurrir" |
| `.Times(AtLeast(1))` / `AtMost(3)` / `Between(1, 4)` | rangos |
| *(omitida)* | 1 vez si no hay `Will*`; si hay *n* `WillOnce`, exactamente *n* |

`.Times(0)` es de los usos más valiosos y el que nadie recuerda:

```cpp
EXPECT_CALL(notificador, Enviar(_, _)).Times(0);   // pedido rechazado: NO se avisa al cliente
```

### 5.3 Acciones — *qué* hace el doble

| Acción | Para qué |
|---|---|
| `Return(x)` | devolver un valor |
| `ReturnRef(obj)` | devolver una referencia |
| `Return(ByMove(std::move(p)))` | devolver un `unique_ptr` (**solo vale una vez**) |
| `Throw(std::runtime_error{"caída"})` | **probar el camino de error** |
| `SetArgReferee<0>(valor)` | rellenar un parámetro de salida |
| `DoAll(a1, a2)` | encadenar acciones |
| `Invoke(f)` / lambda | lógica de verdad cuando nada de lo anterior llega |

```cpp
EXPECT_CALL(pasarela, Cobrar(_, _))
    .WillOnce(Return(true))                    // 1ª llamada
    .WillOnce(Throw(std::runtime_error{"timeout"}))  // 2ª: la red se cae
    .WillRepeatedly(Return(false));            // el resto
```

Ese `Throw` es la razón de peso para usar dobles: **en producción no sabéis
provocar un timeout a voluntad; aquí sí.**

---

## 6. `ON_CALL` frente a `EXPECT_CALL`, y los tres caracteres del mock

Esta es la distinción del bloque 12 llevada a la herramienta:

```cpp
ON_CALL(cambio, EurosPorDolar()).WillByDefault(Return(1.10));  // STUB: qué devuelve
EXPECT_CALL(pasarela, Cobrar(9900, _));                        // MOCK: qué debe ocurrir
```

- **`ON_CALL` no verifica nada.** Configura comportamiento. Es vuestro stub.
- **`EXPECT_CALL` es una afirmación.** Si no se cumple, el test falla **en el
  destructor del mock**.

> **La regla de Google:** *"Use `ON_CALL` by default, `EXPECT_CALL` only when you
> intend to verify the interaction."* Cada `EXPECT_CALL` que no sea una
> afirmación real es un acoplamiento gratuito a la implementación.

Y si no configuráis un método que sí se llama, gMock avisa:

```
GMOCK WARNING: Uninteresting mock function call - returning default value.
```

Para controlar eso hay tres envoltorios:

| Envoltorio | Llamada no esperada | Cuándo usarlo |
|---|---|---|
| `NiceMock<T>` | la ignora en silencio | **El habitual.** Interfaces anchas de las que os importan dos métodos |
| `T` (*naggy*) | avisa por consola | Por defecto. El aviso es útil al escribir el test |
| `StrictMock<T>` | **falla el test** | Cuando *"no debe llamarse a nada más"* es parte de la especificación |

```cpp
NiceMock<MockCanvas> canvas;     // no me molestes con Count() y At()
```

> **No abuséis de `StrictMock`.** Convierte cualquier llamada nueva —aunque sea
> inofensiva— en un test rojo. Es el camino más rápido a una suite que hay que
> "arreglar" en cada refactor.

---

## 7. La promesa del bloque 10: el límite diario

El séptimo caso de la kata quedó escrito y sin implementar:

```
[ ] Retirada por encima del límite diario se rechaza
```

La costura ya la diseñamos. Ahora el doble lo declara gMock:

```cpp
class MockReloj : public Reloj {
public:
    MOCK_METHOD(std::chrono::system_clock::time_point, ahora, (), (const, override));
};
```

Necesitamos fabricar fechas concretas. En **C++17 `<chrono>` no tiene
calendario** (eso llegó en C++20), así que una función auxiliar de tres líneas:

```cpp
// C++17. En C++20 esto es, literalmente, sys_days{2026y/September/21}.
std::chrono::system_clock::time_point dia(int anio, int mes, int dia_) {
    std::tm t{};
    t.tm_year = anio - 1900;   t.tm_mon = mes - 1;   t.tm_mday = dia_;
    return std::chrono::system_clock::from_time_t(std::mktime(&t));
}
```

```cpp
TEST(Cuenta, RechazaRetiradaQueSuperaElLimiteDiario) {
    NiceMock<MockReloj> reloj;
    ON_CALL(reloj, ahora()).WillByDefault(Return(dia(2026, 9, 21)));  // STUB: fijo el día

    Cuenta cuenta{reloj};
    cuenta.ingresar(1000_eur);
    cuenta.retirar(400_eur);                                 // dentro del límite

    EXPECT_THROW(cuenta.retirar(200_eur), LimiteDiarioSuperado);
}

TEST(Cuenta, ElLimiteSeReiniciaAlDiaSiguiente) {
    NiceMock<MockReloj> reloj;
    EXPECT_CALL(reloj, ahora())
        .WillOnce(Return(dia(2026, 9, 21)))             // lunes
        .WillRepeatedly(Return(dia(2026, 9, 22)));      // martes

    Cuenta cuenta{reloj};
    cuenta.ingresar(1000_eur);
    cuenta.retirar(500_eur);

    EXPECT_NO_THROW(cuenta.retirar(500_eur));   // otro día, otro límite
}
```

Fijaos en lo que acaba de pasar: **el segundo test hace viajar en el tiempo a la
cuenta**. Sin doble, ese test exige esperar a mañana. Es el ejemplo más limpio
de por qué existe todo este bloque.

Y el `ahora()` es el **stub** del bloque 12 (dirige la ejecución), no un mock:
por eso el primer test usa `ON_CALL` y afirma sobre la excepción, no sobre la
llamada.

---

## 8. Cuatro cosas que solo pasan en C++

### 8.1 Parámetros *move-only*

`ICanvas::Add` recibe un `std::unique_ptr`. gMock lo soporta, pero el valor **no
se puede copiar para guardarlo**, así que no podéis afirmar sobre él después:

```cpp
MOCK_METHOD(void, Add, (std::unique_ptr<IShape> shape), (override));

EXPECT_CALL(canvas, Add(_)).Times(2);                      // ✔ cuenta llamadas
EXPECT_CALL(canvas, Add(Pointee(Field(&IShape::color, 2))));  // ✔ inspecciona por matcher
```

Para **devolver** un move-only hace falta `ByMove`, y solo sirve una vez:

```cpp
EXPECT_CALL(canvas, Remove(_))
    .WillOnce(Return(ByMove(std::make_unique<Circle>())));  // un solo WillOnce
```

### 8.2 Destructores

Un mock necesita destructor **público y virtual** en la interfaz. `ICanvas` lo
tiene `protected` y no virtual (bloque 11, regla de Sutter), lo cual está bien
porque nadie lo posee: el mock se crea en la pila del test y se destruye ahí.
Lo que no podréis es tener un `std::unique_ptr<ICanvas>`.

### 8.3 Mockear sin herencia

Si la clase **no tiene métodos virtuales** —el `std::istream` del bloque 11, o
código heredado— la salida no es gMock: es **polimorfismo estático**.

```cpp
template <typename Reloj>
class Cuenta { /* ... */ };          // el doble se inyecta como parámetro de plantilla
```

Sin coste en tiempo de ejecución, pero la clase deja de poder cambiarse en
caliente. La *cook book* de gMock lo llama *"Mocking Non-virtual Methods"*.

### 8.4 Cuándo se verifica

En el **destructor del mock**. Si el mock vive más que el test —un `static`, o
un `new` sin `delete`— la verificación nunca ocurre y el test miente. Para
forzarla antes:

```cpp
Mock::VerifyAndClearExpectations(&canvas);
```

---

## 9. Orden de las llamadas

Por defecto, **el orden no importa**. Cuando sí importa:

```cpp
{
    InSequence seq;                       // todo lo de este ámbito, en orden
    EXPECT_CALL(bd, Abrir());
    EXPECT_CALL(bd, Escribir(_));
    EXPECT_CALL(bd, Cerrar());
}
```

> **Pensadlo dos veces.** Exigir un orden es acoplarse a la implementación. Se
> justifica cuando el orden **es** el requisito (abrir antes de escribir); no
> para "documentar" cómo está escrito el método hoy.

---

## 10. Errores típicos

| Síntoma | Causa | Solución |
|---|---|---|
| `GMOCK WARNING: Uninteresting mock function call` | Método llamado sin configurar | `NiceMock<T>`, o añadid un `ON_CALL` |
| El test pasa pero no debería | `EXPECT_CALL` escrito **después** de la acción | La expectativa va siempre **antes** del *Act* |
| `Actual: never called` y juraríais que sí | Dos `EXPECT_CALL` solapados: gMock casa con **el último declarado** | Poned el más general primero y el específico después |
| `error: ... marked 'override' but does not override` | Falta `const` en `MOCK_METHOD` | Copiad la firma literal de la interfaz |
| Error de macro con demasiados argumentos | Coma dentro de una plantilla | Paréntesis extra, o un alias `using` |
| El mock nunca falla | Se destruye fuera del test, o nunca | `Mock::VerifyAndClearExpectations` |

---

## 11. Qué ganamos y qué pagamos

**Ganamos:** los tres dobles a mano del bloque 11 se quedan en tres bloques de
`MOCK_METHOD`; los mensajes de fallo dicen qué llamada faltó y con qué
argumentos; la interfaz puede crecer sin romper todos los dobles; y podemos
provocar errores —timeouts, disco lleno— a voluntad.

**Pagamos:** una sintaxis de macros que el compilador explica fatal; tiempo de
compilación; y, sobre todo, **la tentación de afirmar de más**. Cada
`EXPECT_CALL` fija una decisión de implementación. Un test con seis
`EXPECT_CALL` seguidos no describe un comportamiento: describe el cuerpo del
método, y se romperá en el primer refactor.

## 12. Conexión con lo que viene

Ahora que fijar el estado del sistema cuesta dos líneas, escribir **veinte
casos** cuesta veinte copias del mismo test. El **bloque 14** los escribe una
sola vez con `TEST_P`, y por fin cumple la promesa del bloque 9: *"esta tabla,
escrita una vez"*.

## 13. Mantra del bloque

> **"`ON_CALL` para que funcione, `EXPECT_CALL` para afirmar."**
> Si borrando un `EXPECT_CALL` el test sigue probando lo mismo, ese
> `EXPECT_CALL` sobraba.

---

## 14. Referencias

**Para escribir gMock hoy (todo gratuito):**

- **[gMock for Dummies](https://google.github.io/googletest/gmock_for_dummies.html)**
  — el tutorial oficial, de principio a fin en media hora. Cubre los apartados
  4 a 6 de este bloque. Empezad por aquí.
- **[gMock Cheat Sheet](https://google.github.io/googletest/gmock_cheat_sheet.html)**
  — una página con **todos** los matchers, cardinalidades y acciones. Es la que
  hay que tener abierta mientras se escriben tests; sustituye a las tablas del
  apartado 5.
- **[gMock Cookbook](https://google.github.io/googletest/gmock_cook_book.html)**
  — las recetas del apartado 8: *move-only types*, métodos no virtuales,
  plantillas, métodos sobrecargados, mocks de funciones libres. Cuando algo no
  encaja, la respuesta está aquí.
- **[Mocking Reference](https://google.github.io/googletest/reference/mocking.html)**
  y **[Actions Reference](https://google.github.io/googletest/reference/actions.html)**
  — la referencia formal de `MOCK_METHOD`, `EXPECT_CALL` y cada acción.
  Consulta, no lectura.
- **[gMock FAQ](https://google.github.io/googletest/gmock_faq.html)** — la mitad
  de la tabla de errores del apartado 10 sale de aquí, empezando por por qué
  gMock casa con la **última** expectativa declarada.

**Para no pasarse:**

- 📖 Jeff Langr,
  **[*Modern C++ Programming with TDD*](https://pragprog.com/titles/lotdd/modern-c-programming-with-test-driven-development/)**,
  cap. 5 — gMock aplicado con criterio, y la discusión de cuándo una interfaz
  existe solo para poder mockear (que a veces es legítimo y a veces es un olor).
- **[*Software Engineering at Google*, cap. 13](https://abseil.io/resources/swe-book/html/ch13.html)**
  — **gratuito**. La sección *"Over-reliance on mocking"* es el contrapeso
  exacto al apartado 11: tests que pasan siempre y sistemas que no arrancan.
