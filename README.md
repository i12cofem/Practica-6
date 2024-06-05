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
  - __int *A__: Puntero a la primera matriz (submatriz)
  - __int *B__: Puntero a la segunda matriz
  - __int *C__: Puntero a la submatriz donde se almacenará el resultado
  - __int stripe_size__: Número de filas de la submatriz A que este proceso está manejando

### 2.1 Bucle Externo: Itera sobre las filas de la submatriz A

```c
for (int i = 0; i < stripe_size; i++){}
```

Iterar sobre cada fila de la submatriz A. stripe_size indica cuántas filas de A son procesadas por el proceso actual.

### 2.2 Bucle Intermedio: Iterar sobre columnas de la matriz B

```c
for (int j = 0; j < N; j++){}
```
Iterar sobre cada columna de la matriz B. N es el tamaño de las matrices (en este caso, N = 1024).

### 2.3 Inicialización del elemento de la matriz resultante C

```c
C[i * N + j] = 0;
```

Inicializar el elemento __C[i * N + j]__ a 0 antes de comenzar la acumulación. Aquí, __C[i * N + j]__ es el elemento en la fila i y columna j de la matriz resultante C.

### 2.4 Bucle interno: Producto escalar de filas y columnas

```c
for (int k = 0; k < N; k++) {
               C[i * N + j] += A[i * N + k] * B[k * N + j];
}
```

Realizar el producto escalar entre la fila i de la submatriz A y la columna j de la matriz B para calcular el elemento __C[i * N + j]__.

- __A[i * N + k]__: Elemento de la fila i y columna k de la submatriz A.
- __B[k * N + j]__: Elemento de la fila k y columna j de la matriz B.

La multiplicación __A[i * N + k] * B[k * N + j]__ se suma a __C[i * N + j]__ para acumular el producto escalar.

__Ejemplo de Cálculo__

A = |1 2 3|
    |4 5 6|
    |7 8 9|

B = |9 8 7|
    |6 5 4|
    |3 2 1|

Para calcular __C[0]__ (Elemento de la fila 0 y columna 0 de C), tendríamos que hacer __C[0] = 1*9 + 2*6 + 3*3 = 9 + 12 + 9 = 30__. Este proceso se repite para cada elemento de C.

Esto podemos visualizarlo de la siguiente manera:

Supongamos que __N = 1024__ y tenemos __size = 4 procesos__. La __matriz A de tamaño 1024 x 1024__ se divide en 4 submatrices, cada una de tamaño __256 x 1024__. Aquí, __256 es el valor de stripe_size__ (número de filas que cada proceso va a manejar).

A = [ fila_0  ]
    [ fila_0  ]
    [......   ]
    [fila_1023]

Entonces la división de las matrices se realizará de la siguiente manera

Proceso 0: submatriz_A_0 = [ fila_0  ]
                           [ fila_1  ]
                           [......   ]
                           [fila_255 ]

Proceso 1: submatriz_A_1 = [ fila_256  ]
                           [ fila_257  ]
                           [......     ]
                           [fila_511   ]

Proceso 2: submatriz_A_2 = [ fila_512  ]
                           [ fila_513  ]
                           [......     ]
                           [fila_767   ]

Proceso 3: submatriz_A_3 = [ fila_768  ]
                           [ fila_769  ]
                           [......     ]
                           [fila_1023  ]


## 3. Función principal

### 3.1 Inicialización de MPI

```c
MPI_Init(&argc, &argv);
```

Inicializa el entorno MPI.

### 3.2 Obtener el rango y tamaño del comunicador

```c
MPI_Comm_rank(MPI_COMM_WORLD, &rank);
MPI_Comm_size(MPI_COMM_WORLD, &size);
```
__rank__ es el identificador del proceso y __size__ es el número total de procesos.

### 3.3 Inicialización de matrices y asignación de memoria

```c
if (rank == 0) {
       A = (int *)malloc(N * N * sizeof(int));
       B = (int *)malloc(N * N * sizeof(int));
       C = (int *)malloc(N * N * sizeof(int));
       // Initialize matrices A and B
       for (int i = 0; i < N * N; i++) {
           A[i] = rand() % 100;
           B[i] = rand() % 100;
       }
}
```

Condición __if (rank == 0)__: Esta condición verifica si el proceso actual es el __maestro (rank == 0)__. El proceso maestro es responsable de inicializar las __matrices A, B y C__.

### 3.4 Asignación de memoria

```c
A = (int *)malloc(N * N * sizeof(int));
B = (int *)malloc(N * N * sizeof(int));
C = (int *)malloc(N * N * sizeof(int));
```

### 3.5 Asignación de memoria para B

```c
else {
       B = (int *)malloc(N * N * sizeof(int));  // Ensure B is allocated in all processes
}
```

Aunque los procesos trabajadores no inicializan A ni C, necesitan una copia de B para realizar la multiplicación de matrices. __malloc(N * N * sizeof(int))__ asigna memoria para B en cada proceso trabajador.

### 3.6 Asignación de memoria para submatrices

```c
subA = (int *)malloc(stripe_size * N * sizeof(int));
subC = (int *)malloc(stripe_size * N * sizeof(int));
```
Entonces las submatrices subA y subC

- __subA y subC__ son punteros a enteros que almacenarán las submatrices de A y C respectivamente.
- Cada proceso (incluido el maestro) necesita solo una parte de A y C, de tamaño __stripe_size x N__.
- __stripe_size__ es el número de filas de A que cada proceso maneja, calculado como __N / size__.

## 4. Función Scatter Explicación

__PI_Scatter__ Implica un proceso raíz designado que envía datos a todos los procesos en un comunicador. La principal diferencia entre __MPI_Bcast y MPI_Scatter__ es pequeña pero importante. __MPI_Bcast__ envía el mismo dato a todos los procesos mientras __MPI_Scatter__ envía  __fragmentos__ de una matriz a diferentes procesos. Consulte la ilustración a continuación para obtener más aclaraciones.





























