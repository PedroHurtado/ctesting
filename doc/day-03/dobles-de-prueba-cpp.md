# Dobles de prueba en C++: Dummy, Fake, Stub, Spy y Mock

## ¿Qué es un doble de prueba?

Cuando probamos una clase, muchas veces depende de otras cosas: una base de datos, un servidor de correo, un fichero de log... No queremos usar esas cosas reales en un test porque son lentas, difíciles de preparar o tienen efectos secundarios (nadie quiere enviar un email de verdad cada vez que ejecuta los tests).

Un **doble de prueba** es un objeto que **sustituye a una dependencia real** durante un test, igual que un doble de cine sustituye al actor en las escenas peligrosas.

Hay cinco tipos, y cada uno tiene un propósito distinto:

| Tipo      | ¿Qué hace?                                         | En una frase         |
|-----------|----------------------------------------------------|----------------------|
| **Dummy** | Nada. Solo rellena un parámetro.                   | No hace nada         |
| **Fake**  | Funciona de verdad, pero simplificado.             | Funciona a lo barato |
| **Stub**  | Devuelve respuestas fijas preparadas.              | Responde             |
| **Spy**   | Como un stub, pero apunta cómo lo han llamado.     | Responde y apunta    |
| **Mock**  | Sabe de antemano qué llamadas espera y lo verifica.| Exige                |

---

## El código que vamos a probar

Para poder sustituir dependencias en C++, la clase que probamos debe depender de **interfaces** (clases abstractas con métodos virtuales puros), no de clases concretas. Así podemos pasarle la implementación real en producción o un doble en los tests.

```cpp
#include <string>

struct Pedido {
    int id;
    std::string cliente;
};

// Interfaces: contratos que cumplirán tanto las clases reales como los dobles
class IRepositorio {
public:
    virtual ~IRepositorio() = default;
    virtual Pedido buscar(int id) = 0;
};

class IEmail {
public:
    virtual ~IEmail() = default;
    virtual void enviar(const std::string& destino, const std::string& texto) = 0;
};

class ILogger {
public:
    virtual ~ILogger() = default;
    virtual void info(const std::string& mensaje) = 0;
};

// La clase que queremos probar
class ServicioPedidos {
public:
    ServicioPedidos(IRepositorio& r, IEmail& e, ILogger& l)
        : repositorio(r), email(e), logger(l) {}

    Pedido confirmar(int id) {
        Pedido p = repositorio.buscar(id);
        email.enviar(p.cliente, "Pedido confirmado");
        return p;
    }

private:
    IRepositorio& repositorio;
    IEmail& email;
    ILogger& logger;   // lo pide el constructor, pero confirmar() no lo usa
};
```

`ServicioPedidos` necesita tres dependencias. Vamos a ver qué doble encaja con cada una.

---

## 1. Dummy — "no hace nada"

Un dummy existe **solo para poder compilar o construir el objeto**. El constructor exige un `ILogger`, pero el método que probamos nunca lo usa. Le damos uno vacío y listo.

```cpp
class LoggerDummy : public ILogger {
public:
    void info(const std::string&) override {}   // cuerpo vacío
};
```

**Cuándo usarlo:** cuando un parámetro es obligatorio pero irrelevante para el test.

---

## 2. Fake — "funciona a lo barato"

Un fake **tiene lógica real**, pero usa un atajo. Aquí el repositorio guarda los pedidos en un `std::map` en memoria en lugar de en una base de datos.

```cpp
#include <map>

class RepositorioFake : public IRepositorio {
public:
    void guardar(const Pedido& p) { datos[p.id] = p; }

    Pedido buscar(int id) override { return datos.at(id); }

private:
    std::map<int, Pedido> datos;
};
```

Uso:

```cpp
RepositorioFake repo;
repo.guardar({7, "luis@mail.com"});

LoggerDummy logger;
EmailSpy email;   // (lo vemos más abajo)

ServicioPedidos servicio(repo, email, logger);
assert(servicio.confirmar(7).cliente == "luis@mail.com");
```

**Cuándo usarlo:** cuando necesitas un comportamiento realista (guardar, buscar, borrar...) sin montar la infraestructura de verdad.

---

## 3. Stub — "responde"

Un stub **devuelve siempre la misma respuesta preparada**, sin lógica. No importa qué le pidas.

```cpp
class RepositorioStub : public IRepositorio {
public:
    Pedido buscar(int) override {
        return {1, "ana@mail.com"};   // siempre lo mismo
    }
};
```

**Cuándo usarlo:** para controlar **lo que entra** en el código que pruebas. "Si el repositorio devuelve esto, ¿qué hace mi servicio?"

> **Diferencia con el fake:** el fake tiene lógica (si guardas algo, luego lo encuentras); el stub no, solo devuelve un valor fijo.

---

## 4. Spy — "responde y apunta"

Un spy hace su papel, pero además **anota cada llamada**: cuántas veces lo han llamado y con qué argumentos. Al final del test, **tú** consultas esa información.

```cpp
#include <vector>

class EmailSpy : public IEmail {
public:
    struct Llamada {
        std::string destino;
        std::string texto;
    };

    std::vector<Llamada> llamadas;   // aquí se apunta todo

    void enviar(const std::string& destino, const std::string& texto) override {
        llamadas.push_back({destino, texto});
    }
};
```

Uso:

```cpp
RepositorioStub repo;
EmailSpy email;
LoggerDummy logger;

ServicioPedidos servicio(repo, email, logger);
servicio.confirmar(1);

// Eres TÚ quien comprueba después
assert(email.llamadas.size() == 1);
assert(email.llamadas[0].destino == "ana@mail.com");
```

**Cuándo usarlo:** para comprobar **lo que sale** del código (qué llamadas hizo), revisándolo al final del test.

---

## 5. Mock — "exige"

Un mock se crea **sabiendo ya lo que espera recibir**. Durante el test registra lo que le llega y, al final, **él mismo verifica** si se cumplió lo esperado. Si no, falla.

```cpp
#include <stdexcept>

class EmailMock : public IEmail {
public:
    // Las expectativas se fijan ANTES de ejecutar
    EmailMock(std::string destino, std::string texto)
        : destinoEsperado(std::move(destino)), textoEsperado(std::move(texto)) {}

    void enviar(const std::string& destino, const std::string& texto) override {
        ++veces;
        destinoRecibido = destino;
        textoRecibido = texto;
    }

    // El propio mock sabe si se cumplió lo esperado
    void verificar() const {
        if (veces != 1 ||
            destinoRecibido != destinoEsperado ||
            textoRecibido != textoEsperado) {
            throw std::runtime_error("El email no se envió como se esperaba");
        }
    }

private:
    std::string destinoEsperado, textoEsperado;
    std::string destinoRecibido, textoRecibido;
    int veces = 0;
};
```

Uso:

```cpp
RepositorioStub repo;
LoggerDummy logger;
EmailMock email("ana@mail.com", "Pedido confirmado");   // 1. expectativas

ServicioPedidos servicio(repo, email, logger);
servicio.confirmar(1);                                    // 2. ejecución

email.verificar();                                        // 3. el mock comprueba
```

**Cuándo usarlo:** para verificar el **comportamiento** (que se llamó a lo que debía, las veces que debía y con los datos correctos).

> **Diferencia con el spy:** con el spy compruebas **tú** al final; con el mock, la expectativa está **dentro del propio mock** y es él quien decide si el test pasa.

---

## Programa completo

Todo junto, listo para compilar y ejecutar:

```cpp
#include <cassert>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

struct Pedido { int id; std::string cliente; };

class IRepositorio {
public:
    virtual ~IRepositorio() = default;
    virtual Pedido buscar(int id) = 0;
};

class IEmail {
public:
    virtual ~IEmail() = default;
    virtual void enviar(const std::string& destino, const std::string& texto) = 0;
};

class ILogger {
public:
    virtual ~ILogger() = default;
    virtual void info(const std::string& mensaje) = 0;
};

class ServicioPedidos {
public:
    ServicioPedidos(IRepositorio& r, IEmail& e, ILogger& l)
        : repositorio(r), email(e), logger(l) {}

    Pedido confirmar(int id) {
        Pedido p = repositorio.buscar(id);
        email.enviar(p.cliente, "Pedido confirmado");
        return p;
    }

private:
    IRepositorio& repositorio;
    IEmail& email;
    ILogger& logger;
};

// ---------- Dobles ----------

class LoggerDummy : public ILogger {
public:
    void info(const std::string&) override {}
};

class RepositorioFake : public IRepositorio {
public:
    void guardar(const Pedido& p) { datos[p.id] = p; }
    Pedido buscar(int id) override { return datos.at(id); }
private:
    std::map<int, Pedido> datos;
};

class RepositorioStub : public IRepositorio {
public:
    Pedido buscar(int) override { return {1, "ana@mail.com"}; }
};

class EmailSpy : public IEmail {
public:
    struct Llamada { std::string destino; std::string texto; };
    std::vector<Llamada> llamadas;
    void enviar(const std::string& d, const std::string& t) override {
        llamadas.push_back({d, t});
    }
};

class EmailMock : public IEmail {
public:
    EmailMock(std::string d, std::string t)
        : destinoEsperado(std::move(d)), textoEsperado(std::move(t)) {}

    void enviar(const std::string& d, const std::string& t) override {
        ++veces;
        destinoRecibido = d;
        textoRecibido = t;
    }

    void verificar() const {
        if (veces != 1 || destinoRecibido != destinoEsperado || textoRecibido != textoEsperado)
            throw std::runtime_error("El email no se envió como se esperaba");
    }

private:
    std::string destinoEsperado, textoEsperado, destinoRecibido, textoRecibido;
    int veces = 0;
};

// ---------- Tests ----------

int main() {
    LoggerDummy logger;

    // Fake
    RepositorioFake repoFake;
    repoFake.guardar({7, "luis@mail.com"});
    EmailSpy emailAux;
    ServicioPedidos s1(repoFake, emailAux, logger);
    assert(s1.confirmar(7).cliente == "luis@mail.com");

    // Stub + Spy
    RepositorioStub repoStub;
    EmailSpy spy;
    ServicioPedidos s2(repoStub, spy, logger);
    s2.confirmar(1);
    assert(spy.llamadas.size() == 1);
    assert(spy.llamadas[0].destino == "ana@mail.com");

    // Stub + Mock
    EmailMock mock("ana@mail.com", "Pedido confirmado");
    ServicioPedidos s3(repoStub, mock, logger);
    s3.confirmar(1);
    mock.verificar();

    return 0;
}
```

Compilar y ejecutar:

```bash
g++ -std=c++17 -Wall main.cpp -o dobles && ./dobles
```

Si no aparece ningún error, todos los tests han pasado.

---

## Resumen para recordar

- **Dummy** → está, pero no se usa.
- **Fake** → funciona de verdad, con un atajo (memoria en vez de BD).
- **Stub** → devuelve respuestas fijas. Controla lo que **entra**.
- **Spy** → apunta las llamadas; **tú** compruebas al final.
- **Mock** → lleva las expectativas dentro y **él** verifica.

> En proyectos reales no se escriben los mocks a mano: se usan librerías como **GoogleTest + gMock**, que generan todo esto automáticamente. Pero la idea de fondo es exactamente la misma que has visto aquí.
