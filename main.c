#include "philo.h"
int	cleanup(t_philo *philos, int error, int flag, int count)
{
	t_data *table;

	table = NULL;
	if (philos != NULL)
		table = philos->table;
	if (error == 1)
		printf("Error!\n");
	if (flag > 0)
		init_destroy(table, flag, count);
	if (table)
	{
		if (table->threads)
			free(table->threads);
		if(table->forks)
			free(table->forks);
		if (table->meals_eaten)
			free(table->meals_eaten);
		if (table->last_meal_time)
			free(table->last_meal_time);
		free(table);
	}
	if (philos)
		free(philos);
	return (1);
}
int	init_destroy(t_data *table, int flag, int count)
{
	int	i;

	if (flag > 1)
		pthread_mutex_destroy(&table->stop_mutex);
	if (flag > 2)
		pthread_mutex_destroy(&table->print_mutex);
	if (flag > 3)
	{
		i = -1;
		if (table->forks)
		{
			while (++i < count)
				pthread_mutex_destroy(&table->forks[i]);
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
		return (cleanup(NULL, 1, 0, 0));
	philos = malloc(sizeof(t_philo) * ft_atoi(av[1]));
	if (!philos)
		return (cleanup(philos, 1, 0, 0));
	if (av_config(av, table))
		return (cleanup(philos, 1, 0, 0));
	if (init_simulation(table, philos))
		return (cleanup(philos, 1, 0, 0));
	if (thread_start(table, philos))
		return (cleanup(philos, 1, 4, table->philosopher_count));
	cleanup(philos, 0, 4, table->philosopher_count);
	return (0);
}
