# Día 1 — Bloque 1: Metodologías ágiles y XP

> *"La medida principal de progreso es el software funcionando."*
> — Principio 7 del Manifiesto Ágil

---

## 1. El problema en una frase

En un proyecto largo, **el coste de descubrir un error crece con el tiempo que
tarda en descubrirse**; las metodologías clásicas garantizan que se descubra
tarde.

## 2. El problema real: la curva del coste del cambio

El modelo en cascada (requisitos → análisis → diseño → codificación → pruebas →
despliegue) asume algo falso: **que los requisitos se conocen al principio y no
cambian**.

Consecuencia: las pruebas quedan al final, cuando ya no queda ni tiempo ni
presupuesto. Y un fallo de requisito detectado en la fase de pruebas cuesta
órdenes de magnitud más que detectado el primer día.

```
Coste de corregir un defecto
   ^
   |                                        ####  (en producción)
   |                                 ####
   |                        ####            (en pruebas de sistema)
   |               ####                     (en integración)
   |       ####                             (en codificación)
   |  ####                                  (en requisitos)
   +------------------------------------------>  momento de detección
```

La respuesta ágil no es *"hacer las fases más rápido"*: es **aplanar esa curva**
acortando drásticamente el tiempo entre que introduzco un error y que me entero.
Un test automático que falla 20 segundos después de escribir el bug es la
versión extrema de esa idea. **TDD es, antes que nada, un mecanismo de
realimentación rápida.**

## 3. El Manifiesto Ágil (2001)

Cuatro valores. A la izquierda lo que más se valora, a la derecha lo que **sí
tiene valor pero menos**. No es "no documentar", es "no confundir la
documentación con el producto".

| Valoramos más...                        | ...que                                   |
|-----------------------------------------|------------------------------------------|
| Individuos e interacciones              | Procesos y herramientas                  |
| Software funcionando                    | Documentación exhaustiva                 |
| Colaboración con el cliente             | Negociación contractual                  |
| Respuesta ante el cambio                | Seguir un plan                           |

De los 12 principios, los que nos afectan directamente hoy:

- **Entrega temprana y continua** de software con valor.
- **Aceptar el cambio de requisitos**, incluso tarde.
- **Software funcionando** como medida de progreso.
- **Atención continua a la excelencia técnica** y al buen diseño.
- **Simplicidad**: maximizar el trabajo *no* hecho.

> Nota: *ágil* no es Scrum. Scrum es un marco de **gestión** (roles, eventos,
> artefactos) y no dice nada sobre cómo escribir código. XP sí. Por eso este
> curso mira a XP y no a Scrum.

## 4. XP (Extreme Programming): las prácticas que nos importan

Kent Beck, 1996-1999. La idea de XP es **coger las buenas prácticas conocidas y
llevarlas al extremo**:

- Si revisar el código es bueno → revísalo **todo el rato** (*pair programming*).
- Si probar es bueno → prueba **continuamente**, y que lo hagan las máquinas
  (*tests automáticos*).
- Si integrar es bueno → integra **varias veces al día** (*integración continua*).
- Si el diseño es bueno → **rediséñalo constantemente** (*refactorización*).
- Si las iteraciones cortas son buenas → hazlas **muy cortas**.

### Las prácticas clave

| Práctica                      | En qué consiste                                                        | Por qué importa para TDD                                  |
|-------------------------------|------------------------------------------------------------------------|-----------------------------------------------------------|
| **Test-First / TDD**          | El test se escribe antes que el código.                                 | Es el núcleo del curso.                                    |
| **Refactorización continua**  | Mejorar el diseño sin cambiar el comportamiento.                        | **Solo es segura si hay tests**. Son inseparables.         |
| **Integración continua**      | Integrar y ejecutar toda la suite varias veces al día.                  | Sin suite rápida, la CI no existe.                         |
| **Diseño simple**             | La solución más simple que funcione hoy.                                | Menos código = menos tests = menos coste.                  |
| **Pair programming**          | Dos personas, un teclado.                                               | Revisión continua; uno escribe el test, otro el código.    |
| **Propiedad colectiva**       | Cualquiera puede tocar cualquier parte.                                 | Solo es viable con red de seguridad de tests.              |
| **Cliente en el equipo**      | Alguien con autoridad para decidir qué es correcto.                     | Es quien define los criterios de aceptación (día 4).       |
| **Pequeñas entregas**         | Versiones útiles cada pocas semanas.                                    | Obliga a tener el sistema siempre verde.                   |
| **Ritmo sostenible**          | Nada de heroicidades crónicas.                                          | El cansancio es la primera causa de defectos.              |
| **Estándar de codificación**  | Un solo estilo.                                                         | Propiedad colectiva sin guerras de formato.                |
| **Metáfora**                  | Un vocabulario común del sistema.                                       | Los nombres de los tests son documentación.                |

### Lo importante: las prácticas se sostienen entre sí

Esto es lo que casi todo el mundo se salta al "adoptar TDD":

```
   Refactorizo con confianza
            ↑ requiere
      Tests automáticos  ←── requiere ──  Diseño simple y desacoplado
            ↑ requiere                              ↑ requiere
   Integración continua  ────── requiere ──── Refactorizo con confianza
```

Es un círculo. **Si quitas una pieza, las demás se caen.** Un equipo que escribe
tests pero nunca refactoriza acaba con una suite que le estorba. Un equipo que
refactoriza sin tests acaba rompiendo producción. Un equipo con tests que tardan
40 minutos acaba no ejecutándolos.

## 5. Bad: "hacemos ágil"

```
- Sprints de 2 semanas.          ✔ (gestión)
- Daily de 15 minutos.           ✔ (gestión)
- Tablero Kanban precioso.       ✔ (gestión)
- Tests: "cuando haya tiempo".   ✘
- Refactor: "eso es rehacer, no se factura".  ✘
- Integración: rama larga, merge el último día.  ✘
```

Resultado: **la velocidad cae sprint a sprint**. Se le llama *deuda técnica*,
pero es más honesto llamarlo lo que es: cada cambio es más caro que el anterior
porque nadie se atreve a tocar nada.

## 6. Good: agilidad técnica

```
- Cada funcionalidad entra acompañada de sus tests.
- Se refactoriza en verde, en pasos pequeños, todos los días.
- Merge a la rama principal al menos una vez al día.
- La suite completa tarda segundos; nadie duda en ejecutarla.
- "Terminado" significa: implementado + testeado + integrado.
```

## 7. Qué ganamos y qué pagamos

**Ganamos:**
- Detección de defectos en minutos, no en meses.
- Capacidad real de cambiar de opinión sin miedo.
- Un diseño que mejora con el tiempo en vez de degradarse.
- Documentación viva: los tests nunca mienten (o fallan).

**Pagamos:**
- Más código escrito (la suite de tests **es** código que hay que mantener).
- Una curva de aprendizaje real: escribir buenos tests es una habilidad.
- Disciplina diaria. TDD no es un curso, es un hábito.

## 8. Errores habituales al adoptarlo

1. **Escribir los tests después**: técnicamente válido, pero pierdes el
   principal beneficio (el test como presión de diseño).
2. **Testear lo trivial**: `getters` y `setters` que nadie va a romper.
3. **Testear a través de la interfaz de usuario todo**: lento y frágil.
4. **Convertir la suite en un lastre**: tests acoplados a la implementación que
   fallan en cada refactor, cuando el comportamiento no cambió.
5. **Medir cobertura como objetivo**: se optimiza el número, no la calidad.

## 9. Mantra del bloque

> **"Si duele, hazlo más a menudo."**
> Integrar duele → integra cada hora. Probar duele → prueba cada minuto.
> Refactorizar duele → refactoriza siempre. El dolor señala dónde falta
> automatización, no dónde hay que rendirse.

---

## 10. Referencias

**Fuentes primarias** (10 minutos de lectura, merecen la pena):

- **[Manifiesto Ágil](https://agilemanifesto.org/iso/es/manifesto.html)** y sus
  **[12 principios](https://agilemanifesto.org/iso/es/principles.html)** — en
  español, y no llegan a dos páginas entre los dos.
- **[Las reglas de XP](http://www.extremeprogramming.org/rules.html)** — todas
  las prácticas de la tabla del apartado 4, en una sola página.
- **[Ron Jeffries — *What is Extreme Programming?*](https://ronjeffries.com/xprog/what-is-extreme-programming/)**
  — escrito por uno de los tres creadores de XP.

**Para profundizar:**

- 📖 Kent Beck, **[*Extreme Programming Explained*, 2ª ed.](https://www.informit.com/store/extreme-programming-explained-embrace-change-9780321278654)**
  — el libro de XP. Los capítulos de prácticas son lo aplicable hoy mismo.
- 📖 James Shore, **[*The Art of Agile Development*, 2ª ed.](https://www.jamesshore.com/v2/books/aoad2)**
  — **gratuito en la web**. La versión práctica y actualizada de XP.
- Martin Fowler, **[*Is High Quality Software Worth the Cost?*](https://martinfowler.com/articles/is-quality-worth-cost.html)**
  — la curva del apartado 2, argumentada para convencer a un jefe de proyecto.
- Industrial Logic, **[*TDD and the Lump of Coding Fallacy*](https://www.industriallogic.com/blog/tdd-and-the-lump-of-coding-fallacy/)**
  — respuesta al "los tests nos ralentizan".
