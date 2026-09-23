# Día 3 — Ejercicio corregido: ¿cuántos tests necesita el Paint?

> **El enunciado que os di en clase:** coged el Paint tal y como está en
> `02-Patrones c++/paint`, recorredlo fichero a fichero y contestad a dos
> preguntas: **cuántos tests hay que escribirle a cada pieza y qué prueba cada
> uno**, y **qué dobles (dummy, stub, spy, mock, fake) hay que fabricar** para
> poder escribirlos.
>
> Esto es la corrección. Todavía **no** escribimos `TEST(...)`: eso es el
> bloque 13. Hoy contamos y decidimos. Escribir es lo fácil; lo que cuesta es
> saber *qué* escribir.

---

## 1. Antes de nada: lo hicimos al revés, y a propósito

El orden que hemos seguido estos tres días es:

```
1. La aplicación (días 1 y 2 de Patrones)
2. La refactorización — sin tests
3. Los tests (hoy)
```

Y el orden bueno es exactamente el contrario:

```
1. Los tests           →  son la ESPECIFICACIÓN
2. El contrato en rojo →  Shape, Canvas, Registry, todavía sin implementar
3. La aplicación       →  sale de poner esos tests en verde
4. La refactorización
5. La validación de la refactorización, con esos mismos tests
```

Lo hemos hecho mal a posta para que se note el precio. El precio es este
documento: ahora mismo estamos escribiendo **tests de caracterización** —
tests que no dicen lo que el Paint *debería* hacer, sino lo que *hace*. Por eso
en las tablas de abajo aparece siete veces la palabra **hueco**: sitios donde
el código no decide nada y nosotros tampoco podemos decidir sin preguntar.

Con TDD esos huecos no existen, porque **no se escribe una línea sin un test
que la pida**. Esa es la lección del ejercicio, y sale gratis compararla:

| | Con TDD | Como lo hicimos |
|---|---|---|
| ¿Qué es un test? | La especificación | Una comprobación *a posteriori* |
| ¿Cuántos hacen falta? | Los que hicieron falta para escribir el código | Hay que **deducirlos leyendo el código** ← hoy |
| Los casos raros | Se deciden al escribirlos | Aparecen como huecos sin dueño |
| El diseño | Sale testeable | Sale testeable **si hubo suerte** (aquí la hubo: hay interfaces) |

---

## 2. El ejemplo que os di, corregido

Puse el `Circle` en la pizarra con este esquema:

```
Constructor →  crear un círculo

Arrange
    Point p = {10, 10};
    Circle circle = Circle(2.0, 3, p);
Act
    (vacío)
Assert
    circle.radio    == 2
    circle.color    == 3
    circle.position == {10, 10}
    circle.Area()   == area
```

Dos correcciones, y las dos enseñan algo:

**1. `circle.radio` no existe.** Mirad `Circle.h`: `radius_` es privado y **no
hay getter**. El test no puede tocarlo. Y está bien que no pueda: el radio no
es parte del contrato de una figura, `IShape` no lo menciona. Lo que sí es
observable es el **área**. Así que la aserción del radio se convierte en la del
área:

```cpp
EXPECT_NEAR(3.14159265358979 * 4.0, circle.Area(), 1e-9);   // en vez de radio == 2
```

Esa sustitución es la frontera entre **caja blanca** (miro el campo) y **caja
negra** (miro lo que el objeto promete). Si para probar algo hace falta un
getter que nadie más usa, **el test está mal planteado**, no la clase.

**2. El `Act` vacío no es un error.** En un test de constructor el acto *es*
construir. Se queda en el *Arrange* y el bloque *Act* se deja vacío con un
comentario. Es el único caso donde AAA tiene dos patas.

El resto de vuestro esquema —`Move`, `Clone`, `ToString`— está bien, y de él
sale justo lo que vais a ver en §6.2: esas tres pruebas **no son de `Circle`**.
Son del contrato `IShape`, y por eso se escriben una vez y se ejecutan contra
`Circle` y contra `Square`.

---

## 3. El método: cinco preguntas por método público

Para no contar a ojo. Por cada método público, **un test por cada respuesta**:

| # | Pregunta | Qué genera |
|---|---|---|
| 1 | ¿Cuál es el camino normal descrito en la especificación? | 1 test |
| 2 | ¿Cuántas **clases de equivalencia** distintas tiene la entrada? | 1 test por clase |
| 3 | ¿Dónde están las **fronteras** de esas clases? | 1 test por borde |
| 4 | ¿Qué **excepciones** promete lanzar? | 1 test por excepción |
| 5 | ¿Qué le tiene que **pedir a un colaborador**? | 1 test *por colaboración*, con doble |

Y una regla de corte: **un test comprueba un concepto**. Varios `EXPECT` valen
si describen *el mismo hecho* (el estado tras construir, por ejemplo); no valen
para meter dos hechos distintos en la misma función.

### Bad — contar por métodos

```
Circle tiene 6 métodos públicos → 6 tests. Siguiente clase.
```

Con esa cuenta, `Canvas::At` se lleva **un** test y se quedan fuera las dos
excepciones y el índice de frontera. Y `CommandManager::Redo` se lleva uno,
cuando la mitad de su valor está en el caso *"no había nada que rehacer"*.

### Good — contar por comportamientos

```
Canvas::At →  camino normal (devuelve la figura del índice)         1
              índice == Count()  → out_of_range   (frontera baja)   1
              lienzo vacío       → out_of_range   (frontera 0)      1
                                                              ────  3
```

El número sale más alto y **eso es lo correcto**. Un método de tres líneas con
una guarda tiene tres comportamientos, no uno.

---

## 4. El mapa: los cuatro módulos

El Paint son 20 cabeceras y 15 `.cpp`, pero solo **cuatro módulos**, que son los
que dibujamos en la pizarra. El orden importa: es el orden de dependencia, y por
tanto el orden en que se escriben los tests.

```
   ┌─────────────────────────────────────────────────────────────┐
   │  4. APLICACIÓN      App · IReader/IWriter · Console*        │
   │     depende de ──────────────┐                              │
   ├──────────────────────────────┼──────────────────────────────┤
   │  3. REGISTROS       ShapeFactory · CommandRegistry          │
   │                     ShapeRegistration · CommandRegistration │
   │     depende de ──────────────┐                              │
   ├──────────────────────────────┼──────────────────────────────┤
   │  2. COMANDOS        ICommand · Add/Move/Duplicate           │
   │                     CommandManager                          │
   │     depende de ──────────────┐                              │
   ├──────────────────────────────┼──────────────────────────────┤
   │  1. DOMINIO         Point · IShape · Circle · Square        │
   │                     ICanvas · Canvas                        │
   └─────────────────────────────────────────────────────────────┘
        ▲ sin colaboradores: CERO dobles
        ▼ cuanto más arriba, más dobles
```

Regla que se lee en el dibujo y que vale para cualquier proyecto:

> **Los dobles no se reparten por igual. Se acumulan hacia arriba.**
> El dominio no necesita ninguno. La aplicación no se puede probar sin tres.

---

## 5. Resumen: el número, fichero a fichero

| Módulo | Fichero | Tests | Dobles que necesita |
|---|---|---:|---|
| 1 | `Point.h` | 1 | — |
| 1 | `IShape.h` *(contrato compartido)* | **6** | — |
| 1 | `Circle.cpp` | 4 | — |
| 1 | `Square.cpp` | 4 | — |
| 1 | `ICanvas.h` | 0 | *(es la costura, no el sujeto)* |
| 1 | `Canvas.cpp` | **13** | stub `IShape`, spy + mock `IWriter` |
| 2 | `ICommand.h` | 0 | *(costura)* |
| 2 | `CommandManager.cpp` | **11** | dummy + spy + mock `ICommand` |
| 2 | `AddShapeCommand.cpp` | 6 | fake `ICanvas`, dummy `IShape` |
| 2 | `MoveShapeCommand.cpp` | 7 | fake `ICanvas`, mock `IShape` |
| 2 | `DuplicateShapeCommand.cpp` | 6 | fake `ICanvas`, mock `IShape` |
| 3 | `ShapeFactory.cpp` | 10 | stub + spy `Builder` (lambdas) |
| 3 | `ShapeRegistration.cpp` | 7 | — *(test sociable: fábrica y figuras reales)* |
| 3 | `CommandRegistry.cpp` | 10 | spy `Action` (lambda), **dummies para `AppContext`** |
| 3 | `CommandRegistration.cpp` | **18** | mock `ICanvas`, spy `IWriter` |
| 4 | `IReader.h` / `IWriter.h` | 0 | *(costuras)* |
| 4 | `App.cpp` | **13** | fake `IReader`, spy `IWriter`, fake `ICanvas` |
| 4 | `ConsoleReader.cpp` | 2 | *(redirección de `std::cin`)* |
| 4 | `ConsoleWriter.cpp` | 1 | *(redirección de `std::cout`)* |
| — | `main.cpp` | **0** | — |
| | **TOTAL** | **119** | **12 dobles** |

Los 6 del contrato se escriben una vez y se ejecutan contra dos tipos
(`TYPED_TEST`, bloque 14), así que son **≈125 ejecuciones** con 119 funciones
escritas.

Tres cosas que saltan a la vista y merecen comentario en voz alta:

- **`CommandRegistration.cpp` es el fichero con más tests (18) y no tiene ni una
  clase.** Son nueve lambdas. Ahí está todo el parseo de argumentos y todos los
  mensajes de error del REPL: la lógica se ha ido a donde nadie la busca.
- **`main.cpp` tiene cero.** Y es la prueba de que el *composition root* está
  bien hecho: si `main` mereciera un test, es que tiene lógica que no le toca.
- **La suite actual tiene 12 tests.** Faltan más de cien. Lo medimos con `gcovr`
  en el bloque 15 en vez de creérnoslo.

---

## 6. Módulo 1 — Dominio

### 6.1 `Point.h` — 1 test

Un `struct` de dos enteros sin comportamiento. La tentación es darle cero, y
casi acierta. Se lleva **uno** porque los inicializadores por defecto
(`int x = 0;`) **son un contrato del que otros dependen**: `CommandRegistration`
hace `Point target;` y luego lee encima, y `MoveShapeCommand` tiene
`Point previous_{}`.

```
1. Un Point por defecto vale {0, 0}
```

> **No se prueban los datos, se prueban las decisiones.** Aquí la única
> decisión es el valor por defecto.

### 6.2 `IShape.h` — 6 tests, y son los más rentables del documento

Estos seis **no pertenecen a `Circle` ni a `Square`**: pertenecen a la
interfaz. Cualquier figura que alguien añada mañana (un `Triangle`) tiene que
pasarlos sin escribir una línea nueva. Es el **contrato** del que hablamos en
clase, y el caso de libro para `TYPED_TEST`.

| # | Test | Por qué |
|---|---|---|
| C1 | `MoveTo` deja la figura en la posición pedida | El camino normal |
| C2 | `MoveTo` no altera ni el color ni el área | Nadie promete esto por escrito, y todo el mundo cuenta con ello |
| C3 | `Clone` devuelve una figura con el mismo estado observable | Prototype |
| C4 | Mover el clon **no** mueve al original | El test que pedisteis: copia independiente de verdad |
| C5 | El clon conserva el tipo dinámico | Un `Circle` no puede clonarse en un `Square` |
| C6 | `ToString` contiene tipo, área, color y posición | Lo consume `Canvas::Print` |

**Dobles: ninguno.** Una figura no tiene colaboradores. Es el ejemplo perfecto
del error que abre el §11 del bloque 12 — *doblar lo que no hace falta*.

> Si escribís estos seis por separado para `Circle` y para `Square`, tenéis 12
> tests y **el doble de sitios donde equivocaros**. Escritos como contrato, un
> `Triangle` nuevo hereda la suite gratis.

### 6.3 `Circle.cpp` — 4 tests propios

Los seis de arriba **más** lo que solo es suyo:

```
1. El constructor deja color, posición y área coherentes   ← vuestro AAA, corregido
2. Area() == π·r² para un radio conocido  (EXPECT_NEAR, nunca EXPECT_EQ)
3. Radio 0 → área 0                        (frontera)
4. Radio negativo → ???                    (HUECO: hoy devuelve área positiva)
```

El test 4 no se puede escribir todavía, y eso es lo valioso: **el código no
decide**. `Circle(-2.0, ...)` construye tan feliz y devuelve área `12.56`. Hay
tres respuestas posibles y hay que elegir una: lanzar `std::invalid_argument`,
saturar a 0, o documentar que es responsabilidad de quien construye. Escribid
el test de la que elijáis y **ponedlo en rojo**. Eso ya es TDD.

⚠️ `EXPECT_EQ` sobre `double` no. Nunca. `EXPECT_NEAR` con tolerancia, o
`EXPECT_DOUBLE_EQ` si la operación es exacta (el área de `Square` con lados
enteros lo es; la de `Circle` no, por `kPi`).

### 6.4 `Square.cpp` — 4 tests propios

Simétrico, y aquí se ve el retorno del contrato: cambian **cuatro** líneas.

```
1. El constructor deja color, posición y área coherentes
2. Area() == lado²  para un lado conocido
3. Lado 0 → área 0                        (frontera)
4. Lado negativo → ???                    (mismo HUECO, misma decisión)
```

### 6.5 `Canvas.cpp` — 13 tests

La clase con más comportamiento del dominio, y la que arrastra el Singleton.

| # | Test | Tipo |
|---|---|---|
| 1 | Un lienzo recién vaciado tiene `Count() == 0` | Trivial, arranca el motor |
| 2 | `Add` sube el contador y `At(0)` devuelve esa figura | Camino normal |
| 3 | `Add` conserva el orden de inserción | Contrato implícito de `Print` |
| 4 | `Remove` devuelve **la propiedad** de la figura y baja el contador | Camino normal |
| 5 | Con varias figuras, `Remove` quita la correcta | Equivalencia |
| 6 | `Remove` con un handle desconocido lanza `runtime_error` | Excepción |
| 7 | `Remove(nullptr)` lanza | Frontera |
| 8 | `At(Count())` lanza `out_of_range` | **Frontera**: el primer índice malo |
| 9 | `At(0)` sobre lienzo vacío lanza | Frontera |
| 10 | `Clear` deja el contador a 0 | Camino normal |
| 11 | `Print` sobre lienzo vacío escribe **exactamente una** línea: `(lienzo vacio)` | Colaboración |
| 12 | `Print` escribe una línea por figura, con `[i] ` delante y en orden | Colaboración |
| 13 | `Instance()` devuelve siempre la misma instancia (`&a == &b`) | El test del Singleton |

**Dobles (2):**

- **Stub `IShape`** — una figura con `ToString()` enlatado (`"FIGURA"`). Sin
  esto, el test 12 comprueba a la vez el formato de `Canvas` **y** el de
  `Circle`: si mañana cambia el texto de `Circle`, se rompe un test de
  `Canvas`. Eso es acoplamiento entre tests, y se arregla con un stub de cuatro
  líneas.
- **Spy `IWriter`** para los tests 11 y 12 (hay que *mirar las líneas*), y
  **mock `IWriter`** si preferís el 11 como
  `EXPECT_CALL(w, Write("(lienzo vacio)")).Times(1)`. Los dos valen; la regla
  de la chuleta decide: ¿hay que rebuscar en los datos? spy. ¿Basta con *"esto
  tenía que ocurrir"*? mock.

**Y el peaje del Singleton:** los 13 tests comparten **el mismo objeto**. Sin
precauciones, el test 2 deja una figura que el test 1 se encuentra puesta, y el
orden de ejecución decide quién falla. La *I* de FIRST (*Independent*) se cae
sola. Se tapa con un fixture:

```cpp
class CanvasTest : public ::testing::Test {
protected:
    void SetUp() override { Canvas::Instance().Clear(); }   // ← el impuesto
    ICanvas& canvas = Canvas::Instance();
};
```

Ese `SetUp` **es** el argumento del bloque 12 contra el Singleton, escrito en
código. Guardadlo para el bloque 15.

---

## 7. Módulo 2 — Comandos

Aquí empiezan los dobles de verdad, porque aquí empieza la colaboración.

### 7.1 `CommandManager.cpp` — 11 tests

**La clase más rentable del proyecto**: no depende de nada concreto (solo de
`ICommand`), toda su lógica es la pila, y es donde gMock luce.

| # | Test | Doble |
|---|---|---|
| 1 | Un manager nuevo: `CanUndo()` y `CanRedo()` son `false` | — |
| 2 | `Execute` llama a `Execute()` del comando **exactamente una vez** | mock |
| 3 | Tras `Execute`, `CanUndo()` es `true` | dummy |
| 4 | `Undo` con la pila vacía devuelve `false` **y no llama a nadie** | mock, `Times(0)` |
| 5 | `Undo` llama a `Undo()` del comando y devuelve `true` | mock |
| 6 | Tras deshacer el único comando: `CanUndo()` false, `CanRedo()` true | dummy |
| 7 | `Redo` con la pila vacía devuelve `false` | — |
| 8 | `Redo` vuelve a ejecutar **el mismo** comando y devuelve `true` | mock, `Times(2)` |
| 9 | Una acción nueva invalida el rehacer (`Execute`→`Undo`→`Execute` ⇒ `CanRedo()` false) | dummies |
| 10 | Con dos comandos, `Undo` deshace **el último** (LIFO) | spy con registro de orden |
| 11 | Un comando cuyo `Execute` lanza **no se apila** (`CanUndo()` sigue false) | mock que lanza |

El test 11 no es un capricho: mirad `CommandManager::Execute`, la excepción sale
**antes** del `push_back`. Es un buen comportamiento, no está escrito en ningún
sitio, y sin test nadie garantiza que siga siendo así tras el próximo refactor.

**Dobles (3), y los tres tipos en la misma clase — por eso es el ejemplo de
clase:**

```cpp
// DUMMY — solo relleno: los tests 3, 6 y 9 no miran lo que hace
struct DummyCommand : ICommand { void Execute() override {} void Undo() override {} };

// SPY — el test 10 necesita rebuscar en los datos: el ORDEN
class SpyCommand : public ICommand {
public:
    SpyCommand(std::string name, std::vector<std::string>& log)
        : name_(std::move(name)), log_(log) {}
    void Execute() override { log_.push_back("exec:" + name_); }
    void Undo()    override { log_.push_back("undo:" + name_); }
private:
    std::string name_;
    std::vector<std::string>& log_;
};

// MOCK — a los tests 2, 4, 5 y 8 les basta "esto tenía que ocurrir"
class MockCommand : public ICommand {
public:
    MOCK_METHOD(void, Execute, (), (override));
    MOCK_METHOD(void, Undo,    (), (override));
};
```

Y el test 4, que es el que justifica el mock frente al spy:

```cpp
TEST(CommandManager, UndoConLaPilaVaciaNoLlamaANadie) {
    MockCommand command;
    EXPECT_CALL(command, Undo()).Times(0);      // ← la expectativa, ANTES
    CommandManager manager;

    EXPECT_FALSE(manager.Undo());
}
```

### 7.2 `AddShapeCommand.cpp` — 6 tests

```
1. Execute añade la figura al lienzo, una sola vez
2. Execute cede la propiedad: el lienzo devuelve el MISMO puntero
3. Undo retira exactamente esa figura (Remove con el handle correcto)
4. Undo recupera la propiedad: un redo vuelve a añadir la misma figura
5. HUECO — Undo sin Execute previo llama a Remove(nullptr)
6. HUECO — Execute dos veces seguidas añade un puntero nulo (shape_ ya movido)
```

**El doble tiene que ser un FAKE, no un stub**, y el test 4 es la razón. Ese
ida y vuelta —`Add` cede la propiedad, `Undo` la recupera, el redo la vuelve a
ceder— **no se puede simular con respuestas enlatadas**: el doble tiene que
acordarse de lo que le metieron. Chuleta: *"tiene memoria: le metéis algo y os
lo devuelve"* ⇒ fake. Es exactamente el `FakeCanvas` que ya está escrito en
`tests/test_paint.cpp`.

Los tests 5 y 6 son **caja blanca pura**: no salen de la especificación, salen
de leer `Execute()` y ver que `shape_` queda vacío. Con el `Canvas` real ambos
lanzan; con el `FakeCanvas` actual el 5 devuelve `nullptr` en silencio. Los dos
comportamientos no pueden ser correctos a la vez ⇒ hay que decidir si
`AddShapeCommand` se protege (`if (!handle_) return;`) o si documenta que
`Undo` sin `Execute` es un error de programación.

### 7.3 `MoveShapeCommand.cpp` — 7 tests

```
1. Execute deja la figura en la posición destino
2. Execute le pide la figura al lienzo por el índice dado         (mock: At(2))
3. Undo restaura la posición previa
4. Undo sin Execute no hace nada y no revienta                    (handle_ == nullptr)
5. Execute con índice inválido propaga out_of_range
6. Execute → Undo → Execute vuelve a dejarla en el destino        (el redo)
7. Mover a la misma posición: el Undo la deja donde estaba        (caso trivial)
```

**Dobles:** fake `ICanvas` (tiene que tener una figura dentro) o stub `ICanvas`
cuyo `At()` devuelva un **mock `IShape`**:

```cpp
EXPECT_CALL(shape, MoveTo(Point{7, 8})).Times(1);
```

⚠️ **Esa línea no compila hoy.** `Point` no tiene `operator==`, así que gMock no
sabe comparar el argumento. Es el primer cambio de producción que exige el plan
de pruebas, y es un cambio bueno por sí mismo (§10).

El test 4 tiene truco y por eso lo puse: `Undo()` sin `Execute()` previo es la
**única** de las tres acciones que se protege sola (`if (handle_)`). Comparadlo
con el hueco 5 de `AddShapeCommand` y con el 6 de `Duplicate`: tres comandos,
tres respuestas distintas al mismo caso. **Eso es lo que encuentra un plan de
pruebas y no encuentra una revisión de código.**

### 7.4 `DuplicateShapeCommand.cpp` — 6 tests

```
1. Execute llama a Clone() de la figura del índice, una vez    ← el test de Prototype
2. Execute añade la copia al lienzo (Count 1 → 2)
3. La copia es independiente: mover una no mueve la otra
4. Undo retira la COPIA, no el original (el original sigue ahí)
5. Execute con índice inválido propaga out_of_range
6. HUECO — Undo sin Execute llama a Remove(nullptr)
```

El test 1 es el que os interesa como patrón: **mock `IShape`** con
`EXPECT_CALL(shape, Clone())`. Verifica que el comando **delega** en el
Prototype en vez de construir un `Circle` a mano. Es una colaboración, no un
resultado ⇒ grupo 3 de la pregunta del bloque 12 ⇒ mock.

⚠️ `Clone()` devuelve `std::unique_ptr<IShape>`, que es *move-only*:
`Return(std::move(copia))` **no compila**. Hay que usar `ByMove` o una lambda:

```cpp
EXPECT_CALL(shape, Clone())
    .WillOnce([] { return std::make_unique<Square>(1.0, 0, Point{0, 0}); });
```

Está en el *gMock Cookbook*, apartado *move-only types*. Lo veremos en el
bloque 13; apuntadlo ya, porque es la primera pared con la que choca todo el
mundo.

Y una asimetría para la clase: `AddShapeCommand::Undo` **guarda** lo que
devuelve `Remove` (recupera la propiedad); `DuplicateShapeCommand::Undo` lo
**tira**. El segundo funciona igual porque el redo vuelve a clonar desde el
índice, pero es una diferencia de diseño que ningún test actual nota.

---

## 8. Módulo 3 — Registros y fábricas

Aquí los dobles dejan de ser clases: **son lambdas**. La fábrica y el registro
guardan `std::function`, así que el doble más barato es una lambda de dos
líneas. Que no se os olvide: *un doble no es una clase, es un colaborador bajo
control*.

### 8.1 `ShapeFactory.cpp` — 10 tests

```
 1. Una fábrica nueva no conoce ninguna figura
 2. Tras Register, Knows() la reconoce
 3. Create devuelve lo que fabrica el builder registrado        (STUB builder)
 4. Create le pasa al builder la línea SIN el nombre            (SPY builder)
 5. Create con nombre desconocido lanza runtime_error con el nombre en el mensaje
 6. Create con línea vacía lanza
 7. Register con un nombre repetido sobrescribe al anterior     (comportamiento actual)
 8. ForEachListed recorre en orden alfabético                   (std::map)
 9. ForEachListed compone "nombre <args>", y "nombre" a secas si args está vacío
10. ForEachListed sobre una fábrica vacía no invoca al visitante (Times(0))
```

Los dos dobles, uno al lado del otro, para que se vea la diferencia:

```cpp
// STUB — el test 3 solo necesita que "me dé algo" reconocible
factory.Register("fake", "", "", [](std::istream&) -> std::unique_ptr<IShape> {
    return std::make_unique<Square>(7.0, 42, Point{0, 0});   // 42 = la marca
});

// SPY — el test 4 necesita ver QUÉ le llegó al builder
std::string recibido;
factory.Register("fake", "", "", [&recibido](std::istream& in) {
    std::getline(in, recibido);                              // ← apunta y luego miramos
    return std::unique_ptr<IShape>(new Square(1.0, 0, Point{0, 0}));
});
factory.Create("fake 1 2 3");
EXPECT_EQ(" 1 2 3", recibido);
```

El test 7 documenta algo que **nadie decidió**: hoy registrar `"circle"` dos
veces pisa el builder anterior, sin aviso. ¿Es lo que queremos, o debería
lanzar? En un sistema de plugins esa diferencia es un agujero de seguridad. Hoy
solo es un test que fija la respuesta.

### 8.2 `ShapeRegistration.cpp` — 7 tests

```
1. Registra exactamente dos figuras: circle y square
2. "circle 5 2 10 20" produce un círculo con ese estado
3. "square 3 1 10 20" produce un cuadrado con ese estado
4. La sintaxis y la descripción de ayuda de circle son las esperadas
5. Ídem square
6. HUECO — "circle 5" no falla: color y posición se quedan a 0
7. HUECO — "circle x 2 0 0" no falla: el radio se queda a 0
```

**Dobles: ninguno**, y es deliberado. Son tests **sociables**: fábrica de
verdad + figuras de verdad. Doblar aquí sería doblar código nuestro, rápido y
predecible — el error del §11 del bloque 12.

Los tests 6 y 7 son el agujero más visible del Paint: `in >> radius >> color`
falla en silencio y `RegisterBuiltinShapes` no mira el `istream`. El usuario
teclea `circle 5` y le sale un círculo de radio 5 en el (0,0) y color 0, sin un
solo mensaje. **Un plan de pruebas honesto lo saca; una demo no.**

### 8.3 `CommandRegistry.cpp` — 10 tests

```
 1. Un registro nuevo no conoce ningún verbo
 2. Tras Register, Knows() lo reconoce
 3. Run invoca la acción registrada exactamente una vez           (SPY action)
 4. Run le pasa a la acción el contexto y los argumentos pendientes
 5. Run propaga el Status devuelto — Continue y Quit              (TEST_P, 2 filas)
 6. Run con un verbo desconocido lanza runtime_error con el verbo
 7. Register con un verbo repetido sobrescribe
 8. ForEachListed omite los alias (descripción vacía)
 9. ForEachListed ordena alfabéticamente y compone "verbo <args>"
10. ForEachListed sobre un registro vacío no invoca al visitante
```

**El mejor ejemplo de DUMMY de todo el proyecto está aquí.** Para llamar a
`Run` hay que construir un `AppContext`, y `AppContext` tiene **cinco
referencias**:

```cpp
struct AppContext {
    ICanvas& canvas;  IWriter& writer;  CommandManager& commands;
    const CommandRegistry& registry;    const ShapeFactory& factory;
};
```

En el test 5 —*"`Run` devuelve el `Status` de la acción"*— **ninguna de las
cinco se usa**. Son cinco huecos que hay que rellenar para que el código
compile, y nada más. Definición literal de dummy: *"podríais vaciarlo y el test
pasa igual"*. Con gMock, `NiceMock<MockCanvas>` y `NiceMock<MockWriter>` hacen
de dummy sin quejarse de llamadas inesperadas.

Y un detalle de cobertura que veréis en el bloque 15: el `throw` del test 6 es
**inalcanzable desde la App** (`App::Run` pregunta `Knows()` antes de llamar a
`Run`). Solo se cubre con un test directo del registro. Una rama que solo existe
para el día en que alguien use `CommandRegistry` desde otro sitio — legítima,
pero hay que saber que está.

### 8.4 `CommandRegistration.cpp` — 18 tests

El fichero con más tests y ni una sola clase: nueve lambdas donde vive **todo**
el parseo de argumentos del REPL.

| # | Test |
|---|---|
| 1 | `RegisterBuiltinCommands` registra exactamente 9 verbos |
| 2 | `help` escribe la cabecera `Comandos disponibles:` |
| 3 | `help` lista los verbos del registro con su sintaxis |
| 4 | `help` lista también las figuras de la fábrica |
| 5 | `help` **no** lista los alias `list` ni `quit` |
| 6 | `help` alinea la descripción en la columna 33 |
| 7 | **Frontera de `Pad`**: con un `usage` de 33 o más caracteres deja un único espacio |
| 8 | `print` delega en `canvas.Print(writer)`, una vez |
| 9 | `undo` con historial escribe `deshecho` |
| 10 | `undo` sin historial escribe `nada que deshacer` |
| 11 | `redo` con pila escribe `rehecho` |
| 12 | `redo` sin pila escribe `nada que rehacer` |
| 13 | `move 0 7 8` mueve la figura e imprime el lienzo |
| 14 | `move patata` escribe `uso: move <indice> <x> <y>` **y no toca el lienzo** |
| 15 | `move 1 2` (un argumento de menos) toma el mismo camino de uso |
| 16 | `duplicate 0` clona e imprime |
| 17 | `duplicate` sin argumento escribe `uso: duplicate <indice>` |
| 18 | `exit` devuelve `Quit`, y los alias `list`/`quit` se comportan como `print`/`exit` |

**Dobles:** **mock `ICanvas`** y **spy `IWriter`**. El mock es imprescindible
justo en los tests 14, 15 y 17 — el valor de esos tests **no es el mensaje**,
es que **no pasó nada**:

```cpp
EXPECT_CALL(canvas, Add(::testing::_)).Times(0);
EXPECT_CALL(canvas, Print(::testing::_)).Times(0);
```

Un spy también lo detecta (`EXPECT_EQ(0, canvas.adds)`), pero el mock lo dice
**en el sitio donde importa y falla él solo**. Es el caso de la chuleta:
*"¿mock o spy? Mock si basta con 'no tenía que ocurrir'"*.

El test 7 es el clásico de frontera del bloque 9 aplicado a una línea de
formato: `Pad` compara `text.size() >= kUsageWidth` con `kUsageWidth == 33`.
Los tres casos son 32, 33 y 34 caracteres. Hoy ninguna sintaxis registrada
llega a 33, así que **esa rama no se ejecuta nunca** — otro hallazgo para el
informe de cobertura.

---

## 9. Módulo 4 — Aplicación

### 9.1 `App.cpp` — 13 tests

```
 1. Run imprime la ayuda ANTES de leer la primera línea
 2. Run termina al agotarse la entrada (EOF) sin error
 3. Una línea en blanco se ignora, sin mensaje de error
 4. Una línea con solo espacios se ignora igual                  (frontera)
 5. Un verbo registrado se ejecuta
 6. Un nombre de figura conocido crea la figura y la añade al lienzo
 7. Tras añadir una figura, se imprime el lienzo
 8. Un verbo desconocido escribe "comando desconocido: X" y el bucle SIGUE
 9. "exit" corta el bucle: lo que viene después no se ejecuta
10. Una excepción de una acción se traduce a "error: ..." y no se propaga
11. Una excepción de la fábrica se traduce igual
12. OCP — un verbo registrado desde fuera funciona sin tocar App
13. OCP — una figura registrada desde fuera funciona y aparece en la ayuda
```

**Los tres dobles, y aquí se ve por qué cada uno es de su tipo:**

| Doble | Tipo | Por qué **ese** tipo |
|---|---|---|
| `StringReader` | **fake** | Sirve líneas de un vector y **recuerda por dónde va**. Un stub devolvería siempre la misma; el REPL necesita una secuencia |
| `RecordingWriter` | **spy** | Guarda una lista y el `EXPECT` la mira **al final** |
| `FakeCanvas` | **fake** | Almacena de verdad; el test 6 comprueba que la figura llegó |

Y una confesión sobre el código que ya existe: el `FakeCanvas` de
`test_paint.cpp` **es un híbrido**. Almacena (fake) *y* lleva contadores
`adds`/`removes`/`prints` (spy). Funciona, se lee bien, y de hecho es lo más
común en la práctica. Pero cuando los contadores empiezan a ser la mitad de las
aserciones, la pregunta correcta es: *¿esto no era un mock?* Es justo lo que
prometimos en el bloque 11 —*"`adds` es un `EXPECT_CALL(...).Times(2)`
esperando a nacer"*— y lo que rehacemos en el bloque 13.

El test 1 tiene una trampa que merece la pena señalar: `App::Run()` **imprime la
ayuda al arrancar**, antes de leer nada. Eso significa que *todos* los tests de
`App` tienen la ayuda en el `RecordingWriter` antes que su propia salida. Si
comprobáis `writer.lines[0]`, comprobáis la ayuda. Hay que usar `Contains(...)`,
o quedarse con el tamaño previo. Es una dependencia oculta entre dos
comportamientos, y con TDD probablemente la ayuda habría salido a un método
aparte.

### 9.2 `ConsoleReader.cpp` (2) y `ConsoleWriter.cpp` (1)

```
ConsoleReader: 1. ReadLine devuelve la línea leída y true
               2. Al llegar a EOF devuelve false
ConsoleWriter: 1. Write escribe la línea seguida de '\n'
```

Son adaptadores de cinco líneas sin lógica. **No se doblan: son ellos los que
sustituimos por dobles** en todos los demás tests. Para probarlos hay que
secuestrar el buffer de la consola:

```cpp
std::ostringstream captura;
std::streambuf* anterior = std::cout.rdbuf(captura.rdbuf());
ConsoleWriter{}.Write("hola");
std::cout.rdbuf(anterior);                    // ← restaurar SIEMPRE, o el fixture
EXPECT_EQ("hola\n", captura.str());
```

Ese `rdbuf` global es exactamente lo que estropea la *I* de FIRST, igual que el
Singleton: hay que devolverlo en el `TearDown`, o un test que falle a medias
deja a `std::cout` secuestrado para toda la suite. Si esto os incomoda,
enhorabuena: **es el argumento a favor de haber inventado `IWriter`**.

### 9.3 `main.cpp` — 0 tests

Cablea cuatro objetos y llama a `Run()`. No tiene una sola decisión, y por eso
no tiene tests. Si algún día lo necesita, la respuesta no es probar `main`: es
sacar la lógica de `main`.

---

## 10. Lo que el plan de pruebas obliga a cambiar en el código

Este es el retorno inesperado del ejercicio. Contar tests **encuentra defectos
de diseño** antes de escribir ni uno:

| # | Hallazgo | Qué lo destapa | Arreglo |
|---|---|---|---|
| 1 | `Point` no tiene `operator==` | `EXPECT_CALL(shape, MoveTo(Point{7,8}))` no compila | Añadir `operator==` (y `PrintTo`, para que gMock imprima `(7, 8)` en vez de bytes) |
| 2 | `Clone()` y `Add()` son *move-only* | `Return(std::move(p))` no compila | `ByMove` o lambda en `WillOnce` |
| 3 | `~ICanvas` es `protected` y no virtual | Un `MockCanvas` no se puede tener por `unique_ptr<ICanvas>` | Ninguno: tenedlo **por valor** en el test. La decisión del bloque 11 sigue siendo buena |
| 4 | `Canvas` es Singleton | `Clear()` obligatorio en cada `SetUp` | Ninguno hoy. Es el impuesto, y por eso `ICanvas` existe |
| 5 | Tres comandos, tres respuestas a `Undo` sin `Execute` | Los tests 5 / 4 / 6 de §7 | Decidir **una** política y aplicarla a los tres |
| 6 | Los builders no validan la entrada | Tests 6 y 7 de §8.2 | Comprobar el `istream` y lanzar |
| 7 | La rama `Pad` de 33+ caracteres no se ejecuta nunca | Test 7 de §8.4 | Ninguno; saberlo al leer la cobertura |

Los cambios 1 y 2 son **de producción, pedidos por los tests**. Que un test
obligue a mejorar el diseño de producción no es un efecto secundario: es el
argumento entero a favor de TDD, y aquí lo tenéis pasando en directo sobre
código que ya estaba escrito.

---

## 11. Inventario de dobles: los 12 que hay que fabricar

| # | Doble | Interfaz | Tipo | Para qué | Lo usan |
|---|---|---|---|---|---|
| 1 | `StubShape` | `IShape` | **stub** | `ToString()` enlatado, área y posición fijas | `Canvas` |
| 2 | `DummyShape` | `IShape` | **dummy** | Rellenar un `Add` donde da igual qué figura sea | `AddShapeCommand` |
| 3 | `MockShape` | `IShape` | **mock** | Verificar `Clone()` y `MoveTo(pos)` | `Move`, `Duplicate` |
| 4 | `FakeCanvas` | `ICanvas` | **fake** | Almacena de verdad: el ida y vuelta de la propiedad | Comandos, `App` |
| 5 | `MockCanvas` | `ICanvas` | **mock** | `Times(0)` / `Times(1)` sobre `Add`, `Remove`, `Print` | `CommandRegistration` |
| 6 | `DummyCanvas` | `ICanvas` | **dummy** | Rellenar el `AppContext` | `CommandRegistry` |
| 7 | `StringReader` | `IReader` | **fake** | Sirve una secuencia de líneas y recuerda la posición | `App` |
| 8 | `RecordingWriter` | `IWriter` | **spy** | Guarda lo escrito para rebuscar al final | `Canvas`, `App`, `CommandRegistration` |
| 9 | `MockWriter` | `IWriter` | **mock** | `EXPECT_CALL(Write("(lienzo vacio)"))` | `Canvas` |
| 10 | `DummyCommand` | `ICommand` | **dummy** | Llenar la pila cuando da igual qué comando sea | `CommandManager` |
| 11 | `SpyCommand` | `ICommand` | **spy** | Registrar el **orden** de `Execute`/`Undo` | `CommandManager` |
| 12 | `MockCommand` | `ICommand` | **mock** | `Times`, `InSequence`, y el comando que lanza | `CommandManager` |

Más los **dobles-lambda**, que no llevan clase: stub `Builder`, spy `Builder`,
spy `Action`, spy visitante de `ForEachListed`.

Dos lecturas de esta tabla:

**La primera.** Hay **cinco interfaces** en el Paint (`IShape`, `ICanvas`,
`ICommand`, `IReader`, `IWriter`) y la tabla tiene doce dobles repartidos entre
esas cinco. No es casualidad:

> **Cada interfaz es una costura, y cada costura es un sitio donde cabe un
> doble. El número de dobles posibles lo fijó el diseño, no el test.**

Si el Paint del día 1 —con `Canvas::Instance()` a pelo y `std::cin` dentro de
`App`— hubiera llegado hasta aquí, esta tabla tendría **cero** filas y la de
§5 tendría 12 tests en total. El refactor de los días 1 y 2 no hizo el código
más bonito: hizo posible este documento.

**La segunda.** Cuando dos entradas comparten interfaz y solo cambia el tipo
(4/5/6, 8/9, 10/11/12), **no son tres clases distintas**: con gMock son **una**
—`MockCanvas`, `MockWriter`, `MockCommand`— usada de tres maneras. Como dummy
se envuelve en `NiceMock<>`; como spy se le pone `.WillRepeatedly(...)` y se
mira después; como mock se le pone el `EXPECT_CALL` delante. Los doce de la
tabla son **doce papeles**, no doce ficheros. Y eso es media clase del bloque 13.

---

## 12. Por dónde se empieza: los MITs del Paint

119 tests no se escriben de una sentada, y el criterio del bloque 9
(*impacto × probabilidad × frecuencia*) dice por dónde:

| Orden | Qué | Por qué en ese puesto |
|---|---|---|
| 1.º | Contrato `IShape` (6) | Se escriben una vez, valen para las dos figuras y para todas las futuras |
| 2.º | `CommandManager` (11) | Máxima lógica por línea, cero dependencias concretas, los tres tipos de doble en un sitio |
| 3.º | `Canvas` (13) | Es el estado de la aplicación: si miente, miente todo |
| 4.º | Los tres comandos (19) | Deshacer mal es el fallo que el usuario **sí** nota |
| 5.º | `CommandRegistration` (18) | Donde vive el parseo, y por tanto los bugs |
| 6.º | `App` (13) | Integración; muchos de sus fallos ya los habrán cazado los de arriba |
| 7.º | Fábricas y registros (27) | Estables, poca lógica, cambian poco |
| 8.º | `Console*` (3) y `Point` (1) | Triviales. En TDD habrían sido los primeros; hoy son los últimos |

Fijaos en la inversión de la última fila: **en TDD los triviales van primero**
porque arrancan el motor y cuestan treinta segundos. Escribiendo tests *a
posteriori* van los últimos, porque no encuentran nada. El mismo test cambia de
prioridad según cuándo se escriba, y eso dice más sobre TDD que cualquier
transparencia.

---

## 13. Qué ganamos y qué pagamos

| Ganamos | Pagamos |
|---|---|
| Un número: **119**, no *"unos cuantos"* | Contarlos ha costado más que escribir los diez primeros |
| **Siete defectos de diseño** localizados sin ejecutar nada | Dos exigen tocar producción (`operator==`, validación) |
| Doce dobles **decididos por escrito** antes de teclear gMock | La tentación de escribir los doce antes de necesitarlos ← no lo hagáis |
| Un orden de ataque justificado (MITs) | El orden hay que revisarlo tras cada incidencia |
| La prueba de que el refactor de los días 1-2 servía para algo | — |

Y el aviso, porque este documento se puede usar mal:

> ⚠️ **119 no es un objetivo.** Es una estimación de lo que el código *de hoy*
> necesita para estar descrito. Si mañana `CommandRegistration` deja de parsear
> a mano, sus 18 tests bajan a 6 y **eso es una mejora**, no una regresión.
> El número que baja porque el código se simplifica es una buena noticia; el
> que baja porque borramos tests, no.

---

## 14. Mantra del ejercicio

> ### **Los tests no se cuentan por métodos: se cuentan por comportamientos.**
> ### **Y los dobles no se eligen por clase: se eligen por la pregunta que le hago al colaborador.**
>
> Cada interfaz que escribimos el día 2 es una costura.
> Cada costura es un doble.
> Y cada doble que **no** necesitáis es una interfaz que no había que crear.

---

## 15. Lo que viene ahora

- **Bloque 13 (gMock)** — los doce dobles de §11, pero generados: `MOCK_METHOD`,
  `EXPECT_CALL`, `NiceMock`, y los dos *move-only* de §10.
- **Bloque 14 (paramétricos)** — el contrato `IShape` de §6.2 como `TYPED_TEST`,
  y las tablas de frontera de `Canvas::At` y de `Pad` como `TEST_P`.
- **Bloque 15 (cobertura)** — el contraste entre estos 119 y los 12 que hay. Y
  las dos ramas que §8.3 y §8.4 dicen que no se ejecutan nunca: a ver si `gcovr`
  está de acuerdo.
- **Bloque 16 (sanitizers)** — el hueco 6 de `AddShapeCommand` (añadir un
  puntero nulo tras dos `Execute`) es de los que ASan y UBSan cazan y un
  `EXPECT` no.
- **Bloque 17** — con las siete decisiones de §10 tomadas, ya se puede tocar el
  código a propósito.

---

## Referencias

- **[gMock Cheat Sheet](https://google.github.io/googletest/gmock_cheat_sheet.html)**
  — cardinalidades (`Times`) y matchers de §7 y §8.4.
- **[gMock Cookbook — *Move-Only Types*](https://google.github.io/googletest/gmock_cook_book.html#MoveOnly)**
  — el `Clone()` de §7.4 y el `Add()` de §7.2.
- **[Martin Fowler — *Mocks Aren't Stubs*](https://martinfowler.com/articles/mocksArentStubs.html)**
  — por qué el `FakeCanvas` de §9.1 es un híbrido y qué significa eso.
- 📖 Michael Feathers, *Working Effectively with Legacy Code*, caps. 3-4 —
  **tests de caracterización**: exactamente lo que hacemos en §1.
- 📖 Gerard Meszaros, *xUnit Test Patterns* —
  **[fichas gratuitas](http://xunitpatterns.com/)** de los cinco dobles de §11.
- Bloques del curso: **[9 — Qué probar](../day-02/09_que_probar.md)** (el método
  de §3), **[11 — Caso Paint](../day-02/11_caso_paint.md)** (de dónde salen las
  costuras), **[12 — Dobles de prueba](12_dobles_de_prueba.md)** (la chuleta de
  §11).
