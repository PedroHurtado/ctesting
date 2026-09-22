# Día 2 — Bloque 11: Caso práctico — hacer testeable el Paint

> *"Un test que no puede fallar no es un test."*

---

## 1. El problema en una frase

Cogemos el **Paint** que construimos en el curso anterior —SOLID, cuatro
patrones, README ejemplar, suite en verde— y le pedimos una sola cosa nueva:
**un test más**. No se puede. Y al investigar por qué, descubrimos que la suite
que estaba en verde tampoco probaba nada.

Es el bloque menos teórico del día: no hay concepto nuevo, hay un proyecto real
al que le aplicamos lo de ayer y lo de hoy.

## 2. El punto de partida

El proyecto vive en `02-Patrones c++/paint/`. Un Paint sin interfaz gráfica que
lee órdenes por `cin`:

```
circle 5 2 0 0        square 3 1 10 10        move 0 4 4
duplicate 1           undo / redo             print / help / exit
```

Cuatro patrones dentro: **Singleton** (`Canvas`), **Prototype**
(`IShape::Clone`), **Factory** (`ShapeFactory` con registro) y **Command**
(`ICommand` + `CommandManager`). Y `IWriter` como puerto de salida, para que la
figura no sepa dónde se imprime.

Sobre el papel, un ejemplo de manual. El mapa de dependencias decía otra cosa:

```
        ANTES                                  DESPUÉS

  App ──► Canvas  (concreto, Singleton)   App ──► ICanvas ◄── Canvas
  App ──► std::istream                    App ──► IReader ◄── ConsoleReader
  App ──► IWriter                         App ──► IWriter ◄── ConsoleWriter
  Comandos ──► Canvas                     Comandos ──► ICanvas
  App::Run: if/else de 9 ramas            App::Run: 3 ramas fijas
```

Cinco hallazgos. Los cinco se descubrieron **compilando**, no razonando.

---

## 3. Hallazgo 1 — La suite estaba en verde y no comprobaba nada

El más grave, y el que más veces vais a encontraros fuera del aula.

### Bad

```cmake
# CMakeLists.txt raíz
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Tipo de build" FORCE)
endif()                                  # ← Release define NDEBUG
```

```cpp
// tests/test_paint.cpp
#include <cassert>

assert(canvas.Count() == 1);             // ← con NDEBUG esto es (void)0
```

`Release` define `NDEBUG`, y con `NDEBUG` **`<cassert>` convierte todos los
`assert` en nada**. Literalmente:

```cpp
// lo que dice el estándar que hace <cassert> con NDEBUG definido
#define assert(condition) ((void)0)
```

La suite compilaba, enlazaba, CTest la daba por buena y devolvía 0. Durante
todo el curso anterior. Sin probar una sola cosa.

### Cómo se detecta: el canario

No lo deduzcáis, **provocadlo**. Metes una aserción imposible y compruebas que
falla:

```cpp
int main() {
    assert(false && "canario: esto DEBE fallar");
    // ... el resto de los tests
}
```

```
$ ./paint_tests.exe ; echo $?
0                     ← los assert están desactivados. Suite decorativa.
```

Después del arreglo:

```
Assertion failed: false && "canario: esto DEBE fallar", file test_paint.cpp, line 291
127                   ← ahora sí muerde
```

### Good

```cmake
# tests/CMakeLists.txt
add_executable(paint_tests test_paint.cpp)
target_link_libraries(paint_tests PRIVATE paint_core)

# En Release, CMake define NDEBUG y <cassert> anula todos los assert.
# Lo quitamos SOLO para este target.
if(MSVC)
    target_compile_options(paint_tests PRIVATE /UNDEBUG)
else()
    target_compile_options(paint_tests PRIVATE -UNDEBUG)
endif()
```

### La lección, que es de ayer

Esto es la **S de FIRST**: *Self-validating*. Un test que no puede fallar no es
un test, es una declaración de intenciones que compila.

> **Regla práctica:** la primera vez que entráis en un proyecto ajeno, antes de
> escribir código, **romped un test a propósito**. Si la suite sigue en verde,
> ya sabéis lo que vale.

GoogleTest, por cierto, **no tiene este problema**: `EXPECT_*` y `ASSERT_*` son
funciones de verdad, no macros que dependan de `NDEBUG`. Es una razón más,
además de las del bloque 6, para no montar una suite a base de `assert`.

---

## 4. Hallazgo 2 — El `if/else` que crecía con cada comando

### Bad

```cpp
void App::Run(std::istream& input) {
    while (std::getline(input, line)) {
        std::istringstream in(line);
        std::string verb;
        in >> verb;

        if      (verb == "exit")  break;
        else if (verb == "help")  Help();
        else if (verb == "print") canvas_.Print(writer_);
        else if (verb == "undo")  { /* ... */ }
        else if (verb == "redo")  { /* ... */ }
        else if (verb == "move")  { /* ... */ }
        else if (verb == "duplicate") { /* ... */ }
        else if (factory_.Knows(verb)) { /* ... */ }
        else writer_.Write("comando desconocido: " + verb);
    }                              // ← 9 ramas, y una más por cada comando nuevo
}
```

Lo irónico: **el mismo proyecto ya tenía la solución escrita**. Añadir una
*figura* nueva no obligaba a tocar `ShapeFactory`, porque las figuras se
registran:

```cpp
factory.Register("circle", [](std::istream& in) { /* ... */ });
```

OCP resuelto para las figuras, roto para los verbos, en el mismo ejecutable.

### Good: el mismo registro, para acciones

```cpp
enum class Status { Continue, Quit };

struct AppContext {              // todo lo que una acción puede necesitar
    ICanvas& canvas;
    IWriter& writer;
    CommandManager& commands;
    const CommandRegistry& registry;
    const ShapeFactory& factory;
};

using Action = std::function<Status(AppContext&, std::istream& args)>;
```

Las acciones son **funciones libres sin estado**, no métodos de `App`. Por eso
registrar una no obliga a tocar `App`:

```cpp
registry.Register("undo", "", "deshace la ultima accion",
                  [](AppContext& ctx, std::istream&) {
                      ctx.writer.Write(ctx.commands.Undo() ? "deshecho"
                                                           : "nada que deshacer");
                      return Status::Continue;
                  });
```

Y `Run` se queda en **tres ramas, y siempre tres**:

```cpp
if (registry_.Knows(verb)) {                      // 1. verbo registrado
    if (registry_.Run(verb, context, args) == Status::Quit) break;
} else if (factory_.Knows(verb)) {                // 2. figura conocida
    commands_.Execute(std::make_unique<AddShapeCommand>(
        canvas_, factory_.Create(line)));
} else {                                          // 3. ni una cosa ni otra
    writer_.Write("comando desconocido: " + verb);
}
```

### El test que lo demuestra

Aquí está lo importante para nosotros: **OCP es una afirmación verificable**.
No se comprueba leyendo el código, se comprueba con un test que registra algo
nuevo desde fuera:

```cpp
static void TestNewVerbWithoutTouchingApp() {
    FakeCanvas canvas;
    StringReader reader({"circle 5 2 0 0", "vaciar", "print"});
    RecordingWriter writer;

    App app(canvas, reader, writer);
    app.Registry().Register("vaciar", "", "borra el lienzo",
                            [](AppContext& ctx, std::istream&) {
                                ctx.canvas.Clear();
                                ctx.writer.Write("lienzo vaciado");
                                return Status::Continue;
                            });
    app.Run();

    assert(canvas.Count() == 0);
    assert(writer.Contains("lienzo vaciado"));
}
```

Si mañana alguien mete un `else if` a mano en `App::Run`, este test sigue
pasando... pero ya no hará falta escribirlo así, y eso es justo lo que hay que
vigilar en la revisión de código.

### Regalo: la ayuda dejó de mentir

El `help` tenía el texto escrito a mano. Ahora recorre el registro y la
fábrica, y cada figura declara **su propia** sintaxis. Registramos un triángulo
con parámetros distintos, sin tocar nada, y salió solo:

```
  circle <radio> <color> <x> <y>            anade un circulo
  square <lado> <color> <x> <y>             anade un cuadrado
  triangle <base> <altura> <color> <x> <y>  anade un triangulo   ← solo
```

Un texto de ayuda escrito a mano es duplicación esperando a quedarse obsoleta.

---

## 5. Hallazgo 3 — ¿Se puede mockear un `std::istream`?

El `App::Run(std::istream&)` original parecía una violación de DIP. **No lo
era**, y el matiz importa porque mañana entran los dobles de verdad.

### Lo que sí se podía ya

`App` dependía de `std::istream`, no de `std::cin`. Es una abstracción. Un
*stub* costaba una línea:

```cpp
std::istringstream in("circle 5 2 0 0\nexit\n");
app.Run(in);                                  // ← ya era sustituible
```

### Mockearlo de verdad: sí, pero no al `istream`

`std::istream` **no tiene funciones miembro virtuales**. El compilador es tajante:

```cpp
struct Espia : std::istream {
    std::istream& operator>>(int& v) override { return *this; }
};
// error: 'std::istream& Espia::operator>>(int&)' marked 'override',
//        but does not override
```

`operator>>`, `get`, `read`, `getline`: ninguna es virtual. Lo único virtual es
el destructor. El punto de extensión real es **`std::streambuf`**, que sí está
lleno de virtuales protegidas (`underflow`, `uflow`, `xsgetn`, `seekoff`…):

```cpp
class MockBuf : public std::streambuf {
public:
    int underflow_calls = 0;
protected:
    int_type underflow() override {
        ++underflow_calls;                      // afirmo sobre la interacción
        if (pos_ >= data_.size()) return traits_type::eof();
        chunk_ = data_[pos_++];
        setg(&chunk_, &chunk_, &chunk_ + 1);
        return traits_type::to_int_type(chunk_);
    }
    /* ... */
};

MockBuf buf("circle 5 2 0 0\n");
std::istream in(&buf);              // el istream es real; el falso es el buffer
```

Funciona. Permite incluso simular que la entrada se muere a mitad de línea,
cosa que un `istringstream` no da. Así que la respuesta honesta es **sí, se
puede**.

### Entonces, ¿por qué lo cambiamos igual?

|                       | `std::istream`                   | `IReader`                |
|-----------------------|----------------------------------|--------------------------|
| Stub                  | 1 línea (`istringstream`)        | ~10 líneas               |
| Mock real             | ~25 líneas de `streambuf`        | 1 función virtual        |
| **Qué mockeas**       | **el transporte de bytes**       | **la operación que usas**|
| Interfaz que ve `App` | ~40 miembros                     | 1                        |

La fila que decide es la tercera. Mockear `streambuf` es mockear **una capa por
debajo** de donde está tu lógica: te obliga a razonar sobre `underflow` y bytes
cuando tu REPL solo quiere *"dame la siguiente línea"*. Es la misma razón por
la que mockeáis vuestro repositorio y no el driver de la base de datos.

### Good

```cpp
class IReader {                       // el puerto: simétrico de IWriter
public:
    virtual ~IReader() = default;
    virtual bool ReadLine(std::string& line) = 0;
};
```

Y el doble de test, entero:

```cpp
class StringReader : public IReader {
public:
    explicit StringReader(std::vector<std::string> lines)
        : lines_(std::move(lines)) {}

    bool ReadLine(std::string& line) override {
        if (next_ >= lines_.size()) return false;
        line = lines_[next_++];
        return true;
    }
private:
    std::vector<std::string> lines_;
    std::size_t next_ = 0;
};
```

Con eso, **el REPL completo** se prueba sin tocar la consola:

```cpp
StringReader reader({"circle 5 2 0 0", "duplicate 0", "undo", "exit"});
RecordingWriter writer;
App app(canvas, reader, writer);
app.Run();

assert(writer.Contains("deshecho"));
```

### El principio que se violaba no era DIP

Era **ISP**: la App necesitaba *una* operación y recibía formateo, locales,
`seekg` y flags de estado. Y en la práctica, la costura estaba en el sitio
equivocado.

> **Cuidado con el argumento "es que no se puede testear".** Casi siempre *se
> puede*; lo que pasa es que sale caro o te obliga a probar la capa
> equivocada. Decidlo así, porque si lo decís mal, el primer alumno espabilado
> os contesta "pues yo lo mockeo" — y tendrá razón.

---

## 6. Hallazgo 4 — Una interfaz para testear que casi rompe el dominio

Los comandos dependían de `Canvas`, que es Singleton: imposible probarlos
contra un lienzo falso. La solución es evidente —extraer `ICanvas`— y tiene una
trampa que **no avisa**.

### Bad: la interfaz "normal"

```cpp
class ICanvas {
public:
    virtual ~ICanvas() = default;        // public virtual, lo de siempre
    virtual void Add(std::unique_ptr<IShape> shape) = 0;
    /* ... */
};

class Canvas : public ICanvas {
public:
    static ICanvas& Instance();
private:
    Canvas()  = default;
    ~Canvas() = default;                 // la "cuarta defensa" del Singleton
};
```

Con eso, **esto compila limpio, con `-Wall -Wextra`**:

```cpp
paint::ICanvas& c = paint::Canvas::Instance();
delete &c;                               // ← ni un solo warning
```

Y en ejecución:

```
  ctor Canvas
  dtor Canvas  <-- EJECUTADO
RUN_EXIT=127          (crash: free() sobre memoria estática)
```

El `~Canvas()` privado no protege de nada: el acceso al destructor se comprueba
sobre el **tipo estático** (`ICanvas`, donde es público) y la llamada es
virtual, así que en tiempo de ejecución ya no hay control de acceso. El objeto
es estático: se destruye con el `delete`, se libera memoria que no vino del
*heap*, y el runtime lo destruiría **otra vez** al salir de `main`.

### Bad 2: el intento razonable que tampoco vale

Alguien propone heredar en `protected` para ocultar los miembros:

```cpp
class Canvas : protected ICanvas { /* ... */ };
```

No sirve. La herencia `protected` restringe la conversión `Canvas*` →
`ICanvas*` **fuera** de la clase, pero `Instance()` es miembro de `Canvas`:
hace la conversión dentro y te entrega la referencia ya convertida. La
restricción llega tarde. Comprobado: compila, y el `delete` también.

### Good: destructor `protected` y no virtual

```cpp
class ICanvas {
public:
    virtual void Add(std::unique_ptr<IShape> shape) = 0;
    virtual std::unique_ptr<IShape> Remove(IShape* handle) = 0;
    virtual IShape& At(std::size_t index) = 0;
    virtual std::size_t Count() const = 0;
    /* ... */
protected:
    ~ICanvas() = default;                // ← ni public, ni virtual
};
```

```
error: 'paint::ICanvas::~ICanvas()' is protected within this context
```

Es la regla de Sutter: **destructor `public` y virtual, o `protected` y no
virtual**. Como nadie *posee* el lienzo (siempre se maneja por referencia), la
segunda opción es la correcta, y de paso mantiene la defensa del Singleton.

El precio: nadie puede tener un `unique_ptr<ICanvas>`. Aquí es deliberado.

### Y el doble que todo esto hace posible

```cpp
class FakeCanvas : public ICanvas {       // sin Singleton, y contando llamadas
public:
    void Add(std::unique_ptr<IShape> shape) override {
        ++adds;
        shapes_.push_back(std::move(shape));
    }
    std::size_t Count() const override { return shapes_.size(); }
    /* ... */
    int adds = 0;
    int removes = 0;
private:
    std::vector<std::unique_ptr<IShape>> shapes_;
};
```

### La honestidad que hay que decir en voz alta

En cuanto existe `ICanvas`, **el Singleton no aporta absolutamente nada**: el
*composition root* podría crear el único lienzo y repartir la referencia, que
es justo lo que ya hace `main()`. Sigue ahí porque el patrón había que
enseñarlo, no porque haga falta.

Esa frase, dicha delante del código, vale más que tres transparencias sobre los
inconvenientes del Singleton.

---

## 7. Hallazgo 5 — Verificar en vez de razonar

Los cinco hallazgos salieron igual: **escribiendo el programa más pequeño que
demuestra la afirmación y compilándolo**.

| Afirmación                                    | Cómo se comprobó                    |
|-----------------------------------------------|-------------------------------------|
| "Los `assert` están desactivados"             | `assert(false)` → exit 0            |
| "`delete` del Singleton compila"              | 12 líneas + `g++ -Wall -Wextra`     |
| "Heredar `protected` lo arregla"              | Se compiló: **no lo arregla**       |
| "`istream` no se puede mockear"               | Se compiló: **sí se puede**         |
| "Añadir una figura no toca `App`"             | Un `triangle` registrado desde fuera|

Dos de las cinco intuiciones eran **falsas**, y las dos se habrían colado en
una discusión de pizarra. Mañana trabajamos con IA como apoyo al refactor
(bloque de *code smells*), y esta es la regla de casa: **una afirmación técnica
que no has ejecutado es una hipótesis**, la diga un compañero, un libro, o un
modelo. El compilador no tiene opinión.

---

## 8. Qué ganamos y qué pagamos

| Ganamos                                                | Pagamos                                        |
|--------------------------------------------------------|------------------------------------------------|
| Tests que **pueden fallar** (12, antes 5 decorativos)   | Una línea de CMake que hay que entender         |
| El REPL entero probable sin consola                     | 3 dobles escritos a mano (~40 líneas)           |
| Comandos probables sin arrastrar el Singleton           | Una interfaz más, con un destructor "raro"      |
| Verbos y figuras nuevos sin tocar `App`                 | Indirección: leer el flujo cuesta más           |
| Ayuda que no puede quedarse obsoleta                    | `Register` con cuatro argumentos                |

**Cuándo NO hacer esto:** si el REPL tuviera tres comandos y nadie fuera a
añadir más, el `if/else` era la respuesta correcta. El registro se paga solo
cuando la lista crece o cuando terceros la extienden. OCP no es gratis; es una
apuesta sobre por dónde va a cambiar el código.

---

## 9. Conexión con lo que viene

- **Los dobles de hoy están escritos a mano.** `FakeCanvas`, `StringReader` y
  `RecordingWriter` son unas 40 líneas. Mañana, **gMock** genera eso —y además
  permite afirmar sobre *cuántas veces* y *con qué argumentos* se llamó, que es
  lo que separa un *fake* de un *mock* de verdad.
- **`FakeCanvas` cuenta `adds` y `removes` a mano.** Guardad ese detalle:
  mañana es un `EXPECT_CALL(canvas, Add(_)).Times(2)`.
- **`IReader` es exactamente la costura** que el día 1 llamábamos así. Aquí se
  ve por qué se diseña *antes*: extraerla después obligó a tocar cinco ficheros.
- **La cobertura del día 3** os va a decir, en números, lo que el canario dijo
  en binario. Un proyecto con `NDEBUG` mal puesto da cobertura alta y verifica
  cero.

---

## 10. Mantra

> ### *"Rompe un test a propósito antes de fiarte de la suite. Y compila la afirmación antes de defenderla."*

---

## Referencias

**El hallazgo del `NDEBUG`:**

- **[cppreference — `assert`](https://en.cppreference.com/w/cpp/error/assert)**
  — **gratuito**. La frase exacta del estándar: con `NDEBUG` definido, `assert`
  se expande a `((void)0)`. Media página que explica el hallazgo 1 entero.
- **[CMake — `CMAKE_BUILD_TYPE`](https://cmake.org/cmake/help/latest/variable/CMAKE_BUILD_TYPE.html)**
  — **gratuito**. Qué flags añade cada configuración. Ahí se ve que `Release`
  trae `-DNDEBUG` de serie, y por qué el fallo era invisible.
- Martin Fowler, **[*Self Testing Code*](https://martinfowler.com/bliki/SelfTestingCode.html)**
  — **gratuito**, cinco minutos. La definición de la que sale la *S* de FIRST, y
  por qué una suite en la que no confías es peor que no tener suite.
- **[xUnit Test Patterns — índice de *Test Smells*](http://xunitpatterns.com/Test%20Smells.html)**
  — **gratuito**. El catálogo donde vive este olor y sus parientes. Útil para
  ponerle nombre a lo que os encontréis en proyectos heredados.

**Dobles de prueba (y el puente al día 3):**

- 📖 Gerard Meszaros, **[*xUnit Test Patterns*](https://www.informit.com/store/xunit-test-patterns-refactoring-test-code-9780131495050)**
  — el catálogo canónico. Las fichas **[*Test Double*](http://xunitpatterns.com/Test%20Double.html)**
  y **[*Fake Object*](http://xunitpatterns.com/Fake%20Object.html)** están
  **gratis** en la web y definen exactamente qué son `FakeCanvas` y
  `StringReader`.
- 📖 Michael Feathers, **[*Working Effectively with Legacy Code*](https://www.informit.com/store/working-effectively-with-legacy-code-9780131177055)**
  — el libro de las **costuras**. Los capítulos 9 y 10 ("No puedo meter esta
  clase en un arnés de pruebas") son literalmente el hallazgo 4.
- **[gMock for Dummies](https://google.github.io/googletest/gmock_for_dummies.html)**
  — **gratuito**. Lectura de esta tarde si queréis llegar mañana con ventaja:
  es la versión generada de los dobles que hoy escribimos a mano.

**Los detalles de C++ del hallazgo 4:**

- **[C++ Core Guidelines, C.35](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rc-dtor-virtual)**
  — **gratuito**. *"A base class destructor should be either public and
  virtual, or protected and non-virtual."* La regla, con su razón, en diez
  líneas.
- Herb Sutter, **[*Virtuality* (GotW / C++ Report)](http://www.gotw.ca/publications/mill18.htm)**
  — **gratuito**. El artículo original de donde viene esa guía. La sección
  sobre destructores explica el caso del `delete` por el tipo estático.
- **[cppreference — `std::basic_streambuf`](https://en.cppreference.com/w/cpp/io/basic_streambuf)**
  — **gratuito**. La lista de virtuales protegidas (`underflow`, `uflow`,
  `xsgetn`…): el mapa de por dónde se mockea un stream de verdad.
- **[cppreference — `std::basic_istream`](https://en.cppreference.com/w/cpp/io/basic_istream)**
  — **gratuito**. Contrastadla con la anterior: ni una función miembro virtual.
  Ahí está la respuesta a "¿por qué no puedo heredar y redefinir `operator>>`?".

**Para el argumento de OCP:**

- **[*Software Engineering at Google*, cap. 12](https://abseil.io/resources/swe-book/html/ch12.html)**
  — **gratuito**. Por qué un test debe probar comportamiento y no estructura;
  aplica directo al test del verbo `vaciar`.
- Google Testing Blog, **[*Change-Detector Tests Considered Harmful*](https://testing.googleblog.com/2015/01/testing-on-toilet-change-detector-tests.html)**
  — **gratuito**, dos minutos. El contrapunto: no escribáis tests que solo
  detectan que el código cambió. El del hallazgo 2 prueba una *capacidad*
  (extender sin modificar), no una estructura.
