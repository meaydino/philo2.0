#include "philo.h"

void	*philosopher(void *arg)
{
	t_philo	*philo;
	t_data	*data;
	int		id;

	philo = (t_philo *)arg;
	data = philo->data;
	id = philo->id;
	// Sadece tek sayılı filozoflar için küçük bir gecikme ekleyin
	// Çift sayılı filozoflar hemen başlayabilir
	if (id % 2 == 1)
		ft_sleep(10);
	while (!check_simulation_stop(data))
	{
		// Doğrudan çatal almaya geç, düşünmeyi atla (ilk turda)
		take_forks(philo);
		if (check_simulation_stop(data))
		{
			put_forks(philo);
			break;
		}
		if(philosopher_v2(philo, data, id))
			break;
	}
	return (NULL);
}
int philosopher_v2(t_philo *philo, t_data *data, int id)
{
	// Yemek yeme
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
		return 1;
	// Uyuma
	safe_print(data, id, "uyuyor", BLUE);
	ft_sleep(data->time_to_sleep);
	// Düşünme (bundan sonraki turlarda)
	safe_print(data, id, "düşünüyor", BLUE);
	return 0;
}
void	philo_enough_food(t_data *data)
{
	int all_ate_enough;
	int i;

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
		safe_print(data, 0,
			"Tüm filozoflar yeterince yemek yedi. Simülasyon sona eriyor.",
			GREEN);
		set_simulation_stop(data);
	}
}
