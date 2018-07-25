/**************************************************************************
 *
 * Author:
 * 					Ashkan Moatamed
 *
 *
 *
 * cleanup.c
 *
 *
 *
 * Implementation of the Cleanup
 * Functions defined in alarm_app.h
 *
 *************************************************************************/

/* Declare variables and function prototypes specified in alarm_app.h */
#include "alarm_app.h"

/*
 * The main thread's cleanup routine.
 *
 * Cleanup the main thread by freeing all allocated memory,
 * cancelling all active threads and in general releasing all
 * resources acquired during its life cycle.
 *
 * Precondition: arg can be safely casted into (MLData *).
 */
void cleanup_main(void *arg) {
	/* Main thread's local data. */
	MLData data = *((MLData *) arg);



	/* Alarm pointer used for iterating over the alarms list. */
	Alarm *curr_alarm = NULL;
	/* Thread pointer used for iterating over the threads list. */
	Thread *curr_thread = NULL;



	/* Stores the return status of functions. */
	int status = 0;



	/* Print any potential errors and then reset data.err. */
	print_error(data.err);
	data.err.filename = __FILE__; data.err.linenum = 0;
	data.err.val = 0; data.err.msg = "";



	/* Free memory allocated to data.line. */
	free(data.line);



	/* Check main_result against extreme errors. */
	if ((data.err.val == MUTEX_LOCK_ERR) ||
		(data.err.val == MUTEX_UNLOCK_ERR) ||
		(data.err.val == THREAD_CANCEL_ERR) ||
		(data.err.val == THREAD_JOIN_ERR) ||
		(data.err.val == THREAD_CANCEL_RETVAL_ERR) ||
		(data.err.val == CANCELLATION_ENABLE_ERR) ||
		(data.err.val == CANCELLATION_DISABLE_ERR)) {

		/*
		 * Error scenarios too extreme that even
		 * the cleanup would not be possible since
		 * it would require to perform some number
		 * of the already failed operations such as:
		 * cancelling threads
		 */
		data.err.linenum = __LINE__;
		exit_main(data);
	}



	/* Free memory allocated to the (local) threads list. */
	while (data.thread_list_head != NULL) {
		/* Save the current first element. */
		curr_thread = data.thread_list_head;
		/* Move to the next element. */
		data.thread_list_head = data.thread_list_head->link;

		/* Attempt to cancel the thread. */
		status = cancel_thread(curr_thread->id);
		if (status != 0) {
			data.err.linenum = __LINE__;
			data.err.val = status;
			if (status == THREAD_JOIN_ERR) {
				data.err.msg = THREAD_JOIN_ERR_MSG;
			} else if (status == THREAD_CANCEL_ERR) {
				data.err.msg = THREAD_CANCEL_ERR_MSG;
			} else { /* (status == THREAD_CANCEL_RETVAL_ERR) */
				data.err.msg = THREAD_CANCEL_RETVAL_ERR_MSG;
			}
			exit_main(data);
		}

		/* Detach and free the element saved in curr_thread. */
		curr_thread->link = NULL;
		free(curr_thread);
	}

	/*
	 * No need to lock the mutex since all other threads
	 * have been cancelled and as such there is no need
	 * to create synchronization.
	 */

	/* Free memory allocated to the (global) alarms list. */
	while (alarm_list_head != NULL) {
		/* Save the current first element. */
		curr_alarm = alarm_list_head;
		/* Move to the next element. */
		alarm_list_head = alarm_list_head->link;

		/*
		 * link_handle attributes have been set to NULL
		 * by the alarm handler thread cleanup routine
		 * therefore we only have to worry about the link
		 * attribute which connects nodes in the global
		 * alarms list.
		 */

		/* Detach and free the element saved in curr_alarm. */
		curr_alarm->link = NULL;
		free(curr_alarm);
	}



	/* Destroy the mutex. */
	status = pthread_mutex_destroy(&mutex);
	if (status != 0) {
		data.err.linenum = __LINE__;
		data.err.val = MUTEX_DESTROY_ERR; data.err.msg = MUTEX_DESTROY_ERR_MSG;
		exit_main(data);
	}



	/* Cleanup main thread and terminate. */
	data.err.linenum = __LINE__;
	exit_main(data);
}

/* End the main thread and print the error message if needed. */
void exit_main(MLData data) {
	/*
	 * Print a newline('\n') at the end of
	 * stdout for more aesthetic pleasure!
	 */
	printf("\n");



	/* Print any potential errors. */
	print_error(data.err);



	/* Flush the output and close the application log file. */
	if (app_log != NULL) {
		/* Flush app_log. */
		if (fflush(app_log) != 0) { EXIT_ERRNO(FFLUSH_ERR_MSG); }

		#ifdef APP_LOG_FILE
			/* data.separate_log_file == (app_log != stdout) */
			if (data.separate_log_file) {
				/* We know that app_log != stdout so flush stdout. */
				if (fflush(stdout) != 0) { EXIT_ERRNO(FFLUSH_ERR_MSG); }

				/* Close app_log. */
				if (fclose(app_log) != 0) { EXIT_ERRNO(FCLOSE_ERR_MSG); }
			} /* (!data.separate_log_file) == (app_log == stdout) */
		#endif
	}



	/* Terminate process. */
	exit(data.err.val);
}
