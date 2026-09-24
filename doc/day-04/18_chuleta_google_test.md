# Día 4 — Bloque 18: La chuleta de Google Test

Hoy es un día de teclear. Esta es la única teoría: **todo lo que Google Test y
gMock nos dan, en una hoja**, para tenerla abierta al lado del editor.

Nada es nuevo: se vio los días 2 y 3. Aquí está junto y ordenado.

Un solo escenario para todo el documento, el del bloque 12:

```cpp
struct Cuenta { std::string email; int saldoCentimos = 0; };

class INotificador {                                   // el colaborador
public:
    virtual ~INotificador() = default;
    virtual bool Enviar(const std::string& destinatario, const std::string& texto) = 0;
};

class Avisador {                                       // el sujeto bajo prueba
public:
    explicit Avisador(INotificador& n) : notificador_{n} {}
    bool Revisar(const Cuenta& c) {                    // avisa si el saldo es negativo
        if (c.saldoCentimos >= 0) return false;
        return notificador_.Enviar(c.email, "Saldo negativo: " + Euros(c.saldoCentimos));
    }
    int RevisarTodas(const std::vector<Cuenta>& cs);   // devuelve cuántos avisos salieron
private:
    INotificador& notificador_;
};
```

Los cinco apartados:

| # | Pregunta | Herramienta |
|---|---|---|
| 1 | ¿Cómo compruebo un resultado? | `EXPECT_*` / `ASSERT_*` / `EXPECT_THAT` |
| 2 | ¿Cómo compruebo que me llamaron *n* veces y con *qué*? | `EXPECT_CALL(...).Times(n)` |
| 3 | ¿Cómo preparo y limpio antes y después? | `SetUp` / `TearDown` y compañía |
| 4 | ¿Cómo fabrico un doble? | `MOCK_METHOD` + `NiceMock` / `ON_CALL` / `EXPECT_CALL` |
| 5 | ¿Qué es un test y qué es una suite? | `TEST`, `TEST_F`, `TEST_P` |

> Todo el código de este documento se ha compilado y ejecutado (C++17,
> `-Wall -Wextra`, GoogleTest 1.18.0). Las salidas que aparecen son reales.

---

## 1. Aserciones

### La regla de oro: `EXPECT` sigue, `ASSERT` corta

| Familia | Si falla… | Úsala cuando… |
|---|---|---|
| `EXPECT_*` | Apunta el fallo **y sigue** | Casi siempre. Ves todos los fallos de una vez |
| `ASSERT_*` | Apunta el fallo **y sale del test** | Seguir no tiene sentido o sería peligroso (índice, puntero nulo) |

```cpp
TEST(Aserciones, AssertCortaEExpectSigue) {
    std::vector<int> v;                                  // vacío a propósito
    EXPECT_EQ(1u, v.size()) << "EXPECT: apunta el fallo y sigue";
    ASSERT_EQ(1u, v.size()) << "ASSERT: apunta el fallo y sale del test";
    EXPECT_EQ(42, v[0]);                                 // ← nunca se ejecuta. Menos mal
}
```

```
chuleta_test.cpp:81: Failure
Expected equality of these values:
  1u
    Which is: 1
  v.size()
    Which is: 0
EXPECT: apunta el fallo y sigue

chuleta_test.cpp:82: Failure
Expected equality of these values:
  1u
    Which is: 1
  v.size()
    Which is: 0
ASSERT: apunta el fallo y sale del test

[  FAILED  ] Aserciones.AssertCortaEExpectSigue (0 ms)
```

Dos fallos, no tres: el `ASSERT` impidió leer `v[0]` de un vector vacío.
Y el `<< "..."` añade vuestro mensaje al fallo. Funciona con **todas**.

### Las que vais a usar (todas existen también como `ASSERT_*`)

| Qué compruebo | Aserción | Ejemplo |
|---|---|---|
| Verdadero / falso | `EXPECT_TRUE(c)` · `EXPECT_FALSE(c)` | `EXPECT_FALSE(avisador.Revisar(cuenta))` |
| Igual / distinto | `EXPECT_EQ(esperado, real)` · `EXPECT_NE` | `EXPECT_EQ("-12.00 EUR", Euros(-1200))` |
| Orden | `EXPECT_LT` `EXPECT_LE` `EXPECT_GT` `EXPECT_GE` | `EXPECT_GE(saldo, 0)` |
| Cadenas C (`char*`) | `EXPECT_STREQ` · `EXPECT_STRCASEEQ` | `EXPECT_STREQ("abc", s.c_str())` |
| Coma flotante | `EXPECT_DOUBLE_EQ` · `EXPECT_FLOAT_EQ` | `EXPECT_DOUBLE_EQ(0.3, 0.1 + 0.2)` |
| Coma flotante con margen | `EXPECT_NEAR(a, b, margen)` | `EXPECT_NEAR(3.14159, 3.14, 0.01)` |
| Lanza esta excepción | `EXPECT_THROW(sentencia, Tipo)` | `EXPECT_THROW(Dividir(1, 0), std::invalid_argument)` |
| No lanza nada | `EXPECT_NO_THROW(sentencia)` | `EXPECT_NO_THROW(Dividir(1, 2))` |
| Lanza lo que sea | `EXPECT_ANY_THROW(sentencia)` | |
| Cualquier cosa, con *matchers* | `EXPECT_THAT(valor, matcher)` | ver abajo |
| Fallar / apuntar a mano | `FAIL()` · `ADD_FAILURE()` · `SUCCEED()` | `FAIL() << "no debería llegar aquí"` |

Tres trampas que salen siempre:

- **`EXPECT_EQ` con `char*`** compara **punteros**, no texto. Usad `std::string` o `EXPECT_STREQ`.
- **`EXPECT_EQ` con `double`** falla con `0.1 + 0.2`. Usad `EXPECT_DOUBLE_EQ` o `EXPECT_NEAR`.
- **El orden importa para leer el fallo**: primero el esperado, luego el real.

### `EXPECT_THAT`: una aserción, muchas preguntas

```cpp
EXPECT_THAT(Euros(-1200), AllOf(StartsWith("-12"), HasSubstr("EUR")));
EXPECT_THAT(std::vector<int>({1, 2, 3}), ElementsAre(1, 2, 3));
```

| Para… | Matchers |
|---|---|
| Valores | `Eq(x)` `Ne(x)` `Lt(x)` `Ge(x)` `DoubleNear(x, m)` `_` (cualquiera) |
| Cadenas | `HasSubstr` `StartsWith` `EndsWith` `MatchesRegex` `StrCaseEq` |
| Contenedores | `ElementsAre(...)` `UnorderedElementsAre(...)` `Contains(x)` `IsEmpty()` `SizeIs(n)` `Each(m)` |
| Combinar | `AllOf(m1, m2)` `AnyOf(m1, m2)` `Not(m)` |
| Punteros y campos | `IsNull()` `NotNull()` `Pointee(m)` `Field(&T::campo, m)` `Property(&T::get, m)` |

**Los mismos matchers sirven dentro de `EXPECT_CALL`.** Aprendéis una vez, usáis dos.

---

## 2. Comprobar que un spy fue llamado *n* veces y con *estos* valores

Hay dos maneras. Las dos responden a la misma pregunta: **¿qué le llegó al
colaborador?**

### 2.1 Spy a mano: apunta en una lista y miráis la lista al final

```cpp
class NotificadorSpy : public INotificador {
public:
    struct Mensaje {
        std::string destinatario, texto;
        bool operator==(const Mensaje& o) const {
            return destinatario == o.destinatario && texto == o.texto;
        }
    };
    bool Enviar(const std::string& d, const std::string& t) override {
        enviados.push_back({d, t});                      // apunta y ya
        return true;
    }
    std::vector<Mensaje> enviados;                       // ← lo que mira el test
};

TEST(Spy, AMano_CuantasVecesYConQue) {
    NotificadorSpy espia;                                            // Arrange
    Avisador avisador{espia};

    avisador.RevisarTodas({{"ana@x.com", -1200},                     // Act
                           {"luis@x.com", 500},
                           {"eva@x.com", -1}});

    ASSERT_EQ(2u, espia.enviados.size());                            // Assert: cuántas
    EXPECT_THAT(espia.enviados, ElementsAre(                         //         y con qué
        NotificadorSpy::Mensaje{"ana@x.com", "Saldo negativo: -12.00 EUR"},
        NotificadorSpy::Mensaje{"eva@x.com", "Saldo negativo: -0.01 EUR"}));
}
```

El `ASSERT` del tamaño va primero: si no hay dos mensajes, mirar dentro no tiene sentido.

### 2.2 Con gMock: se dice **antes** y se comprueba **solo**

```cpp
class MockNotificador : public INotificador {
public:
    MOCK_METHOD(bool, Enviar,
                (const std::string& destinatario, const std::string& texto), (override));
};

TEST(Spy, ConGMock_CuantasVecesYConQue) {
    MockNotificador notificador;                                     // Arrange
    Avisador avisador{notificador};

    EXPECT_CALL(notificador, Enviar("ana@x.com", HasSubstr("-12.00")))  // con qué
        .Times(1)                                                       // cuántas
        .WillOnce(Return(true));                                        // qué responde
    EXPECT_CALL(notificador, Enviar("eva@x.com", _))
        .Times(1)
        .WillOnce(Return(true));

    int avisos = avisador.RevisarTodas({{"ana@x.com", -1200},        // Act
                                        {"luis@x.com", 500},
                                        {"eva@x.com", -1}});

    EXPECT_EQ(2, avisos);                                            // Assert
}   // ← aquí, al destruirse el mock, gMock comprueba los EXPECT_CALL
```

**Ojo, que es lo que más confunde:** en gMock el `EXPECT_CALL` va **antes** del
Act. Si lo ponéis después, la llamada ya ocurrió y nadie la estaba esperando.

### La anatomía de `EXPECT_CALL`

```
EXPECT_CALL(objeto, Metodo(matcher1, matcher2))     ← CON QUÉ   (valores exactos, _, HasSubstr...)
    .Times(n)                                       ← CUÁNTAS   (se puede omitir)
    .WillOnce(Return(x))                            ← QUÉ HACE  la 1ª vez
    .WillRepeatedly(Return(y));                     ← QUÉ HACE  el resto
```

| Quiero decir… | Escribo |
|---|---|
| Exactamente 3 veces | `.Times(3)` |
| Nunca | `.Times(0)` |
| Al menos / como mucho | `.Times(AtLeast(2))` · `.Times(AtMost(2))` |
| Me da igual cuántas | `.Times(AnyNumber())` |
| Con cualquier argumento | `Enviar(_, _)` |
| En este orden | Declarar los `EXPECT_CALL` dentro de un bloque con `InSequence orden;` |
| Quiero el valor para mirarlo yo | `.WillOnce(DoAll(SaveArg<1>(&texto), Return(true)))` |

Si no ponéis `.Times()`, gMock lo deduce: un `WillOnce` → 1 vez; dos `WillOnce` → 2 veces.

**N veces, sin importar los argumentos:**

```cpp
EXPECT_CALL(notificador, Enviar(_, _)).Times(3).WillRepeatedly(Return(true));
avisador.RevisarTodas({{"a@x", -1}, {"b@x", -2}, {"c@x", -3}});
```

**En orden:**

```cpp
{
    InSequence orden;
    EXPECT_CALL(notificador, Enviar("ana@x.com", _)).WillOnce(Return(true));
    EXPECT_CALL(notificador, Enviar("eva@x.com", _)).WillOnce(Return(true));
}
avisador.RevisarTodas({{"ana@x.com", -1}, {"eva@x.com", -1}});
```

**Capturar el argumento** (el estilo spy, dentro de gMock):

```cpp
std::string texto;
EXPECT_CALL(notificador, Enviar(_, _))
    .WillOnce(DoAll(SaveArg<1>(&texto), Return(true)));   // SaveArg<1>: el 2º parámetro

avisador.Revisar({"ana@x.com", -1200});

EXPECT_EQ("Saldo negativo: -12.00 EUR", texto);
```

### Así falla — y así se lee

**Cuando falla el *cuántas*** (`.Times(2)`, pero solo se llamó una vez):

```
chuleta_test.cpp:172: Failure
Actual function call count doesn't match EXPECT_CALL(notificador, Enviar("ana@x.com", _))...
         Expected: to be called twice
           Actual: called once - unsatisfied and active
```

**Cuando falla el *con qué*** (esperaba `-12.00`, llegó `-13.00`):

```
Unexpected mock function call - returning default value.
    Function call: Enviar(@0xd1343ff1f0 "ana@x.com", @0xd1343ff030 "Saldo negativo: -13.00 EUR")
          Returns: false
Google Mock tried the following 1 expectation, but it didn't match:

chuleta_test.cpp:267: EXPECT_CALL(notificador, Enviar("ana@x.com", HasSubstr("-12.00")))...
  Expected arg #1: has substring "-12.00"
           Actual: "Saldo negativo: -13.00 EUR"
```

Leedlo así: *"llegó esta llamada, probé esta expectativa, falló el argumento #1"*.
Los argumentos se cuentan desde 0.

### ¿Spy a mano o `EXPECT_CALL`?

| | Spy a mano | `EXPECT_CALL` |
|---|---|---|
| Dónde va la comprobación | **Después** del Act | **Antes** del Act |
| Quién comprueba | Vosotros, con `EXPECT_*` | gMock, al destruir el mock |
| Bueno para… | Rebuscar en los datos, muchas llamadas | *"Tenía que ocurrir esto"* |
| Coste | Escribir la clase | Una línea |

---

## 3. Before / After

Google Test no tiene `@Before` ni `@After` sueltos. Los tiene una **clase fixture**,
y se usan con `TEST_F` en lugar de `TEST`.

```cpp
class CicloDeVida : public ::testing::Test {
protected:
    static void SetUpTestSuite()    { std::puts("  SetUpTestSuite    (1 vez, antes del primero)"); }
    static void TearDownTestSuite() { std::puts("  TearDownTestSuite (1 vez, despues del ultimo)"); }
    CicloDeVida()                   { std::puts("    constructor"); }
    ~CicloDeVida() override         { std::puts("    destructor"); }
    void SetUp() override           { std::puts("    SetUp"); }
    void TearDown() override        { std::puts("    TearDown"); }
};

TEST_F(CicloDeVida, Primero) { std::puts("      cuerpo Primero"); }
TEST_F(CicloDeVida, Segundo) { std::puts("      cuerpo Segundo"); }

class EntornoGlobal : public ::testing::Environment {
public:
    void SetUp() override    { std::puts("Environment::SetUp (antes de TODO)"); }
    void TearDown() override { std::puts("Environment::TearDown (despues de TODO)"); }
};
// en main():  ::testing::AddGlobalTestEnvironment(new EntornoGlobal);
```

Salida real, recortada:

```
Environment::SetUp (antes de TODO)
...
  SetUpTestSuite    (1 vez, antes del primero)
[ RUN      ] CicloDeVida.Primero
    constructor
    SetUp
      cuerpo Primero
    TearDown
    destructor
[       OK ] CicloDeVida.Primero (0 ms)
[ RUN      ] CicloDeVida.Segundo
    constructor
    SetUp
      cuerpo Segundo
    TearDown
    destructor
[       OK ] CicloDeVida.Segundo (0 ms)
  TearDownTestSuite (1 vez, despues del ultimo)
...
Environment::TearDown (despues de TODO)
```

**Fijaos:** constructor y destructor salen **en cada test**. Cada `TEST_F` recibe
un objeto fixture **nuevo**. Por eso los tests no se contaminan entre sí.

| Se ejecuta… | Before | After | Equivale en JUnit a… |
|---|---|---|---|
| Antes/después de **cada test** | constructor o `SetUp()` | destructor o `TearDown()` | `@BeforeEach` / `@AfterEach` |
| Una vez por **suite** | `static SetUpTestSuite()` | `static TearDownTestSuite()` | `@BeforeAll` / `@AfterAll` |
| Una vez por **programa** | `Environment::SetUp()` | `Environment::TearDown()` | — |

¿Constructor o `SetUp`? **Constructor por defecto.** `SetUp` solo si necesitáis
llamar a algo virtual o usar `ASSERT_*` en la preparación (en un constructor no se puede).

Lo compartido en `SetUpTestSuite` es **estado que sobrevive entre tests**: usadlo
solo para cosas caras y de solo lectura.

---

## 4. Creación de dobles

Un solo mock generado sirve para casi todos los dobles. **Lo que cambia es cómo
lo usa el test**:

```cpp
class MockNotificador : public INotificador {
public:
    MOCK_METHOD(bool, Enviar,
                (const std::string& destinatario, const std::string& texto), (override));
};
//          ↑ retorno  ↑ nombre   ↑ parámetros, entre paréntesis        ↑ calificadores
//                                                                      (override), (const, override)…
```

| Doble | Se fabrica con… | Ejemplo |
|---|---|---|
| **Dummy** | `NiceMock<>` y nada más | `NiceMock<MockNotificador> dummy;` |
| **Stub** | `NiceMock<>` + `ON_CALL` | `ON_CALL(stub, Enviar(_, _)).WillByDefault(Return(false));` |
| **Spy** | Clase a mano con una lista, o `SaveArg` | ver apartado 2 |
| **Mock** | `EXPECT_CALL` (con `StrictMock<>` si no queréis sorpresas) | ver abajo |
| **Fake** | Clase a mano que funciona de verdad | ver abajo |

**Dummy** — está porque el constructor lo pide; el test no llega a usarlo:

```cpp
TEST(Dobles, Dummy) {
    NiceMock<MockNotificador> dummy;
    Avisador avisador{dummy};
    EXPECT_FALSE(avisador.Revisar({"ana@x.com", 500}));   // saldo positivo: nadie llama a Enviar
}
```

**Stub** — da de comer una respuesta; el `EXPECT` habla de **vuestro** código:

```cpp
TEST(Dobles, Stub) {
    NiceMock<MockNotificador> stub;
    ON_CALL(stub, Enviar(_, _)).WillByDefault(Return(false));   // "el correo falla"
    Avisador avisador{stub};
    EXPECT_FALSE(avisador.Revisar({"ana@x.com", -1200}));
}
```

**Mock** — sabe de antemano qué llamada espera; el `EXPECT` habla de **la llamada**:

```cpp
TEST(Dobles, Mock) {
    StrictMock<MockNotificador> mock;
    EXPECT_CALL(mock, Enviar("ana@x.com", "Saldo negativo: -12.00 EUR")).WillOnce(Return(true));
    Avisador avisador{mock};
    avisador.Revisar({"ana@x.com", -1200});
}
```

**Fake** — una implementación de verdad, pero pequeña. Se escribe a mano:

```cpp
class NotificadorFake : public INotificador {
public:
    bool Enviar(const std::string& d, const std::string& t) override {
        buzones[d].push_back(t);
        return true;
    }
    std::vector<std::string> Buzon(const std::string& d) const {
        auto it = buzones.find(d);
        return it == buzones.end() ? std::vector<std::string>{} : it->second;
    }
private:
    std::map<std::string, std::vector<std::string>> buzones;
};

TEST(Dobles, Fake) {
    NotificadorFake fake;
    Avisador avisador{fake};
    avisador.RevisarTodas({{"ana@x.com", -100}, {"ana@x.com", -200}});
    EXPECT_THAT(fake.Buzon("ana@x.com"), ElementsAre(HasSubstr("-1.00"), HasSubstr("-2.00")));
    EXPECT_THAT(fake.Buzon("luis@x.com"), IsEmpty());
}
```

### `ON_CALL` frente a `EXPECT_CALL`

| | `ON_CALL` | `EXPECT_CALL` |
|---|---|---|
| Dice… | *"Si te llaman, responde esto"* | *"Te tienen que llamar así"* |
| ¿Falla si no ocurre? | No | **Sí** |
| Doble que fabrica | Stub | Mock |

### Los tres caracteres del mock, ante una llamada que nadie esperaba

| Envoltorio | Llamada no esperada | Úsalo para… |
|---|---|---|
| `NiceMock<M>` | Silencio | Dummies y stubs |
| `M` (a secas, *naggy*) | Aviso `GMOCK WARNING`, el test pasa | Por defecto |
| `StrictMock<M>` | **El test falla** | Mocks donde cualquier llamada extra es un error |

---

## 5. Test y suite

### Test

Un **test** es una función con dos nombres: la suite a la que pertenece y el
caso que prueba.

```cpp
TEST(Avisador, NoAvisaSiElSaldoEsPositivo) {
//   ↑ suite    ↑ nombre del test
    NiceMock<MockNotificador> n;
    EXPECT_FALSE(Avisador{n}.Revisar({"a@x", 1}));
}
```

El nombre completo es `Avisador.NoAvisaSiElSaldoEsPositivo`. Así sale en la
salida y así se filtra.

### Suite

**Sí existe, pero no se declara.** Una suite es **el conjunto de tests que
comparten el primer nombre**. No hay `describe { }` que los envuelva: basta con
repetir el nombre.

```cpp
TEST(Avisador, NoAvisaSiElSaldoEsPositivo) { ... }   // ┐
TEST(Avisador, AvisaSiElSaldoEsNegativo)   { ... }   // ├ suite "Avisador"
TEST(Avisador, ElAvisoLlevaElSaldo)        { ... }   // ┘
```

Si la suite necesita preparación común, **la suite se convierte en una clase**
(el fixture del apartado 3) y los tests pasan a `TEST_F`. El primer nombre es
entonces **el nombre de la clase**.

> En código antiguo veréis *test case* donde hoy se dice *test suite*
> (`SetUpTestCase`, `TYPED_TEST_CASE`…). Es lo mismo con el nombre viejo.

### Las cuatro formas de declarar un test

| Macro | Qué es | Primer argumento |
|---|---|---|
| `TEST(Suite, Nombre)` | Test suelto | Nombre libre de suite |
| `TEST_F(Fixture, Nombre)` | Test con Before/After | La clase fixture |
| `TEST_P(Fixture, Nombre)` + `INSTANTIATE_TEST_SUITE_P` | El mismo test, una vez por parámetro | Clase que hereda de `TestWithParam<T>` |
| `TYPED_TEST(Fixture, Nombre)` | El mismo test, una vez por tipo | Clase plantilla |

`TEST_P` en un vistazo (la tabla de valores frontera del día 2, escrita una vez):

```cpp
class AvisadorPorSaldo : public ::testing::TestWithParam<std::pair<int, bool>> {};

TEST_P(AvisadorPorSaldo, AvisaSoloSiEsNegativo) {
    auto [saldo, avisa] = GetParam();
    NiceMock<MockNotificador> n;
    ON_CALL(n, Enviar(_, _)).WillByDefault(Return(true));
    EXPECT_EQ(avisa, Avisador{n}.Revisar({"a@x", saldo}));
}

INSTANTIATE_TEST_SUITE_P(Frontera, AvisadorPorSaldo,
    ::testing::Values(std::make_pair(-1, true),
                      std::make_pair( 0, false),
                      std::make_pair( 1, false)));
```

```
[ RUN      ] Frontera/AvisadorPorSaldo.AvisaSoloSiEsNegativo/0
[       OK ] Frontera/AvisadorPorSaldo.AvisaSoloSiEsNegativo/0 (0 ms)
[ RUN      ] Frontera/AvisadorPorSaldo.AvisaSoloSiEsNegativo/1
[       OK ] Frontera/AvisadorPorSaldo.AvisaSoloSiEsNegativo/1 (0 ms)
[ RUN      ] Frontera/AvisadorPorSaldo.AvisaSoloSiEsNegativo/2
[       OK ] Frontera/AvisadorPorSaldo.AvisaSoloSiEsNegativo/2 (0 ms)
```

### Apagar, saltar y filtrar

| Quiero… | Escribo | Sale como |
|---|---|---|
| Desactivar un test (compila, no corre) | `TEST(Avisador, DISABLED_Pendiente)` | `[ DISABLED ]` + `YOU HAVE 1 DISABLED TEST` |
| Saltarlo según el entorno | `GTEST_SKIP() << "sin SMTP en esta maquina";` | `[  SKIPPED ]` |
| Ejecutar una suite | `./tests --gtest_filter=Avisador.*` | |
| Ejecutar todo menos una suite | `./tests --gtest_filter=-Spy.*` | |
| Ver qué tests hay | `./tests --gtest_list_tests` | |
| Repetir para cazar tests inestables | `./tests --gtest_repeat=100 --gtest_shuffle` | |

---

## 6. La chuleta de la chuleta

| Necesito… | Uso |
|---|---|
| Comprobar un valor | `EXPECT_EQ(esperado, real)` |
| Parar si esto falla | `ASSERT_*` |
| Comprobar texto, listas, combinaciones | `EXPECT_THAT(valor, matcher)` |
| Que me llamen *n* veces con *estos* valores | `EXPECT_CALL(m, F(v1, v2)).Times(n)` — **antes** del Act |
| Mirar el argumento yo mismo | `SaveArg<i>(&var)` o un spy a mano |
| Preparar/limpiar en cada test | Fixture + `TEST_F` + constructor/destructor |
| Preparar una vez por suite | `static SetUpTestSuite()` |
| Un doble que no hace nada | `NiceMock<M>` |
| Un doble que responde | `NiceMock<M>` + `ON_CALL(...).WillByDefault(...)` |
| Un doble que exige | `StrictMock<M>` + `EXPECT_CALL` |
| Un doble que funciona | Clase *fake* a mano |
| Agrupar tests | Mismo primer nombre en `TEST(Suite, ...)` |
| El mismo test con muchos datos | `TEST_P` + `INSTANTIATE_TEST_SUITE_P` |

---

## 7. Mantra del bloque

> **`EXPECT` para seguir, `ASSERT` para parar.
> `ON_CALL` para responder, `EXPECT_CALL` para exigir — y siempre antes del Act.**

---

## Referencias

- **[GoogleTest Primer](https://google.github.io/googletest/primer.html)** —
  gratuito. La introducción oficial: test, suite, fixture y el ciclo de vida
  exactamente como aquí. Diez minutos.
- **[Assertions Reference](https://google.github.io/googletest/reference/assertions.html)** —
  gratuito. La lista completa de `EXPECT_*`/`ASSERT_*`, incluidas las que no
  caben en la tabla (predicados, *death tests*).
- **[Matchers Reference](https://google.github.io/googletest/reference/matchers.html)** —
  gratuito. Todos los matchers para `EXPECT_THAT` y `EXPECT_CALL`. La página
  que más vais a abrir hoy.
- **[gMock Cheat Sheet](https://google.github.io/googletest/gmock_cheat_sheet.html)** —
  gratuito. La chuleta oficial de gMock: `MOCK_METHOD`, cardinalidades,
  acciones, secuencias. Complementa el apartado 2 y el 4.
- **[Mocking Reference](https://google.github.io/googletest/reference/mocking.html)** —
  gratuito. La referencia de `EXPECT_CALL`/`ON_CALL` cláusula a cláusula, para
  cuando la chuleta se queda corta.
- **[Testing Reference](https://google.github.io/googletest/reference/testing.html)** —
  gratuito. `TEST_P`, `TYPED_TEST`, `Environment` y las clases base, con sus
  firmas exactas.
- **[Advanced GoogleTest Topics](https://google.github.io/googletest/advanced.html)** —
  gratuito. Filtros, `GTEST_SKIP`, repetición y barajado, y todas las banderas
  de la línea de comandos.
