#include "philo.h"

int try_take_forks(t_philo *philo)
{
    t_data  *table;
    int     left_fork;
    int     right_fork;

    table = philo->table;
    left_fork = philo->id;
    right_fork = (philo->id + 1) % table->philosopher_count;

    if (table->philosopher_count == 1)
    {
        one_philosopher(table, philo, left_fork);
        return 0;
    }
    if (philo->id % 2 == 0)
    {
        if (single_phiolosopher(philo, table, left_fork, right_fork) == 0)
            return (0);
    }
    else
    {
        if (double_philosopher(philo, table, left_fork, right_fork) == 0)
            return (0);
    }
    return (1);
}
int	single_phiolosopher(t_philo *philo, t_data *table, int left_fork,
		int right_fork)
{
	if (pthread_mutex_lock(&table->forks[left_fork]) != 0)
		return (0);
	safe_print(table, philo->id, "has taken a fork", YELLOW);
	if (pthread_mutex_lock(&table->forks[right_fork]) != 0)
	{
		pthread_mutex_unlock(&table->forks[left_fork]);
		return (0);
	}
	safe_print(table, philo->id, "has taken a fork", YELLOW);
	return (1);
}
int	double_philosopher(t_philo *philo, t_data *table, int left_fork,
		int right_fork)
{
	if (pthread_mutex_lock(&table->forks[right_fork]) != 0)
		return (0);
	safe_print(table, philo->id, "has taken a fork", YELLOW);
	if (pthread_mutex_lock(&table->forks[left_fork]) != 0)
	{
		pthread_mutex_unlock(&table->forks[right_fork]);
		return (0);
	}
	safe_print(table, philo->id, "has taken a fork", YELLOW);
	return (1);
}

// Daha dengeli bir filozof davranışı için take_forks fonksiyonu
void take_forks(t_philo *philo)
{
    t_data  *table;

    table = philo->table;

    // Çatalları almak için deneme yapıyoruz
    while (!check_simulation_stop(table))
    {
        if (try_take_forks(philo))
        {
            // Yemek zamanını güncellemek kritik - hemen güncelle
            table->last_meal_time[philo->id] = get_current_time_ms();
            return;
        }
        // Çatalları alamadık, kısa bir süre bekleyip tekrar deneyelim
        usleep(500); // Kısa bekleme ile yeniden deneme (0.5ms)
    }
}

void	put_forks(t_philo *philo)
{
	t_data *table;
	int left_fork;
	int right_fork;

	table = philo->table;
	left_fork = philo->id;
	right_fork = (philo->id + 1) % table->philosopher_count;

	// Mutex'leri bırakma
	pthread_mutex_unlock(&table->forks[left_fork]);
	pthread_mutex_unlock(&table->forks[right_fork]);
}
