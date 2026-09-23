# Día 3 — Bloque 16: Sanitizers

> *"En C++, que el test pase no significa que el programa esté bien."*

---

## 1. El problema en una frase

El bloque 15 os dice qué código **no** ejecutan los tests; este bloque va del
código que **sí** ejecutan, que da el resultado correcto, y que aun así está
roto.

## 2. Definición

Un **sanitizer** es instrumentación que el compilador inyecta en el binario para
detectar, **en tiempo de ejecución**, errores que el lenguaje no detecta:
accesos fuera de rango, uso de memoria liberada, desbordamiento con signo,
carreras entre hilos. No son analizadores estáticos: **solo ven el código que
los tests ejecutan**, y por eso cobertura y sanitizers se usan juntos.

| Sanitizer | Bandera | Caza | Coste |
|---|---|---|---|
| **ASan** (*Address*) | `-fsanitize=address` | Fuera de rango, *use-after-free*, doble `delete`, fugas | ~2× tiempo, ~3× memoria |
| **UBSan** (*Undefined Behavior*) | `-fsanitize=undefined` | Desbordamiento con signo, desplazamientos inválidos, `nullptr`, conversiones malas | ~20 % |
| **TSan** (*Thread*) | `-fsanitize=thread` | Carreras de datos, *deadlocks* potenciales | ~10× tiempo |
| **LSan** (*Leak*) | incluido en ASan | Fugas de memoria al salir | — |
| **MSan** (*Memory*) | `-fsanitize=memory` (solo Clang) | Lectura de memoria sin inicializar | ~3× |

> **ASan y UBSan se combinan**: `-fsanitize=address,undefined`. **TSan va
> solo**: es incompatible con ASan.

---

## 3. Bad — el test verde sobre código roto

```cpp
int suma(const std::vector<int>& v) {
    int total = 0;
    for (size_t i = 0; i <= v.size(); ++i)   // ← <= en vez de <
        total += v[i];
    return total;
}
```

```cpp
TEST(Suma, SumaLosTresElementos) {
    std::vector<int> v{1, 2, 3};
    EXPECT_EQ(6, suma(v));          // ← PASA
}
```

Y pasa **de verdad**, ejecutado:

```
6
exit=0
```

El acceso `v[3]` está fuera de rango: es **comportamiento indefinido**. Hoy lee
un byte que casualmente vale 0 y la suma sale bien. Mañana, con otro compilador,
otro nivel de optimización o un vector de otro tamaño, lee basura, o casca en
producción a las tres de la madrugada.

Lo importante: **ni el test ni la cobertura pueden detectar esto**. La línea se
ejecuta, la cobertura da 100 %, el `EXPECT_EQ` se cumple. No hay nada que mirar.

## 4. Good — el mismo test con ASan

Basta recompilar. Ni una línea de test cambia:

```bash
g++ -O0 -g -fsanitize=address -fno-omit-frame-pointer suma.cpp -o suma_test
./suma_test
```

```
=================================================================
==31==ERROR: AddressSanitizer: heap-buffer-overflow on address 0x60200000001c
READ of size 4 at 0x60200000001c thread T0
    #0 0x55613444e42b in suma(std::vector<int> const&) /proyecto/suma.cpp:6
    #1 0x55613444e62d in main /proyecto/suma.cpp:9

0x60200000001c is located 0 bytes to the right of 12-byte region
                          [0x602000000010,0x60200000001c)
allocated by thread T0 here:
    #0 operator new(unsigned long)
    #5 std::vector<int>::vector(std::initializer_list<int>, ...)
    #6 0x55613444e5eb in main /proyecto/suma.cpp:9

SUMMARY: AddressSanitizer: heap-buffer-overflow /proyecto/suma.cpp:6 in suma(...)
```

Cuatro datos en cuatro líneas: **qué** (lectura fuera del bloque), **dónde**
(`suma.cpp:6`), **de qué bloque** (los 12 bytes del vector) y **quién lo
reservó** (la línea 9). El bug de un carácter, señalado con el dedo.

### UBSan, y la trampa que se lleva a todo el mundo por delante

```cpp
int doble(int x) { return x * 2; }
doble(INT_MAX);
```

```
ub.cpp:3:31: runtime error: signed integer overflow: 2147483647 * 2
             cannot be represented in type 'int'
-2
exit=0          ← ⚠️ CERO
```

**UBSan avisa por consola y deja que el programa siga.** El test pasa, la CI se
queda tan contenta y el aviso se pierde entre el ruido del log. Hay que decirle
que pare:

```bash
UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 ./mi_test
```

```
    #0 in doble(int) /proyecto/ub.cpp:3
    #1 in main /proyecto/ub.cpp:4
exit=1          ← ahora sí rompe la CI
```

Lo mismo con ASan y las fugas: `ASAN_OPTIONS=detect_leaks=1`.

> **Si montáis sanitizers en CI sin `halt_on_error=1`, no habéis montado nada.**

---

## 5. El montaje

```cmake
option(ENABLE_SANITIZERS "ASan + UBSan" OFF)

if(ENABLE_SANITIZERS AND NOT MSVC AND NOT MINGW)
    target_compile_options(mi_test PRIVATE
        -fsanitize=address,undefined -fno-omit-frame-pointer -g -O1)
    target_link_options(mi_test PRIVATE -fsanitize=address,undefined)
endif()
```

```bash
cmake -S . -B build-san -DCMAKE_BUILD_TYPE=Debug -DENABLE_SANITIZERS=ON
cmake --build build-san
UBSAN_OPTIONS=halt_on_error=1 ctest --test-dir build-san --output-on-failure
```

**Dónde encaja en el día a día:** no en el ciclo Rojo-Verde-Refactor (los
sanitizers lo hacen lento), sino en una **pasada nocturna de la CI** sobre la
suite completa. Misma disciplina que la cobertura del bloque 15: una compilación
aparte, no la de trabajar.

## 6. ⚠️ En el aula: MinGW no trae sanitizers

Comprobado en la máquina del curso (GCC 14.2 MinGW-w64 UCRT):

```
ld.exe: cannot find -lasan
ld.exe: cannot find -lubsan
ld.exe: cannot find -ltsan
```

No es un fallo de configuración: las bibliotecas de runtime **no se distribuyen**
con MinGW-w64. Las cuatro alternativas reales, con su precio:

| Alternativa | Qué da | Coste |
|---|---|---|
| **WSL / Linux / Docker** | Los tres sanitizers, tal cual salen aquí | Una VM. **La recomendada** |
| **MSVC** `/fsanitize=address` | ASan (VS 2019 16.9+) | Cambiar de compilador |
| **MSYS2 `clang64`** | ASan y UBSan | Otro toolchain que mantener |
| **`-D_GLIBCXX_ASSERTIONS`** | Índices y precondiciones de la STL | Gratis; cubre mucho menos |

La última funciona **hoy, en vuestra máquina**, y habría cazado el `v[3]` del
apartado 3:

```cmake
target_compile_options(mi_lib PRIVATE $<$<CONFIG:Debug>:-D_GLIBCXX_ASSERTIONS>)
```

Es el mínimo decente si no podéis cambiar de toolchain. Los detalles, en el
[documento de instalación](../00_instalacion.md#8-sanitizers-leed-esto-antes-de-intentarlo-en-windows).

---

## 7. Qué ganamos y qué pagamos

**Ganamos:** errores de memoria y de UB localizados con fichero, línea y pila de
llamadas, sin escribir ni un test nuevo; los bugs "que solo pasan en producción"
convertidos en fallos reproducibles; y, con TSan, carreras que de otro modo
aparecen una vez cada mil ejecuciones.

**Pagamos:** binarios lentos y pesados; una compilación más que mantener; falsos
positivos ocasionales en bibliotecas de terceros (que se silencian con un
fichero de supresiones); y **nada en MinGW**.

> **El límite honesto:** un sanitizer solo ve lo que se ejecuta. Su eficacia es
> exactamente la cobertura de vuestra suite. Por eso este bloque va detrás
> del 15 y no delante.

## 8. Conexión con lo que viene

Tenemos tests (12-14) y dos redes que avisan de lo que se nos escapa (15-16).
Con eso, **cambiar el código ya no da miedo**: es la condición que el día 1 puso
para la R de *Refactor*. El **bloque 17** la aprovecha para atacar los *code
smells* del Paint —y para poner la IA a trabajar bajo la única regla que la hace
segura: que los tests manden—.

## 9. Mantra del bloque

> **"El test dice que hace lo correcto; el sanitizer, que lo hace de forma
> legal."**
> En C++ hacen falta los dos.

---

## 10. Referencias

**Documentación oficial (toda gratuita):**

- **[AddressSanitizer — wiki de google/sanitizers](https://github.com/google/sanitizers/wiki/AddressSanitizer)**
  — cómo leer el informe del apartado 4: qué significa *"0 bytes to the right"*,
  los *shadow bytes* y las opciones de `ASAN_OPTIONS`. La referencia práctica.
- **[Clang — AddressSanitizer](https://clang.llvm.org/docs/AddressSanitizer.html)**
  y **[UndefinedBehaviorSanitizer](https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html)**
  — la lista completa de comprobaciones de UBSan (`-fsanitize=signed-integer-overflow`,
  `bounds`, `null`…) y cómo activarlas por separado. Válido también para GCC.
- **[Clang — ThreadSanitizer](https://clang.llvm.org/docs/ThreadSanitizer.html)**
  — si tenéis concurrencia, este es el que más bugs os va a encontrar por euro
  invertido, y el más difícil de interpretar sin leer esto antes.
- **[GCC — opciones de instrumentación](https://gcc.gnu.org/onlinedocs/gcc/Instrumentation-Options.html)**
  — qué soporta **vuestra** build de GCC exactamente. Consultadlo antes de
  pelearos con `cannot find -lasan`.

**Contexto:**

- 📖 Jeff Langr,
  **[*Modern C++ Programming with TDD*](https://pragprog.com/titles/lotdd/modern-c-programming-with-test-driven-development/)**
  — el capítulo sobre código heredado y herramientas: por qué en C++ la suite de
  tests, sola, no basta como red de seguridad.
- **[*Software Engineering at Google*, cap. 11](https://abseil.io/resources/swe-book/html/ch11.html)**
  — **gratuito**. Cómo encajan las pasadas instrumentadas (sanitizers, cobertura)
  en una CI que además tiene que ser rápida: la respuesta es la del apartado 5,
  una compilación aparte.
