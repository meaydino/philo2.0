#include "philo.h"
int	check_simulation_stop(t_data *data)
{
	int	stop;

	pthread_mutex_lock(&data->stop_mutex);
	stop = data->simulation_stop;
	pthread_mutex_unlock(&data->stop_mutex);
	return (stop);
}

void	set_simulation_stop(t_data *data)
{
	pthread_mutex_lock(&data->stop_mutex);
	data->simulation_stop = 1;
	pthread_mutex_unlock(&data->stop_mutex);
}

void	safe_print(t_data *data, int id, char *message, char *color)
{
	pthread_mutex_lock(&data->print_mutex);
	if (!check_simulation_stop(data))
	{
		printf("%s%lld ms: %d %s%s\n", color,
			get_current_time_ms() - data->simulation_start, id + 1, message, RESET);
	}
	pthread_mutex_unlock(&data->print_mutex);
}