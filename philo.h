#ifndef PHILO_H
# define PHILO_H

# include <stdio.h>
# include <stdlib.h>
# include <pthread.h>
# include <unistd.h>
# include <sys/time.h>

// Filozof durumları
# define THINKING 0
# define EATING 1
# define SLEEP 2

// Renkli çıktı için
# define RED "\x1B[31m" //dead
# define GREEN "\x1B[32m" // is eating
# define YELLOW "\x1B[33m" // has taken a fork
# define BLUE "\x1B[34m" // is sleeping
# define MAGENTA "\x1B[35m" // is thinking
# define RESET "\x1B[0m" // resetleme rengi

typedef struct s_data
{
	int				philosopher_count;         // Filozof sayısı
	int				must_eat_count;            // Her filozofun yemesi gereken minimum yemek sayısı
	long long		simulation_start;          // Simülasyon başlangıç zamanı
	int				*meals_eaten;              // Her filozofun yediği yemek sayısı
	long long		*last_meal_time;           // Her filozofun son yemek yeme zamanı
	pthread_mutex_t	*forks;                    // Çatallar (mutex'ler)
	pthread_t		*threads;                  // Thread'ler
	pthread_t		death_monitor;             // Ölüm kontrolü için thread
	int				time_to_die;               // Açlıktan ölme süresi (ms)
	int				time_to_eat;               // Yemek yeme süresi (ms)
	int				time_to_sleep;             // Uyuma süresi (ms)
	int				simulation_stop;           // Simülasyonu durdurma bayrağı
	pthread_mutex_t	stop_mutex;                // Simülasyonu durdurma bayrağı için mutex
	pthread_mutex_t	print_mutex;               // Yazdırma için mutex
}	t_data;

typedef struct s_philo
{
	int				id;                // Filozof ID'si (0'dan başlar)
	t_data			*table;             // Program verileri
}	t_philo;

// Fonksiyon prototipleri
int			philo_dead_control(t_data *table);
void		*death_monitor_func(void *arg);
int	        thread_start(t_data *table, t_philo *philos);
void		*philosopher(void *arg);
long long	get_current_time_ms(void);
int			mutex_initialization(t_data *table, t_philo *philo);
int			init_simulation(t_data *table, t_philo *philo);
int			av_config(char **av, t_data *table);
void		ft_sleep(long long ms);
void		take_forks(t_philo *philo);
void		put_forks(t_philo *philo);
void		philo_enough_food(t_data *table);
int			cleanup(t_philo *philos, int error, int flag, int count);
int			ft_atoi(char *str);
int			arg_ctrl(char *str);
int			check_simulation_stop(t_data *table);
void		set_simulation_stop(t_data *table);
void		safe_print(t_data *table, int id, char *message, char *color);
int         thread_start_v2(t_data *table);
int         philosopher_v2(t_philo *philo, t_data *table, int id);
int         double_philosopher(t_philo *philo, t_data *table, int left_fork, int right_fork);
int         single_phiolosopher(t_philo *philo, t_data *table, int left_fork, int right_fork);
void 		one_philosopher(t_data *table , t_philo *philo, int left_fork);
int			str_contains(const char *str, const char *substr);
int 		check_philo_death(t_data *table, int i);
int			init_destroy(t_data *table, int flag, int count);
int 		try_take_forks(t_philo *philo);







#endif
