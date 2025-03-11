#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

// Filozof durumları
#define THINKING 0
#define HUNGRY 1
#define EATING 2

// Renkli çıktı için
#define RED "\x1B[31m"
#define GREEN "\x1B[32m"
#define YELLOW "\x1B[33m"
#define BLUE "\x1B[34m"
#define RESET "\x1B[0m"

typedef struct s_data
{
    int philosopher_count;          // Filozof sayısı
    pthread_t *threads;             // Thread'ler
    pthread_t death_monitor;        // Ölüm kontrolü için thread
    pthread_mutex_t *forks;         // Çatallar (mutex'ler)
    pthread_mutex_t state_mutex;    // Durum değişiklikleri için mutex
    pthread_mutex_t *can_eat;        // Yemek yiyebilme koşul değişkenleri
    int *states;                    // Filozofların durumları
    int *meals_eaten;               // Her filozofun yediği yemek sayısı
    long long *last_meal_time;      // Her filozofun son yemek yeme zamanı
    int time_to_die;                // Açlıktan ölme süresi (ms)
    int time_to_eat;                // Yemek yeme süresi (ms)
    int time_to_sleep;              // Uyuma süresi (ms)
    int must_eat_count;             // Her filozofun yemesi gereken minimum yemek sayısı
    int simulation_stop;            // Simülasyonu durdurma bayrağı
    long long simulation_start;     // Simülasyon başlangıç zamanı
} t_data;

typedef struct s_philo
{
    int id;                 // Filozof ID'si (0'dan başlar)
    t_data *data;           // Program verileri
} t_philo;

// Mevcut zamanı milisaniye cinsinden al
long long get_current_time_ms()
{
    struct timeval time;

    gettimeofday(&time, NULL);
    return (time.tv_sec * 1000) + (time.tv_usec / 1000);
}

// MS cinsinden daha doğru bekler
void ft_sleep(long long ms)
{
    long long start_time = get_current_time_ms();
    long long current_time;

    while (1)
    {
        current_time = get_current_time_ms();
        if (current_time - start_time >= ms)
            break;
        usleep(100); // Daha kısa aralıklarla döngü, daha hassas zamanlama
    }
}

// Filozof durumunu kontrol eder ve uygunsa yemek yemesine izin verir
void check_philosopher(t_data *data, int id)
{
    int left = (id + data->philosopher_count - 1) % data->philosopher_count;
    int right = (id + 1) % data->philosopher_count;

    // Eğer filozof aç ve her iki çatal da müsaitse
    if (data->states[id] == HUNGRY &&
        data->states[left] != EATING &&
        data->states[right] != EATING)
    {
        data->states[id] = EATING;
        // Son yemek zamanını güncelle
        data->last_meal_time[id] = get_current_time_ms();
        pthread_cond_signal(&data->can_eat[id]);
    }
}

// Filozofun çatalları almasını ve yemek yemesini simüle eder
void take_forks(t_philo *philo)
{
    t_data *data = philo->data;
    int id = philo->id;

    pthread_mutex_lock(&data->state_mutex);

    data->states[id] = HUNGRY;
    printf(YELLOW "%lld ms: %d numaralı filozof acıktı.\n" RESET,
           get_current_time_ms() - data->simulation_start, id + 1);

    // Filozofun durumunu kontrol et ve uygunsa yemesine izin ver
    check_philosopher(data, id);

    // Eğer yemek yiyemezse, sinyal gelene kadar bekle
    while (data->states[id] != EATING && !data->simulation_stop)
    {
        pthread_cond_wait(&data->can_eat[id], &data->state_mutex);
    }

    if (!data->simulation_stop)
    {
        printf(GREEN "%lld ms: %d numaralı filozof yemek yiyor.\n" RESET,
               get_current_time_ms() - data->simulation_start, id + 1);
    }

    pthread_mutex_unlock(&data->state_mutex);
}

// Filozofun çatalları bırakmasını simüle eder
void put_forks(t_philo *philo)
{
    t_data *data = philo->data;
    int id = philo->id;

    pthread_mutex_lock(&data->state_mutex);

    data->states[id] = THINKING;
    printf(BLUE "%lld ms: %d numaralı filozof düşünüyor.\n" RESET,
           get_current_time_ms() - data->simulation_start, id + 1);

    // Sol ve sağdaki filozofların durumlarını kontrol et
    check_philosopher(data, (id + data->philosopher_count - 1) % data->philosopher_count);
    check_philosopher(data, (id + 1) % data->philosopher_count);

    pthread_mutex_unlock(&data->state_mutex);
}

// Ölüm kontrolü yapan thread fonksiyonu
void *death_monitor_func(void *arg)
{
    t_data *data = (t_data *)arg;
    long long current_time;

    // Ölüm kontrolünden önce biraz bekleyelim
    ft_sleep(data->time_to_die / 2);

    while (!data->simulation_stop)
    {
        pthread_mutex_lock(&data->state_mutex);
        current_time = get_current_time_ms();

        for (int i = 0; i < data->philosopher_count; i++)
        {
            // Eğer filozof yemek yemiyorsa ve son yemek yediği zamandan beri time_to_die ms geçtiyse
            if (current_time - data->last_meal_time[i] > data->time_to_die)
            {
                printf(RED "%lld ms: %d numaralı filozof açlıktan öldü! Son yemek: %lld ms önce\n" RESET,
                       current_time - data->simulation_start, i + 1,
                       current_time - data->last_meal_time[i]);
                data->simulation_stop = 1;
                pthread_mutex_unlock(&data->state_mutex);
                return NULL;
            }
        }

        pthread_mutex_unlock(&data->state_mutex);
        usleep(1000); // 1ms bekleyerek CPU kullanımını azalt
    }

    return NULL;
}

// Filozof thread'inin çalıştığı fonksiyon
///////////////////////////////////////////////////////////////////////////
void *philosopher(void *arg)
{
    t_philo *philo = (t_philo *)arg;
    t_data *data = philo->data;
    int id = philo->id;

    // Çift numaralı filozoflar biraz beklesin (deadlock'u önlemek için)
    if (id % 2 == 0)
        ft_sleep(10); // Daha küçük bir bekleme süresi

    while (!data->simulation_stop)
    {
        // Düşünme
        printf(BLUE "%lld ms: %d numaralı filozof düşünüyor.\n" RESET,
               get_current_time_ms() - data->simulation_start, id + 1);
        ft_sleep(data->time_to_sleep);

        if (data->simulation_stop)
            break;

        // Çatalları al ve yemek ye
        take_forks(philo);

        if (data->simulation_stop)
            break;

        // Yemek yeme
        ft_sleep(data->time_to_eat);

        pthread_mutex_lock(&data->state_mutex);
        data->meals_eaten[id]++;

        // Eğer her filozof yeterince yemek yediyse
        if (data->must_eat_count > 0 && data->meals_eaten[id] >= data->must_eat_count)
        {
            int all_ate_enough = 1;
            for (int i = 0; i < data->philosopher_count; i++)
            {
                if (data->meals_eaten[i] < data->must_eat_count)
                {
                    all_ate_enough = 0;
                    break;
                }
            }

            if (all_ate_enough)
            {
                data->simulation_stop = 1;
                printf(GREEN "%lld ms: Tüm filozoflar yeterince yemek yedi. Simülasyon sona eriyor.\n" RESET,
                       get_current_time_ms() - data->simulation_start);
                pthread_mutex_unlock(&data->state_mutex);
                break;
            }
        }
        pthread_mutex_unlock(&data->state_mutex);

        // Çatalları bırak
        put_forks(philo);
    }

    return NULL;
}
/////////////////////////////////////////////////////////////////////////

// Tüm kaynakları temizler
void cleanup(t_data *data, t_philo *philos)
{
    if (data->threads)
        free(data->threads);

    if (data->forks)
    {
        for (int i = 0; i < data->philosopher_count; i++)
            pthread_mutex_destroy(&data->forks[i]);
        free(data->forks);
    }

    if (data->can_eat)
    {
        for (int i = 0; i < data->philosopher_count; i++)
            pthread_cond_destroy(&data->can_eat[i]);
        free(data->can_eat);
    }

    pthread_mutex_destroy(&data->state_mutex);

    if (data->states)
        free(data->states);

    if (data->meals_eaten)
        free(data->meals_eaten);

    if (data->last_meal_time)
        free(data->last_meal_time);

    if (philos)
        free(philos);

    free(data);
}

int init_simulation(t_data *data)
{
    // Thread'ler için bellek ayır
    data->threads = malloc(sizeof(pthread_t) * data->philosopher_count);
    if (!data->threads)
        return 1;

    // Çatallar için bellek ayır
    data->forks = malloc(sizeof(pthread_mutex_t) * data->philosopher_count);
    if (!data->forks)
        return 1;

    // Durum değişkenleri için bellek ayır
    data->can_eat = malloc(sizeof(pthread_cond_t) * data->philosopher_count);
    if (!data->can_eat)
        return 1;

    // Filozof durumları için bellek ayır
    data->states = malloc(sizeof(int) * data->philosopher_count);
    if (!data->states)
        return 1;

    // Yemek sayıları için bellek ayır
    data->meals_eaten = malloc(sizeof(int) * data->philosopher_count);
    if (!data->meals_eaten)
        return 1;

    // Son yemek zamanları için bellek ayır
    data->last_meal_time = malloc(sizeof(long long) * data->philosopher_count);
    if (!data->last_meal_time)
        return 1;

    // Mutex ve durum değişkenlerini başlat
    pthread_mutex_init(&data->state_mutex, NULL);

    // Simülasyon başlangıç zamanını kaydet
    data->simulation_start = get_current_time_ms();

    for (int i = 0; i < data->philosopher_count; i++)
    {
        pthread_mutex_init(&data->forks[i], NULL);
        pthread_cond_init(&data->can_eat[i], NULL);
        data->states[i] = THINKING;
        data->meals_eaten[i] = 0;
        data->last_meal_time[i] = data->simulation_start; // Başlangıçta tüm filozoflar yeni yemiş kabul edilir
    }

    return 0;
}

int main(int argc, char **argv)
{
    t_data *data;
    t_philo *philos;

    // Argüman kontrolü
    if (argc < 5 || argc > 6)
    {
        printf("Kullanım: %s <filozof_sayısı> <ölüm_süresi> <yeme_süresi> <uyuma_süresi> [her_filozofun_yemesi_gereken_yemek_sayısı]\n", argv[0]);
        return 1;
    }

    // Program verisi için bellek ayır
    data = malloc(sizeof(t_data));
    if (!data)
        return 1;

    // Verileri oku
    data->philosopher_count = atoi(argv[1]);
    data->time_to_die = atoi(argv[2]);
    data->time_to_eat = atoi(argv[3]);
    data->time_to_sleep = atoi(argv[4]);
    data->must_eat_count = (argc == 6) ? atoi(argv[5]) : -1;
    data->simulation_stop = 0;

    // Veri doğrulaması
    if (data->philosopher_count <= 0 || data->time_to_die <= 0 ||
        data->time_to_eat <= 0 || data->time_to_sleep <= 0 ||
        (argc == 6 && data->must_eat_count <= 0))
    {
        printf("Geçersiz argümanlar. Tüm değerler pozitif olmalıdır.\n");
        free(data);
        return 1;
    }

    // Simülasyonu başlat
    if (init_simulation(data))
    {
        cleanup(data, NULL);
        printf("Simülasyon başlatılamadı: Bellek hatası\n");
        return 1;
    }

    // Filozofları oluştur
    philos = malloc(sizeof(t_philo) * data->philosopher_count);
    if (!philos)
    {
        cleanup(data, NULL);
        return 1;
    }

    // Filozof thread'lerini başlat
    for (int i = 0; i < data->philosopher_count; i++)
    {
        philos[i].id = i;
        philos[i].data = data;
        if (pthread_create(&data->threads[i], NULL, philosopher, &philos[i]) != 0)
        {
            data->simulation_stop = 1;
            for (int j = 0; j < i; j++)
                pthread_join(data->threads[j], NULL);
            cleanup(data, philos);
            printf("Thread oluşturulamadı\n");
            return 1;
        }
    }

    // Ölüm kontrolü thread'ini başlat (filozof thread'lerinden sonra)
    if (pthread_create(&data->death_monitor, NULL, death_monitor_func, data) != 0)
    {
        data->simulation_stop = 1;
        for (int i = 0; i < data->philosopher_count; i++)
            pthread_join(data->threads[i], NULL);
        cleanup(data, philos);
        printf("Ölüm kontrolü thread'i oluşturulamadı\n");
        return 1;
    }

    // Thread'lerin tamamlanmasını bekle
    for (int i = 0; i < data->philosopher_count; i++)
        pthread_join(data->threads[i], NULL);
    pthread_join(data->death_monitor, NULL);

    // Temizlik
    cleanup(data, philos);

    return 0;
}
