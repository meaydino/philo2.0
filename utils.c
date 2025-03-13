#include "philo.h"

long long	get_current_time_ms(void)
{
	struct timeval	time;

	gettimeofday(&time, NULL);
	return ((time.tv_sec * 1000) + (time.tv_usec / 1000));
}

void ft_sleep(long long ms)
{
    long long start_time;
    long long current_time;

    start_time = get_current_time_ms();
    while (1)
    {
        current_time = get_current_time_ms();
        if (current_time - start_time >= ms)
            break;
        usleep(50); // Daha hassas zaman dilimlerinde kontrol (0.05ms)
    }
}

int	arg_ctrl(char *str)
{
	int	i;

	i = 0;
	while (str[i])
	{
		if (str[i] < '0' || str[i] > '9')
			return (1);
		i++;
	}
	return (0);
}
int	str_contains(const char *str, const char *substr)
{
	int	i;
	int	j;

	i = 0;
	while (str[i])
	{
		j = 0;
		while (substr[j] && str[i + j] == substr[j])
			j++;
		if (substr[j] == '\0')
			return (1);
		i++;
	}
	return (0);
}

int	ft_atoi(char *str)
{
	int i;
	int sign;
	int result;

	i = 0;
	sign = 1;
	result = 0;
	if (arg_ctrl(str))
		return (0);
	while (str[i] == ' ' || str[i] == '\t' || str[i] == '\n' || str[i] == '\v'
		|| str[i] == '\f' || str[i] == '\r')
		i++;
	if (str[i] == '-' || str[i] == '+')
	{
		if (str[i] == '-')
			sign = -1;
		i++;
	}
	while (str[i] >= '0' && str[i] <= '9')
	{
		result = result * 10 + str[i] - '0';
		i++;
	}
	return (result * sign);
}


// void *death_monitor_func(void *arg)
// {
//     t_data      *data;
//     long long   current_time;
//     long long   time_since_last_meal;
//     int         i;

//     data = (t_data *)arg;
//     while (!check_simulation_stop(data))
//     {
//         i = 0;
//         while (i < data->philosopher_count && !check_simulation_stop(data))
//         {
//             pthread_mutex_lock(&data->state_mutex);
//             current_time = get_current_time_ms();
//             time_since_last_meal = current_time - data->last_meal_time[i];
//             if (time_since_last_meal >= data->time_to_die)
//             {
//                 // Simülasyonu hemen durdur
//                 set_simulation_stop(data);

//                 // Ölüm mesajını göster - tam time_to_die zamanında
//                 pthread_mutex_lock(&data->print_mutex);
//                 printf("%s%lld ms: %d açlıktan öldü!%s\n", RED,
//                 data->last_meal_time[i] + data->time_to_die - data->simulation_start,
//                 i + 1, RESET);
//                 pthread_mutex_unlock(&data->print_mutex);
//                 pthread_mutex_unlock(&data->state_mutex);
//                 return(NULL);
//             }
//             if (data->time_to_die - time_since_last_meal < 5)
//             {
//                 pthread_mutex_unlock(&data->state_mutex);
//                 usleep(100);
//             }
//             else
//             {
//                 pthread_mutex_unlock(&data->state_mutex);
//                 i++;  // Sadece ölüm yakın değilse bir sonraki filozofa geç
//             }
//         }
//         usleep(200);
//     }
//     return (NULL);
// }
