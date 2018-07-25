/**************************************************************************
 *
 * Author:
 * 					Ashkan Moatamed
 *
 *
 *
 * alarm_handler.c
 *
 *
 *
 * Implementation of the alarm handler
 * thread routine defined in alarm_app.h
 *
 *************************************************************************/

/* Declare variables and function prototypes specified in alarm_app.h */
#include "alarm_app.h"

/*
 * The alarm handler thread routine.
 *
 * Precondition: arg can be safely casted into (uint_fast32_t *).
 *
 * Returns: arg
 */
void * alarm_handler(void *arg) {
	/* Save the current thread(alarm handler)'s ID. */
	const uint_fast64_t id = (uint_fast64_t) pthread_self();



	/* The message type that this thread should handle. */
	const uint_fast32_t msg_type = *((uint_fast32_t *) arg);

	/*
	 * Pointer to the head of the local alarms list
	 * which this thread is currently handling.
	 */
	Alarm *handle_list_head = NULL;
	/* Alarm pointer used for iterating over the alarms list. */
	Alarm *curr_alarm = NULL;



	/*
	 * Stores the current amount of time passed since
	 * the last time the thread updated its own local
	 * alarms list. Each thread will update its list
	 * after ALARM_THREAD_UPDATE_PERIOD many seconds.
	 */
	uint_fast64_t time_since_update = 0;
	/*
	 * Stores the current amount of time passed from the thread's
	 * creation since it is incremented by 1 every second. It is
	 * used for determining which alarms to print.
	 *
	 * Note that the value will eventually reach the maximum and
	 * wrap around back to 0. We know this from the standard which
	 * states: "A computation involving unsigned operands can never
	 * overflow, because a result that cannot be represented by the
	 * resulting unsigned integer type is reduced modulo the number
	 * that is one greater than the largest value that can be
	 * represented by the resulting type."
	 *
	 * However, this is not something that we need to be concerned
	 * with since the maximum value for an unsigned integer represented
	 * in 64-bits is: 2^64 - 1 and since we increment time_since_create
	 * by 1 every second, it will need to be incremented 2^64 times for
	 * it to wrap around back to 0 but this will take 2^64 seconds at the
	 * every least(the reason for at least 2^64 seconds is that the thread
	 * is not necessarily running the entire time for one and secondly,
	 * the uint_fast64_t guarantees at least 64-bits.)
	 *
	 * 2^64 seconds is approximately 584.5 billion years therefore
	 * the design is safe enough since:
	 * 584.5 billion years is more than 100 times the expected life
	 * of our solar system which is about 5 billion years at which
	 * time, the Sun's elements will "swell" up, swallow the Earth,
	 * and eventually die off into a small white dwarf.
	 */
	uint_fast64_t time_since_create = 0;



	/* Dummy variable used when setting the cancel state of this thread. */
	int old_state = 0;



	/*
	 * Set up this thread's cleanup routines to be called
	 * when it is inevitably cancelled by the main thread.
	 */
	pthread_cleanup_push(cleanup_alarm_handler, (void *) handle_list_head);



	/*
	 * Infinite loop to update the alarms handling list,
	 * then actually handling(printing) the alarms.
	 *
	 * The thread will terminate when it is cancelled by
	 * the main thread on an appropriate type C command
	 * or at the termination of the main thread itself.
	 */
	while (true) {
		/* Disable cancellation. */
		if (pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &old_state) != 0) {
			EXIT_ERR(CANCELLATION_DISABLE_ERR_MSG, CANCELLATION_DISABLE_ERR);
		}

		/* Lock the mutex. */
		if (pthread_mutex_lock(&mutex) != 0) {
			EXIT_ERR(MUTEX_LOCK_ERR_MSG, MUTEX_LOCK_ERR);
		}

		/*
		 * Critical Section:
		 * Read the global alarms list and update
		 * the local alarms list which this thread
		 * is going to handle(i.e. print).
		 */
		for (curr_alarm = alarm_list_head; curr_alarm != NULL; curr_alarm = curr_alarm->link) {
			if ((!curr_alarm->is_assigned) &&
				(curr_alarm->msg_type == msg_type)) {

				/* Set the alarm's state to ASSIGNED. */
				curr_alarm->is_assigned = true;

				/*
				 * Insert the alarm pointed to by curr_alarm into the local
				 * alarms list in sorted order using the insert_alarm method.
				 */
				insert_alarm(&handle_list_head, curr_alarm, next_handled_alarm,
							insert_first_handled_alarm, insert_after_handled_alarm);

				/* Print status message informing the user of the internal state. */
				fprintf(app_log, "Alarm with message type = %" PRIuFAST32 \
							" has been assigned to Alarm thread with ID = %" \
							PRIuFAST64 " at %" PRIuFAST64 ".\n", msg_type, id, now());
			} else if (curr_alarm->msg_type > msg_type) {
				/*
				 * The alarms list is sorted by message types first
				 * therefore if the current alarm that we are looking at,
				 * has a larger message type than the one we are looking
				 * for, then it is simply not possible to find any more
				 * alarms that we have to handle.
				 */
				break;
			}
		}

		/* Unlock the mutex. */
		if (pthread_mutex_unlock(&mutex) != 0) {
			EXIT_ERR(MUTEX_UNLOCK_ERR_MSG, MUTEX_UNLOCK_ERR);
		}

		/* Enable cancellation. */
		if (pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &old_state) != 0) {
			EXIT_ERR(CANCELLATION_ENABLE_ERR_MSG, CANCELLATION_ENABLE_ERR);
		}

		/* Check if there are any pending cancellation requests. */
		pthread_testcancel(); /* pthread_testcancel() never fails. */



		/*
		 * If no alarms have been selected then just yield the CPU
		 * to another thread that has been readied for execution.
		 *
		 * If any alarms have been selected then loop to print all
		 * of the assigned alarms' messages until the printing period
		 * is reached and then yield the CPU to another thread that
		 * has been readied for execution.
		 */
		if (handle_list_head != NULL) {
			/* (handle_list_head != NULL) implies that the list is NOT empty. */
			for (time_since_update = 0; time_since_update != ALARM_THREAD_UPDATE_PERIOD; /* Update inside. */) {
				sleep(1); /* Sleep for 1 second. */
				time_since_create++; time_since_update++;

				/* Loop over all assigned alarms and print the appropriate ones. */
				for (curr_alarm = handle_list_head; curr_alarm != NULL; curr_alarm = curr_alarm->link_handle) {
					/*
					 * If the current alarm that we are looking at, has a wait time
					 * that is a divisor of time_since_create, then it means that it
					 * should be printed. The reason is that time_since_create is
					 * incremented by 1 every second therefore in the given situation,
					 * it means that the proper amount of time(the alarm's wait_time
					 * seconds) has passed since the last time the alarm's message was
					 * printed.
					 */
					if ((time_since_create % curr_alarm->wait_time) == 0) {
						fprintf(app_log, "Alarm with message type = %" PRIuFAST32 \
									" being printed by Alarm thread with ID = %" PRIuFAST64 \
									" at %" PRIuFAST64 ".\nAlarm message: |%s|\n",
									msg_type, id, now(), curr_alarm->msg);
					}
				}
			}
		}

		/* In the Linux implementation, sched_yield() always succeeds. */
		if (sched_yield() != 0) {
			EXIT_ERR(CPU_YIELD_ERR_MSG, CPU_YIELD_ERR);
		}
	}



	/*
	 * POSIX.1 permits pthread_cleanup_push() and pthread_cleanup_pop()
	 * to be implemented as macros that expand to text containing
	 * '{' and '}', respectively. For this reason, the caller must ensure
	 * that calls to these functions are paired within the same function,
	 * and at the same lexical nesting level. In other words, a cleanup
	 * handler is established only during the execution of a specified
	 * section of code.
	 *
	 * Therefore, the following call is needed to pair the call to
	 * pthread_cleanup_push made in the beginning of this thread's
	 * execution.
	 *
	 * The pop is however placed in a section of code that will never be
	 * reached since every alarm handling thread is meant to be cancelled
	 * by the main thread which is when the cleanup routines will be
	 * actually popped and executed.
	 */
	pthread_cleanup_pop(1);



	/* This return will never be reached. */
	return arg;
}

/*
 * The alarm handler thread cleanup routine.
 *
 * Precondition: arg can be safely casted into (Alarm *).
 */
void cleanup_alarm_handler(void *arg) {
	/* Head of the alarms list which the thread was handling. */
	Alarm *handle_list_head = (Alarm *) arg;
	/* Alarm pointer used for iterating over the alarms list. */
	Alarm *curr_alarm = NULL;

	/*
	 * No need to lock mutex since the link_handle attribute
	 * is indeed part of the global shared data and so different
	 * alarm handler threads can access it but based on the
	 * implementation, they will never access it. This means that
	 * the shared data is disjoint and as such it is unnecessary
	 * to lock it.
	 *
	 * This is due to the fact that alarm handler threads do not
	 * even consider alarms that this thread is handling since they
	 * all have their is_assigned attribute set to true.
	 *
	 * Disjoint data does not require locking due to its nature.
	 */

	/*
	 * Set each node's link_handle attribute in the thread's alarms
	 * list to NULL(i.e. detach them to prevent segmentation fault).
	 */
	while (handle_list_head != NULL) {
		/* Save the current first element. */
		curr_alarm = handle_list_head;
		/* Move to the next element. */
		handle_list_head = handle_list_head->link_handle;
		/* Detach the element saved in curr_alarm. */
		curr_alarm->link_handle = NULL;
	}
}
