#include "philo.h"

void	*death_monitor_func(void *arg)
{
	t_data		*data;
	long long	current_time;
	int			i;

	data = (t_data *)arg;
	while (!check_simulation_stop(data))
	{
		i = 0;
		while (i < data->philosopher_count && !check_simulation_stop(data))
		{
			pthread_mutex_lock(&data->state_mutex);
			current_time = get_current_time_ms();
			// Filozof açlıktan öldü mü?
			if (current_time - data->last_meal_time[i] > data->time_to_die)
			{
				safe_print(data, i, "açlıktan öldü!", RED);
				set_simulation_stop(data);
				pthread_mutex_unlock(&data->state_mutex);
				return (NULL);
			}
			pthread_mutex_unlock(&data->state_mutex);
			i++;
		}
		ft_sleep(1); // Ölüm kontrolü için kısa bekleme
	}
	return (NULL);
}

int	philo_dead_control(t_data *data)
{
	if (pthread_create(&data->death_monitor, NULL, death_monitor_func,
			data) != 0)
	{
		printf("Ölüm kontrolü thread'i oluşturulamadı\n");
		set_simulation_stop(data);
		return (1);
	}
	return (0);
}

// Thread başlatma
int	thread_start(t_data *data, t_philo *philos)
{
	int	i;
	int	j;

	i = -1;
	while (++i < data->philosopher_count)
	{
		philos[i].id = i;
		philos[i].data = data;
		if (pthread_create(&data->threads[i], NULL, philosopher,
				&philos[i]) != 0)
		{
			set_simulation_stop(data);
			j = -1;
			while (++j < i)
				pthread_join(data->threads[j], NULL);
			cleanup(data, philos);
			printf("Thread oluşturulamadı\n");
			return (1);
		}
	}
	thread_start_v2(data, philos);
	return (0);
}
int	thread_start_v2(t_data *data, t_philo *philos)
{
	int i;
	// Ölüm kontrolü threadini başlat
	if (philo_dead_control(data))
	{
		cleanup(data, philos);
		return (1);
	}
	// Thread'lerin tamamlanmasını bekle
	i = -1;
	while (++i < data->philosopher_count)
		pthread_join(data->threads[i], NULL);
	pthread_join(data->death_monitor, NULL);
	cleanup(data, philos);
	return (0);
}
void one_philosopher(t_data *data , t_philo *philo, int left_fork)
{
	// Sadece tek çatalı alabilir, ikinci çatal olmadığı için bir süre bekler ve ölür
	pthread_mutex_lock(&data->forks[left_fork]);
	safe_print(data, philo->id, "has taken a fork", YELLOW);

	// Tek çatalla bekle - time_to_die süresinden fazla beklemeye gerek yok
	// Ölüm kontrolü thread'i ölümü tespit edecektir
	ft_sleep(data->time_to_die + 10);

	pthread_mutex_unlock(&data->forks[left_fork]);
}
