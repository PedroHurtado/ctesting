# Día 2 — Bloque 9: Qué probar (equivalencia, frontera, MITs y MIMs)

> *"Ochocientos tests no son garantía de nada si los ochocientos prueban lo
> mismo."*

---

## 1. El problema en una frase

El día 1 demostramos que probarlo todo es imposible; hoy toca la pregunta
práctica: de todos los casos posibles, **¿cuáles son los que valen?**

## 2. Definición: las tres técnicas

| Técnica | Idea | Qué reduce |
|---|---|---|
| **Clases de equivalencia** | Entradas que recorren el **mismo camino** son intercambiables: basta una | El número de casos por variable |
| **Valores frontera** | Los errores viven en los **bordes** de cada clase | Dónde se coloca ese caso |
| **Pruebas por pares** (*pairwise*) | La mayoría de defectos surge de la interacción de **dos** factores | La explosión combinatoria entre variables |

Las tres son **análisis de caja negra**: se aplican sobre la especificación,
antes de mirar el código. Eso es lo que las hace compatibles con TDD, donde el
código todavía no existe.

## 3. Bad: tests elegidos por intuición

Especificación de la tarifa de envío:

```
peso <= 0        → error
0 < peso <= 5    → 3,00 EUR
5 < peso <= 20   → 7,50 EUR
peso > 20        → 12,00 EUR
```

```cpp
TEST(Envio, Tarifas) {
    EXPECT_DOUBLE_EQ(3.00, tarifa(1.0));
    EXPECT_DOUBLE_EQ(3.00, tarifa(2.0));
    EXPECT_DOUBLE_EQ(3.00, tarifa(3.0));
    EXPECT_DOUBLE_EQ(3.00, tarifa(4.0));     // ← cuatro tests, UN camino
    EXPECT_DOUBLE_EQ(7.50, tarifa(10.0));
    EXPECT_DOUBLE_EQ(7.50, tarifa(15.0));
    EXPECT_DOUBLE_EQ(12.00, tarifa(50.0));
    EXPECT_DOUBLE_EQ(12.00, tarifa(1000.0));
}
```

Ocho assertions, sensación de trabajo bien hecho, y **ni un solo caso en una
frontera**. Este código pasa los ocho:

```cpp
double tarifa(double peso) {
    if (peso < 5.0)  return 3.00;       // ← ¡< en vez de <=! El bug está aquí
    if (peso < 20.0) return 7.50;       // ← y aquí
    return 12.00;
}
```

Y el caso `peso = 0` ni se ha probado. Ese `if` desplazado en uno —el error
*off-by-one*— es, con diferencia, el defecto más común de la industria; y es
precisamente el que estos ocho tests **no pueden ver**.

## 4. Good: equivalencia + frontera

### Paso 1 — Particionar en clases

| Clase | Rango | Válida | Representante |
|---|---|---|---|
| C1 | `peso <= 0` | ✗ inválida | `-1` |
| C2 | `0 < peso <= 5` | ✓ | `2.5` |
| C3 | `5 < peso <= 20` | ✓ | `12` |
| C4 | `peso > 20` | ✓ | `50` |

Regla: **un representante por clase válida, y uno por cada clase inválida por
separado** (si metes dos entradas inválidas en el mismo test, la primera
validación puede ocultar la segunda).

### Paso 2 — Añadir los bordes

Por cada frontera *b*, tres candidatos: **justo debajo, justo encima, y el
valor exacto**. El valor exacto es el que caza el `<` frente al `<=`.

```
      0            5                    20
──────┼────────────┼─────────────────────┼──────────►
    0⁻ 0 0⁺     5⁻ 5 5⁺              20⁻ 20 20⁺
```

### Paso 3 — La suite resultante

```cpp
struct Caso { double peso; double esperado; const char* porque; };

// Clases de equivalencia (un representante cada una)
TEST(Tarifa, PesoNoPositivoEsRechazado)        { EXPECT_THROW(tarifa(-1.0), PesoInvalido); }
TEST(Tarifa, HastaCincoKilosCuestaTresEuros)   { EXPECT_DOUBLE_EQ(3.00,  tarifa(2.5)); }
TEST(Tarifa, DeCincoAVeinteCuestaSieteCincuenta){ EXPECT_DOUBLE_EQ(7.50, tarifa(12.0)); }
TEST(Tarifa, MasDeVeinteCuestaDoceEuros)       { EXPECT_DOUBLE_EQ(12.00, tarifa(50.0)); }

// Fronteras (aquí es donde aparecen los bugs)
TEST(Tarifa, CeroKilosEsRechazado)             { EXPECT_THROW(tarifa(0.0), PesoInvalido); }
TEST(Tarifa, ElMinimoPesoValidoEntraEnElPrimerTramo) { EXPECT_DOUBLE_EQ(3.00, tarifa(0.01)); }
TEST(Tarifa, CincoKilosExactosSiguenEnElPrimerTramo) { EXPECT_DOUBLE_EQ(3.00, tarifa(5.0)); }
TEST(Tarifa, PasadosLosCincoSaltaAlSegundoTramo)     { EXPECT_DOUBLE_EQ(7.50, tarifa(5.01)); }
TEST(Tarifa, VeinteKilosExactosSiguenEnElSegundoTramo){ EXPECT_DOUBLE_EQ(7.50, tarifa(20.0)); }
TEST(Tarifa, PasadosLosVeinteSaltaAlTercerTramo)     { EXPECT_DOUBLE_EQ(12.00, tarifa(20.01)); }
```

Diez tests en vez de ocho. Pero los dos de `5.0` y `20.0` **matan el bug del
apartado 3**, y `0.0` cubre el caso que faltaba. Diferencia de coste:
despreciable. Diferencia de valor: total.

> Mañana veremos los **tests paramétricos** (`TEST_P`): exactamente esta tabla,
> escrita una vez y ejecutada con N juegos de datos.

## 5. Cuando hay varias variables: por pares

```
descuento(importe, cliente, cupon, canal)
  importe: 3 tramos · cliente: 3 tipos · cupon: 4 valores · canal: 3
  → 3 × 3 × 4 × 3 = 108 combinaciones
```

108 tests no se escriben, y si se escriben no se mantienen. La observación
empírica, medida repetidamente desde los años 90: **la gran mayoría de los
defectos se dispara por un solo factor o por la interacción de dos**, no de
cuatro. Cubrir todos los **pares** de valores baja de 108 a unos 12 casos.

En la práctica se hace con una herramienta (las tenéis en
[pairwise.org](https://www.pairwise.org/)); lo importante hoy es el criterio:

> Antes de escribir el test número 40 de la misma función, pregúntate si lo que
> te falta es un **par** que nadie ha cubierto o una **repetición** de una clase
> que ya tienes.

## 6. Las fuentes de casos que no se te ocurren solo

Tres listas del día 1, aplicadas ahora con el código delante:

- **CORRECT** — *Conformance, Ordering, Range, Reference, Existence, Cardinality,
  Time*. Recorre las siete letras sobre cada parámetro: `Existence` te recuerda
  el `nullptr` y el `optional` vacío; `Cardinality` el 0-1-N; `Range` los
  bordes del apartado 4; `Time` el orden de llamadas y los *timeouts*.
- **Right-BICEPS** — el resultado correcto, los bordes, las relaciones
  inversas, las comprobaciones cruzadas, las condiciones de error y el
  rendimiento.
- **Del caso de uso al test** (bloque 2 del día 1): precondiciones → tests de
  error; flujo principal → *happy path*; alternativos y excepciones → un test
  cada uno.

## 7. MITs: los tests que sí o sí

No todos los tests valen lo mismo, y el tiempo de una iteración es finito. Los
**MITs** (*Most Important Tests*) son la respuesta a "si solo pudiera escribir
cinco, ¿cuáles?".

Se priorizan con tres preguntas, no con una:

| Criterio | Pregunta | Sube la prioridad cuando… |
|---|---|---|
| **Impacto** | ¿Qué pasa si falla en producción? | Hay dinero, datos o seguridad de por medio |
| **Probabilidad** | ¿Qué probabilidad tiene de estar mal? | Código complejo, recién tocado o con historial de bugs |
| **Frecuencia** | ¿Cuánto se ejecuta? | Está en el camino que usan todos los clientes |

```
Prioridad ≈ Impacto × Probabilidad × Frecuencia
```

Aplicado a la `Cuenta` del bloque 10:

| Caso | Impacto | Prob. | Frec. | ¿MIT? |
|---|---|---|---|---|
| Retirar más del saldo se rechaza | Alto (dinero) | Media | Alta | **Sí** |
| Retirada por encima del límite diario | Alto | **Alta** (lógica con fechas) | Media | **Sí** |
| Importe negativo se rechaza | Alto | Media | Baja | **Sí** |
| Cuenta nueva tiene saldo 0 | Bajo | Baja | — | No (trivial) |
| `to_string()` formatea con 2 decimales | Bajo | Baja | Alta | No |

Dos avisos, porque MIT se malinterpreta con facilidad:

- **MIT no es "los únicos tests".** Es el orden en que se escriben cuando el
  tiempo aprieta. En TDD los tests triviales salen gratis: son el primer ciclo.
- **La lista caduca.** Un módulo que falló en producción sube de probabilidad
  inmediatamente. Revisad los MITs después de cada incidencia.

## 8. MIMs: medir sin engañarse

Los **MIMs** (*Most Important Metrics*) son las pocas métricas que de verdad
dicen algo sobre la salud de la suite.

| Métrica | Qué indica | Señal de alarma |
|---|---|---|
| **Defectos escapados a producción** | La medida real de la eficacia | Sube dos trimestres seguidos |
| **Tiempo de la suite unitaria** | Si el ciclo TDD sigue vivo | > 10 s ⇒ la gente deja de lanzarla |
| **Tests inestables** (*flaky*) | La confianza en el rojo | > 1 % ⇒ el rojo empieza a ignorarse |
| **Cobertura de líneas/ramas** | Qué **no** está probado | Baja de golpe; o se convierte en objetivo |
| **MTTR** (tiempo de reparación) | Lo rápido que se diagnostica | Sube ⇒ los tests no localizan el fallo |
| **Edad del rojo en CI** | Disciplina del equipo | Horas ⇒ la rama principal está rota |

### La métrica que más daño hace mal usada

```
"Objetivo de empresa: 90 % de cobertura."
```

```cpp
// Consecuencia directa, vista en proyectos reales:
TEST(Informe, Cobertura) {
    Informe informe;
    informe.generar();          // ← ejecuta 400 líneas
    SUCCEED();                  // ← y no comprueba NADA
}
```

Cobertura: excelente. Valor: cero. Es la **ley de Goodhart**: *cuando una medida
se convierte en objetivo, deja de ser una buena medida*.

La lectura correcta de la cobertura es **al revés**:

> La cobertura **alta no demuestra nada**. La cobertura **baja sí demuestra
> algo**: ese código no está probado. Úsala para encontrar agujeros, nunca como
> nota del examen.

El día 3 la medimos de verdad con `gcov`/`gcovr`, incluida la cobertura de
ramas, que es la que detecta el `if` a medias del apartado 3.

## 9. Roles: quién prueba qué

| Rol | De qué responde | Qué **no** es suyo |
|---|---|---|
| **Desarrollador** | Unitarios y de integración de su código. La base de la pirámide. | Delegar en QA lo que puede probar él |
| **QA / tester** | Diseño de casos difíciles, exploratorias, sistema, no funcionales | Ser el filtro que tapa la falta de unitarios |
| **PO / cliente** | Criterios de aceptación: **qué** es correcto | Decidir **cómo** se prueba |
| **Arquitecto / técnico de referencia** | Testabilidad del diseño, estrategia y entornos | Escribir él todos los tests |
| **Equipo entero** | Que la rama principal esté verde | "El build roto es de quien lo rompió" |

Las dos ideas de fondo, que son culturales antes que técnicas:

- **La calidad no es un departamento.** Un QA que "revisa al final" es un cuello
  de botella y una coartada. En XP (día 1) el tester se integra en el equipo y
  trabaja **antes**, diseñando casos, no después.
- **Quien escribe el código escribe sus tests.** No por dogma: porque es el que
  tiene el contexto, y en TDD el test es anterior al código. Lo que QA aporta
  encima es precisamente lo que el desarrollador no ve: el caso raro, la
  combinación perversa, la exploratoria.

Esto enlaza directo con el día 4: los criterios de aceptación escritos por el
PO en lenguaje de negocio son los escenarios Gherkin de ATDD/BDD.

## 10. Qué ganamos y qué pagamos

**Ganamos:** suites pequeñas que encuentran bugs de verdad, un criterio
defendible ante un jefe de proyecto (*por qué* 10 tests valen más que 200), y
métricas que informan en lugar de decorar.

**Pagamos:** análisis antes de teclear. Es trabajo intelectual, no mecánico —y
es exactamente lo que una IA todavía hace regular, porque requiere conocer el
dominio y el riesgo del negocio.

## 11. Mantra del bloque

> **"Los bugs no viven en el centro de los rangos; viven en los bordes."**
> Una clase de equivalencia te dice cuántos tests necesitas. Una frontera te
> dice cuáles.

---

## 12. Referencias

**Técnicas de diseño de casos (gratuito):**

- **[ISTQB — Partición de equivalencia](https://glossary.istqb.org/en_US/term/equivalence-partitioning)**
  y **[Análisis de valores frontera](https://glossary.istqb.org/en_US/term/boundary-value-analysis)**
  — las definiciones estándar del sector, las que usa cualquier QA con
  certificación. El **[glosario completo](https://glossary.istqb.org/)** es la
  referencia para hablar el mismo idioma con el departamento de calidad.
- **[pairwise.org](https://www.pairwise.org/)** — la técnica del apartado 5, los
  datos empíricos que la sostienen y la lista de herramientas que generan las
  combinaciones por ti.
- **[Wikipedia — All-pairs testing](https://en.wikipedia.org/wiki/All-pairs_testing)**
  — resumen corto con los estudios del NIST sobre cuántos defectos dependen de
  uno, dos o más factores. Útil para justificarlo ante un jefe.

**Métricas, y cómo no destrozarlas:**

- Martin Fowler, **[*Test Coverage*](https://martinfowler.com/bliki/TestCoverage.html)**
  — **gratuito**, tres minutos. Por qué la cobertura sirve para encontrar código
  sin probar y para nada más. El apartado 8 es esto.
- **[Ley de Goodhart](https://en.wikipedia.org/wiki/Goodhart%27s_law)**
  — el principio general: toda métrica convertida en objetivo se corrompe.
  Vale para cobertura, para *story points* y para todo lo demás.
- Martin Fowler, **[*Cannot Measure Productivity*](https://martinfowler.com/bliki/CannotMeasureProductivity.html)**
  — **gratuito**. Lectura de acompañamiento para el jefe de proyecto que pide
  "una métrica que mida al equipo".

**Roles y organización:**

- **[*Software Engineering at Google*, cap. 11](https://abseil.io/resources/swe-book/html/ch11.html)**
  — **gratuito**. La sección sobre por qué Google disolvió el rol de "tester
  que prueba al final" y qué puso en su lugar. Es el apartado 9 con datos.
- **[ISTQB Certified Tester Foundation Level](https://www.istqb.org/certifications/certified-tester-foundation-level/)**
  — el temario público describe los roles y responsabilidades formales; útil si
  trabajáis con un departamento de QA certificado.
- 📖 Hunt & Thomas, **[*Pragmatic Unit Testing*](https://pragprog.com/titles/utj2/pragmatic-unit-testing-in-java-8-with-junit/)**
  — el origen de **Right-BICEPS** y **CORRECT** del apartado 6, con el catálogo
  completo de preguntas para generar casos.
