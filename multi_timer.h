/*
 * multi_timer.h - Timer multiplexer using TimerOne library
 * 
 * Allows multiple timers to be queued to the one hardware timer.
 * Callback functions can then be executed upon timer expiry. 
 * 
 * 
 * Author: Fergus William Leahy
 * 
 */
#ifndef MULTI_TIMER_H
#define MULTI_TIMER_H
#include <TimerOne.h>

typedef struct timer Timer;
/*Initialises the timer */
void mt_init();

/*
 * Adds a timer to the timer queue.
 * 
 * Parameters: timer specifies the timer duration,
 *             val is passed to the callback function upon timer expiry 
 *             and call to run_next_expired()
 * Returns NULL if queue is full, a Timer otherwise. */
Timer *mt_set_timer(double timer, int immediate, void(*callback)(void *), void *val);

/* Returns the number of timers that have expired*/
int mt_timer_expired();

/* Runs the callback function for the next expired timer, 
 * returns 1 if a function has been run, 0 otherwise */
int mt_run_next_expired_timer();

/* Runs the callback function for all expired timers */
void mt_run_all_expired_timers();

/* removes the timer referenced by the id */
void mt_remove_timer(Timer *t);

#endif /* MULTI_TIMER_H */