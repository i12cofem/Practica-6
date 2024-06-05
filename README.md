# Practica 6: Arquitecturas paralelas
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

[![image.png](https://i.postimg.cc/sxH9vx11/image.png)](https://postimg.cc/D4sbN2rK)


```c
MPI_Scatter(
    void* send_data,
    int send_count,
    MPI_Datatype send_datatype,
    void* recv_data,
    int recv_count,
    MPI_Datatype recv_datatype,
    int root,
    MPI_Comm communicator)
```

El primer parámetro, __send_data__ es una matriz de datos que reside en el proceso raíz. El segundo y tercer parámetro, __send_count y send_datatype__, dictan cuántos elementos de un tipo de datos MPI específico se enviarán a cada proceso. Si __send_count__ es uno y __send_datatype__ es __MPI_INT__, entonces el proceso cero obtiene el primer número entero de la matriz, el proceso uno obtiene el segundo número entero, y así sucesivamente. Si __send_count__ es dos, entonces el proceso cero obtiene el primer y segundo número entero, el proceso uno obtiene el tercero y el cuarto, y así sucesivamente. En la práctica, __send_count__ suele ser igual al número de elementos de la matriz dividido por el número de procesos.

Los parámetros de recepción del prototipo de función son casi idénticos con respecto a los parámetros de envío. El __recv_data__ parámetro es un búfer de datos que puede contener __recv_count__ elementos que tienen un tipo de datos de __recv_datatype__. Los últimos parámetros, root y communicator, indican el proceso raíz que está dispersando la matriz de datos y el comunicador en el que residen los procesos.

```c
MPI_Scatter(A, stripe_size * N, MPI_INT, subA, stripe_size * N, MPI_INT, 0, MPI_COMM_WORLD);
```

## 5. Función Bcast Explicación

Un broadcast es una de las técnicas de comunicación colectiva estándar. Durante un broadcast, un proceso envía los mismos datos a todos los procesos en un comunicador. Uno de los usos principales de la transmisión es enviar entradas del usuario a un programa paralelo o enviar parámetros de configuración a todos los procesos.

El patrón de comunicación de una transmisión se ve así:

[![image.png](https://i.postimg.cc/bwFtfWWH/image.png)](https://postimg.cc/gx8JvSYn)

```c
MPI_Bcast(
    void* data,
    int count,
    MPI_Datatype datatype,
    int root,
    MPI_Comm communicator)
```

Aunque el proceso raíz y el proceso receptor realizan trabajos diferentes, todos llaman a la misma __MPI_Bcast__ función. Cuando el proceso raíz (en nuestro ejemplo, era el proceso cero) llama __MPI_Bcast__, la vairable __data__ se enviará a todos los demás procesos. Cuando todos los procesos receptores llamen __MPI_Bcast__, la variable __data__ se completará con los datos del proceso raíz.

```c
MPI_Bcast(B, N * N, MPI_INT, 0, MPI_COMM_WORLD);
```

## 6. LLamada de la función de multiplicación

```c
matrix_multiply(subA, B, subC, stripe_size);
```
Cada proceso ejecuta la función __matrix_multiply__ en su __submatriz subA y la matriz completa B__ para calcular una submatriz de resultado __subC__.

## 7. Función Gather.

__MPI_Gather__ es la inversa de __MPI_Scatter__. En lugar de distribuir elementos de un proceso a muchos procesos, __MPI_Gather__ toma elementos de muchos procesos y los reúne en un solo proceso. Esta rutina es muy útil para muchos algoritmos paralelos, como la clasificación y búsqueda paralelas. A continuación se muestra una ilustración sencilla de este algoritmo.

[![image.png](https://i.postimg.cc/sDFYxMjQ/image.png)](https://postimg.cc/BtgLwQ14)

Similar a __MPI_Scatter__, __MPI_Gather__ toma elementos de cada proceso y los reúne en el proceso raíz. Los elementos están ordenados por el rango del proceso del que fueron recibidos. El prototipo de función para __MPI_Gather__ es idéntico al de __MPI_Scatter__.

```c
MPI_Gather(
    void* send_data,
    int send_count,
    MPI_Datatype send_datatype,
    void* recv_data,
    int recv_count,
    MPI_Datatype recv_datatype,
    int root,
    MPI_Comm communicator)
```

En __MPI_Gather__, solo el proceso raíz necesita tener un búfer de recepción válido. Todos los demás procesos de llamada pueden pasar __NULL__ por __recv_data__. Además, no olvide que el parámetro __recv_count__ es el recuento de elementos recibidos por proceso , no la suma total de recuentos de todos los procesos. Esto a menudo puede confundir a los programadores MPI principiantes.

```c
MPI_Gather(subC, stripe_size * N, MPI_INT, C, stripe_size * N, MPI_INT, 0, MPI_COMM_WORLD);
```

## Ejemplo Visual

[![image.png](https://i.postimg.cc/280g75Rj/image.png)](https://postimg.cc/MvjsWqpg)

## Tiempos de ejecución

[![image.png](https://i.postimg.cc/1zwGp69N/image.png)](https://postimg.cc/CZMn0RnF)


























