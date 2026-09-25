# Día 5 — Bloque 21: Refactorización de código con IA, paso a paso

> *"Una afirmación técnica que no has ejecutado es una hipótesis."*
> — la regla del curso (bloque 17)

---

## 1. El problema en una frase

Un asistente de IA refactoriza en diez segundos lo que a nosotros nos cuesta
diez minutos... y a veces cambia el comportamiento con total seguridad y sin
avisar.

## 2. Lo que ya sabemos del bloque 17

El protocolo de seis pasos:

```
 1. Tests en verde   2. Cobertura de lo que toco   3. Pedir un refactor pequeño y con nombre
 4. Leer el diff     5. Ejecutar los tests          6. Commit pequeño
```

Hoy lo aplicamos a un caso completo. Y vamos a descubrir que **los pasos 1 y 2
no bastan**: hace falta un tipo de test más.

---

## 3. El código heredado

Puntos de fidelidad de una tienda. Llega así, sin tests:

```cpp
int puntosFidelidad(int importeCentimos, bool esSocio, int diaSemana, bool usaApp) {
    int puntos = 0;
    if (importeCentimos > 0) {
        if (esSocio) {
            puntos = importeCentimos * 3 / 100;      // 3 puntos por euro
            if (diaSemana == 2) {                    // martes: doble
                puntos = puntos * 2;
            }
        } else {
            puntos = importeCentimos / 100;          // 1 punto por euro
        }
        if (usaApp) {
            puntos = puntos + 10;
        }
    }
    return puntos;
}
```

Los olores, con lo que ya sabéis: **condicional anidado** (bloque 17),
**números mágicos** (`2`, `3`, `10`), **primitivos** (`int` para dinero,
`int` para el día).

---

## 4. Paso 1 y 2 — la red: tests y cobertura

Un test por regla, con AAA y nombres claros:

```cpp
TEST(Puntos, SinCompraNoHayPuntos)   { EXPECT_EQ(0,   puntosFidelidad(0,     false, 1, false)); }
TEST(Puntos, NoSocioUnPuntoPorEuro)  { EXPECT_EQ(100, puntosFidelidad(10000, false, 1, false)); }
TEST(Puntos, SocioTresPuntosPorEuro) { EXPECT_EQ(300, puntosFidelidad(10000, true,  1, false)); }
TEST(Puntos, SocioElMartesDobla)     { EXPECT_EQ(600, puntosFidelidad(10000, true,  2, false)); }
TEST(Puntos, LaAppSumaDiezPuntos)    { EXPECT_EQ(110, puntosFidelidad(10000, false, 1, true));  }
```

Cinco tests en verde. Parece una buena red.

---

## 5. Paso 3 — pedirlo bien

### Bad

```
Mejora este código.
```

Resultado: la IA decide qué es "mejor". Cambia nombres, tipos, firma, añade
`constexpr` y un `enum`... un diff de 60 líneas imposible de revisar.

### Good

```
Refactoriza esta función C++17 aplicando SOLO:
1. Replace Nested Conditional with Guard Clauses
2. Decompose Conditional

Restricciones:
- No cambies la firma ni el comportamiento para NINGUNA entrada.
- No añadas dependencias ni cambies tipos.
- Devuelve solo la función.
```

Qué tiene de bueno:

| Parte del prompt | Por qué |
|---|---|
| **Nombres del catálogo** | La IA conoce el catálogo de Fowler. Un nombre = un cambio acotado |
| **"Para NINGUNA entrada"** | Le recuerda que el comportamiento es sagrado (aunque no lo garantiza) |
| **Restricciones de firma y tipos** | El diff se queda pequeño y revisable |

---

## 6. La propuesta

Esta es una propuesta **típica** de un asistente. La hemos escrito para
reproducir un fallo que aparece a menudo en refactors de aritmética entera:

```cpp
int puntosFidelidad(int importeCentimos, bool esSocio, int diaSemana, bool usaApp) {
    if (importeCentimos <= 0) return 0;

    const int euros = importeCentimos / 100;          // ← "extraer variable explicativa"
    int puntos = esSocio ? euros * 3 : euros;
    if (esSocio && diaSemana == 2) puntos *= 2;
    if (usaApp) puntos += 10;
    return puntos;
}
```

Se lee mucho mejor. La explicación que la acompaña suele ser convincente:
*"he extraído `euros` para eliminar la duplicación de `/ 100`; el
comportamiento es idéntico"*.

## 7. Paso 4 y 5 — ¿la aceptamos?

Pasamos los tests (salida real):

```
[       OK ] Puntos.SinCompraNoHayPuntos (0 ms)
[       OK ] Puntos.NoSocioUnPuntoPorEuro (0 ms)
[       OK ] Puntos.SocioTresPuntosPorEuro (0 ms)
[       OK ] Puntos.SocioElMartesDobla (0 ms)
[       OK ] Puntos.LaAppSumaDiezPuntos (0 ms)
```

Y la cobertura de la función nueva (salida real de `gcovr`):

```
File                                    Branches    Taken  Cover   Missing
------------------------------------------------------------------------------
ia.cpp                                        10       10   100%
------------------------------------------------------------------------------
File                                       Lines     Exec  Cover   Missing
ia.cpp                                         7        7   100%
```

**Todo verde. 100 % de líneas. 100 % de ramas.** Y la función **tiene un
bug**.

---

## 8. El test que falta: equivalencia con el original

Mientras dura el refactor, **el código viejo es el oráculo**. Se deja una
copia (`puntosFidelidad_legado`) y se comparan las dos con muchas entradas,
**sobre todo las que no son redondas**:

```cpp
TEST(Puntos, EquivalenteAlLegado) {
    const int importes[] = {-100, 0, 1, 33, 99, 100, 150, 199, 1999, 10000};
    for (int importe : importes)
        for (bool socio : {false, true})
            for (int dia = 1; dia <= 7; ++dia)
                for (bool app : {false, true}) {
                    SCOPED_TRACE(::testing::Message() << "importe=" << importe << " socio=" << socio
                                                      << " dia=" << dia << " app=" << app);
                    EXPECT_EQ(puntosFidelidad_legado(importe, socio, dia, app),
                              puntosFidelidad(importe, socio, dia, app));
                }
}
```

Salida real:

```
test_puntos.cpp:27: Failure
Expected equality of these values:
  esperado
    Which is: 2
  obtenido
    Which is: 0
Google Test trace:
test_puntos.cpp:23: importe=99 socio=true dia=1 app=false
```

Un socio que gasta **0,99 €**: el original le da **2 puntos**, la versión nueva
**0**.

### Por qué

```
 original:  99 * 3 / 100  =  297 / 100  =  2     (multiplica y luego divide)
 IA:        99 / 100 * 3  =    0 * 3    =  0     (divide y luego multiplica)
```

En enteros, **el orden importa**: dividir primero trunca antes de tiempo.
Con los importes de los tests (`10000`, múltiplos de 100) las dos versiones
dan lo mismo. Por eso los cinco tests pasaban.

> **La cobertura mide qué líneas se ejecutan, no con qué valores.** El 100 %
> del §7 era verdad y no servía de nada. Ya lo dijimos en el bloque 15; aquí
> está la prueba.

---

## 9. La versión buena

Se mantiene la forma que proponía la IA, pero con el redondeo del original:

```cpp
constexpr int kMartes = 2;

int puntosBase(int importeCentimos, bool esSocio) {
    const int puntosPorEuro = esSocio ? 3 : 1;
    return importeCentimos * puntosPorEuro / 100;    // ← multiplicar ANTES de dividir
}

int puntosFidelidad(int importeCentimos, bool esSocio, int diaSemana, bool usaApp) {
    if (importeCentimos <= 0) return 0;

    int puntos = puntosBase(importeCentimos, esSocio);
    if (esSocio && diaSemana == kMartes) puntos *= 2;
    if (usaApp) puntos += 10;
    return puntos;
}
```

```
[==========] 6 tests from 1 test suite ran. (0 ms total)
[  PASSED  ] 6 tests.
```

Y después:

1. Se añade un test normal con el caso que lo destapó
   (`puntosFidelidad(99, true, 1, false) == 2`). Queda para siempre.
2. Se borran `puntosFidelidad_legado` y el test de equivalencia. Solo servían
   durante el refactor.

---

## 10. El protocolo, completo

| Paso | Qué | Nuevo hoy |
|---|---|---|
| 1 | Tests en verde | |
| 2 | Cobertura de lo que vais a tocar | |
| 3 | Prompt con **nombres del catálogo** y restricciones | ✔ |
| 4 | Leer el **diff**, no la explicación | |
| 5 | Tests + **test de equivalencia** con el legado y valores no redondos | ✔ |
| 6 | Test de regresión para lo que se encontró; borrar el legado; commit | ✔ |

## 11. Qué pedirle a la IA y qué no

| Bien | Con cuidado | No |
|---|---|---|
| Aplicar un refactor con nombre | Generar valores **esperados** de un test | Decidir **qué** refactorizar |
| Proponer nombres | Cambios en aritmética o en tipos | Aceptar un diff grande "porque pasan los tests" |
| Detectar olores y duplicación | Cualquier cosa con punteros o vida de objetos | Sustituir la revisión del diff |
| Escribir la rejilla del test de equivalencia | | |
| Explicar código heredado | | |

> **Los valores esperados salen de ejecutar, no de la IA.** Si le pedís "los
> tests de esta función", los números que escriba en `EXPECT_EQ` son lo que
> **cree** que devuelve el código. Para un test de caracterización, el valor
> esperado se obtiene **ejecutando el código viejo**.

## 12. Dónde se equivoca más en C++ (lo que hay que mirar en el diff)

| Mira... | Porque la IA puede... |
|---|---|
| Divisiones y módulos enteros | Cambiar el orden de las operaciones (este bloque) |
| `<` frente a `<=` | Invertir una frontera al "simplificar" |
| `signed` / `unsigned`, `size_t` | Introducir conversiones que dan la vuelta con negativos |
| `std::string_view`, referencias devueltas | Devolver algo que apunta a un temporal |
| Bucles que modifican un contenedor | Invalidar iteradores |
| Dos operaciones que antes eran una | Romper la atomicidad (bloque 24 lo enseña con un escenario) |

---

## 13. Qué ganamos y qué pagamos

**Ganamos:** la parte mecánica del refactor, en segundos; ideas de nombres y
de estructura; y, con el test de equivalencia, una red mucho más fuerte que la
de los tests "de toda la vida".

**Pagamos:** tiempo de revisión (siempre); una copia temporal del código
viejo; y el riesgo de fiarse de una explicación bien escrita.

## 14. Mantra del bloque

> **"La IA propone, los tests deciden. Y el código viejo es el oráculo hasta
> que lo borras."**

---

## 15. Referencias

- **[Martin Fowler — *Exploring Gen AI*](https://martinfowler.com/articles/exploring-gen-ai.html)**
  — **gratuito**. Experimentos reales de Thoughtworks con asistentes en
  refactorización; lo que salió bien y lo que no.
- **[Approval Tests (Llewellyn Falco)](https://approvaltests.com/)**
  — **gratuito**. La versión industrial del test de equivalencia del §8, con
  librería para C++ (`ApprovalTests.cpp`).
- **[Wikipedia — *Characterization test*](https://en.wikipedia.org/wiki/Characterization_test)**
  — **gratuito**. De dónde sale la idea de que el valor esperado se obtiene
  ejecutando el código, no razonando.
- 📖 Michael Feathers,
  **[*Working Effectively with Legacy Code*](https://www.informit.com/store/working-effectively-with-legacy-code-9780131177055)**
  — cap. 13. Tests de caracterización para código que no entiendes. Con IA o
  sin ella, es el mismo problema.
- **[SEI CERT C++ — INT rules](https://wiki.sei.cmu.edu/confluence/pages/viewpage.action?pageId=88046333)**
  — **gratuito**. Las reglas de aritmética entera de §12, con ejemplos de fallo.
