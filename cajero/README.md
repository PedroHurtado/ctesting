# cajero — el proyecto del día 5

La `Cuenta` del curso con el tipo `Dinero` (bloque 19), probada en los dos
bucles del bloque 24:

- **Bucle interno:** tests unitarios con GoogleTest (`tests/`).
- **Bucle externo:** escenarios Gherkin ejecutados con cucumber-cpp (`features/`).

```
cajero/
├── CMakeLists.txt
├── aceptacion.sh                      ← compila y ejecuta Cucumber (Linux/WSL)
├── src/
│   ├── dinero.h                       ← bloque 19
│   └── cuenta.h                       ← Cuenta, retirarEnCajeroAjeno, transferir
├── tests/                             ← GoogleTest
│   ├── dinero_test.cpp
│   └── cuenta_test.cpp
└── features/                          ← Gherkin (bloque 23)
    ├── retirar_efectivo.feature
    ├── comision_cajero_ajeno.feature
    ├── transferencias.feature
    ├── support/wire.rb
    └── step_definitions/              ← pasos en C++ (bloque 24)
        ├── cucumber.wire
        ├── cuenta_steps.cpp
        └── transferencia_steps.cpp
```

## Tests unitarios (Windows o Linux)

Solo necesitan GoogleTest. Si no encuentra cucumber-cpp, CMake lo avisa y
compila solo esta parte.

```bash
cmake -S . -B build -G Ninja
cmake --build build
ctest --test-dir build --output-on-failure
```

## Tests de aceptación (Linux / WSL)

Necesitan cucumber-cpp instalado (bloque 24, §3). Con el prefijo del aula
(`~/cuke/env`):

```bash
./aceptacion.sh                        # salida completa
./aceptacion.sh --format progress      # un punto por paso
./aceptacion.sh features/transferencias.feature
```

Si está instalado en otro sitio: `PREFIJO=/ruta ./aceptacion.sh`.

Resultado esperado:

```
9 scenarios (9 passed)
34 steps (34 passed)
```
