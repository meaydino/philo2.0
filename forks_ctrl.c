#include "philo.h"

int	try_take_forks(t_philo *philo)
{
	t_data	*data;
	int		left_fork;
	int		right_fork;

	data = philo->data;
	left_fork = philo->id;
	right_fork = (philo->id + 1) % data->philosopher_count;
	// Tekli çatalları almak için çift ve tek filozoflar için farklı stratejiler
	if(data->philosopher_count == 1)
	{
		one_philosopher(data, philo, left_fork);
		return 0;
	}
	if (philo->id % 2 == 0)
	{
		if (single_phiolosopher(philo, data, left_fork, right_fork) == 0)
			return (0);
	}
	else
	{
		if (double_philosopher(philo, data, left_fork, right_fork) == 0)
			return (0);
	}
	return (1);
}
int	single_phiolosopher(t_philo *philo, t_data *data, int left_fork,
		int right_fork)
{
	if (pthread_mutex_lock(&data->forks[left_fork]) != 0)
		return (0);
	safe_print(data, philo->id, "sol çatalı aldı", YELLOW);
	if (pthread_mutex_lock(&data->forks[right_fork]) != 0)
	{
		pthread_mutex_unlock(&data->forks[left_fork]);
		return (0);
	}
	safe_print(data, philo->id, "sağ çatalı aldı", YELLOW);
	return (1);
}
int	double_philosopher(t_philo *philo, t_data *data, int left_fork,
		int right_fork)
{
	if (pthread_mutex_lock(&data->forks[right_fork]) != 0)
		return (0);
	safe_print(data, philo->id, "sağ çatalı aldı", YELLOW);
	if (pthread_mutex_lock(&data->forks[left_fork]) != 0)
	{
		pthread_mutex_unlock(&data->forks[right_fork]);
		return (0);
	}
	safe_print(data, philo->id, "sol çatalı aldı", YELLOW);
	return (1);
}

void	take_forks(t_philo *philo)
{
	t_data	*data;

	data = philo->data;

	// Deadlock önleme stratejisini kaldırıyoruz, çünkü
	// zaten philosopher fonksiyonunda gecikme eklendi

	while (!check_simulation_stop(data))
	{
		if (try_take_forks(philo))
		{
			pthread_mutex_lock(&data->state_mutex);
			data->states[philo->id] = EATING;
			data->last_meal_time[philo->id] = get_current_time_ms();
			pthread_mutex_unlock(&data->state_mutex);
			return;
		}
		ft_sleep(1); // Kısa bekleme ile yeniden deneme
	}
}

void	put_forks(t_philo *philo)
{
	t_data *data;
	int left_fork;
	int right_fork;

	data = philo->data;
	left_fork = philo->id;
	right_fork = (philo->id + 1) % data->philosopher_count;

	// Mutex'leri bırakma
	pthread_mutex_unlock(&data->forks[left_fork]);
	pthread_mutex_unlock(&data->forks[right_fork]);

	// Durum güncelleme
	pthread_mutex_lock(&data->state_mutex);
	data->states[philo->id] = THINKING;
	pthread_mutex_unlock(&data->state_mutex);

	safe_print(data, philo->id, "çatalları bıraktı", BLUE);
}
