# Día 1 — Bloque 3: Diseño para poder testear

> *"Si es difícil de testear, está mal diseñado. El test no es la víctima:
> es el mensajero."*

---

## 1. El problema en una frase

La mayoría de las veces que alguien dice *"esto no se puede testear"*, lo que
está diciendo en realidad es *"esto está demasiado acoplado"*.

## 2. La idea central: la testabilidad es una consecuencia, no un objetivo

No se "añade testabilidad" a un código. La testabilidad **aparece** cuando el
diseño cumple tres condiciones:

1. **Control**: puedo poner el objeto en el estado que quiero probar.
2. **Observación**: puedo ver el resultado sin recurrir a trucos.
3. **Aislamiento**: puedo ejecutarlo sin arrastrar medio sistema.

Si falla alguna de las tres, no falta un *framework* de test: falta un refactor.

## 3. Diseño simple: las cuatro reglas de Kent Beck

En orden de prioridad. Un diseño es **simple** (que no es lo mismo que *fácil*)
si, por este orden:

1. **Pasa todos los tests.** Si no funciona, lo demás da igual.
2. **Revela la intención.** Se lee y se entiende qué quiere hacer.
3. **No tiene duplicación.** Cada decisión vive en un único sitio.
4. **Tiene el mínimo de elementos.** Ni una clase, ni un método, ni un
   parámetro de más.

Y el corolario que más cuesta aceptar: **YAGNI** (*You Aren't Gonna Need It*).
La abstracción "por si acaso" es el *code smell* más caro del sector, porque
además parece buen diseño.

### Bad: abstracción especulativa

```cpp
// Necesito guardar un informe en un fichero. Por si acaso, dejo esto montado:
template <typename T, typename Serializer, typename Compressor,
          typename Encryptor, typename Transport>
class GenericReportPipeline {
    // ... 200 líneas, 5 puntos de extensión
    // En producción solo existe una instanciación. Nunca ha habido otra.
};
```

Coste real: cinco conceptos que entender, errores de plantilla ilegibles, y
tests que necesitan cinco dobles para instanciar nada.

### Good: lo que hace falta hoy

```cpp
class ReportWriter {
public:
    explicit ReportWriter(std::ostream& out) : out_(out) {}
    void write(const Report& r) { out_ << format(r); }
private:
    std::ostream& out_;
};
```

Un concepto. Y, de regalo, **es testeable**: le paso un `std::ostringstream` y
compruebo el texto. Cuando de verdad aparezca el segundo destino, extraigo la
interfaz — con los tests puestos, que es cuando refactorizar es barato.

## 4. SOLID leído en clave de testabilidad

Los mismos cinco principios del curso anterior, ahora con otra lente.

| Principio | En clave de test                                                                 |
|-----------|----------------------------------------------------------------------------------|
| **SRP**   | Una razón para cambiar = un motivo para fallar. El nombre del test sale solo.     |
| **OCP**   | Añadir un caso no obliga a reescribir los tests que ya pasaban.                   |
| **LSP**   | Los tests de la base deben pasar tal cual para **toda** derivada (*test contract*).|
| **ISP**   | Interfaz gorda = mock gordo. 12 métodos a implementar para probar uno.            |
| **DIP**   | Sin inversión no hay dónde enchufar el doble. **Es el principio clave del curso.** |

### DIP: el principio que hace posible el test unitario

**Bad — dependencia hormigonada:**

```cpp
class ProcesadorPedidos {
public:
    void procesar(const Pedido& p) {
        MySqlConnection db("prod-db:3306");     // ← ¿test? necesito una BD
        db.insert(p);

        SmtpClient smtp("smtp.empresa.com");    // ← ¿test? envío correos reales
        smtp.send(p.email(), "Pedido confirmado");

        auto hoy = std::chrono::system_clock::now();  // ← ¿test? no controlo el reloj
        if (esFestivo(hoy)) p.marcarRetrasado();
    }
};
```

Para probar la regla *"si es festivo, se marca como retrasado"* —tres líneas de
lógica de negocio— necesitas: una base de datos levantada, un servidor SMTP, y
esperar a que sea festivo. **No es testeable. Y no es culpa del test.**

Síntomas del mismo problema, todos habituales:

- `new` / instanciación concreta dentro del método.
- Singletons (`Logger::instance()`, `Config::get()`).
- Llamadas a reloj, sistema de ficheros, red o `rand()` incrustadas.
- Estado global (`static`, variables de entorno leídas al vuelo).

**Good — dependencias inyectadas:**

```cpp
// Interfaces mínimas (ISP): una responsabilidad cada una
struct IRepositorioPedidos {
    virtual ~IRepositorioPedidos() = default;
    virtual void guardar(const Pedido&) = 0;
};

struct INotificador {
    virtual ~INotificador() = default;
    virtual void notificar(const std::string& destino,
                           const std::string& mensaje) = 0;
};

struct IReloj {
    virtual ~IReloj() = default;
    virtual std::chrono::system_clock::time_point ahora() const = 0;
};

class ProcesadorPedidos {
public:
    ProcesadorPedidos(IRepositorioPedidos& repo,
                      INotificador& notif,
                      const IReloj& reloj)
        : repo_(repo), notif_(notif), reloj_(reloj) {}

    void procesar(Pedido& p) {
        repo_.guardar(p);
        notif_.notificar(p.email(), "Pedido confirmado");
        if (esFestivo(reloj_.ahora())) p.marcarRetrasado();
    }

private:
    IRepositorioPedidos& repo_;
    INotificador&        notif_;
    const IReloj&        reloj_;
};
```

Y ahora el test, sin BD, sin SMTP y sin esperar a Navidad:

```cpp
struct RelojFijo : IReloj {
    std::chrono::system_clock::time_point t;
    std::chrono::system_clock::time_point ahora() const override { return t; }
};
struct RepoEnMemoria : IRepositorioPedidos {
    std::vector<Pedido> guardados;
    void guardar(const Pedido& p) override { guardados.push_back(p); }
};
struct NotificadorNulo : INotificador {
    void notificar(const std::string&, const std::string&) override {}
};

TEST(ProcesadorPedidos, EnFestivoMarcaElPedidoComoRetrasado) {
    RepoEnMemoria repo;  NotificadorNulo notif;
    RelojFijo reloj{ fecha(2026, 12, 25) };          // Navidad, a voluntad
    ProcesadorPedidos sut(repo, notif, reloj);

    Pedido p{"ana@ejemplo.com"};
    sut.procesar(p);

    EXPECT_TRUE(p.estaRetrasado());
}
```

Milisegundos, determinista, y **el nombre del test explica la regla de negocio**.
Esas clases `RelojFijo`, `RepoEnMemoria` y `NotificadorNulo` son *dobles de
prueba*; el día 3 las generaremos automáticamente con gMock.

> `sut` = *System Under Test*. Es la convención para nombrar el objeto que se
> está probando. Usadla: hace evidente de un vistazo quién es el protagonista.

### Las "costuras" (*seams*)

Michael Feathers, *Working Effectively with Legacy Code*: una **costura** es un
punto donde puedes cambiar el comportamiento **sin editar el código en ese
punto**. En C++ las costuras habituales son:

| Tipo de costura       | Mecanismo                                        |
|-----------------------|--------------------------------------------------|
| **De objeto**         | Interfaz + inyección (la que acabamos de ver)    |
| **De plantilla**      | Parámetro de política (`template <class Clock>`)  |
| **De enlace**         | Sustituir una implementación al enlazar          |
| **De preprocesador**  | `#define` — último recurso, evitadlo             |

Si un método no tiene ninguna costura, no se puede testear en aislamiento.
**Refactorizar para testear = crear costuras.**

## 5. "Dile, no preguntes" (*Tell, Don't Ask*)

Consecuencia directa de la Ley de Demeter. No preguntes por el estado de un
objeto para decidir tú; **dile lo que quieres y deja que decida él**.

### Bad: preguntar

```cpp
// El que llama saca las tripas del objeto y decide por él
if (cuenta.getSaldo() >= importe && cuenta.getEstado() == Estado::Activa
    && !cuenta.getTitular().estaBloqueado()) {
    cuenta.setSaldo(cuenta.getSaldo() - importe);
    registro.log("Retirada de " + std::to_string(importe));
}
```

Problemas:
- La **regla de negocio vive fuera** del objeto que la debería contener.
- Está duplicada en todos los sitios que retiran dinero.
- Para testear la regla tienes que testear al llamante, con todo su contexto.
- `cuenta.getTitular().estaBloqueado()` es un tren de llamadas (Demeter).

### Good: decir

```cpp
class Cuenta {
public:
    // La regla vive donde viven los datos
    bool retirar(double importe) {
        if (!puedeRetirar(importe)) return false;
        saldo_ -= importe;
        return true;
    }
private:
    bool puedeRetirar(double importe) const {
        return estado_ == Estado::Activa && !titular_.estaBloqueado()
               && saldo_ >= importe;
    }
    double saldo_;
    Estado estado_;
    Titular titular_;
};
```

El test resultante es directo, y prueba **la regla**, no la orquestación:

```cpp
TEST(Cuenta, NoPermiteRetirarMasDelSaldoDisponible) {
    Cuenta c = CuentaBuilder().activa().conSaldo(100).build();
    EXPECT_FALSE(c.retirar(150));
    EXPECT_DOUBLE_EQ(100.0, c.saldo());
}
```

> Regla práctica: **una clase llena de `getters` públicos y sin comportamiento
> es una estructura de datos disfrazada**. Y su lógica está desperdigada en
> quien la usa — que es justo lo que no se puede testear.

## 6. "Divide y vencerás": separar decisión de efecto

El patrón más útil para hacer testeable código "sucio": separar la parte que
**decide** (lógica pura, fácil de testear) de la que **actúa** (E/S, efectos,
difícil de testear).

### Bad: todo mezclado

```cpp
void generarInformeMensual() {
    auto datos = leerDeBD();                       // efecto
    double total = 0;
    for (auto& d : datos)
        if (d.fecha.mes() == mesActual()) total += d.importe;   // decisión
    std::ofstream f("informe.txt");                 // efecto
    f << "Total: " << total;                        // decisión (formato)
}
```

No hay ni un punto donde agarrar esto para probarlo.

### Good: núcleo puro, cáscara fina

```cpp
// Núcleo funcional: sin E/S, determinista, trivial de testear
double totalDelMes(const std::vector<Movimiento>& datos, int mes) {
    double total = 0;
    for (const auto& d : datos)
        if (d.fecha.mes() == mes) total += d.importe;
    return total;
}

std::string formatearInforme(double total) {
    return "Total: " + std::to_string(total);
}

// Cáscara imperativa: solo pega piezas, casi sin lógica que romper
void generarInformeMensual(IRepositorio& repo, std::ostream& salida, int mes) {
    salida << formatearInforme(totalDelMes(repo.leerTodos(), mes));
}
```

Se conoce como **núcleo funcional / cáscara imperativa** (*functional core,
imperative shell*). La regla: **cuanto más adentro, más puro y más testeado;
cuanto más afuera, más fino y menos lógica**.

Ahora los tests son triviales y rapidísimos:

```cpp
TEST(Informe, SumaSoloLosMovimientosDelMesIndicado) {
    std::vector<Movimiento> movs{ {fecha(2026,9,1), 100.0},
                                  {fecha(2026,8,30), 50.0} };
    EXPECT_DOUBLE_EQ(100.0, totalDelMes(movs, 9));
}
```

## 7. Señales de que el diseño te está pidiendo ayuda

Si al escribir un test te ves haciendo alguna de estas cosas, **para y
refactoriza**: el test está diagnosticando el diseño.

| Lo que haces en el test                               | Lo que revela del diseño              |
|-------------------------------------------------------|---------------------------------------|
| Necesitas 8 líneas de *setup* antes del `assert`      | Constructor/objeto con demasiado peso |
| Tocas ficheros, BD o red para un test "unitario"      | Falta inyección (DIP)                 |
| `#define private public` o `friend class Test`        | Estás probando la implementación      |
| `sleep()` para que "dé tiempo"                        | Falta controlar el tiempo / la concurrencia |
| Un mock devuelve otro mock que devuelve otro mock     | Ley de Demeter rota                   |
| El test falla según el orden de ejecución             | Estado global / singleton             |
| Hay que cambiar 30 tests por un refactor interno      | Tests acoplados a la implementación   |

## 8. Qué ganamos y qué pagamos

**Ganamos:** código donde el test es fácil de escribir — y, por el mismo
precio, código fácil de reutilizar, de cambiar y de entender.

**Pagamos:** más indirección. Una interfaz extra, un parámetro más en el
constructor. **Ojo:** el equilibrio importa. Inyectar *todo* lleva a la otra
patología, clases con 9 dependencias en el constructor, que es a su vez un
síntoma de SRP roto.

## 9. Mantra del bloque

> **"El test es el primer cliente de tu API."**
> Si al primer cliente le resulta incómoda, a los demás también.
> Escucha lo que te dice el test: casi siempre habla del diseño, no del test.

---

## 10. Referencias

**Diseño simple y YAGNI:**

- Martin Fowler, **[*Beck Design Rules*](https://martinfowler.com/bliki/BeckDesignRules.html)**
  — las cuatro reglas del apartado 3, explicadas en una página.
- Martin Fowler, **[*Yagni*](https://martinfowler.com/bliki/Yagni.html)** — el
  coste real de la abstracción especulativa.

**Costuras e inyección de dependencias:**

- 📖 Michael Feathers, **[*Working Effectively with Legacy Code*](https://www.informit.com/store/working-effectively-with-legacy-code-9780131177055)**
  — **la referencia** sobre costuras (*seams*). Los capítulos 4 y 25 tienen
  ejemplos en C++ directamente aplicables.
- Martin Fowler, **[*Clock Wrapper*](https://martinfowler.com/bliki/ClockWrapper.html)**
  — el caso concreto del `IReloj` del apartado 4.
- Martin Fowler, **[*Mocks Aren't Stubs*](https://martinfowler.com/articles/mocksArentStubs.html)**
  — vocabulario de dobles de prueba; lectura recomendada **antes del día 3**.

**Tell, Don't Ask y núcleo funcional:**

- Martin Fowler, **[*Tell Don't Ask*](https://martinfowler.com/bliki/TellDontAsk.html)**
  — el principio del apartado 5, con su matiz: no es un absoluto.
- Gary Bernhardt, **[*Boundaries*](https://www.destroyallsoftware.com/talks/boundaries)**
  — charla donde se acuña *functional core, imperative shell* (apartado 6).

**Diseño orientado a objetos:**

- 📖 Freeman & Pryce, **[*Growing Object-Oriented Software, Guided by Tests*](http://www.growing-object-oriented-software.com/)**
  — la tesis de este bloque desarrollada durante un libro entero.
- Robert C. Martin, **[*Solid Relevance*](https://blog.cleancoder.com/uncle-bob/2020/10/18/Solid-Relevance.html)**
  — repaso corto de SOLID, útil como recordatorio del curso anterior.
- **[C++ Core Guidelines — sección de interfaces (I)](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#S-interfaces)**
  — cómo se escribe en C++ moderno lo que aquí hemos visto en pizarra.
