# Día 5 — Bloque 23: Gherkin, escenarios en un lenguaje que entiende todo el mundo

> *"Si el cliente no puede leer el test, el test no sirve como especificación."*

---

## 1. El problema en una frase

Los criterios de aceptación en un Word se quedan viejos el primer día; los
tests en C++ no los lee nadie de negocio. Hace falta algo que lean **los dos**:
las personas y la máquina.

## 2. Definición

**Gherkin** es un lenguaje casi natural para escribir escenarios. Solo tiene
unas pocas **palabras clave**; el resto de cada línea es texto libre.

Un fichero Gherkin se llama *feature* y tiene extensión `.feature`. Admite más
de 70 idiomas, entre ellos el español: basta con la primera línea
`# language: es`.

### Las palabras clave

| Español | Inglés | Para qué | Equivale en AAA |
|---|---|---|---|
| `Característica:` | `Feature:` | La historia de usuario | — |
| `Regla:` | `Rule:` | Una regla de negocio (tarjeta azul) | — |
| `Escenario:` | `Scenario:` | Un ejemplo concreto (tarjeta verde) | Un `TEST` |
| `Dado` / `Dada` / `Dados` / `Dadas` | `Given` | El estado de partida | *Arrange* |
| `Cuando` | `When` | La acción | *Act* |
| `Entonces` | `Then` | El resultado esperado | *Assert* |
| `Y` / `Pero` | `And` / `But` | Otro paso del mismo tipo que el anterior | — |
| `Antecedentes:` | `Background:` | Pasos `Dado` comunes a todos los escenarios | Un fixture |
| `Esquema del escenario:` + `Ejemplos:` | `Scenario Outline:` + `Examples:` | El mismo escenario con varios datos | Un `TEST_P` |

---

## 3. Un fichero completo

Este fichero se ejecuta de verdad en el bloque 24:

```gherkin
# language: es
Característica: Retirar efectivo
  Como titular de una cuenta
  Quiero retirar efectivo de mi cuenta
  Para disponer de mi dinero sin pasar por la oficina

  Regla: No se puede retirar más de lo que hay

    Escenario: Retirada con saldo suficiente
      Dado una cuenta con un saldo de 100 euros
      Cuando retiro 30 euros
      Entonces el saldo es de 70 euros

    Escenario: Se puede retirar todo el saldo
      Dado una cuenta con un saldo de 100 euros
      Cuando retiro 100 euros
      Entonces el saldo es de 0 euros

    Escenario: Retirada sin saldo suficiente
      Dado una cuenta con un saldo de 100 euros
      Cuando retiro 101 euros
      Entonces la operación se rechaza por "saldo insuficiente"
      Y el saldo es de 100 euros

  Regla: Solo se pueden retirar importes positivos

    Esquema del escenario: Importe no válido
      Dado una cuenta con un saldo de 100 euros
      Cuando retiro <importe> euros
      Entonces la operación se rechaza por "importe no valido"
      Y el saldo es de 100 euros

      Ejemplos:
        | importe |
        | 0       |
        | -50     |
```

Fijaos en que **ya conocéis todo lo que hay aquí**:

- La historia (Como / Quiero / Para) es la del bloque 22.
- Los escenarios son los criterios CA-06 a CA-10 del ejercicio del día 4.
- 100, 101 y 0 son las **fronteras** del bloque 9.
- El `Esquema` con su tabla es un `TEST_P` (bloque 14).
- El `Y el saldo es de 100 euros` tras el rechazo es la **postcondición** que
  pedía la kata del día 2.

---

## 4. Bad — el escenario imperativo

El error más común, con diferencia:

```gherkin
Escenario: Retirar dinero
  Dado que abro el navegador en "https://banco.test/login"
  Y escribo "ana@correo.es" en el campo "email"
  Y escribo "1234" en el campo "password"
  Y pulso el botón "Entrar"
  Y pulso en el menú "Cuentas"
  Y selecciono la cuenta "ES12 3456"
  Cuando escribo "30" en el campo "importe"
  Y pulso el botón "Retirar"
  Entonces veo el texto "70,00 €" en el elemento "#saldo"
```

Qué duele:

- **Habla de la pantalla, no del negocio.** Si mañana el botón se llama
  "Sacar", el escenario se rompe aunque la regla no haya cambiado.
- **Nueve pasos para una regla que cabe en tres.** Negocio no lo lee.
- **Datos que no importan** (el email, la contraseña, el número de cuenta).
  ¿Importa que sea Ana? ¿Importa la cuenta? No se sabe.
- **No se puede ejecutar sin interfaz gráfica.** Es lento y frágil (FIRST, día 1).

## 5. Good — el escenario declarativo

```gherkin
Escenario: Retirada con saldo suficiente
  Dado una cuenta con un saldo de 100 euros
  Cuando retiro 30 euros
  Entonces el saldo es de 70 euros
```

Dice **qué** pasa, no **cómo** se hace clic. El "cómo" va en el código de los
pasos (bloque 24): hoy llama a la clase `Cuenta`; mañana podría llamar a una
API REST o a la interfaz gráfica, **sin tocar el `.feature`**.

> **Prueba rápida:** ¿el escenario seguiría siendo verdad si el banco fuera
> una ventanilla con un empleado y papel? Si sí, es declarativo.

---

## 6. Otros olores de los escenarios

| Olor | Ejemplo | Arreglo |
|---|---|---|
| **Varios `Cuando`** | `Cuando ingreso 50 ... Y retiro 30 ... Y transfiero...` | Un escenario = **una** acción. Parte en varios |
| **`Entonces` que mira las tripas** | `Entonces la tabla MOVIMIENTOS tiene 1 fila` | Mirar lo que ve el usuario: `Entonces hay 1 movimiento` |
| **Datos incidentales** | `Dado el cliente "Ana García, DNI 1234X, nacida en 1980"` | Solo los datos que **cambian el resultado** |
| **Antecedentes gigantes** | 15 líneas de `Antecedentes` | Si no se ven de un vistazo, el escenario se entiende mal. Resumid en un paso |
| **Escenarios que dependen de otro** | El 2º usa la cuenta creada en el 1º | Cada escenario empieza de cero (la **I** de FIRST) |
| **Título que no dice nada** | `Escenario: Prueba 3` | El título es la **regla** o el **ejemplo**: `Retirada sin saldo suficiente` |

---

## 7. Antecedentes y tablas de datos

Cuando todos los escenarios empiezan igual, el `Dado` común va en
`Antecedentes`. Y cuando un paso necesita varios datos, se le pasa una tabla:

```gherkin
# language: es
Característica: Transferencias entre cuentas
  Como titular de una cuenta
  Quiero transferir dinero a otra cuenta
  Para pagar a otras personas sin usar efectivo

  Antecedentes:
    Dadas las siguientes cuentas:
      | titular | saldo |
      | Ana     | 100   |
      | Luis    | 0     |

  Escenario: Transferencia con saldo suficiente
    Cuando Ana transfiere 40 euros a Luis
    Entonces Ana tiene 60 euros
    Y Luis tiene 40 euros

  Escenario: Una transferencia sin fondos no toca ninguna cuenta
    Cuando Ana transfiere 500 euros a Luis
    Entonces la transferencia se rechaza por "saldo insuficiente"
    Y Ana tiene 100 euros
    Y Luis tiene 0 euros
```

| | `Esquema del escenario` + `Ejemplos` | Tabla de datos en un paso |
|---|---|---|
| Qué hace | Ejecuta el escenario **una vez por fila** | Pasa **toda** la tabla a **un** paso |
| Equivale a | `TEST_P` | Un `std::vector` de filas como parámetro |
| Úsala para | Fronteras, clases de equivalencia | Preparar varios objetos a la vez |

## 8. Etiquetas

Una línea con `@algo` encima de un escenario o una característica:

```gherkin
@lento @cajero
Escenario: Retirada en un cajero de otra entidad
```

Sirven para **elegir qué se ejecuta**: `cucumber --tags "not @lento"`. Es el
`--gtest_filter` del bloque 6.

---

## 9. Lista de comprobación: BRIEF

Seis preguntas antes de dar un escenario por bueno (Seb Rose y Gáspár Nagy):

| Letra | Significa | Pregunta |
|---|---|---|
| **B** | *Business language* | ¿Usa palabras de negocio, no de programación? |
| **R** | *Real data* | ¿Usa datos concretos (100 €), no abstractos ("una cantidad")? |
| **I** | *Intention revealing* | ¿Dice **qué** se quiere, no **cómo** se hace? |
| **E** | *Essential* | ¿Sobra algún paso o dato? |
| **F** | *Focused* | ¿Prueba **una sola** regla? |
| **Brief** | Breve | ¿Cabe en unas 5 líneas? |

---

## 10. Qué ganamos y qué pagamos

**Ganamos:** un documento que lee negocio **y** ejecuta la máquina; un
vocabulario común (el del dominio) entre todos los perfiles; y la
especificación siempre al día, porque si se queda vieja, falla.

**Pagamos:** una capa más entre el requisito y el código (los pasos del
bloque 24); disciplina para mantener los escenarios declarativos; y la
tentación de usar Gherkin para **todo**, incluso para lo que un `TEST` de
GoogleTest dice mejor.

> **Regla práctica:** Gherkin para lo que tiene que leer negocio.
> GoogleTest para todo lo demás. La mayoría de los tests siguen siendo
> unitarios (la **pirámide** del día 1).

## 11. Mantra del bloque

> **"Escribe el escenario para que lo lea tu cliente, no tu compilador."**

---

## 12. Referencias

- **[Referencia de Gherkin (cucumber.io)](https://cucumber.io/docs/gherkin/reference/)**
  — **gratuito**. Todas las palabras clave, en una página.
- **[Palabras clave de Gherkin en español (gherkin-languages.json)](https://github.com/cucumber/gherkin/blob/main/gherkin-languages.json)**
  — **gratuito**. Busca `"es"`: la lista oficial de `Dado`, `Dada`, `Dados`...
- **[Cucumber — *Writing better Gherkin*](https://cucumber.io/docs/bdd/better-gherkin/)**
  — **gratuito**. Declarativo frente a imperativo (§4 y §5), con más ejemplos.
- **[Seb Rose — *Keep your scenarios BRIEF*](https://cucumber.io/blog/bdd/keep-your-scenarios-brief/)**
  — **gratuito**. La lista de §9 explicada por uno de sus autores.
- 📖 Seb Rose y Gáspár Nagy,
  **[*Formulation* (The BDD Books)](https://leanpub.com/bddbooks-formulation)**
  — el libro dedicado entero a escribir buenos escenarios.
- 📖 Matt Wynne y Aslak Hellesøy,
  **[*The Cucumber Book* (2ª ed.)](https://pragprog.com/titles/hwcuc2/the-cucumber-book-second-edition/)**
  — el libro de referencia de Cucumber, escrito por sus creadores.
