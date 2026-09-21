# Día 1 — Bloque 4: FIRST, BICEPS y CORRECT

> Tres acrónimos que responden a tres preguntas distintas:
> **FIRST** → ¿cómo debe *ser* un test?
> **BICEPS** → ¿qué debo *comprobar*?
> **CORRECT** → ¿con qué *datos* lo pruebo?

---

## 1. El problema en una frase

"Escribe tests" es un consejo inútil sin criterios; estos tres acrónimos son la
versión operativa: **calidad del test, cobertura de comprobaciones y elección
de datos**.

---

# FIRST — Cómo debe ser un buen test

| Letra | Significado   | Regla                                                    |
|-------|---------------|----------------------------------------------------------|
| **F** | *Fast*        | Milisegundos. Si tarda, no se ejecuta.                   |
| **I** | *Independent* | No depende de otros tests ni del orden.                  |
| **R** | *Repeatable*  | Mismo resultado siempre, en cualquier máquina.           |
| **S** | *Self-validating* | Pasa o falla. Sin interpretar salidas a ojo.         |
| **T** | *Timely*      | Se escribe justo antes del código (TDD).                 |

## F — Fast

El umbral no es estético, es de comportamiento: si la suite tarda más de lo que
se tarda en perder el hilo, **se deja de ejecutar**.

```cpp
// Bad: 2 segundos de reloj de pared
TEST(Cache, ExpiraPasadoElTiempo) {
    Cache c{std::chrono::seconds(2)};
    c.put("k", "v");
    std::this_thread::sleep_for(std::chrono::seconds(3));  // ✘ lento y frágil
    EXPECT_FALSE(c.get("k").has_value());
}

// Good: el tiempo es una dependencia inyectada (bloque 3)
TEST(Cache, ExpiraPasadoElTiempo) {
    RelojFalso reloj{ t0 };
    Cache c{ reloj, std::chrono::seconds(2) };
    c.put("k", "v");
    reloj.avanzar(std::chrono::seconds(3));                // ✔ instantáneo
    EXPECT_FALSE(c.get("k").has_value());
}
```

## I — Independent

Cada test se monta y se desmonta solo. Nada de "este test deja la base
preparada para el siguiente".

```cpp
// Bad: estado compartido entre tests
static Carrito carritoGlobal;                    // ✘

TEST(Carrito, SePuedeAnadirUnProducto) { carritoGlobal.anadir(libro); ... }
TEST(Carrito, CalculaElTotal) {
    EXPECT_EQ(20.0, carritoGlobal.total());      // ✘ depende del test anterior
}
```

Google Test **no garantiza el orden** (y `--gtest_shuffle` lo aleatoriza a
propósito). Un test que solo pasa en orden alfabético es una bomba de relojería.

```cpp
// Good: fixture; se construye y destruye una instancia por test
class CarritoTest : public ::testing::Test {
protected:
    Carrito carrito;                             // ✔ nuevo en cada test
};

TEST_F(CarritoTest, SePuedeAnadirUnProducto) { ... }
TEST_F(CarritoTest, CalculaElTotal)          { ... }
```

## R — Repeatable

Mismo resultado en tu portátil, en el de tu compañero y en la CI, hoy y en
agosto. Enemigos clásicos en C++:

| Enemigo                        | Solución                                     |
|--------------------------------|----------------------------------------------|
| Reloj del sistema              | Inyectar `IReloj`                            |
| `rand()` sin semilla fija      | Semilla constante o generador inyectado      |
| Ruta absoluta `C:\temp\...`    | Directorio temporal por test                 |
| Orden de `std::unordered_map`  | No asumir orden; ordenar antes de comparar   |
| Locale (`,` vs `.`)            | Fijar locale o no depender de formato        |
| Red / servicios externos       | Doble de prueba                              |
| Concurrencia sin sincronizar   | Diseño determinista; `ThreadSanitizer`       |

> A un test que unas veces pasa y otras falla se le llama **flaky**. Un test
> *flaky* es **peor que no tener test**: entrena al equipo a ignorar el rojo.

## S — Self-validating

```cpp
// Bad: valida el humano
TEST(Factura, CalculaElIva) {
    std::cout << factura.total() << std::endl;   // ✘ ¿y esto está bien o no?
}

// Good: valida la máquina
TEST(Factura, AplicaElIvaGeneralDel21PorCiento) {
    EXPECT_DOUBLE_EQ(121.0, Factura{100.0}.total());
}
```

## T — Timely

El test se escribe **justo antes** del código que lo hace pasar. Escrito
después, sigue sirviendo de red de seguridad, pero se pierden dos cosas:
la presión de diseño sobre la API, y la garantía de que el test **puede fallar**
(un test escrito sobre código ya funcionando nunca se ha visto en rojo).

---

# BICEPS — Qué comprobar en cada test

Cuando te bloqueas mirando una función sin saber qué probar, recorre las seis
letras. Sobre este ejemplo:

```cpp
class Pila {
public:
    void push(int v);
    int  pop();                 // lanza std::out_of_range si está vacía
    int  cima() const;
    bool vacia() const;
    std::size_t tam() const;
private:
    std::vector<int> datos_;
};
```

| Letra | Significado           | Pregunta                                  |
|-------|-----------------------|-------------------------------------------|
| **B** | *Boundary*            | ¿Y en los bordes?                         |
| **I** | *Inverse*             | ¿Y si deshago la operación?               |
| **C** | *Cross-check*         | ¿Coincide con otra forma de calcularlo?   |
| **E** | *Error conditions*    | ¿Y si el entorno falla?                   |
| **P** | *Performance*         | ¿Sigue siendo aceptable a escala?         |
| **S** | *Stress / Right-BICEP*| ¿Son correctos los resultados esperados?  |

> Variante habitual: **Right-BICEP**, donde *Right* = "¿los resultados son los
> correctos?" (el *happy path*) y las seis letras son las comprobaciones
> adicionales.

### B — Boundary (bordes)

```cpp
TEST(Pila, PopSobrePilaVaciaLanza) {
    Pila p;
    EXPECT_THROW(p.pop(), std::out_of_range);
}
TEST(Pila, UnSoloElementoSeApilaYDesapila) {
    Pila p; p.push(7);
    EXPECT_EQ(7, p.pop());
    EXPECT_TRUE(p.vacia());
}
```

### I — Inverse (operación inversa)

```cpp
TEST(Pila, PushSeguidoDePopDejaLaPilaComoEstaba) {
    Pila p; p.push(1); p.push(2);
    auto antes = p.tam();
    p.push(99);
    p.pop();
    EXPECT_EQ(antes, p.tam());
}
```

Ejemplos típicos de inversa: serializar/deserializar, cifrar/descifrar,
insertar/borrar, `to_string`/`parse`.

### C — Cross-check (comprobación cruzada)

Calcular el mismo resultado por otra vía independiente.

```cpp
TEST(OrdenacionRapida, CoincideConLaDeLaBibliotecaEstandar) {
    std::vector<int> a{5,3,9,1,3}, b = a;
    quicksortPropio(a);
    std::sort(b.begin(), b.end());
    EXPECT_EQ(b, a);
}
```

También vale contra una implementación lenta pero obviamente correcta, o contra
datos históricos conocidos.

### E — Error conditions (condiciones de error)

Lo que pasa cuando **el entorno** falla, no la entrada: disco lleno, red caída,
memoria agotada, permisos, fichero corrupto, timeout. En C++ se prueba
inyectando un doble que falla a propósito.

```cpp
TEST(Exportador, PropagaElErrorSiElDestinoNoEsEscribible) {
    EscritorQueFalla escritor;                     // doble que lanza siempre
    Exportador e{escritor};
    EXPECT_THROW(e.exportar(datos), IOError);
}
```

### P — Performance (rendimiento)

No microbenchmarks en la suite unitaria, pero sí **guardias de orden de
magnitud** que detectan si alguien convierte un O(n) en un O(n²).

```cpp
TEST(Indice, BusquedaEnCienMilElementosEsSubLineal) {
    Indice idx = construir(100'000);
    auto t0 = std::chrono::steady_clock::now();
    for (int i = 0; i < 1000; ++i) idx.buscar(i);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                  std::chrono::steady_clock::now() - t0).count();
    EXPECT_LT(ms, 50);
}
```

### S — Stress / Simultaneidad

Volumen alto, uso prolongado, concurrencia. Suele ir en una suite aparte porque
rompe la "F" de FIRST.

---

# CORRECT — Cómo elegir los datos frontera

CORRECT es el acompañante de la "B" de BICEPS: **siete dimensiones donde buscar
casos límite**.

| Letra | Dimensión        | Qué preguntarse                                 |
|-------|------------------|-------------------------------------------------|
| **C** | *Conformance*    | ¿El dato tiene el formato esperado?             |
| **O** | *Ordering*       | ¿Importa el orden? ¿Está como se espera?        |
| **R** | *Range*          | ¿Está dentro de un rango razonable?             |
| **R** | *Reference*      | ¿Depende de algo externo o de un estado previo? |
| **E** | *Existence*      | ¿Existe? ¿Null, vacío, cero, ausente?           |
| **C** | *Cardinality*    | ¿Cero, uno, muchos? (la regla 0-1-n)            |
| **T** | *Time*           | ¿Orden temporal, concurrencia, fechas raras?    |

### Aplicado a un ejemplo concreto

```cpp
// Parsea "NOMBRE;EMAIL;EDAD" y devuelve el cliente, o nullopt si es inválido
std::optional<Cliente> parsearCliente(const std::string& linea);
```

| Letra | Casos de prueba que genera                                              |
|-------|-------------------------------------------------------------------------|
| **C** | `"Ana;ana@x.com;30"` ✔ · `"Ana,ana@x.com,30"` (separador malo) · `"Ana;correo-no-valido;30"` · `"Ana;a@x.com;treinta"` |
| **O** | Campos intercambiados: `"30;Ana;a@x.com"`                               |
| **R** | Edad `0`, `-1`, `17`, `18`, `150`, `999`, `2147483648` (desbordamiento) |
| **R** | ¿Necesita catálogo de dominios? ¿Depende del locale para los decimales? |
| **E** | `""`, `";;"`, `"Ana;;30"`, `"Ana;a@x.com;"`, línea con solo espacios     |
| **C** | 0 campos, 2 campos, 3 campos (correcto), 4 campos                        |
| **T** | Fecha de alta futura; año bisiesto; cambio de hora; `31/02`             |

Una función de una línea de firma → **más de 20 casos candidatos**. Y ahora es
cuando aplicas lo del bloque 2: clases de equivalencia para quedarte con 8.

### La regla 0-1-n (la "C" de Cardinality)

La que más defectos encuentra por unidad de esfuerzo. Para **cualquier**
colección, prueba siempre:

```cpp
TEST(Estadisticas, MediaDeListaVaciaDevuelveNullopt)  { ... }  // 0
TEST(Estadisticas, MediaDeUnSoloElementoEsEseElemento) { ... } // 1
TEST(Estadisticas, MediaDeVariosElementos)             { ... } // n
```

Y sus primos de la "R" de Range en C++, donde duele de verdad:

```cpp
EXPECT_EQ(..., f(std::numeric_limits<int>::max()));   // desbordamiento
EXPECT_EQ(..., f(std::numeric_limits<int>::min()));
EXPECT_THROW(dividir(1, 0), std::domain_error);       // división por cero
EXPECT_TRUE(std::isnan(g(std::nan(""))));             // NaN, ±inf
```

> **Aviso C++**: `EXPECT_EQ(0.1 + 0.2, 0.3)` **falla**. Para coma flotante usa
> `EXPECT_DOUBLE_EQ` / `EXPECT_FLOAT_EQ` (comparación por ULPs) o
> `EXPECT_NEAR(valor, esperado, tolerancia)`. Lo veremos el día 2.

---

## Cómo se usan los tres juntos

```
1. FIRST     → ¿cómo escribo el test?      (calidad del test)
2. BICEPS    → ¿qué tengo que comprobar?   (cobertura de comprobaciones)
3. CORRECT   → ¿con qué datos lo pruebo?   (selección de casos frontera)
```

En la práctica, ante una función nueva:

1. Escribo el ***happy path*** (el "Right" de Right-BICEP).
2. Recorro **CORRECT** y apunto los candidatos frontera.
3. Los agrupo por **clases de equivalencia** y me quedo con los representativos.
4. Añado **inversa** y **cross-check** si la función las admite.
5. Añado **condiciones de error** por cada dependencia externa.
6. Reviso que todos cumplan **FIRST**.

## Qué ganamos y qué pagamos

**Ganamos:** dejar de mirar la pantalla sin saber qué test escribir. Son
listas de comprobación, y funcionan justamente por eso.

**Pagamos:** aplicados sin criterio producen 40 tests para una función de
cinco líneas. Son un **generador de candidatos**, no una obligación: el filtro
sigue siendo el riesgo real de cada caso.

## Mantra del bloque

> **"FIRST te dice cómo escribirlo, BICEPS qué mirar, CORRECT dónde mirar."**
> Y si solo te llevas una cosa: **0, 1, n y el borde**.

---

## Referencias

**Origen de los tres acrónimos:**

- 📖 Andy Hunt & Dave Thomas, **[*Pragmatic Unit Testing*](https://pragprog.com/titles/utj2/pragmatic-unit-testing-in-java-8-with-junit/)**
  — donde se acuñan **Right-BICEP** y **CORRECT**. Los ejemplos son en Java,
  pero los dos capítulos que nos interesan son independientes del lenguaje.
- 📖 Robert C. Martin, **[*Clean Code*](https://www.informit.com/store/clean-code-a-handbook-of-agile-software-craftsmanship-9780132350884)**,
  capítulo 9 — origen de **FIRST** (son ocho páginas).

**Qué hace bueno a un test:**

- Kent Beck, **[*Test Desiderata*](https://kentbeck.github.io/TestDesiderata/)**
  — doce propiedades deseables de un test. La versión ampliada y matizada de
  FIRST, por el autor de TDD.
- Robert C. Martin, **[*Test Definitions*](https://blog.cleancoder.com/uncle-bob/2017/05/05/TestDefinitions.html)**
  — qué es exactamente un test unitario, de integración y de aceptación.

**Tests no deterministas (la "R" de *Repeatable*):**

- Martin Fowler, **[*Eradicating Non-Determinism in Tests*](https://martinfowler.com/articles/nonDeterminism.html)**
  — catálogo de causas (reloj, concurrencia, recursos, orden) y su remedio.
- Google Testing Blog, **[*Flaky Tests at Google*](https://testing.googleblog.com/2016/05/flaky-tests-at-google-and-how-we.html)**
  — el coste real de un test intermitente, con cifras.

**C++ en concreto:**

- **[GoogleTest — referencia de assertions](https://google.github.io/googletest/reference/assertions.html)**
  — incluye `EXPECT_DOUBLE_EQ`, `EXPECT_NEAR` y las de excepciones del
  apartado CORRECT. La tendremos abierta todo el día de mañana.
- **[Glosario ISTQB — *boundary value analysis*](https://glossary.istqb.org/en_US/term/boundary-value-analysis)**
  — la definición formal de lo que aquí llamamos "la B de BICEPS".
