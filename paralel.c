/* WSCAD - 9th Marathon of Parallel Programming
 * Simple Brute Force Algorithm for the
 * Traveling-Salesman Problem
 * Author: Emilio Francesquini - francesquini@ic.unicamp.br
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <mpi.h>

// defines
#define ROOT 0

// Estrutura para armazenar informações sobre distâncias entre cidades
typedef struct
{
    int to_town;
    int dist;
} d_info;

int min_distance; // Armazena a menor distância encontrada
int nb_towns;     // Número total de cidades
int rank, size;   // variáveis MPI

d_info **d_matrix;   // Matriz de distâncias entre as cidades
int *dist_to_origin; // Distâncias da cidade inicial para as demais

// Verifica se uma cidade já está presente em um caminho
int present(int town, int depth, int *path)
{
    int i;
    for (i = 0; i < depth; i++)
        if (path[i] == town)
            return 1;
    return 0;
}

// Algoritmo principal de solução do Problema do Caixeiro Viajante (TSP)
void tsp(int depth, int current_length, int *path)
{
    int i;
    // Poda: Ignora caminhos cujo comprimento total já é maior do que a menor distância encontrada até agora
    if (current_length >= min_distance)
        return;

    if (depth == nb_towns)
    {
        // Se o caminho estiver completo, atualiza a menor distância encontrada
        current_length += dist_to_origin[path[nb_towns - 1]];
        if (current_length < min_distance)
            min_distance = current_length;
    }
    else
    {
        // Continua construindo o caminho
        int town, me, dist;
        me = path[depth - 1];
        for (i = 0; i < nb_towns; i++)
        {
            town = d_matrix[me][i].to_town;
            if (!present(town, depth, path))
            {
                path[depth] = town;
                dist = d_matrix[me][i].dist;
                tsp(depth + 1, current_length + dist, path);
            }
        }
    }
}

// Heurística: Inicializa a matriz de distâncias usando o método do vizinho mais próximo
void greedy_shortest_first_heuristic(int *x, int *y)
{
    int i, j, k, dist;
    int *tempdist;

    tempdist = (int *)malloc(sizeof(int) * nb_towns);

    for (i = 0; i < nb_towns; i++)
    {
        for (j = 0; j < nb_towns; j++)
        {
            int dx = x[i] - x[j];
            int dy = y[i] - y[j];
            tempdist[j] = dx * dx + dy * dy;
        }

        for (j = 0; j < nb_towns; j++)
        {
            int tmp = INT_MAX;
            int town = 0;
            for (k = 0; k < nb_towns; k++)
            {
                if (tempdist[k] < tmp)
                {
                    tmp = tempdist[k];
                    town = k;
                }
            }
            tempdist[town] = INT_MAX;
            d_matrix[i][j].to_town = town;
            dist = (int)sqrt(tmp);
            d_matrix[i][j].dist = dist;
            if (i == 0)
                dist_to_origin[town] = dist;
        }
    }

    free(tempdist);
}

// Inicialização do problema TSP
void init_tsp()
{
    int i, st;

    min_distance = INT_MAX;

    if (rank == ROOT)
    {
        st = scanf("%u", &nb_towns);
        if (st != 1)
            exit(1);
    }

    printf("process[%d]: debug 1\n", rank);
    MPI_Bcast(&nb_towns, 1, MPI_INT, ROOT, MPI_COMM_WORLD);
    printf("process[%d]: debug 2\n", rank);

    // Aloca espaço para a matriz de distâncias e o vetor de distâncias da cidade inicial
    d_matrix = (d_info **)malloc(sizeof(d_info *) * nb_towns);
    for (i = 0; i < nb_towns; i++)
        d_matrix[i] = (d_info *)malloc(sizeof(d_info) * nb_towns);
    dist_to_origin = (int *)malloc(sizeof(int) * nb_towns);

    int *x = NULL;
    int *y = NULL;

    if (rank == ROOT)
    {

        x = (int *)malloc(sizeof(int) * nb_towns);
        y = (int *)malloc(sizeof(int) * nb_towns);

        for (i = 0; i < nb_towns; i++)
        {
            st = scanf("%u %u", x + i, y + i);
            if (st != 2)
                exit(1);
        }
    }

    // Distribui os arrays x e y para todos os processos
    MPI_Bcast(x, nb_towns, MPI_INT, ROOT, MPI_COMM_WORLD);
    MPI_Bcast(y, nb_towns, MPI_INT, ROOT, MPI_COMM_WORLD);

    // Aplica a heurística do vizinho mais próximo para inicializar a matriz de distâncias
    greedy_shortest_first_heuristic(x, y);

    if (rank == ROOT)
    {
        free(x);
        free(y);
    }
}

// Executa o problema TSP e retorna a menor distância encontrada
int run_tsp()
{
    int i, *path;

    init_tsp();

    // Aloca espaço para o caminho
    path = (int *)malloc(sizeof(int) * nb_towns);
    path[0] = 0;

    for (i = rank; i < nb_towns; i += size)
    {
        path[1] = i; // Cada processo começa de uma cidade diferente
        // Executa o algoritmo TSP
        tsp(2, dist_to_origin[i], path);
    }

    // Coleta os resultados de cada processo
    int *result_buffer = (int *)malloc(sizeof(int) * size);
    MPI_Gather(&min_distance, 1, MPI_INT, result_buffer, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Processo 0 encontra o resultado final
    if (rank == 0)
    {
        min_distance = INT_MAX;
        for (i = 0; i < size; i++)
        {
            if (result_buffer[i] < min_distance)
            {
                min_distance = result_buffer[i];
            }
        }
    }

    // Libera a memória alocada
    free(path);
    free(result_buffer);
    for (i = 0; i < nb_towns; i++)
        free(d_matrix[i]);
    free(d_matrix);

    // Retorna a menor distância encontrada
    return min_distance;
}

int main(int argc, char **argv)
{

    // nenhuma chamada a funções MPI antes deste ponto
    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int num_instances, st;
    if (rank == ROOT)
    {

        st = scanf("%u", &num_instances);
        if (st != 1)
            exit(1);
    }

    printf("process[%d]: Before Bcast, num_instances is %d\n", rank, num_instances);

    /* everyone calls bcast, data is taken from root and ends up in everyone's num_instances */
    MPI_Bcast(&num_instances, 1, MPI_INT, ROOT, MPI_COMM_WORLD);

    printf("process[%d]: After Bcast, num_instances is %d\n", rank, num_instances);

    // Executa o problema TSP para o número de instâncias fornecido
    while (num_instances-- > 0)
        printf("%d\n", run_tsp());

    MPI_Finalize();
    // nenhuma chamada a funções MPI depois deste ponto

    return 0;
}
