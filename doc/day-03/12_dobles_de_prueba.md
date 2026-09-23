# Día 3 — Bloque 12: Dobles de prueba — dummy, stub, spy, mock y fake

> *"El doble no imita al colaborador. Imita solo lo que el test necesita de él."*

---

## 1. El problema

Queremos avisar por correo al titular cuando su cuenta se queda en números
rojos. El código es de cinco líneas:

```cpp
bool Avisador::Revisar(const Cuenta& c) {
    if (c.saldoCentimos >= 0) return false;                 // nada que avisar
    return notificador_.Enviar(c.email, "Saldo negativo: " + Euros(c.saldoCentimos));
}
```

Y ahora escribid el test.

No se puede. `notificador_` manda un correo **de verdad**. El test tardaría
segundos, necesitaría servidor, llenaría un buzón real y fallaría cuando la red
vaya mal. Rompe la **F**, la **I** y la **R** de FIRST del día 1.

La solución es poner en su lugar **otra cosa que encaje en el hueco**. Eso es un
doble de prueba.

## 2. De dónde viene el nombre

Del cine. Cuando el actor tiene que saltar de un coche en marcha, **no salta el
actor**: salta un doble. Se parece lo justo, hace esa escena concreta, y nadie
espera que sepa interpretar el resto de la película.

En los tests pasa igual. Y como en el cine, **hay cinco tipos de doble según el
trabajo que le pidas**.

La palabra "mock" se usa para los cinco, y por eso nadie se entiende. Hoy los
separamos.

---

## 3. La única pregunta que hay que hacerse

Antes de escribir nada, respondé a esto:

### **¿Para qué necesito al doble?**

Solo hay tres respuestas posibles:

```
1. Para NADA.
   Solo tengo que rellenar un hueco para poder construir el objeto.
   ──────────────────────────────────────────────────────►  DUMMY

2. Para que ME DÉ algo.
   Mi código le pregunta, y necesito controlar lo que le responde.
       · me basta con una respuesta fija ──────────────────►  STUB
       · necesito que se comporte de verdad ───────────────►  FAKE

3. Para COMPROBAR lo que mi código le hizo.
   Lo que quiero probar es la llamada en sí.
       · quiero mirar los datos con calma, al final ───────►  SPY
       · me basta con "esto tenía que ocurrir" ────────────►  MOCK
```

Y hay un truco para saber en qué grupo estáis, que funciona siempre:

> ### **Escribid primero el `EXPECT`. El `EXPECT` os dice el doble.**
>
> - Si el `EXPECT` habla **del resultado de vuestro código** → grupo 1 o 2.
> - Si el `EXPECT` habla **de lo que recibió el colaborador** → grupo 3.

Ahora los cinco, uno a uno, **sobre el mismo ejemplo**. Todo el código de este
bloque compila y pasa.

---

## 4. El escenario

Una cuenta, y el colaborador que manda correos:

```cpp
struct Cuenta {
    std::string email;
    int         saldoCentimos = 0;
};

std::string Euros(int centimos);           // -1200  ->  "-12.00 EUR"

class INotificador {                       // el colaborador problemático
public:
    virtual ~INotificador() = default;
    virtual bool Enviar(const std::string& destinatario, const std::string& texto) = 0;
};

class Avisador {                           // el sujeto bajo prueba
public:
    explicit Avisador(INotificador& notificador) : notificador_{notificador} {}
    bool Revisar(const Cuenta& cuenta);
private:
    INotificador& notificador_;
};
```

**Fijaos en una cosa:** los cuatro primeros dobles son **la misma interfaz**,
`INotificador`. Lo que cambia no es el colaborador: **es lo que el test quiere
saber**.

---

## 5. DUMMY — el maniquí

> **En el cine:** en el asiento de atrás del coche tiene que ir un pasajero.
> Ponen un muñeco. No habla, no se mueve, nadie lo mira. Está para que el coche
> no vaya vacío.

**Qué quiero probar:** que una cuenta con saldo positivo **no genera aviso**.

En ese caso `Enviar` **no se llama nunca**. Pero el constructor de `Avisador`
exige un `INotificador`. Necesito algo que rellene el hueco.

```cpp
class NotificadorDummy : public INotificador {
public:
    bool Enviar(const std::string&, const std::string&) override { return true; }
};                                        // ↑ da igual lo que devuelva: no se llama
```

```cpp
TEST(Avisador, UnaCuentaEnPositivoNoGeneraAviso) {
    NotificadorDummy relleno;                          // obligatorio; nunca se usa
    Avisador avisador{relleno};

    EXPECT_FALSE(avisador.Revisar(Cuenta{"ana@ejemplo.com", 5000}));
}                       // ↑ el EXPECT habla del RESULTADO. El doble ni aparece.
```

**Lo reconocéis porque:** podríais borrar el cuerpo del doble entero y el test
seguiría pasando.

> ⚠️ **Aviso:** si un test necesita **tres** dummies, el problema no es el test.
> Es que vuestro constructor pide demasiadas cosas.

---

## 6. STUB — el actor con una sola frase

> **En el cine:** el actor que solo dice *"Señor, el tren sale a las cinco."*
> Siempre esa frase, toma tras toma. Está ahí para que el protagonista pueda
> seguir la escena.

**Qué quiero probar:** que si el correo **no sale**, el aviso no se da por hecho.

Necesito que `Enviar` devuelva `false`. Con el servidor real no sé provocarlo.
Con un stub, es una línea.

```cpp
class NotificadorQueFalla : public INotificador {
public:
    bool Enviar(const std::string&, const std::string&) override { return false; }
};                                                      // ↑ siempre lo mismo
```

```cpp
TEST(Avisador, SiElCorreoNoSaleElAvisoNoSeDaPorHecho) {
    NotificadorQueFalla correoCaido;
    Avisador avisador{correoCaido};

    EXPECT_FALSE(avisador.Revisar(Cuenta{"ana@ejemplo.com", -1200}));
}                       // ↑ el EXPECT sigue hablando del RESULTADO de mi código
```

**Lo reconocéis porque:** el doble **da de comer** a vuestro código, y el
`EXPECT` **no menciona al doble**.

> **Esta es la razón más importante para usar dobles.** En producción no sabéis
> tirar el servidor de correo a voluntad. Aquí sí, y es gratis.

---

## 7. SPY — el continuista que toma notas

> **En el cine:** la persona que apunta todo lo que pasa en cada toma. No
> interrumpe. Al terminar le preguntáis, y os lo cuenta.

**Qué quiero probar:** que el aviso va **al titular** y **lleva el saldo**.

Ahora el resultado de `Revisar` me da igual. Lo que quiero saber es **qué le
llegó al notificador**. Así que necesito un doble que lo apunte.

```cpp
class NotificadorSpy : public INotificador {
public:
    struct Mensaje { std::string destinatario; std::string texto; };

    bool Enviar(const std::string& destinatario, const std::string& texto) override {
        enviados.push_back({destinatario, texto});     // apunta y ya
        return true;
    }
    std::vector<Mensaje> enviados;                     // ← lo que el test mira
};
```

```cpp
TEST(Avisador, ElAvisoVaAlTitularYLlevaElSaldo) {
    NotificadorSpy espia;
    Avisador avisador{espia};

    avisador.Revisar(Cuenta{"ana@ejemplo.com", -1200});

    ASSERT_EQ(1u, espia.enviados.size());                      // ← ahora el EXPECT
    EXPECT_EQ("ana@ejemplo.com", espia.enviados[0].destinatario); //  SÍ habla
    EXPECT_EQ("Saldo negativo: -12.00 EUR", espia.enviados[0].texto); // del doble
}
```

**Lo reconocéis porque:** el doble **guarda una lista**, y el `EXPECT` va al
final, mirando esa lista.

---

## 8. MOCK — el director con el guion en la mano

> **En el cine:** el director dice **antes** de rodar: *"aquí María dice su frase
> y sale por la izquierda"*. Si no pasa, grita **"¡corten!"** en el momento. No
> espera al final.

**Qué quiero probar:** exactamente lo mismo que en el spy.

Y ese es el detalle que hay que entender: **spy y mock comprueban lo mismo**. La
diferencia es **cuándo se escribe la expectativa** y **quién falla**.

```cpp
class NotificadorMock : public INotificador {
public:
    MOCK_METHOD(bool, Enviar, (const std::string& destinatario, const std::string& texto),
                (override));
};                     // ↑ gMock escribe por vosotros la lista del spy
```

```cpp
TEST(Avisador, ElAvisoVaAlTitularYLlevaElSaldo_ConMock) {
    NotificadorMock notificador;
    EXPECT_CALL(notificador, Enviar("ana@ejemplo.com", "Saldo negativo: -12.00 EUR"))
        .WillOnce(Return(true));          // ← la expectativa, ANTES de actuar

    Avisador avisador{notificador};
    avisador.Revisar(Cuenta{"ana@ejemplo.com", -1200});
}                                         // ← no hay Assert: falla él solo, aquí
```

### Los dos, el mismo test, puestos uno al lado del otro

|                          | **Spy**                            | **Mock**                          |
|--------------------------|------------------------------------|-----------------------------------|
| La expectativa se escribe| **después** de actuar              | **antes** de actuar               |
| ¿Quién falla?            | **vosotros**, con un `EXPECT`      | **el doble**, solo                |
| ¿Quién escribe el doble? | vosotros (~8 líneas)               | gMock (1 macro)                   |
| Se lee como              | Arrange-Act-**Assert**             | **Arrange**-Act                   |

Y así fallan los dos cuando el aviso no llega (**salida real**):

```
[ RUN      ] Fallo.ConSpy
Expected equality of these values:
  1u
    Which is: 1
  espia.enviados.size()
    Which is: 0                                  ← "esperaba 1 mensaje, hay 0"

[ RUN      ] Fallo.ConMock
Actual function call count doesn't match
EXPECT_CALL(notificador, Enviar("ana@ejemplo.com", "Saldo negativo: -12.00 EUR"))...
         Expected: to be called once
           Actual: never called - unsatisfied and active
```

El del mock dice **qué llamada esperaba y con qué argumentos**, sin que vosotros
escribáis nada. Por eso, cuando basta con *"esto tenía que ocurrir"*, el mock
gana.

El spy gana cuando queréis **rebuscar** en lo que llegó: ordenar, contar,
comparar el tercer mensaje con el primero.

---

## 9. FAKE — el decorado que funciona de verdad

> **En el cine:** la cocina del plató. Los grifos dan agua y el fuego calienta.
> **No es** la cocina del restaurante, pero **funciona**.

Aquí cambiamos de colaborador, y el motivo es importante.

**Qué quiero probar:** que de tres cuentas, se avisa solo a las dos en rojo.

Para eso el código necesita un repositorio del que sacar las cuentas. Y el test
necesita **meter** esas tres cuentas primero. Un stub tendría que enlatar la
respuesta de `Todas()`, otra para `Buscar()`, otra para después de guardar…
insoportable.

El colaborador nuevo y el sujeto que lo usa:

```cpp
class IRepositorio {
public:
    virtual ~IRepositorio() = default;
    virtual void                  Guardar(const Cuenta& c) = 0;
    virtual std::optional<Cuenta> Buscar(const std::string& email) const = 0;
    virtual std::vector<Cuenta>   Todas() const = 0;
};

class AvisadorDeCartera {
public:
    AvisadorDeCartera(IRepositorio& repo, INotificador& notificador)
        : repo_{repo}, notificador_{notificador} {}

    int RevisarTodas();        // devuelve a cuántos se avisó
private:
    IRepositorio& repo_;
    INotificador& notificador_;
};
```

Lo que hace falta es algo que **se comporte como un repositorio**, pero sin base
de datos:

```cpp
class RepositorioEnMemoria : public IRepositorio {
public:
    void Guardar(const Cuenta& c) override { cuentas_[c.email] = c; }

    std::optional<Cuenta> Buscar(const std::string& email) const override {
        auto it = cuentas_.find(email);
        if (it == cuentas_.end()) return std::nullopt;   // ← comportamiento REAL
        return it->second;
    }
    std::vector<Cuenta> Todas() const override {
        std::vector<Cuenta> v;
        for (const auto& [email, c] : cuentas_) v.push_back(c);
        return v;
    }
private:
    std::map<std::string, Cuenta> cuentas_;              // ← guarda de verdad
};
```

```cpp
TEST(AvisadorDeCartera, AvisaSoloALasCuentasEnRojo) {
    RepositorioEnMemoria banco;                          // funciona de verdad
    banco.Guardar(Cuenta{"ana@ejemplo.com",  -1200});
    banco.Guardar(Cuenta{"luis@ejemplo.com",  3000});
    banco.Guardar(Cuenta{"eva@ejemplo.com",   -500});

    NotificadorSpy espia;
    AvisadorDeCartera avisador{banco, espia};

    EXPECT_EQ(2, avisador.RevisarTodas());

    std::vector<std::string> a_quien;
    for (const auto& m : espia.enviados) a_quien.push_back(m.destinatario);
    EXPECT_THAT(a_quien, UnorderedElementsAre("ana@ejemplo.com", "eva@ejemplo.com"));
}
```

El test **se lee como la situación real**: hay tres clientes, dos deben cero.

**Lo reconocéis porque:** el doble **tiene memoria**. Lo que le metéis, os lo
devuelve.

> ⚠️ **El precio del fake, y es serio:** tiene lógica, luego puede tener bugs. Si
> el fake está mal, vuestros tests mienten en verde. Por eso **un fake necesita
> sus propios tests**:
>
> ```cpp
> TEST(RepositorioEnMemoria, SeComportaComoUnRepositorioDeVerdad) {
>     RepositorioEnMemoria banco;
>     EXPECT_FALSE(banco.Buscar("nadie@ejemplo.com").has_value());
>
>     banco.Guardar(Cuenta{"ana@ejemplo.com", 100});
>     banco.Guardar(Cuenta{"ana@ejemplo.com", 900});          // sobrescribe
>
>     EXPECT_EQ(900, banco.Buscar("ana@ejemplo.com")->saldoCentimos);
>     EXPECT_EQ(1u,  banco.Todas().size());
> }
> ```
>
> Si eso os parece demasiado trabajo, **es que no necesitabais un fake**:
> necesitabais un stub.

---

## 10. La chuleta

Pegadla en el monitor:

| Doble | En una frase | Lo reconocéis porque |
|---|---|---|
| **Dummy** | Relleno. No hace nada | Podríais vaciarlo y el test pasa igual |
| **Stub** | Responde siempre lo mismo | Da de comer; el `EXPECT` no lo menciona |
| **Fake** | Funciona de verdad, en pequeño | Tiene memoria: le metéis algo y os lo devuelve |
| **Spy** | Apunta lo que le llega | Guarda una lista; el `EXPECT` la mira al final |
| **Mock** | Sabe de antemano qué espera | La expectativa va **antes**; falla él solo |

Y las tres dudas que salen siempre:

| Duda | Respuesta |
|---|---|
| *"¿Stub o mock?"* | Mirad el `EXPECT`. ¿Habla del resultado de **vuestro** código? Stub. ¿Habla de la **llamada**? Mock |
| *"¿Spy o mock?"* | Mock si basta *"tenía que ocurrir"*. Spy si necesitáis rebuscar en los datos |
| *"¿Stub o fake?"* | Contad las respuestas distintas que hace falta enlatar. Una o dos: stub. Cinco: fake |

---

## 11. Bad — doblar lo que no toca

El error de principiante no es elegir mal el tipo. Es **usar un doble donde no
hacía falta ninguno**.

Un carrito con descuento. Todo esto es código vuestro, rápido y predecible:

```cpp
struct Producto { std::string nombre; int precioCentimos; };

class IDescuento {
public:
    virtual ~IDescuento() = default;
    virtual int Aplicar(int totalCentimos) const = 0;
};

class DescuentoFijo : public IDescuento {                   // 10 %
public:
    int Aplicar(int total) const override { return total - total / 10; }
};
```

Y el test que escribe casi todo el mundo la primera vez:

```cpp
TEST(Carrito, CalculaElTotal) {
    ProductoMock p1, p2;                                 // ← mocks de DATOS
    EXPECT_CALL(p1, Precio()).WillOnce(Return(1000));
    EXPECT_CALL(p2, Precio()).WillOnce(Return(2000));
    DescuentoMock d;
    EXPECT_CALL(d, Aplicar(3000)).WillOnce(Return(2700));  // ← copia la lógica

    Carrito c{&d};
    c.Anadir(&p1);
    c.Anadir(&p2);

    EXPECT_EQ(2700, c.Total());     // ← esto prueba que 2700 es 2700
}
```

`Producto` es un **dato**: se construye, no se dobla. Y el `EXPECT_CALL` del
descuento **repite el cálculo** que dice estar probando: si está mal en
producción y mal igual aquí, el test pasa.

## 12. Good — doblar solo la frontera

```cpp
TEST(Carrito, AplicaElDescuentoAlTotal) {
    DescuentoFijo diez;                 // objeto real: es lógica pura y rápida
    Carrito c{&diez};

    c.Anadir(Producto{"A", 1000});      // datos reales: son structs
    c.Anadir(Producto{"B", 2000});

    EXPECT_EQ(2700, c.TotalEnCentimos());
}
```

La regla, en una frase:

> **Doblad lo que sale del proceso** —red, disco, reloj, base de datos, hilos,
> azar—. **No dobléis lo que es vuestro, rápido y predecible.**

Es la **costura** del día 1: está donde el código se os va de las manos, y ahí es
donde va el doble. En el 70 % de los tests **no hace falta ningún doble**.

---

## 13. Qué ganamos y qué pagamos

**Ganamos:** tests en milisegundos; poder provocar el error que en producción no
sabéis provocar (el correo caído del apartado 6); y un aviso de diseño, porque
un colaborador imposible de doblar suele estar mal separado.

**Pagamos:** el test se acopla a **cómo** está hecho el código, no solo a qué
produce. Cada `EXPECT_CALL` es una decisión de implementación congelada, y
encarece el refactor. De ahí la regla del apartado 12.

> **El aviso honesto:** una suite llena de mocks puede estar al 95 % de cobertura
> y no enterarse de que el sistema no arranca, porque nadie ha juntado nunca las
> piezas de verdad. Los dobles **no sustituyen** a los tests de integración.

## 14. Conexión con lo que viene

Los cinco dobles de hoy están **escritos a mano**, como los tres del bloque 11
(`FakeCanvas`, `StringReader`, `RecordingWriter`). En el **bloque 13**, gMock
escribe los dummies, stubs, spies y mocks por vosotros con una macro.

El **fake seguirá escribiéndose a mano**: gMock no genera fakes, porque un fake
tiene lógica. Esa es la razón práctica de haber aprendido a distinguirlos.

## 15. Mantra del bloque

> **"Stub para entrar, mock para salir."**
> Lo que alimenta a vuestro código es un stub. Lo que vuestro código provoca
> hacia fuera es un mock. Si dudáis, mirad sobre qué va el `EXPECT`.

---

## 16. Referencias

**Lo primero que hay que leer (gratuito):**

- **[Martin Fowler — *TestDouble*](https://martinfowler.com/bliki/TestDouble.html)**
  — **dos minutos**: los cinco tipos, una frase cada uno. Es la chuleta del
  apartado 10 escrita por quien puso el nombre. Empezad por aquí.
- **[Martin Fowler — *Mocks Aren't Stubs*](https://martinfowler.com/articles/mocksArentStubs.html)**
  — más largo, y el que de verdad fija la diferencia: comprobar **el estado**
  (apartados 5, 6 y 9) frente a comprobar **la interacción** (7 y 8).

**El catálogo completo, con una ficha por tipo (gratuito):**

- **[xUnit Patterns — *Test Double*](http://xunitpatterns.com/Test%20Double.html)**,
  con **[Dummy](http://xunitpatterns.com/Dummy%20Object.html)**,
  **[Stub](http://xunitpatterns.com/Test%20Stub.html)**,
  **[Spy](http://xunitpatterns.com/Test%20Spy.html)**,
  **[Mock](http://xunitpatterns.com/Mock%20Object.html)** y
  **[Fake](http://xunitpatterns.com/Fake%20Object.html)** — el origen de la
  clasificación, de Gerard Meszaros. Cada ficha trae además los antipatrones.

**Para el bloque siguiente:**

- **[gMock for Dummies](https://google.github.io/googletest/gmock_for_dummies.html)**
  — **gratuito**. Su sección *"When to use gMock"* dice lo mismo que el apartado
  3, desde el lado de la herramienta. Es la lectura previa al bloque 13.

**Libros:**

- 📖 Gerard Meszaros,
  **[*xUnit Test Patterns*](https://www.informit.com/store/xunit-test-patterns-refactoring-test-code-9780131495050)**
  — caps. 11 y 23: el catálogo entero, con el coste de mantenimiento de cada
  tipo medido en proyectos reales.
- 📖 Jeff Langr,
  **[*Modern C++ Programming with TDD*](https://pragprog.com/titles/lotdd/modern-c-programming-with-test-driven-development/)**,
  cap. 5 — los dobles **en C++**, con los problemas que no tienen Java ni C#:
  punteros, propiedad y destructores virtuales.
- **[*Software Engineering at Google*, cap. 13](https://abseil.io/resources/swe-book/html/ch13.html)**
  — **gratuito**. Por qué a gran escala prefieren **fakes** a mocks, y qué les
  costó descubrirlo. El contrapeso al entusiasmo por `EXPECT_CALL`.
