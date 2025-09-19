#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <pthread.h>

double* load_vector(const char* filename, int* out_size);
void avaliar(double* a, double* b, int size, double prod_escalar);

typedef struct {
    double* a;
    double* b;
    int start;
    int end;
    double partial_sum;
} ThreadData;

void* dot_product_worker(void* arg) {
    ThreadData* data = (ThreadData*) arg;
    double sum = 0.0;
    for (int i = data->start; i < data->end; i++) {
        sum += data->a[i] * data->b[i];
    }
    data->partial_sum = sum;  // guarda no struct
    return NULL;
}

int main(int argc, char* argv[]) {
    srand(time(NULL));

    if(argc < 4) {
        printf("Uso: %s n_threads a_file b_file\n", argv[0]);
        return 1;
    }
  
    int n_threads = atoi(argv[1]);
    if (n_threads <= 0) {
        printf("Número de threads deve ser > 0\n");
        return 1;
    }

    int a_size = 0, b_size = 0;
    double* a = load_vector(argv[2], &a_size);
    if (!a) {
        printf("Erro ao ler arquivo %s\n", argv[2]);
        return 1;
    }
    double* b = load_vector(argv[3], &b_size);
    if (!b) {
        printf("Erro ao ler arquivo %s\n", argv[3]);
        return 1;
    }
    
    if (a_size != b_size) {
        printf("Vetores a e b tem tamanhos diferentes! (%d != %d)\n", a_size, b_size);
        return 1;
    }

    pthread_t threads[n_threads];
    ThreadData td[n_threads];

    int chunk = a_size / n_threads;
    int rest = a_size % n_threads;

    int start = 0;
    for (int t = 0; t < n_threads; t++) {
        int end = start + chunk + (t < rest ? 1 : 0); // distribui sobra
        td[t].a = a;
        td[t].b = b;
        td[t].start = start;
        td[t].end = end;
        td[t].partial_sum = 0.0;
        pthread_create(&threads[t], NULL, dot_product_worker, &td[t]);
        start = end;
    }

    double result = 0.0;
    for (int t = 0; t < n_threads; t++) {
        pthread_join(threads[t], NULL);
        result += td[t].partial_sum;
    }
    
    avaliar(a, b, a_size, result);

    free(a);
    free(b);

    return 0;
}
