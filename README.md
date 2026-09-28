# Cluster-en-MPI


Práctica de Computación Paralela y Distribuida: implementación de un clúster MPI con 4 nodos y evaluación de rendimiento en cargas distribuidas.

## Contenido del repositorio

- `ej1.c` — Cálculo de π por integración numérica.
- `ej2.c` — Multiplicación de matrices distribuidas.

## Descripción de los programas

### ej1.c — Cálculo de π

Calcula el valor de π integrando numéricamente la función 4/(1+x²) entre 0 y 1 con la regla del punto medio. Cada proceso se encarga de una parte de los pasos y al final se suman los resultados con `MPI_Reduce`.

El valor de N (cantidad de subdivisiones) se pasa como argumento. Si no se indica nada, se usa un valor por defecto de mil millones (10⁹).

```bash
mpirun --hostfile <hostfile> -np <procesos> ./ej1 [N]
```

### ej2.c — Multiplicación de matrices

Multiplica dos matrices de tamaño N×N repartiendo las filas entre los procesos. Cada rank calcula N/P filas, donde P es el número total de procesos. Por eso N debe ser divisible entre la cantidad de procesos.

```bash
mpirun --hostfile <hostfile> -np <procesos> ./ej2 <N>
```

## Compilación

```bash
mpicc -O2 -o ej1 ej1.c
mpicc -O2 -o ej2 ej2.c
```

## Configuración del clúster

1. **Instalar MPI y SSH en cada nodo:**
   ```bash
   sudo apt update
   sudo apt install -y build-essential openmpi-bin libopenmpi-dev openssh-server
   sudo systemctl enable --now ssh
   ```

2. **Configurar `/etc/hosts`** en cada máquina con las IPs y nombres de los nodos.

3. **Configurar SSH sin contraseña** desde el master:
   ```bash
   ssh-keygen -t ed25519 -C "mpi-master"
   ssh-copy-id usuario@nodo1
   ssh-copy-id usuario@node2
   ssh-copy-id usuario@node3
   ```

4. **Configurar `~/.ssh/config`** en el master con los alias de cada nodo.

5. **Crear el hostfile:**
   ```
   master slots=24
   nodo1 slots=24
   node2 slots=24
   node3 slots=24
   ```

## Ejemplos de ejecución

```bash
# Prueba pequeña: 4 procesos, N=1e7
mpirun -np 4 ./ej1 10000000

# Prueba de estrés: 96 procesos, N por defecto (1e9)
mpirun --hostfile hosts-96 -np 96 ./ej1

# Matrices: 96 procesos, N=4800
mpirun --hostfile hosts-96 -np 96 ./ej2 4800

# Matrices: 48 procesos, N=4800
mpirun --hostfile hosts-48 -np 48 ./ej2 4800
```

## Notas

- En las pruebas de matrices se debe verificar siempre que N sea múltiplo del número total de procesos.
- Los tiempos se miden con `MPI_Wtime()` y se sincronizan con `MPI_Barrier` antes de empezar y antes de terminar.
- El monitoreo del uso de CPU se puede hacer con el monitor de recursos del sistema o con `htop`.
