#include "philo.h"
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
