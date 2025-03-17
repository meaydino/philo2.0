#include "philo.h"

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
