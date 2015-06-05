/*
 * multi_timer.c - Timer multiplexer using TimerOne library
 * 
 * Allows multiple timers to be queued to the one hardware timer.
 * Callback functions can then be executed upon timer expiry. 
 * 
 * 
 * Author: Fergus William Leahy
 * 
 */
#include "multi_timer.h"
#include <Arduino.h>
#include <TimerOne.h>

/* Useful values macros */
#define UNSET_TIMER -1
#define NUM_OF_TIMERS 10
#define MAX_TIME_S 8 /* Timer limit is 8.3....s */
#define ONE_SECOND 1000000 /* One second in microseconds */
#define MAX_TIME_MICRO_S (MAX_TIME_S * ONE_SECOND)

/* Helper function macros */
#define enable_interrupts() sei()
#define disable_interrupts() cli()
#define has_timer_expired(now, time) (time->timer <= now)
#define is_expired(time) (time->timer == 0)

typedef struct timer {
	unsigned long timer;
	int immediate;
	void *value; /* values to pass to callback */
	void (*callback)(void *);
	struct timer *next;
}Timer;

static Timer timers[NUM_OF_TIMERS]; /* Array of timers */
Timer sentinel;
Timer *freeList;
static int32_t next_timer = MAX_TIME_MICRO_S; /* Next time till expiry */
Timer *timer_iter = NULL; /* Iterator for running expired timers */
static uint8_t expired_timers = 0; /* Count of timers that have expired */

/* Interrupt service routine for timer expiry */
void timer_ISR() {
	timer_iter = sentinel.next;
	unsigned long now = micros();
	for (Timer *t = sentinel.next; t != NULL; t = t->next) {
		if (has_timer_expired(now, t)) { /* Check if now expired */
			if (t->immediate) {
				t->callback(t->value);
				t->next = freeList;
				freeList = t;
			}
			else {
				expired_timers++;
				t->timer = 0;
			}
		}
		else {
			next_timer = (t->timer < MAX_TIME_MICRO_S ? 
										t->timer : MAX_TIME_MICRO_S);
			break;
		}
	}
	Timer1.setPeriod(next_timer);
}

void mt_init() {
	pinMode(4, OUTPUT); /* Pin 4 is used for timer interrupt */
	freeList = timers;
	for (int i = 1; i < NUM_OF_TIMERS; i++) {
		timers[i-1].next = &(timers[i]);
	}
	Timer1.initialize(MAX_TIME_MICRO_S);
	Timer1.attachInterrupt(timer_ISR);
}

Timer *mt_set_timer(double timer, int immediate, void(*callback)(void *), void *func_val) {
	Timer *newT, *t;
	if (freeList != NULL) {
		newT = freeList;
		freeList = newT->next;
		newT->timer = timer * ONE_SECOND + micros(); /* Timer is in microseconds */
		newT->value = func_val; /* Store value to pass to callback func */
		newT->callback = callback;
		newT->immediate = immediate;

		for (t = &sentinel; t->next != NULL && newT->timer > t->next->timer; t = t->next)
			; /* Find next slot to insert timer in order */
		newT->next = t->next;
		t->next = newT;

		if (newT->timer < next_timer) { 
			next_timer = newT->timer; /* Check if less than next timer */
			Timer1.setPeriod(timer * ONE_SECOND);
		}
		return newT;
	}
	return NULL; /* No space in queue */
}

void mt_remove_timer(Timer *t) {
	Timer *it;
	for (it = &sentinel; it->next != NULL && it->next != t; it = it->next)
		; /* Find next slot to insert timer in order */
	it->next = t->next;
	t->next = freeList;
	freeList = t;
}

int mt_timer_expired() {
	return expired_timers;
}

int mt_run_next_expired_timer() {
	disable_interrupts();
	for (;timer_iter != NULL; timer_iter = timer_iter->next) {
		if (is_expired(timer_iter)) {
			Timer *to_run = timer_iter; /* Save timer incase of later interrupt */
			timer_iter->next = freeList;
			freeList = timer_iter;
			expired_timers--;
			enable_interrupts(); 
			/* Enable interrupts from now incase of long callback func */
			to_run->callback(to_run->value);
			return 1;
		}
	enable_interrupts();
	return 0;
	}
}

void mt_run_all_expired_timers() {
	while (expired_timers) {
		mt_run_next_expired_timer();
	}
}