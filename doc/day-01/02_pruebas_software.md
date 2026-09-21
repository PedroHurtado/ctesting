# Día 1 — Bloque 2: Pruebas de software

> *"Las pruebas pueden demostrar la presencia de errores, nunca su ausencia."*
> — Edsger W. Dijkstra

---

## 1. El problema en una frase

**Probarlo todo es matemáticamente imposible**, así que probar bien no consiste
en probar mucho, sino en **elegir qué probar**.

## 2. Caja negra vs caja blanca

Las dos grandes familias, según qué información usamos para diseñar el caso.

### Caja negra (funcional)

No miro el código. Solo la **especificación**: entradas, salidas, contrato.

```cpp
// Especificación:
//   descuento(importe, esCliente) devuelve el importe con descuento.
//   - Cliente VIP: 20 %
//   - No VIP e importe > 100: 10 %
//   - En otro caso: sin descuento
//   - importe negativo: lanza std::invalid_argument
double descuento(double importe, bool esVip);
```

Casos que se deducen **solo de la especificación**:

| Caso                | importe | esVip | Esperado                  |
|---------------------|---------|-------|---------------------------|
| VIP                 | 50      | true  | 40                        |
| No VIP, > 100       | 200     | false | 180                       |
| No VIP, <= 100      | 80      | false | 80                        |
| Frontera            | 100     | false | 100 (¡no 90!)             |
| Inválido            | -1      | false | `std::invalid_argument`   |

**Ventaja:** el test sobrevive a un cambio de implementación.
**Riesgo:** puedes dejar ramas de código sin ejercitar sin enterarte.

### Caja blanca (estructural)

Miro el código y diseño casos para **recorrer sus caminos**.

```cpp
double descuento(double importe, bool esVip) {
    if (importe < 0) throw std::invalid_argument("importe negativo");  // rama A
    if (esVip)       return importe * 0.80;                            // rama B
    if (importe > 100) return importe * 0.90;                          // rama C
    return importe;                                                     // rama D
}
```

Aquí el objetivo es medible: **cobertura de sentencias**, **de ramas**, **de
condiciones**, **de caminos**. Lo veremos con `gcov`/`gcovr` el día 3.

**Ventaja:** detecta código muerto y ramas olvidadas.
**Riesgo:** el test se acopla a la implementación y solo prueba *lo que está
escrito*, no lo que *debería* estar escrito. Si falta un `if`, la caja blanca
no te lo dice jamás.

### Caja gris

Lo que se hace en la práctica: **diseño en caja negra, reviso con caja blanca**.
Escribo los casos desde la especificación; luego miro la cobertura y, si hay una
rama sin cubrir, me pregunto *"¿qué requisito no he escrito?"* — no *"¿qué test
me falta para subir el porcentaje?"*.

> **Regla que usaremos todo el curso:** la cobertura no es un objetivo, es un
> **detector de huecos**. Nunca escribas un test para subir el número.

## 3. Pruebas de funcionamiento vs pruebas de cualidades

Dos preguntas distintas:

- **¿Hace lo que debe?** → pruebas **funcionales**.
- **¿Lo hace suficientemente bien?** → pruebas de **cualidades** (atributos de
  calidad, *-ilities*, requisitos no funcionales).

| Cualidad (ISO/IEC 25010) | Pregunta                                | Cómo se prueba                            |
|--------------------------|------------------------------------------|-------------------------------------------|
| **Rendimiento**          | ¿Es rápido y eficiente?                  | Benchmarks, pruebas de carga              |
| **Fiabilidad**           | ¿Aguanta? ¿Se recupera?                  | Pruebas de estrés, de fallo inyectado     |
| **Seguridad**            | ¿Resiste un uso malicioso?               | Fuzzing, análisis estático, pentest       |
| **Mantenibilidad**       | ¿Puedo cambiarlo sin romperlo?           | Métricas, revisión, ¡la propia suite!     |
| **Portabilidad**         | ¿Funciona en el otro compilador/SO?      | Matriz de CI                              |
| **Usabilidad**           | ¿Se entiende?                            | Pruebas con usuarios                      |

En C++ hay una categoría extra que no se puede ignorar: **corrección de
memoria** (fugas, *use-after-free*, *data races*). No la cubre una `ASSERT_EQ`;
la cubren herramientas: `valgrind`, `-fsanitize=address,undefined,thread`.

> Un test unitario verde con AddressSanitizer apagado puede estar ocultando un
> `use-after-free`. Día 3 activamos sanitizers en el proyecto.

## 4. La imposibilidad de probarlo todo

### Bad: "vamos a probar todas las combinaciones"

```cpp
int suma(int a, int b);
```

Dos `int` de 32 bits. Combinaciones posibles:

```
2^32 × 2^32 = 2^64 ≈ 1,8 × 10^19 casos
```

A mil millones de casos por segundo → **más de 580 años**. Para una función de
dos líneas.

Y eso solo son las **entradas**. Súmale:

- **Estado previo** del objeto (una clase con 5 flags booleanos = 32 estados).
- **Orden** de las llamadas (n! secuencias).
- **Entorno**: SO, compilador, nivel de optimización, locale, zona horaria.
- **Concurrencia**: entrelazados de hilos, prácticamente infinitos.

**Conclusión:** la pregunta correcta nunca es *"¿lo hemos probado todo?"*, sino
**"¿hemos probado lo que más probabilidad tiene de estar mal?"**.

### Good: reducir el espacio con criterio

Tres herramientas, que desarrollaremos el día 2:

1. **Clases de equivalencia**: si `5`, `7` y `9` recorren exactamente el mismo
   camino, probar los tres no aporta más que probar uno.
2. **Valores frontera**: los errores viven en los bordes.
   Para la condición `importe > 100`, los casos valiosos son `99`, `100`, `101`
   — no `50` ni `5000`.
3. **Tabla de decisión / pares (*pairwise*)**: la mayoría de defectos surge de
   la interacción de **dos** factores, no de diez. Probar todos los **pares**
   reduce cientos de combinaciones a decenas.

Ejemplo de la reducción sobre `descuento`:

```
Espacio teórico:    (todos los double) × (true/false)   → infinito
Clases:             negativo | 0..100 | >100            × VIP/no VIP  → 6 casos
+ Fronteras:        -0.01, 0, 100, 100.01                             → 5 más
Total razonable:    ~8 casos bien elegidos
```

De infinito a ocho. **Eso es diseño de pruebas.**

## 5. La pirámide de pruebas

```
                  /\
                 /  \      E2E / Aceptación     ← pocos, lentos, frágiles
                /----\                            caros de mantener
               /      \    Integración           ← algunos
              /--------\                           (BD, ficheros, red, módulos)
             /          \  Unitarios             ← muchos, rápidos, baratos
            /____________\                         milisegundos, sin E/S
```

| Nivel          | Qué prueba                          | Tiempo típico | Cuándo se ejecuta      |
|----------------|-------------------------------------|---------------|------------------------|
| **Unitario**   | Una clase/función aislada           | < 1 ms        | En cada guardado       |
| **Integración**| Varias piezas juntas, o con E/S real| 10 ms – 1 s   | En cada commit         |
| **Sistema/E2E**| El producto completo                | segundos–min  | En la CI nocturna      |
| **Aceptación** | Que es lo que el cliente pidió      | variable      | Por historia de usuario|

**El antipatrón**: el *cono de helado* (muchos E2E, pocos unitarios). Suite que
tarda horas, falla aleatoriamente y nadie se cree. Cuando un E2E falla, tardas
medio día en saber **dónde**; cuando falla un unitario, lo sabes por el nombre
del test.

**Lo que haremos:** días 2 y 3, la base de la pirámide (Google Test + gMock).
Día 4, la cúspide (BDD/Cucumber).

## 6. De requisito a caso de prueba

Los requisitos y casos de uso **son la materia prima de los tests**. Un requisito
que no se puede convertir en un caso de prueba es un requisito mal escrito.

### Bad: requisito intestable

> *"El sistema debe ser rápido y fácil de usar."*

¿Rápido cuánto? ¿Medido dónde? ¿Con cuántos usuarios? No hay test posible: no
hay criterio de aceptación.

### Good: requisito testable

> *"El cálculo de la nómina de 1.000 empleados debe completarse en menos de
> 2 segundos en el hardware de referencia."*

Esto ya es un test:

```cpp
TEST(Nomina, MilEmpleadosEnMenosDeDosSegundos) {
    auto empleados = generar(1000);
    auto t0 = std::chrono::steady_clock::now();
    calcularNominas(empleados);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                  std::chrono::steady_clock::now() - t0).count();
    EXPECT_LT(ms, 2000);
}
```

### Del caso de uso a los casos de prueba

Un caso de uso trae, gratis, tres familias de tests:

| Parte del caso de uso   | Test que genera                                  |
|-------------------------|--------------------------------------------------|
| **Precondiciones**      | ¿Qué pasa si NO se cumplen? (test de error)      |
| **Flujo principal**     | El *happy path* (1 test)                         |
| **Flujos alternativos** | Un test por cada alternativa                     |
| **Excepciones**         | Un test por cada excepción                       |
| **Postcondiciones**     | Los asserts de cada uno de los anteriores        |

Ejemplo — caso de uso *"Retirar efectivo"*:

```
Precondición: cuenta activa, saldo suficiente    → test: cuenta bloqueada lanza
Flujo principal: retira, saldo decrece           → test: happy path
Alternativo: importe > límite diario             → test: rechaza por límite
Excepción: cajero sin billetes                   → test: propaga error
Postcondición: saldo_final == saldo_inicial - x  → assert de cada test
```

Un caso de uso de cinco líneas → cuatro tests. **Ese es el trabajo del día 2.**

## 7. Qué ganamos y qué pagamos

**Ganamos:** vocabulario común, criterio para elegir casos, y la capacidad de
justificar ante un jefe de proyecto *por qué* 8 tests bien elegidos valen más
que 200 generados a bulto.

**Pagamos:** hay que pensar antes de teclear. El diseño de casos es trabajo
intelectual, no mecánico — y es exactamente lo que una IA aún hace regular.

## 8. Mantra del bloque

> **"No se prueba para demostrar que funciona; se prueba para descubrir
> dónde falla."**
> Un test que nunca ha fallado no ha demostrado nada todavía.

---

## 9. Referencias

**La pirámide y los niveles de prueba:**

- Martin Fowler, **[*Test Pyramid*](https://martinfowler.com/bliki/TestPyramid.html)**
  — el original, media página. Versión larga y con código:
  **[*The Practical Test Pyramid*](https://martinfowler.com/articles/practical-test-pyramid.html)**.
- Google Testing Blog, **[*Just Say No to More End-to-End Tests*](https://testing.googleblog.com/2015/04/just-say-no-to-more-end-to-end-tests.html)**
  — el antipatrón del cono de helado, con datos reales.
- **[*Software Engineering at Google*, cap. 11](https://abseil.io/resources/swe-book/html/ch11.html)**
  — **gratuito**. Cómo clasifican Google los tests por tamaño y alcance.

**Diseño de casos y terminología:**

- **[Glosario ISTQB](https://glossary.istqb.org/)** — la terminología estándar
  del sector. Especialmente
  **[análisis de valores frontera](https://glossary.istqb.org/en_US/term/boundary-value-analysis)**
  y **[partición de equivalencia](https://glossary.istqb.org/en_US/term/equivalence-partitioning)**
  (lo aplicaremos mañana).
- **[pairwise.org](https://www.pairwise.org/)** — pruebas por pares: la técnica
  y las herramientas para reducir combinaciones.

**Cualidades del software:**

- **[ISO/IEC 25010, en español](https://iso25000.com/index.php/normas-iso-25000/iso-25010)**
  — el modelo de calidad completo (las ocho características).

**Clásicos:**

- E. W. Dijkstra, **[*Notes on Structured Programming* (EWD249)](https://www.cs.utexas.edu/~EWD/ewd02xx/EWD249.PDF)**
  — PDF manuscrito. La cita de la cabecera está en la sección 3.
- 📖 Gerard Meszaros, **[*xUnit Test Patterns*](https://www.informit.com/store/xunit-test-patterns-refactoring-test-code-9780131495050)**
  — catálogo de referencia; el **[sitio web es gratuito](http://xunitpatterns.com/)**.
