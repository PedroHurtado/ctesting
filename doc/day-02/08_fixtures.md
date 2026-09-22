# Día 2 — Bloque 8: Fixtures y ciclo de vida del test

> *"Cada test empieza el mundo de cero."*

---

## 1. El problema en una frase

Cuando veinte tests necesitan el mismo *Arrange*, o lo copias veinte veces, o
lo compartes mal y los tests empiezan a depender unos de otros.

## 2. Definición

Un **fixture** (*test fixture*, o *contexto de prueba*) es el estado conocido
desde el que arranca un test. En GoogleTest es una clase que hereda de
`::testing::Test`, y los tests que la usan se declaran con `TEST_F`.

La propiedad que lo hace seguro, y que hay que grabarse:

> **GoogleTest construye una instancia NUEVA del fixture para cada `TEST_F`, y
> la destruye al terminar.** Los tests nunca comparten los miembros del fixture.

Eso es lo que sostiene la **I** de FIRST (*Independent*) del día 1.

## 3. Bad: el *Arrange* copiado veinte veces

```cpp
TEST(Pedido, CalculaElSubtotal) {
    Catalogo catalogo;
    catalogo.registrar({"libro", 20.0});
    catalogo.registrar({"taza",  10.0});
    Cliente cliente{"Ana", Segmento::Estandar};
    Pedido pedido{cliente, catalogo};               // ← 5 líneas de preparación
    pedido.anadir("libro", 2);

    EXPECT_DOUBLE_EQ(40.0, pedido.subtotal());
}

TEST(Pedido, AplicaElDescuentoDeClienteVip) {
    Catalogo catalogo;
    catalogo.registrar({"libro", 20.0});
    catalogo.registrar({"taza",  10.0});
    Cliente cliente{"Ana", Segmento::Vip};          // ← lo único que cambia
    Pedido pedido{cliente, catalogo};
    pedido.anadir("libro", 2);

    EXPECT_DOUBLE_EQ(36.0, pedido.total());
}
// ... y así dieciocho veces más
```

El coste real no es teclear: es que **el día que el constructor de `Pedido`
cambie, hay que tocar veinte tests**. Y que la única línea relevante de cada
test queda escondida entre cinco de ruido.

### Bad peor: el estado compartido "para no repetir"

```cpp
// ¡NO!
Catalogo g_catalogo;                       // ← global, viva entre tests
Pedido   g_pedido{g_cliente, g_catalogo};

TEST(Pedido, AnadirIncrementaLasLineas) {
    g_pedido.anadir("libro", 1);
    EXPECT_EQ(1, g_pedido.lineas().size());    // ← pasa... si se ejecuta el primero
}

TEST(Pedido, VaciarDejaElPedidoSinLineas) {
    g_pedido.vaciar();
    EXPECT_EQ(0, g_pedido.lineas().size());
}
```

Esta suite pasa en verde hoy y falla el martes que viene, cuando alguien añada
un test en medio. Es el caso que caza `--gtest_shuffle` del bloque 6.

## 4. Good: el fixture

```cpp
class PedidoTest : public ::testing::Test {
protected:                                  // ← protected: los TEST_F son subclases
    void SetUp() override {
        catalogo.registrar({"libro", 20.0});
        catalogo.registrar({"taza",  10.0});
    }

    Pedido pedidoDe(Segmento segmento) {    // ← helper: expresa la VARIACIÓN
        Cliente cliente{"Ana", segmento};
        Pedido p{cliente, catalogo};
        p.anadir("libro", 2);
        return p;
    }

    Catalogo catalogo;
};

TEST_F(PedidoTest, CalculaElSubtotalSumandoPrecioPorCantidad) {
    auto pedido = pedidoDe(Segmento::Estandar);
    EXPECT_DOUBLE_EQ(40.0, pedido.subtotal());
}

TEST_F(PedidoTest, AplicaUnDiezPorCientoDeDescuentoAlClienteVip) {
    auto pedido = pedidoDe(Segmento::Vip);
    EXPECT_DOUBLE_EQ(36.0, pedido.total());
}
```

Dos cosas a la vez:

- El **ruido común** sube al `SetUp`.
- Lo que **varía** se hace explícito en el nombre del helper (`pedidoDe(Vip)`).

Y cada `TEST_F` recibe un `catalogo` recién construido. Comprobadlo:

```cpp
class ContadorTest : public ::testing::Test {
protected:
    int veces = 0;
};

TEST_F(ContadorTest, Primero)  { ++veces; EXPECT_EQ(1, veces); }
TEST_F(ContadorTest, Segundo)  { ++veces; EXPECT_EQ(1, veces); }  // ← pasa: es OTRA instancia
```

## 5. El ciclo de vida, en orden

Para una suite con dos `TEST_F`:

```
SetUpTestSuite()            ← UNA vez, antes de todo (static, opcional)
   ├─ constructor  → SetUp() → cuerpo del TEST_F #1 → TearDown() → destructor
   └─ constructor  → SetUp() → cuerpo del TEST_F #2 → TearDown() → destructor
TearDownTestSuite()         ← UNA vez, al final (static, opcional)
```

Y por encima de todo, `::testing::Environment` con su `SetUp`/`TearDown`, que
se ejecuta una vez por **proceso** (arrancar un servidor de pruebas, inicializar
una biblioteca C, fijar el `locale`).

### ¿Constructor o `SetUp`? ¿Destructor o `TearDown`?

| Usa… | Cuando… |
|---|---|
| **Constructor / destructor** | El caso normal en C++ moderno. RAII: el destructor limpia aunque el test aborte. |
| **`SetUp()`** | La preparación puede **fallar** y quieres usar `ASSERT_*` en ella, o necesitas llamar a un método virtual. |
| **`TearDown()`** | Quieres **comprobar algo** al liberar. En un destructor no se puede lanzar, y un `ASSERT_*` ahí es ilegal. |

```cpp
class FicheroTest : public ::testing::Test {
protected:
    void SetUp() override {
        ruta = std::filesystem::temp_directory_path() / "datos.csv";
        std::ofstream f{ruta};
        ASSERT_TRUE(f.is_open()) << "no se pudo crear " << ruta;  // ← por esto, SetUp
        f << "id,nombre\n1,Ana\n";
    }
    void TearDown() override { std::filesystem::remove(ruta); }

    std::filesystem::path ruta;
};
```

Un `ASSERT_*` que falla en `SetUp` **aborta ese test** y GoogleTest **no
ejecuta su cuerpo** — pero sí llama a `TearDown`. Es exactamente lo que quieres:
si el escenario no se pudo montar, el resultado del test no significa nada.

### `SetUpTestSuite`: potente y peligroso

```cpp
class ConsultaTest : public ::testing::Test {
protected:
    static void SetUpTestSuite() {          // ← UNA vez para toda la suite
        indice_ = new IndiceEnMemoria{cargarDe("corpus_grande.txt")};   // 3 segundos
    }
    static void TearDownTestSuite() { delete indice_; indice_ = nullptr; }

    static IndiceEnMemoria* indice_;
};
IndiceEnMemoria* ConsultaTest::indice_ = nullptr;
```

Sirve para preparaciones caras. Pero el recurso es **compartido y mutable**, y
ahí vuelve el problema del apartado 3. La condición para usarlo:

> Solo si el recurso es de **solo lectura** durante los tests. En cuanto un
> test lo modifica, has reintroducido la dependencia de orden, y `--gtest_shuffle`
> te lo demostrará.

## 6. Cuándo el fixture es la respuesta equivocada

El fixture reduce duplicación, pero tiene su propio antipatrón: el
**fixture-dios** con quince miembros del que cada test usa dos.

```cpp
// Bad: para leer un TEST_F hay que subir 40 líneas a ver qué contiene cada miembro
class SistemaTest : public ::testing::Test {
protected:
    Catalogo catalogo;  Cliente cliente;   Pedido pedido;
    Almacen  almacen;   Factura factura;   Auditoria auditoria;
    Tarifa   tarifa;    Usuario usuario;   Sesion sesion;
    // ...
};
```

Síntomas y remedios:

| Síntoma | Qué significa | Remedio |
|---|---|---|
| Cada test usa 2 de los 15 miembros | Hay **varias suites** metidas en una | Dividir en varios fixtures |
| El `SetUp` tiene `if`s según el test | El escenario **no es común** | Helpers con parámetros, como `pedidoDe()` |
| Hay que leer el `SetUp` para entender el test | El *Arrange* es **invisible** | Construir en el test lo que sea relevante |

Y la tensión de fondo, que conviene decir en voz alta:

> En el código de producción, DRY manda. En el código de test, **la claridad
> manda sobre DRY**. Un test debe entenderse **leyéndolo entero de arriba
> abajo**, sin saltar a otro sitio. Duplicar tres líneas obvias suele ser mejor
> que esconderlas en un `SetUp` remoto.

La regla práctica: sube al fixture lo que es **ruido idéntico para todos**; deja
en el test lo que **define ese caso**.

### La alternativa: constructores expresivos

```cpp
// Test Data Builder: el escenario se lee como una frase, sin fixture
auto pedido = UnPedido().deClienteVip().con("libro", 2).build();
EXPECT_DOUBLE_EQ(36.0, pedido.total());
```

Es el patrón *Test Data Builder* de Meszaros. Más trabajo inicial, pero escala
mucho mejor que un fixture cuando los escenarios se multiplican.

## 7. Nombrar la suite

Con `TEST_F`, el primer argumento es **el nombre de la clase fixture**, no un
nombre libre. Convención del curso: `<Clase>Test`.

```cpp
class CuentaTest : public ::testing::Test { ... };
TEST_F(CuentaTest, RetirarDisminuyeElSaldo) { ... }
```

Y se pueden mezclar `TEST` y `TEST_F` en el mismo fichero: los casos que no
necesitan contexto no tienen por qué pagarlo.

## 8. Qué ganamos y qué pagamos

**Ganamos:** el *Arrange* común escrito una sola vez, aislamiento garantizado
entre tests, limpieza fiable de recursos (ficheros, conexiones, temporales), y
un sitio natural donde poner los *helpers* del dominio.

**Pagamos:** una indirección. El test ya no se lee entero en su cuerpo, y el
fixture tiende a engordar si nadie lo vigila. Es un coste real: revisad los
fixtures igual que revisáis el código de producción.

## 9. Mantra del bloque

> **"Un fixture por escenario, no un fixture por fichero."**
> Si tienes que leer el `SetUp` para entender el test, el fixture ha dejado de
> ayudar y ha empezado a esconder.

---

## 10. Referencias

**Documentación (gratuita):**

- **[GoogleTest — Testing Reference](https://google.github.io/googletest/reference/testing.html)**
  — la definición exacta de `TEST_F`, `SetUp`, `TearDown`, `SetUpTestSuite` y
  `::testing::Environment`, con el orden de llamadas del apartado 5.
- **[GoogleTest — Primer, *Test Fixtures*](https://google.github.io/googletest/primer.html)**
  — la introducción corta, con el mismo ejemplo de cola que usa medio internet.
- **[GoogleTest — Advanced Guide](https://google.github.io/googletest/advanced.html)**
  — entornos globales, compartir recursos entre suites y los avisos sobre
  estado compartido del apartado 5.
- **[GoogleTest FAQ](https://google.github.io/googletest/faq.html)**
  — "¿por qué mi fixture se construye tantas veces?" está respondido ahí, y es
  la pregunta que siempre sale la primera semana.

**Patrones y antipatrones de fixture:**

- 📖 Gerard Meszaros, **[*xUnit Test Patterns*](https://www.informit.com/store/xunit-test-patterns-refactoring-test-code-9780131495050)**
  — el catálogo de referencia: *Fresh Fixture*, *Shared Fixture*, *Test Data
  Builder*, y los olores *Obscure Test* y *Erratic Test* del apartado 6.
  Resumen **gratuito** en **[xunitpatterns.com](http://xunitpatterns.com/)**.
- Google Testing Blog, **[*Keep Tests Focused*](https://testing.googleblog.com/2018/06/testing-on-toilet-keep-tests-focused.html)**
  — **gratuito**, dos minutos. Por qué un test que comprueba cinco cosas y un
  fixture que prepara quince son el mismo problema.
- **[*Software Engineering at Google*, cap. 12 "Unit Testing"](https://abseil.io/resources/swe-book/html/ch12.html)**
  — **gratuito**. La sección sobre claridad frente a DRY en el código de test
  desarrolla el argumento del apartado 6 con casos reales.

**Aplicado a C++:**

- 📖 Jeff Langr, **[*Modern C++ Programming with TDD*](https://pragprog.com/titles/lotdd/modern-c-programming-with-test-driven-development/)**,
  cap. 3 — fixtures en C++ con RAII: cuándo el destructor basta y cuándo hace
  falta `TearDown` de verdad.
