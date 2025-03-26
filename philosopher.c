#include "philo.h"

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
    safe_print(table, id, "is eating", GREEN);
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
    safe_print(table, id, "is sleeping", BLUE);
    ft_sleep(table->time_to_sleep);
    safe_print(table, id, "is thinking", MAGENTA);
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
