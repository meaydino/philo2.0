# include <stdio.h>
# include <stdlib.h>
# include <pthread.h>
# include <unistd.h>
# include <sys/time.h>

// Filozof durumları
# define THINKING 0
# define HUNGRY 1
# define EATING 2
# define SLEEP 3

// Renkli çıktı için
# define RED "\x1B[31m"
# define GREEN "\x1B[32m"
# define YELLOW "\x1B[33m"
# define BLUE "\x1B[34m"
# define RESET "\x1B[0m"

typedef struct s_data
{
	int				philosopher_count;         // Filozof sayısı
	int				must_eat_count;            // Her filozofun yemesi gereken minimum yemek sayısı
	long long		simulation_start;          // Simülasyon başlangıç zamanı
	int				*meals_eaten;              // Her filozofun yediği yemek sayısı
	long long		*last_meal_time;           // Her filozofun son yemek yeme zamanı
	pthread_mutex_t	state_mutex;               // Durum değişiklikleri için mutex
	pthread_mutex_t	*forks;                    // Çatallar (mutex'ler)
	int				*states;                   // Filozofların durumları
	pthread_t		*threads;                  // Thread'ler
	pthread_t		death_monitor;             // Ölüm kontrolü için thread
	int				time_to_die;               // Açlıktan ölme süresi (ms)
	int				time_to_eat;               // Yemek yeme süresi (ms)
	int				time_to_sleep;             // Uyuma süresi (ms)
	int				simulation_stop;           // Simülasyonu durdurma bayrağı
	pthread_mutex_t	stop_mutex;                // Simülasyonu durdurma bayrağı için mutex
	pthread_mutex_t	print_mutex;               // Yazdırma için mutex
}	t_data;

typedef struct s_philo
{
	int				id;                // Filozof ID'si (0'dan başlar)
	t_data			*table;             // Program verileri
}	t_philo;

int try_take_forks(t_philo *philo)
{
    t_data  *table;
    int     left_fork;
    int     right_fork;

    table = philo->table;
    left_fork = philo->id;
    right_fork = (philo->id + 1) % table->philosopher_count;

    if (table->philosopher_count == 1)
    {
        one_philosopher(table, philo, left_fork);
        return 0;
    }
    if (philo->id % 2 == 0)
    {
        if (single_phiolosopher(philo, table, left_fork, right_fork) == 0)
            return (0);
    }
    else
    {
        if (double_philosopher(philo, table, left_fork, right_fork) == 0)
            return (0);
    }
    return (1);
}
int	single_phiolosopher(t_philo *philo, t_data *table, int left_fork,
		int right_fork)
{
	if (pthread_mutex_lock(&table->forks[left_fork]) != 0)
		return (0);
	safe_print(table, philo->id, "has taken a fork", YELLOW);
	if (pthread_mutex_lock(&table->forks[right_fork]) != 0)
	{
		pthread_mutex_unlock(&table->forks[left_fork]);
		return (0);
	}
	safe_print(table, philo->id, "has taken a fork", YELLOW);
	return (1);
}
int	double_philosopher(t_philo *philo, t_data *table, int left_fork,
		int right_fork)
{
	if (pthread_mutex_lock(&table->forks[right_fork]) != 0)
		return (0);
	safe_print(table, philo->id, "has taken a fork", YELLOW);
	if (pthread_mutex_lock(&table->forks[left_fork]) != 0)
	{
		pthread_mutex_unlock(&table->forks[right_fork]);
		return (0);
	}
	safe_print(table, philo->id, "has taken a fork", YELLOW);
	return (1);
}

// Daha dengeli bir filozof davranışı için take_forks fonksiyonu
void take_forks(t_philo *philo)
{
    t_data  *table;

    table = philo->table;

    // Çatalları almak için deneme yapıyoruz
    while (!check_simulation_stop(table))
    {
        if (try_take_forks(philo))
        {
            // Çatallar alındı, yemek yeme durumuna geçiş
            pthread_mutex_lock(&table->state_mutex);
            table->states[philo->id] = EATING;
            // Yemek zamanını güncellemek kritik - hemen güncelle
            table->last_meal_time[philo->id] = get_current_time_ms();
            pthread_mutex_unlock(&table->state_mutex);
            return;
        }
        // Çatalları alamadık, kısa bir süre bekleyip tekrar deneyelim
        usleep(500); // Kısa bekleme ile yeniden deneme (0.5ms)
    }
}

void	put_forks(t_philo *philo)
{
	t_data *table;
	int left_fork;
	int right_fork;

	table = philo->table;
	left_fork = philo->id;
	right_fork = (philo->id + 1) % table->philosopher_count;

	// Mutex'leri bırakma
	pthread_mutex_unlock(&table->forks[left_fork]);
	pthread_mutex_unlock(&table->forks[right_fork]);

	// Durum güncelleme
	pthread_mutex_lock(&table->state_mutex);
	table->states[philo->id] = THINKING;
	pthread_mutex_unlock(&table->state_mutex);
}
int	av_config(char **av, t_data *table)
{
	table->threads = NULL;
	table->forks = NULL;
	table->states = NULL;
	table->meals_eaten = NULL;
	table->last_meal_time = NULL;
	table->philosopher_count = ft_atoi(av[1]);
	table->time_to_die = ft_atoi(av[2]);
	table->time_to_eat = ft_atoi(av[3]);
	table->time_to_sleep = ft_atoi(av[4]);
	table->simulation_stop = 0;
	table->must_eat_count = -1;
	if (av[5])
		table->must_eat_count = ft_atoi(av[5]);
	if (table->philosopher_count <= 0 || table->time_to_die <= 0
		|| table->time_to_eat <= 0 || table->time_to_sleep <= 0 || (av[5]
			&& table->must_eat_count <= 0))
	{
		return (1);
	}
	return (0);
}

int	mutex_initialization(t_data *table)
{
	int	i;
	int	error;

	i = -1;
	error = 0;
	error = pthread_mutex_init(&table->state_mutex, NULL);
	if (error != 0)
		return (1);
	error = pthread_mutex_init(&table->stop_mutex, NULL);
	if (error != 0)
		return (init_destroy(table, 1));
	error = pthread_mutex_init(&table->print_mutex, NULL);
	if (error != 0)
		return (init_destroy(table, 2));
	table->simulation_start = get_current_time_ms();
	while (++i < table->philosopher_count)
	{
		if (pthread_mutex_init(&table->forks[i], NULL) != 0)
			return (cleanup(table, NULL, 0, 3));
		table->states[i] = 0;
		table->meals_eaten[i] = 0;
		table->last_meal_time[i] = table->simulation_start;
	}
	return (0);
}

int	init_simulation(t_data *table)
{
	table->threads = malloc(sizeof(pthread_t) * table->philosopher_count);
	if (!table->threads)
		return (1);
	table->forks = malloc(sizeof(pthread_mutex_t) * table->philosopher_count);
	if (!table->forks)
		return (1);
	table->states = malloc(sizeof(int) * table->philosopher_count);
	if (!table->states)
		return (1);
	table->meals_eaten = malloc(sizeof(int) * table->philosopher_count);
	if (!table->meals_eaten)
		return (1);
	table->last_meal_time = malloc(sizeof(long long) * table->philosopher_count);
	if (!table->last_meal_time)
		return (1);
	if (mutex_initialization(table))
		return (1);
	return (0);
}
void *death_monitor_func(void *arg)
{
    t_data      *table;
    int         i;

    table = (t_data *)arg;
    while (!check_simulation_stop(table))
    {
        i = 0;
        while (i < table->philosopher_count && !check_simulation_stop(table))
        {
            if (check_philo_death(table, i))
                return (NULL);
            i++;
        }
        usleep(200);
    }
    return (NULL);
}

int check_philo_death(t_data *table, int i)
{
    long long current_time;
    long long time_since_last_meal;

    pthread_mutex_lock(&table->state_mutex);
    current_time = get_current_time_ms();
    time_since_last_meal = current_time - table->last_meal_time[i];
    if (time_since_last_meal >= table->time_to_die)
    {
        set_simulation_stop(table);
        pthread_mutex_lock(&table->print_mutex);
        printf("%s%lld ms: %d dead%s\n", RED,
            table->last_meal_time[i] + table->time_to_die - table->simulation_start,
            i + 1, RESET);
        pthread_mutex_unlock(&table->print_mutex);
        pthread_mutex_unlock(&table->state_mutex);
        return (1);
    }
    pthread_mutex_unlock(&table->state_mutex);
    return (0);
}



int	philo_dead_control(t_data *table)
{
	if (pthread_create(&table->death_monitor, NULL, death_monitor_func,
			table) != 0)
	{
		printf("Ölüm kontrolü thread'i oluşturulamadı\n");
		set_simulation_stop(table);
		return (1);
	}
	return (0);
}

// Thread başlatma
int	thread_start(t_data *table, t_philo *philos)
{
	int	i;
	int	j;

	i = -1;
	while (++i < table->philosopher_count)
	{
		philos[i].id = i;
		philos[i].table = table;
		if (pthread_create(&table->threads[i], NULL, philosopher,
				&philos[i]) != 0)
		{
			set_simulation_stop(table);
			j = -1;
			while (++j < i)
				pthread_join(table->threads[j], NULL);
			// Thread oluşturma hatası durumunda temizlik yap
			return (1);
		}
	}
	return (thread_start_v2(table));
}
int thread_start_v2(t_data *table)
{
    int i;
    // Ölüm kontrolü threadini başlat
    if (philo_dead_control(table))
    {
        // Zaten başlatılmış thread'leri bekleyelim
        i = -1;
        while (++i < table->philosopher_count)
            pthread_join(table->threads[i], NULL);

        // Temizliği burada yapma (cleanup fonksiyonu main'de çağrılıyor)
        return (1);
    }

    // Thread'lerin tamamlanmasını bekle
    i = -1;
    while (++i < table->philosopher_count)
        pthread_join(table->threads[i], NULL);

    pthread_join(table->death_monitor, NULL);

    // Temizliği burada yapma (cleanup fonksiyonu main'de çağrılıyor)
    return (0);
}
void one_philosopher(t_data *table , t_philo *philo, int left_fork)
{
	// Sadece tek çatalı alabilir, ikinci çatal olmadığı için bir süre bekler ve ölür
	pthread_mutex_lock(&table->forks[left_fork]);
	safe_print(table, philo->id, "has taken a fork", YELLOW);

	// Tek çatalla bekle - time_to_die süresinden fazla beklemeye gerek yok
	// Ölüm kontrolü thread'i ölümü tespit edecektir
	ft_sleep(table->time_to_die + 10);

	pthread_mutex_unlock(&table->forks[left_fork]);
}
void *philosopher(void *arg)
{
    t_philo *philo;
    t_data  *table;
    int     id;

    philo = (t_philo *)arg;
    table = philo->table;
    id = philo->id;
    if (id % 2 == 1)
        ft_sleep(table->time_to_eat / 2); // Daha dengeli bir başlangıç gecikmesi
    while (!check_simulation_stop(table))
    {
        take_forks(philo);
        if (check_simulation_stop(table))
        {
            put_forks(philo);
            break;
        }
        if (philosopher_v2(philo, table, id))
            break;
        if (table->philosopher_count % 2 == 1)
            ft_sleep(table->time_to_eat / 4);
    }
    return (NULL);
}
int philosopher_v2(t_philo *philo, t_data *table, int id)
{
    pthread_mutex_lock(&table->state_mutex);
    table->last_meal_time[id] = get_current_time_ms();
    pthread_mutex_unlock(&table->state_mutex);
    safe_print(table, id, "eating", GREEN);
    ft_sleep(table->time_to_eat);
    pthread_mutex_lock(&table->state_mutex);
    table->meals_eaten[id]++;
    pthread_mutex_unlock(&table->state_mutex);
    put_forks(philo);
    if (check_simulation_stop(table))
        return 1;
    philo_enough_food(table);
    if (check_simulation_stop(table))
        return 1;
    safe_print(table, id, "sleeping", BLUE);
    ft_sleep(table->time_to_sleep);
    safe_print(table, id, "thinking", BLUE);
    return 0;
}
void	philo_enough_food(t_data *table)
{
	int all_ate_enough;
	int i;

	all_ate_enough = 1;
	i = -1;

	pthread_mutex_lock(&table->state_mutex);
	while (++i < table->philosopher_count)
	{
		if (table->meals_eaten[i] < table->must_eat_count)
		{
			all_ate_enough = 0;
			break ;
		}
	}
	pthread_mutex_unlock(&table->state_mutex);

	if (all_ate_enough && table->must_eat_count > 0)
		set_simulation_stop(table);
}
int	check_simulation_stop(t_data *table)
{
	int	stop;

	pthread_mutex_lock(&table->stop_mutex);
	stop = table->simulation_stop;
	pthread_mutex_unlock(&table->stop_mutex);
	return (stop);
}

void	set_simulation_stop(t_data *table)
{
	pthread_mutex_lock(&table->stop_mutex);
	table->simulation_stop = 1;
	pthread_mutex_unlock(&table->stop_mutex);
}


void	safe_print(t_data *table, int id, char *message, char *color)
{
	pthread_mutex_lock(&table->print_mutex);
	if (!check_simulation_stop(table) || str_contains(message, "dead"))
	{
		printf("%s%lld ms: %d %s%s\n", color, get_current_time_ms()
			- table->simulation_start, id + 1, message, RESET);
	}
	pthread_mutex_unlock(&table->print_mutex);
}
long long	get_current_time_ms(void)
{
	struct timeval	time;

	gettimeofday(&time, NULL);
	return ((time.tv_sec * 1000) + (time.tv_usec / 1000));
}

void ft_sleep(long long ms)
{
    long long start_time;
    long long current_time;

    start_time = get_current_time_ms();
    while (1)
    {
        current_time = get_current_time_ms();
        if (current_time - start_time >= ms)
            break;
        usleep(50); // Daha hassas zaman dilimlerinde kontrol (0.05ms)
    }
}

int	arg_ctrl(char *str)
{
	int	i;

	i = 0;
	while (str[i])
	{
		if (str[i] < '0' || str[i] > '9')
			return (1);
		i++;
	}
	return (0);
}
int	str_contains(const char *str, const char *substr)
{
	int	i;
	int	j;

	i = 0;
	while (str[i])
	{
		j = 0;
		while (substr[j] && str[i + j] == substr[j])
			j++;
		if (substr[j] == '\0')
			return (1);
		i++;
	}
	return (0);
}

int	ft_atoi(char *str)
{
	int i;
	int sign;
	int result;

	i = 0;
	sign = 1;
	result = 0;
	if (arg_ctrl(str))
		return (-1);
	while (str[i] == 32 || (str[i] >= 9 && str[i] <= 13))
		i++;
	if (str[i] == '-' || str[i] == '+')
	{
		if (str[i] == '-')
			sign = -1;
		i++;
	}
	while (str[i] >= '0' && str[i] <= '9')
	{
		result = result * 10 + str[i] - '0';
		i++;
	}
	return (result * sign);
}
int	cleanup(t_data *table, t_philo *philos, int error, int flag)
{
	if (error == 1)
		printf("Error!\n");
	if (flag > 0)
		init_destroy(table, flag);
	if (table->threads)
		free(table->threads);
	if (table->meals_eaten)
		free(table->meals_eaten);
	if (table->last_meal_time)
		free(table->last_meal_time);
	if (philos)
		free(philos);
	free(table);
	return (1);
}
int	init_destroy(t_data *table, int flag)
{
	int	i;

	if (flag > 0)
		pthread_mutex_destroy(&table->state_mutex);
	if (flag > 1)
		pthread_mutex_destroy(&table->stop_mutex);
	if (flag > 2)
		pthread_mutex_destroy(&table->print_mutex);
	if (flag > 3)
	{
		i = -1;
		if (table->forks)
		{
			while (++i < table->philosopher_count)
				pthread_mutex_destroy(&table->forks[i]);
			free(table->forks);
		}
	}
	return (1);
}
int	main(int ac, char **av)
{
	t_data	*table;
	t_philo	*philos;

	if (ac < 5 || ac > 6)
	{
		printf("Check Arguments!\n");
		return (1);
	}
	table = malloc(sizeof(t_data));
	if (!table)
		return (cleanup(NULL, NULL, 1, 0));
	if (av_config(av, table) || init_simulation(table))
		return (cleanup(table, NULL, 1, 0));
	philos = malloc(sizeof(t_philo) * table->philosopher_count);
	if (!philos)
		return (cleanup(table, NULL, 1, 4));
	if (thread_start(table, philos))
		return (cleanup(table, philos, 1, 4));
	cleanup(table, philos, 0, 4);
	return (0);
}
