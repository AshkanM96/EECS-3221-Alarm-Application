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
	pthread_t tid = pthread_self();
	const uint_fast64_t id = (uint_fast64_t) tid;



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
	 * Alarm pointer used to save the pointer to the previous
	 * node in the local alarms list when removing replaced or
	 * cancelled alarms from the list.
	 */
	Alarm *prev_alarm = NULL;



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
	 * when it is cancelled by the main thread when it is
	 * terminating or it self terminates in which case the
	 * cleanup routine does nothing.
	 */
	pthread_cleanup_push(cleanup_alarm_handler, (void *) handle_list_head);



	/* Disable cancellation. */
	if (pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &old_state) != 0) {
		EXIT_ERR(CANCELLATION_DISABLE_ERR_MSG, CANCELLATION_DISABLE_ERR);
	}

	/* Obtain reader lock. */
	obtain_alarm_read_lock(NULL);

	/*
	 * Critical Section:
	 * Read the global alarms list and update the local
	 * alarms list which this thread is going to handle.
	 *
	 *
	 * The reason why we only require a reader lock on the
	 * global alarms list is that we are not changing any
	 * globally shared data on the design level. The link_handle
	 * attribute just allow us to embed an alarm handler's
	 * local list into the global list for faster runtime and
	 * also memory allocation efficiency.
	 *
	 * The is_assigned flag makes it so that different alarm
	 * handler threads do not even attempt to access alarms that
	 * have already been assigned.
	 *
	 * Message types(msg_type) further help us in making the data
	 * disjoint by only allowing alarm handlers to manage alarms
	 * that have the same type as them.
	 *
	 * Finally, since we enforce unique threads of a given message
	 * type(this is enforced by the main and command handler threads),
	 * we arrive at the following conclusion: The local lists are
	 * accessible globally but remain solely accessed by the owning
	 * alarm handler thread.
	 *
	 *
	 * However, note that even without one of the above, we may not
	 * be able to guarantee that a reader lock would be enough.
	 * For example:
	 * 		If we allowed multiple threads of a given type, then there
	 * 		would actually be an issue with the reader lock since many
	 * 		alarm handler threads of a given type could find an unassigned
	 * 		Alarm structure of their desired type at the same time and
	 * 		all of them would be able to overwrite each other's selection
	 * 		process.
	 */
	for (curr_alarm = alarm_list_head; curr_alarm != NULL; curr_alarm = curr_alarm->link) {
		if ((!curr_alarm->is_assigned) &&
			(curr_alarm->msg_type == msg_type)) {

			/* Set the alarm's state to ASSIGNED. */
			curr_alarm->is_assigned = true;
			/* Set the alarm's handler ID pointer. */
			curr_alarm->handler_id = &tid;

			/*
			 * Insert the alarm pointed to by curr_alarm into the local
			 * alarms list in sorted order using the insert_alarm method.
			 *
			 * The return of the following invocation will always be NULL.
			 */
			insert_alarm(&handle_list_head, curr_alarm, next_handled_alarm,
						insert_first_handled_alarm, insert_after_handled_alarm,
						false, NULL, NULL);

			/* Print status message informing the user of the internal state. */
			fprintf(app_log, "Alarm with message type = %" PRIuFAST32 \
						" and message number = %" PRIuFAST32 \
						" has been assigned to Alarm thread with ID = %" \
						PRIuFAST64 " at %" PRIuFAST64 ".\n", msg_type,
						curr_alarm->msg_num, id, now());
		}
	}

	/* Release reader lock. */
	release_alarm_read_lock(NULL);

	/* Enable cancellation. */
	if (pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &old_state) != 0) {
		EXIT_ERR(CANCELLATION_ENABLE_ERR_MSG, CANCELLATION_ENABLE_ERR);
	}

	/* Check if there are any pending cancellation requests. */
	pthread_testcancel(); /* pthread_testcancel() never fails. */



	/*
	 * Infinite loop to update the alarms handling list,
	 * then actually handling(printing) the alarms.
	 *
	 * This thread will terminate when all of its alarms
	 * have been replaced or cancelled by the command handler
	 * thread on appropriate type A or type C commands or
	 * at the termination of the main thread itself.
	 *
	 * We know that the thread's local alarms list is not
	 * empty(handle_list_head != NULL) in the first iteration
	 * of the loop since this is enforced by the main and
	 * command handler threads before creating a new alarm
	 * handler thread.
	 *
	 * Every iteration, also check if any alarms have been
	 * replaced or cancelled by an appropriate type A or
	 * type C command and if so, then remove them from the
	 * thread's list and signal the alarm cancel conditional
	 * variable signifying that the alarm has been cancelled
	 * so that the command handler thread can be woken up.
	 */
	while (true) {
		sleep(1); /* Sleep for 1 second. */
		time_since_create++;

		/* Loop over all assigned alarms and print the appropriate ones. */
		prev_alarm = NULL;
		for (curr_alarm = handle_list_head; curr_alarm != NULL; /* Update inside. */) {
			/*
			 * First check to make sure that the alarm we are looking at, has not
			 * been replaced or cancelled by the command handler thread before
			 * attempting to print its message.
			 */
			if ((curr_alarm->is_replaced) || (curr_alarm->is_cancelled)) {
				/* Print status message informing the user of the internal state. */
				fprintf(app_log, "Alarm thread with ID = %" PRIuFAST64 \
							" stopped printing %s alarm with message type = %" \
							PRIuFAST32 " and message number = %" PRIuFAST32 \
							" at %" PRIuFAST64 ".\n", id,
							(curr_alarm->is_replaced ? "replaced" : "cancelled"),
							msg_type, curr_alarm->msg_num, now());

				/* Remove the alarm from the thread's local alarms list. */
				if (curr_alarm == handle_list_head) {
					/* The current alarm is the first alarm in the list. */

					/* Update the head pointer of the list. */
					handle_list_head = handle_list_head->link_handle;

					/* Detach and reset the element saved in curr_alarm. */
					curr_alarm->link_handle = NULL;
					curr_alarm->is_assigned = false;
					curr_alarm->handler_id = NULL;

					/* Update curr_alarm. */
					curr_alarm = handle_list_head;
				} else { /* (curr_alarm != handle_list_head) */
					/* The current alarm is NOT the first alarm in the list. */

					/* Make the prev node point to the next node in the list. */
					prev_alarm->link_handle = curr_alarm->link_handle;

					/* Detach and reset the element saved in curr_alarm. */
					curr_alarm->link_handle = NULL;
					curr_alarm->is_assigned = false;
					curr_alarm->handler_id = NULL;

					/* Update curr_alarm. */
					curr_alarm = prev_alarm->link_handle;
				}

				/* Signal alarm_cancel_cond_var. */
				if (pthread_cond_signal(&alarm_cancel_cond_var) != 0) {
					EXIT_ERR(COND_VAR_SIGNAL_ERR_MSG, COND_VAR_SIGNAL_ERR);
				}
				/* In the Linux implementation, sched_yield() always succeeds. */
				if (sched_yield() != 0) {
					EXIT_ERR(CPU_YIELD_ERR_MSG, CPU_YIELD_ERR);
				}
			} else { /* (!curr_alarm->is_replaced) && (!curr_alarm->is_cancelled) */
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
								" and message number = %" PRIuFAST32 \
								" being printed by Alarm thread with ID = %" \
								PRIuFAST64 " at %" PRIuFAST64 ".\nAlarm message: |%s|\n",
								msg_type, curr_alarm->msg_num, id, now(), curr_alarm->msg);
				}

				/* Move to the next element. */
				prev_alarm = curr_alarm;
				curr_alarm = curr_alarm->link_handle;
			}
		}

		/* Check to see if this thread is still handling any alarms. */
		if (handle_list_head == NULL) {
			/* Print status message informing the user of the internal state. */
			fprintf(app_log, "Alarm thread with ID = %" PRIuFAST64 \
						" handling alarms with message type = %" \
						PRIuFAST32 " is self terminating at %" \
						PRIuFAST64 ".\n", id, msg_type, now());

			/* Terminate and cleanup this thread. */
			pthread_exit(arg);
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
	 * No need to obtain any locks since all accessed parts of the
	 * shared data are not going to be accessed by any other alarm
	 * handler threads and certainly not by the main or command handler
	 * threads since they are both blocked.
	 *
	 * This is due to the fact that alarm handler threads of a given
	 * message type are unique and so no other alarm handler thread
	 * will even consider Alarms of the message type that this thread
	 * is handling even if they were not assigned(which they also are).
	 *
	 * Disjoint data does not require locking due to its nature.
	 */

	/*
	 * Set each node's link_handle attribute in the thread's alarms
	 * list to NULL(i.e. detach them to prevent segmentation fault),
	 * is_assigned to false, and finally handler_id to NULL as well.
	 */
	while (handle_list_head != NULL) {
		/* Save the current first element. */
		curr_alarm = handle_list_head;
		/* Move to the next element. */
		handle_list_head = handle_list_head->link_handle;

		/*
		 * Remove the element saved in curr_alarm
		 * from the thread's handled alarms list.
		 */
		curr_alarm->link_handle = NULL;
		curr_alarm->is_assigned = false;
		curr_alarm->handler_id = NULL;
	}
}
