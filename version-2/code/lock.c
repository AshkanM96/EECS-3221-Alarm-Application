/**************************************************************************
 *
 * Author:
 * 					Ashkan Moatamed
 *
 *
 *
 * lock.c
 *
 *
 *
 * Implementation of the Lock Obtaining
 * and Releasing Functions defined in
 * alarm_app.h
 *
 *************************************************************************/

/* Declare variables and function prototypes specified in alarm_app.h */
#include "alarm_app.h"

/*
 * Obtain a reader lock on the global
 * alarms list for the calling thread.
 */
void obtain_alarm_read_lock(MLData *data_ptr) {
	/* Obtain semaphore reader lock. */
	if (sem_wait(&alarm_r_bin_sem) != 0) {
		if (data_ptr != NULL) {
			/* Cleanup main thread and terminate. */
			print_error(data_ptr->err);
			data_ptr->err.filename = __FILE__; data_ptr->err.linenum = __LINE__;
			data_ptr->err.val = SEM_WAIT_ERR; data_ptr->err.msg = SEM_WAIT_ERR_MSG;
			pthread_exit(data_ptr);
		} else { /* (data_ptr == NULL) */
			/* Terminate process since caller was NOT the main thread. */
			EXIT_ERR(SEM_WAIT_ERR_MSG, SEM_WAIT_ERR);
		}
	}

	/* Wait on writers if we are the first reader. */
	if (++reader_count == 1) {
		if (sem_wait(&alarm_rw_bin_sem) != 0) {
			if (data_ptr != NULL) {
				/* Cleanup main thread and terminate. */
				print_error(data_ptr->err);
				data_ptr->err.filename = __FILE__; data_ptr->err.linenum = __LINE__;
				data_ptr->err.val = SEM_WAIT_ERR; data_ptr->err.msg = SEM_WAIT_ERR_MSG;
				pthread_exit(data_ptr);
			} else { /* (data_ptr == NULL) */
				/* Terminate process since caller was NOT the main thread. */
				EXIT_ERR(SEM_WAIT_ERR_MSG, SEM_WAIT_ERR);
			}
		}
	}

	/* Release semaphore reader lock so that other readers can also continue. */
	if (sem_post(&alarm_r_bin_sem) != 0) {
		if (data_ptr != NULL) {
			/* Cleanup main thread and terminate. */
			print_error(data_ptr->err);
			data_ptr->err.filename = __FILE__; data_ptr->err.linenum = __LINE__;
			data_ptr->err.val = SEM_SIGNAL_ERR; data_ptr->err.msg = SEM_SIGNAL_ERR_MSG;
			pthread_exit(data_ptr);
		} else { /* (data_ptr == NULL) */
			/* Terminate process since caller was NOT the main thread. */
			EXIT_ERR(SEM_SIGNAL_ERR_MSG, SEM_SIGNAL_ERR);
		}
	}
}

/*
 * Release previously obtained reader lock on
 * the global alarms list by the calling thread.
 */
void release_alarm_read_lock(MLData *data_ptr) {
	/* Obtain semaphore reader lock. */
	if (sem_wait(&alarm_r_bin_sem) != 0) {
		if (data_ptr != NULL) {
			/* Cleanup main thread and terminate. */
			print_error(data_ptr->err);
			data_ptr->err.filename = __FILE__; data_ptr->err.linenum = __LINE__;
			data_ptr->err.val = SEM_WAIT_ERR; data_ptr->err.msg = SEM_WAIT_ERR_MSG;
			pthread_exit(data_ptr);
		} else { /* (data_ptr == NULL) */
			/* Terminate process since caller was NOT the main thread. */
			EXIT_ERR(SEM_WAIT_ERR_MSG, SEM_WAIT_ERR);
		}
	}

	/* Signal writers so that they can also continue if we are the last reader. */
	if (--reader_count == 0) {
		if (sem_post(&alarm_rw_bin_sem) != 0) {
			if (data_ptr != NULL) {
				/* Cleanup main thread and terminate. */
				print_error(data_ptr->err);
				data_ptr->err.filename = __FILE__; data_ptr->err.linenum = __LINE__;
				data_ptr->err.val = SEM_SIGNAL_ERR; data_ptr->err.msg = SEM_SIGNAL_ERR_MSG;
				pthread_exit(data_ptr);
			} else { /* (data_ptr == NULL) */
				/* Terminate process since caller was NOT the main thread. */
				EXIT_ERR(SEM_SIGNAL_ERR_MSG, SEM_SIGNAL_ERR);
			}
		}
	}

	/* Release semaphore reader lock so that other readers can also continue. */
	if (sem_post(&alarm_r_bin_sem) != 0) {
		if (data_ptr != NULL) {
			/* Cleanup main thread and terminate. */
			print_error(data_ptr->err);
			data_ptr->err.filename = __FILE__; data_ptr->err.linenum = __LINE__;
			data_ptr->err.val = SEM_SIGNAL_ERR; data_ptr->err.msg = SEM_SIGNAL_ERR_MSG;
			pthread_exit(data_ptr);
		} else { /* (data_ptr == NULL) */
			/* Terminate process since caller was NOT the main thread. */
			EXIT_ERR(SEM_SIGNAL_ERR_MSG, SEM_SIGNAL_ERR);
		}
	}
}



/*
 * The only invocations of this function should be from the command handler thread.
 *
 * Disable cancellation for the caller and then obtain all necessary locks.
 */
void cmd_handler_obtain_locks(int *old_state_ptr) {
	/* Disable cancellation. */
	if (pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, old_state_ptr) != 0) {
		EXIT_ERR(CANCELLATION_DISABLE_ERR_MSG, CANCELLATION_DISABLE_ERR);
	}

	/*
	 * The command handler thread, first processes all type A commands.
	 * To do this, it is going to need to access both the commands list
	 * and then alarms list as a writer. This means that it has to obtain
	 * a writer lock on the alarms list but also lock cmd_mutex.
	 * Therefore, consider the following strategies:
	 *
	 * 1. Command handler thread first locks cmd_mutex and then obtains a
	 * writer lock on the alarms list.
	 *
	 * 2. Command handler thread first obtains a writer lock on the alarms
	 * list and then locks cmd_mutex.
	 *
	 *
	 *
	 * The first of the two startegies can lead to a very bad performance
	 * in the following example(while the second strategy does not):
	 *
	 * 		Step 1: The command handler thread attempts to lock cmd_mutex
	 * 			but is blocked since the main thread already owns the lock.
	 *
	 * 		Step 2: The main thread eventually finishes its current action
	 * 			and unlocks the cmd_mutex thus allowing the command handler
	 * 			thread to obtain the lock on cmd_mutex while the main thread
	 * 			is waiting for user I/O(next command to be entered).
	 *
	 * 		Step 3: The command handler thread attempts to obtain a writer
	 * 			lock on the alarms list but is blocked since many alarm handler
	 * 			threads have obtained reader locks.
	 *
	 * 		Step 4: The main thread now attempts to lock cmd_mutex but is
	 * 			blocked since the command handler thread already owns the lock.
	 *
	 *
	 * Explanation: In the above scenario, we can see that both application
	 * critical threads(the main and the command handler threads) have been
	 * blocked and are going to have to wait for an unknown amount of time until
	 * all alarm handler threads have released their reader locks on the alarms
	 * list. At this point, the command handler thread can be allowed to obtain
	 * a writer lock and continue till it eventually releases both of its obtained
	 * locks thus allowing the application to move forward.
	 *
	 * In the second strategy however, only the command handler thread would be
	 * blocked until it could obtain a writer lock while the main thread would be
	 * free to execute. Note that when the command handler thread obtains a writer
	 * lock, the main thread will be blocked regardless.
	 */

	/* Obtain writer lock. */
	if (sem_wait(&alarm_rw_bin_sem) != 0) {
		EXIT_ERR(SEM_WAIT_ERR_MSG, SEM_WAIT_ERR);
	}

	/* Lock cmd_mutex. */
	if (pthread_mutex_lock(&cmd_mutex) != 0) {
		EXIT_ERR(MUTEX_LOCK_ERR_MSG, MUTEX_LOCK_ERR);
	}
}

/*
 * The only invocations of this function should be from the command handler thread.
 *
 * Release all necessary locks in the opposite order of obtaining them when
 * invoking cmd_handler_obtain_locks and then enable cancellation for the
 * caller. Finally check if there are any pending cancellation requests.
 */
void cmd_handler_release_locks(int *old_state_ptr) {
	/* Unlock cmd_mutex. */
	if (pthread_mutex_unlock(&cmd_mutex) != 0) {
		EXIT_ERR(MUTEX_UNLOCK_ERR_MSG, MUTEX_UNLOCK_ERR);
	}

	/* Release writer lock. */
	if (sem_post(&alarm_rw_bin_sem) != 0) {
		EXIT_ERR(SEM_SIGNAL_ERR_MSG, SEM_SIGNAL_ERR);
	}

	/* Enable cancellation. */
	if (pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, old_state_ptr) != 0) {
		EXIT_ERR(CANCELLATION_ENABLE_ERR_MSG, CANCELLATION_ENABLE_ERR);
	}

	/* Check if there are any pending cancellation requests. */
	pthread_testcancel(); /* pthread_testcancel() never fails. */



	/*
	 * Yield the CPU to another thread that has been readied for execution.
	 *
	 * In the Linux implementation, sched_yield() always succeeds.
	 */
	if (sched_yield() != 0) {
		EXIT_ERR(CPU_YIELD_ERR_MSG, CPU_YIELD_ERR);
	}
}
