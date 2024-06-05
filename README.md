# Practica-6
El código desplegado en el cluster de docker básicamente se trata de un programa destinado a la multiplicación de 2 matrices, pero esta vez resuelto por un sistema distribuido.

## 1.Inclusiones y definiciones

```c

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 1024  // Define el tamaño de la matriz

```

Estas líneas incluyen las bibliotecas necesarias: __mpi.h__ para la programación paralela con MPI, __stdio.h__ para las funciones de entrada y salida, __stdlib.h__ para la gestión de memoria dinámica, y __time.h__ para la generación de semillas aleatorias. También define el tamaño de la matriz N como 1024.

## 2. Función de multiplicación de matrices

```c
void matrix_multiply(int *A, int *B, int *C, int stripe_size) {
   for (int i = 0; i < stripe_size; i++) {
       for (int j = 0; j < N; j++) {
           C[i * N + j] = 0;
           for (int k = 0; k < N; k++) {
               C[i * N + j] += A[i * N + k] * B[k * N + j];
           }
       }
   }
}
```

Esta función realiza la multiplicación de matrices. Toma submatrices A y B, y almacena el resultado en C.  stripe_size indica cuántas filas de la matriz A son procesadas por cada proceso.

- Parámetros
  - int *A: Puntero a la primera matriz (submatriz)
  - int *B: Puntero a la segunda matriz
  - int *C: Puntero a la submatriz donde se almacenará el resultado
  - int stripe_size: Número de filas de la submatriz A que este proceso está manejando

### 2.1 Bucle Externo: Itera sobre las filas de la submatriz A

```c
for (int i = 0; i < stripe_size; i++){}
```

Iterar sobre cada fila de la submatriz A. stripe_size indica cuántas filas de A son procesadas por el proceso actual.




































