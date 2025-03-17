#include "philo.h"

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
        printf("%s%lld ms: %d açlıktan öldü!%s\n", RED,
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
