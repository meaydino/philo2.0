#include "philo.h"

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
