#include "philo.h"
void	*philosopher(void *arg)
{
	t_philo	*philo;
	t_data	*data;
	int		id;

	philo = (t_philo *)arg;
	data = philo->data;
	id = philo->id;

	// Başlamak için çok kısa bir bekleme (çift numaralı filozoflar için)
	if (id % 2 == 0)
		ft_sleep(10);
	while (!check_simulation_stop(data))
	{
		// Düşünme
		safe_print(data, id, "düşünüyor", BLUE);
		ft_sleep(10); // Kısa düşünme süresi
		// Çatalları almaya çalışma
		take_forks(philo);
		if (check_simulation_stop(data))
		{
			put_forks(philo); // Simülasyon durmuşsa çatalları bırak
			break ;
		}
        if(philosopher_v2(philo, data, id))
            break;
	}
	return (NULL);
}
int philosopher_v2(t_philo *philo, t_data *data, int id)
{
    safe_print(data, id, "yemek yiyor", GREEN);
	ft_sleep(data->time_to_eat);
	// Yemek sayısını güncelleme
	pthread_mutex_lock(&data->state_mutex);
	data->meals_eaten[id]++;
	pthread_mutex_unlock(&data->state_mutex);
	// Çatalları bırakma
	put_forks(philo);
	if (check_simulation_stop(data))
		return 1;
	// Tokluğu kontrol etme
	philo_enough_food(data);
	if (check_simulation_stop(data))
		return 1; ;
	// Uyuma
	safe_print(data, id, "uyuyor", BLUE);
	ft_sleep(data->time_to_sleep);
    return 0;
}
void	philo_enough_food(t_data *data)
{
	int	all_ate_enough;
	int	i;

	all_ate_enough = 1;
	i = -1;

	pthread_mutex_lock(&data->state_mutex);
	while (++i < data->philosopher_count)
	{
		if (data->meals_eaten[i] < data->must_eat_count)
		{
			all_ate_enough = 0;
			break ;
		}
	}
	pthread_mutex_unlock(&data->state_mutex);

	if (all_ate_enough && data->must_eat_count > 0)
	{
		safe_print(data, 0, "Tüm filozoflar yeterince yemek yedi. Simülasyon sona eriyor.", GREEN);
		set_simulation_stop(data);
	}
}