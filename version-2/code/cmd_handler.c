/**************************************************************************
 *
 * Author:
 * 					Ashkan Moatamed
 *
 *
 *
 * cmd_handler.c
 *
 *
 *
 * Implementation of the command handler
 * thread routine and its related functions
 * defined in alarm_app.h
 *
 *************************************************************************/

/* Declare variables and function prototypes specified in alarm_app.h */
#include "alarm_app.h"

/*
 * The command handler thread routine.
 *
 * Precondition: arg can be safely casted into (pthread_t *).
 *
 * Returns: arg
 */
void * cmd_handler(void *arg) {
	/* Save the current thread(command handler thread)'s ID. */
	const pthread_t tid = *((pthread_t *) arg);
	const uint_fast64_t id = (uint_fast64_t) tid;



	/* Command type A pointer used for iterating over the commands list. */
	CmdA *curr_cmda = NULL;
	/* Alarm pointer used for iterating over the alarms list. */
	Alarm *curr_alarm = NULL;
	/*
	 * Alarm pointer used to save the pointer to the previous
	 * node in the global alarms list when removing nodes from
	 * the list as a result of an appropriate type C command.
	 */
	Alarm *prev_alarm = NULL;
	/*
	 * Pointer to the thread ID of the alarm_handler thread which
	 * has been cancelled as a result of an appropriate type A or
	 * type C command.
	 */
	pthread_t *handler_id = NULL;
	/* Command type B pointer used for iterating over the commands list. */
	CmdB *curr_cmdb = NULL;
	/* Command type C pointer used for iterating over the commands list. */
	CmdC *curr_cmdc = NULL;
	/* Stores the message number of the alarm to be cancelled. */
	uint_fast32_t cancel_msg_num = 0;



	/* Stores the return status of functions. */
	int status = 0;



	/* Dummy variable used when setting the cancel state of this thread. */
	int old_state = 0;



	/*
	 * Set up this thread's cleanup routines to be called
	 * when it is inevitably cancelled by the main thread.
	 */
	pthread_cleanup_push(cleanup_cmd_handler, (void *) (&status));



	/*
	 * Infinite loop to process new commands read from the commands list.
	 *
	 * Since the entire body of the while loop is a critical section, break
	 * it down into multiple parts by obtaining and releasing locks for each
	 * part. This way, the command handler thread creates a much smaller
	 * bottleneck on the application by allowing other threads to obtain locks.
	 *
	 * Furthermore, use a few goto statements in order to short circuit its
	 * execution allowing it to bypass processing commands of a given type, if
	 * it knows there are no commands of that type.
	 *
	 * At any point, the command handler thread knows that there is at least one
	 * command that it has to execute but due to the above decision to break up
	 * its critical section, it would have to obtain and release locks multiple
	 * times before it can start processing type C commands. This would mean that
	 * if there are only type C commands to execute, the initial lock obtaining
	 * and releasing was just a waste of resources thus creating the need for
	 * short circuiting.
	 */
	while (true) {
		/* Lock new_cmd_insert_mutex. */
		status = pthread_mutex_lock(&new_cmd_insert_mutex);
		if (status != 0) {
			EXIT_ERR(MUTEX_LOCK_ERR_MSG, MUTEX_LOCK_ERR);
		}

		/*
		 * Wait for the main thread to signal new_cmd_insert_cond_var signifying
		 * that there has been at least one new (valid) command in the commands
		 * list waiting to be executed. This causes the command handler thread
		 * to be only running when it actually has to perform some actions and
		 * not wasting resources when there is nothing to do.
		 *
		 * A conditional wait (whether timed or not) is a cancellation point.
		 * When the cancelability enable state of a thread is set to
		 * PTHREAD_CANCEL_DEFERRED, a side effect of acting upon a cancellation
		 * request while in a conditional wait is that the mutex is (in effect)
		 * re-acquired before calling the first cancellation cleanup handler.
		 * The effect is as if the thread were unblocked, allowed to execute up
		 * to the point of returning from the call to pthread_cond_timedwait()
		 * or pthread_cond_wait(), but at that point notices the cancellation
		 * request and instead of returning to the caller of pthread_cond_timedwait()
		 * or pthread_cond_wait(), starts the thread cancellation activities,
		 * which includes calling cancellation cleanup handlers.
		 *
		 * When using conditional variables, there is always a boolean predicate
		 * involving shared variables associated with each conditional wait that
		 * is true if the thread should proceed. Spurious wakeups from
		 * pthread_cond_timedwait() or pthread_cond_wait() functions may occur.
		 * Since the return from pthread_cond_timedwait() or pthread_cond_wait()
		 * does not imply anything about the value of the predicate, the predicate
		 * should be re-evaluated upon such return.
		 *
		 * Mesa-style implies while loop. Hoare-style implies if statement.
		 */
		while ((cmda_list_head == NULL) && (cmdb_list_new_elm == NULL) && (cmdc_list_head == NULL)) {
			status = pthread_cond_wait(&new_cmd_insert_cond_var, &new_cmd_insert_mutex);
			if (status != 0) {
				EXIT_ERR(COND_VAR_WAIT_ERR_MSG, COND_VAR_WAIT_ERR);
			}
		} /* (cmda_list_head != NULL) || (cmdb_list_new_elm != NULL) || (cmdc_list_head != NULL) */

		/* Unlock new_cmd_insert_mutex. */
		status = pthread_mutex_unlock(&new_cmd_insert_mutex);
		if (status != 0) {
			EXIT_ERR(MUTEX_UNLOCK_ERR_MSG, MUTEX_UNLOCK_ERR);
		}

		/* Check if there are any pending cancellation requests. */
		pthread_testcancel(); /* pthread_testcancel() never fails. */



		/* Obtain all necessary locks. */
		cmd_handler_obtain_locks(&old_state);



		/* Short circuit execution. */
		if (cmda_list_head == NULL) {
			/* No type A commands to execute. */
			if (cmdb_list_new_elm == NULL) {
				/*
				 * No type B commands to execute.
				 *
				 * We must therefore have: (cmdc_list_head != NULL)
				 */
				goto TYPE_C_AFTER_OBTAIN_LOCKS;
			} else { /* (cmdb_list_new_elm != NULL) */
				goto TYPE_B_AFTER_OBTAIN_LOCKS;
			}
		} /* (cmda_list_head != NULL) */



		/* Lock alarm_cancel_mutex for potential conditional wait. */
		status = pthread_mutex_lock(&alarm_cancel_mutex);
		if (status != 0) {
			EXIT_ERR(MUTEX_LOCK_ERR_MSG, MUTEX_LOCK_ERR);
		}

		/*
		 * Critical Section Part 1:
		 * Read the global commands list and execute all type A commands.
		 */
		while (cmda_list_head != NULL) {
			/* Allocate memory for the new alarm node. */
			curr_alarm = MALLOC(Alarm);
			if (curr_alarm == NULL) {
				EXIT_ERR(ALLOC_ALARM_ERR_MSG, ALLOC_ALARM_ERR);
			}

			/* Initialize the new alarm node's attributes. */
			curr_alarm->link = NULL; curr_alarm->link_handle = NULL;
			curr_alarm->wait_time = cmda_list_head->wait_time;
			curr_alarm->msg_type = cmda_list_head->msg_type;
			curr_alarm->msg_num = cmda_list_head->msg_num;
			strcpy(curr_alarm->msg, cmda_list_head->msg); /* Set curr_alarm's message. */
			curr_alarm->is_assigned = false;
			curr_alarm->handler_id = NULL;
			curr_alarm->is_replaced = false;
			curr_alarm->is_cancelled = false;

			/*
			 * Save the current first command A node in curr_cmda.
			 *
			 * Move the head to the next command A node.
			 *
			 * Detach and free the command A node saved in curr_cmda.
			 */
			curr_cmda = cmda_list_head;
			cmda_list_head = cmda_list_head->link;
			curr_cmda->link = NULL;
			free(curr_cmda);

			/*
			 * Insert the new alarm pointed to by curr_alarm into the global
			 * alarms list in sorted order using the insert_alarm method.
			 */
			handler_id = insert_alarm(&alarm_list_head, curr_alarm, next_alarm,
						insert_first_alarm, insert_after_alarm,
						true, &alarm_cancel_cond_var, &alarm_cancel_mutex);

			/* Print status message informing the user of the internal state. */
			fprintf(app_log, "Alarm with message type = %" PRIuFAST32 \
						" and message number = %" PRIuFAST32 \
						" inserted by Command thread with ID = %" \
						PRIuFAST64 " into the alarms list at %" PRIuFAST64 \
						".\n", curr_alarm->msg_type, curr_alarm->msg_num, id, now());

			/*
			 * (handler_id != NULL) implies (alarm handler
			 * thread with ID *handler_id has self terminated)
			 */
			if (handler_id != NULL) {
				/*
				 * Free allocated resources by joining with
				 * alarm handler thread and freeing memory.
				 */
				cmd_handler_join_with_alarm_handler(handler_id);
			} /* (handler_id == NULL) */
		}
		cmda_list_tail = NULL; /* Update type A commands list tail. */
		errno = 0;

		/* Unlock alarm_cancel_mutex. */
		status = pthread_mutex_unlock(&alarm_cancel_mutex);
		if (status != 0) {
			EXIT_ERR(MUTEX_UNLOCK_ERR_MSG, MUTEX_UNLOCK_ERR);
		}



		/* Short circuit execution. */
		if (cmdb_list_new_elm == NULL) {
			/* No type B commands to execute. */
			if (cmdc_list_head == NULL) {
				/*
				 * The only way we could get here
				 * is that if there was at least one
				 * type A command to execute but no
				 * type B or type C commands to execute
				 * so just release all locks and restart
				 * the processing.
				 */
				goto TYPE_C_BEFORE_RELEASE_LOCKS;
			} else { /* (cmdc_list_head != NULL) */
				goto TYPE_B_BEFORE_RELEASE_LOCKS;
			}
		} /* (cmdb_list_new_elm != NULL) */



		/* Release all necessary locks. */
		cmd_handler_release_locks(&old_state);



		/* Give other threads a chance to obtain locks. */



		/* Obtain all necessary locks. */
		cmd_handler_obtain_locks(&old_state);
TYPE_B_AFTER_OBTAIN_LOCKS:



		/*
		 * Critical Section Part 2:
		 * Read the global commands list and execute all type B commands.
		 */
		for (curr_cmdb = cmdb_list_new_elm; curr_cmdb != NULL; curr_cmdb = curr_cmdb->link) {
			/* assert(!curr_cmdb->is_processed) */

			/* Set the command's state to PROCESSED. */
			curr_cmdb->is_processed = true;

			/* Create the alarm handler thread. */
			status = pthread_create(&(curr_cmdb->id), NULL,
						alarm_handler, (void *) (&(curr_cmdb->msg_type)));
			if (status != 0) {
				EXIT_ERR(THREAD_CREATE_ERR_MSG, THREAD_CREATE_ERR);
			}

			/* Print status message informing the user of the internal state. */
			fprintf(app_log, "New Alarm thread with ID = %" PRIuFAST64 \
						" for message type = %" PRIuFAST32 \
						" created by Command thread with ID = %" \
						PRIuFAST64 " at %" PRIuFAST64 ".\n",
						(uint_fast64_t) curr_cmdb->id,
						curr_cmdb->msg_type, id, now());
		}
		cmdb_list_new_elm = NULL;



		/* Short circuit execution. */
		if (cmdc_list_head == NULL) {
			/*
			 * The only way we could get here
			 * is that there was at least one
			 * type B command that needed to be
			 * executed but there were no type C
			 * commands to be executed so just
			 * release all locks and restart the
			 * processing.
			 */
			goto TYPE_C_BEFORE_RELEASE_LOCKS;
		} /* (cmdc_list_head != NULL) */



TYPE_B_BEFORE_RELEASE_LOCKS:
		/* Release all necessary locks. */
		cmd_handler_release_locks(&old_state);



		/* Give other threads a chance to obtain locks. */



		/* Obtain all necessary locks. */
		cmd_handler_obtain_locks(&old_state);
TYPE_C_AFTER_OBTAIN_LOCKS:



		/* Lock alarm_cancel_mutex for potential conditional wait. */
		status = pthread_mutex_lock(&alarm_cancel_mutex);
		if (status != 0) {
			EXIT_ERR(MUTEX_LOCK_ERR_MSG, MUTEX_LOCK_ERR);
		}

		/*
		 * Critical Section Part 3:
		 * Read the global commands list and execute all type C commands.
		 */
		while (cmdc_list_head != NULL) {
			/*
			 * Save the current first command C node in curr_cmdc.
			 *
			 * Move the head to the next command C node.
			 *
			 * Save the message number of the alarm that should be cancelled.
			 *
			 * Detach and free the command C node saved in curr_cmdc.
			 */
			curr_cmdc = cmdc_list_head;
			cmdc_list_head = cmdc_list_head->link;
			cancel_msg_num = curr_cmdc->msg_num;
			curr_cmdc->link = NULL;
			free(curr_cmdc);

			/*
			 * Find the corresponding alarm node and
			 * remove it from the alarms list while
			 * also freeing all allocated resources.
			 */
			prev_alarm = NULL;
			for (curr_alarm = alarm_list_head; curr_alarm != NULL; /* Update inside. */) {
				if (curr_alarm->msg_num == cancel_msg_num) {
					/* Remove the alarm from the global alarms list. */
					if (curr_alarm == alarm_list_head) {
						/* The current alarm is the first alarm in the list. */

						/* Update the head pointer of the list. */
						alarm_list_head = alarm_list_head->link;
					} else { /* (curr_alarm != alarm_list_head) */
						/* The current alarm is NOT the first alarm in the list. */

						/* Make the prev node point to the next node in the list. */
						prev_alarm->link = curr_alarm->link;
					}

					/*
					 * Set the cancelled flag for the element
					 * saved in curr_alarm and remove it from
					 * the global alarms list.
					 */
					curr_alarm->link = NULL;
					curr_alarm->is_cancelled = true;

					/* Terminate the searching for loop. */
					break;
				} else { /* (curr_alarm->msg_num != cancel_msg_num) */
					/* Move to the next element. */
					prev_alarm = curr_alarm;
					curr_alarm = curr_alarm->link;
				}
			}

			/*
			 * At this point we know the following:
			 * 		1. curr_alarm != NULL
			 * 		2. curr_alarm->link == NULL
			 * 		3. curr_alarm->is_cancelled
			 */

			/* Reset handler_id to NULL. */
			handler_id = NULL;

			/*
			 * We only have to worry about the
			 * cancellation of an assigned alarm.
			 */
			if (curr_alarm->is_assigned) {
				/*
				 * We know that the alarm handler thread is
				 * going to cancel itself since the current
				 * alarm is being handled(is_assigned) and no
				 * other alarm is being handled(link_handle == NULL)
				 * by this alarm handler thread.
				 */
				if (curr_alarm->link_handle == NULL) {
					/*
					 * Save the alarm handler thread's ID so
					 * that we can find the corresponding type
					 * B command node, remove it from the
					 * commands list and free all allocated
					 * resources.
					 */
					handler_id = curr_alarm->handler_id;
				}

				/*
				 * Block this thread(command handler) until another thread(alarm handler) signals
				 * the appropriate conditional variable(alarm_cancel_cond_var). While this thread
				 * is blocked, the mutex(alarm_cancel_mutex) is released, then re-aquired before
				 * this thread is woken up and the call returns.
				 *
				 * We do not need to release the writer lock on the alarms list since while this
				 * thread is blocked, the main thread will also be blocked but since the alarm
				 * handler thread that set the is_assigned of the Alarm structure pointed to by
				 * curr_alarm is not blocked, it will eventually signal the conditional variable
				 * which will wake this thread back up.
				 *
				 * The reason for this is that the alarm handler threads only obtain a reader lock
				 * in the beginning of their creation and pick out all alarms that they are handling
				 * and from then on, they just loop to print the previously picked out alarms.
				 *
				 * When using conditional variables, there is always a boolean predicate involving
				 * shared variables associated with each conditional wait that is true if the thread
				 * should proceed. Spurious wakeups from pthread_cond_timedwait() or pthread_cond_wait()
				 * functions may occur. Since the return from pthread_cond_timedwait() or
				 * pthread_cond_wait() does not imply anything about the value of the predicate, the
				 * predicate should be re-evaluated upon such return.
				 *
				 * Mesa-style implies while loop. Hoare-style implies if statement.
				 */
				while (curr_alarm->is_assigned) {
					status = pthread_cond_wait(&alarm_cancel_cond_var, &alarm_cancel_mutex);
					if (status != 0) {
						EXIT_ERR(COND_VAR_WAIT_ERR_MSG, COND_VAR_WAIT_ERR);
					}
				} /* (!curr_alarm->is_assigned) */
			}

			/* Print status message informing the user of the internal state. */
			fprintf(app_log, "Alarm with message type = %" PRIuFAST32 \
						" and message number = %" PRIuFAST32 \
						" has been deleted from the alarms list by Command thread with ID = %" \
						PRIuFAST64 " at %" PRIuFAST64 ".\n",
						curr_alarm->msg_type, curr_alarm->msg_num, id, now());

			/*
			 * If the alarm was assigned to an alarm handler thread,
			 * then its link_handle attribute has been set to NULL by
			 * the thread's cleanup routine.
			 *
			 * If the alarm was unassigned then its link_handle
			 * attribute was initialized to NULL and has remained so.
			 *
			 * Therefore, at this point, we should be able to safely
			 * free the Alarm structure pointed to by curr_alarm.
			 */
			free(curr_alarm);

			/*
			 * (handler_id != NULL) implies (alarm handler
			 * thread with ID *handler_id has self terminated)
			 */
			if (handler_id != NULL) {
				/*
				 * Free allocated resources by joining with
				 * alarm handler thread and freeing memory.
				 */
				cmd_handler_join_with_alarm_handler(handler_id);
			} /* (handler_id == NULL) */
		}
		cmdc_list_tail = NULL; /* Update type C commands list tail. */

		/* Unlock alarm_cancel_mutex. */
		status = pthread_mutex_unlock(&alarm_cancel_mutex);
		if (status != 0) {
			EXIT_ERR(MUTEX_UNLOCK_ERR_MSG, MUTEX_UNLOCK_ERR);
		}



TYPE_C_BEFORE_RELEASE_LOCKS:
		/* Release all necessary locks. */
		cmd_handler_release_locks(&old_state);
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
	 * reached since the command handler thread is meant to be cancelled
	 * by the main thread which is when the cleanup routines will be
	 * actually popped and executed.
	 */
	pthread_cleanup_pop(1);



	/* This return will never be reached. */
	return arg;
}

/*
 * The only invocations of this function should be from the command handler thread.
 *
 * If handler_id is NULL then do nothing and just return 0 to the caller.
 *
 * If handler_id is not NULL then join with the alarm handler thread with
 * ID *handler_id and release all allocated resources and return the alarm
 * handler thread's message type which is guaranteed to be non-zero.
 */
uint_fast32_t cmd_handler_join_with_alarm_handler(pthread_t *handler_id) {
	/* Stores the return value of the current method. */
	uint_fast32_t result = 0;



	/* Command type B pointer used for iterating over the commands list. */
	CmdB *curr_cmdb = NULL;
	/*
	 * Command type B pointer used to save the pointer to the previous
	 * node in the global commands list when removing nodes from the list
	 * as a result of an appropriate type A or type C command.
	 */
	CmdB *prev_cmdb = NULL;



	/* Return to caller immediately if handler_id is NULL. */
	if (handler_id == NULL) { return 0; }



	/*
	 * Find the corresponding type B command node
	 * and remove it from the commands list while
	 * also freeing all allocated resources.
	 */
	prev_cmdb = NULL;
	for (curr_cmdb = cmdb_list_head; curr_cmdb != NULL; /* Update inside. */) {
		/* pthread_equal never fails. */
		if ((curr_cmdb->is_processed) &&
			(pthread_equal(curr_cmdb->id, *handler_id) != 0)) {

			/* Remove the command from the global commands list. */
			if (curr_cmdb == cmdb_list_head) {
				/* The current command is the first command in the list. */

				/*
				 * Update the tail pointer of the list if
				 * there is only 1 element in the list.
				 */
				if (cmdb_list_head == cmdb_list_tail) {
					cmdb_list_tail = NULL;
				}
				/* Update the head pointer of the list. */
				cmdb_list_head = cmdb_list_head->link;

				/* Detach the element saved in curr_cmdb. */
				curr_cmdb->link = NULL;

				/*
				 * No need to update curr_cmdb since we are going to
				 * terminate the entire loop with a break statement.
				 *
				 * Update:
				 * curr_cmdb = cmdb_list_head;
				 */
			} else { /* (curr_cmdb != cmdb_list_head) */
				/* The current command is NOT the first command in the list. */

				/* Make the prev node point to the next node in the list. */
				prev_cmdb->link = curr_cmdb->link;

				/*
				 * Update the tail pointer of the list if
				 * the node being removed is the last
				 * node in the list.
				 */
				if (curr_cmdb == cmdb_list_tail) {
					cmdb_list_tail = prev_cmdb;
				}

				/* Detach the element saved in curr_cmdb. */
				curr_cmdb->link = NULL;

				/*
				 * No need to update curr_cmdb since we are going to
				 * terminate the entire loop with a break statement.
				 *
				 * Update:
				 * curr_cmdb = prev_cmdb->link;
				 */
			}

			/* Terminate the searching for loop. */
			break;
		} else { /* (!curr_cmdb->is_processed) || (pthread_equal(curr_cmdb->id, *handler_id) == 0) */
			/* Move to the next element. */
			prev_cmdb = curr_cmdb;
			curr_cmdb = curr_cmdb->link;
		}
	}

	/*
	 * At this point we know the following:
	 * 		1. curr_cmdb != NULL
	 * 		2. curr_cmdb->is_processed
	 * 		3. pthread_equal(curr_cmdb->id, *handler_id)
	 *
	 * Join with the alarm handler thread
	 * to free the allocated resources. We
	 * do not care about its return value
	 * which is why we pass NULL as the
	 * second argument to pthread_join.
	 */

	/* Save the alarm handler thread's message type. */
	result = curr_cmdb->msg_type;

	/* Print status message informing the user of the internal state. */
	fprintf(app_log, "Command thread with ID = %" PRIuFAST64 \
				" is joining with Alarm thread with ID = %" \
				PRIuFAST64 " handling alarms with message type = %" \
				PRIuFAST32 " at %" PRIuFAST64 ".\n",
				(uint_fast64_t) pthread_self(), (uint_fast64_t) curr_cmdb->id,
				result, now());

	/* Join with the alarm handler thread. */
	if (pthread_join(curr_cmdb->id, NULL) != 0) {
		EXIT_ERR(THREAD_JOIN_ERR_MSG, THREAD_JOIN_ERR);
	}

	/* Free the element saved in curr_cmdb. */
	free(curr_cmdb);



	return result;
}

/*
 * The command handler thread cleanup routine.
 *
 * Precondition: arg can be safely casted into (int *).
 */
void cleanup_cmd_handler(void *arg) {
	/* Stores the return status of functions. */
	int status = *((int *) arg);
	/*
	 * The above use of arg is just so that we can avoid
	 * a -Wunused-variable warning/error during compilation
	 * otherwise the argument is indeed unused. However, we
	 * have to have it since pthread_cleanup_push requires
	 * the cleanup handler to have the above signature.
	 */



	/*
	 * The pthread_mutex_trylock() function shall be equivalent
	 * to pthread_mutex_lock(), except that if the mutex object
	 * referenced by the argument is currently locked (by any
	 * thread, including the current thread), the call shall
	 * return immediately.
	 *
	 * The pthread_mutex_trylock() function shall return zero if
	 * a lock on the mutex object referenced by the argument is
	 * acquired. Otherwise, an error number is returned to indicate
	 * the error.
	 *
	 * The pthread_mutex_trylock() function shall fail if the mutex
	 * could not be acquired because it was already locked in which
	 * case it will return EBUSY.
	 *
	 *
	 * We first invoke pthread_mutex_trylock() in an attempt to
	 * lock the mutex. Then if it was unlocked, then the function
	 * will return zero and if it was locked, it will return EBUSY.
	 *
	 * Since the only thread that actually locks new_cmd_insert_mutex
	 * and/or alarm_cancel_mutex is the command handler thread itself,
	 * then we can indeed attempt to unlock the mutexes in either of
	 * the two situations described above.
	 */



	/* Guarantee that new_cmd_insert_mutex is unlocked. */
	status = pthread_mutex_trylock(&new_cmd_insert_mutex);
	if ((status != 0) && (status != EBUSY)) {
		EXIT_ERR(MUTEX_TRYLOCK_ERR_MSG, MUTEX_TRYLOCK_ERR);
	}
	status = pthread_mutex_unlock(&new_cmd_insert_mutex);
	if (status != 0) {
		EXIT_ERR(MUTEX_UNLOCK_ERR_MSG, MUTEX_UNLOCK_ERR);
	}

	/* Guarantee that alarm_cancel_mutex is unlocked. */
	status = pthread_mutex_trylock(&alarm_cancel_mutex);
	if ((status != 0) && (status != EBUSY)) {
		EXIT_ERR(MUTEX_TRYLOCK_ERR_MSG, MUTEX_TRYLOCK_ERR);
	}
	status = pthread_mutex_unlock(&alarm_cancel_mutex);
	if (status != 0) {
		EXIT_ERR(MUTEX_UNLOCK_ERR_MSG, MUTEX_UNLOCK_ERR);
	}
}
