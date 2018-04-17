/*
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
 */

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



	/* Command type A pointer used for iterating over the commands list. */
	CmdA *curr_cmda = NULL;
	/* Command type B pointer used for iterating over the commands list. */
	CmdB *curr_cmdb = NULL;
	/* Command type C pointer used for iterating over the commands list. */
	CmdC *curr_cmdc = NULL;
	/* Alarm pointer used for iterating over the alarms list. */
	Alarm *curr_alarm = NULL;



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
		(data.err.val == SEM_WAIT_ERR) ||
		(data.err.val == SEM_SIGNAL_ERR) ||
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



	/* Attempt to cancel the command handler thread. */
	if (data.mode > CMD_THREAD_CREATE_FAIL) {
		status = cancel_thread(data.cmd_thread_tid);
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
	}



	/* Free memory allocated to the type A commands list. */
	while (cmda_list_head != NULL) {
		/* Save the current first element. */
		curr_cmda = cmda_list_head;
		/* Move to the next element. */
		cmda_list_head = cmda_list_head->link;

		/* Detach and free the element saved in curr_cmda. */
		curr_cmda->link = NULL;
		free(curr_cmda);
	}
	cmda_list_tail = NULL;

	/* Free memory allocated to the type C commands list. */
	while (cmdc_list_head != NULL) {
		/* Save the current first element. */
		curr_cmdc = cmdc_list_head;
		/* Move to the next element. */
		cmdc_list_head = cmdc_list_head->link;

		/* Detach and free the element saved in curr_cmdc. */
		curr_cmdc->link = NULL;
		free(curr_cmdc);
	}
	cmdc_list_tail = NULL;

	/*
	 * Free memory allocated to the type B commands list
	 * while also cancelling all alarm handler threads.
	 */
	while (cmdb_list_head != NULL) {
		/* Save the current first element. */
		curr_cmdb = cmdb_list_head;
		/* Move to the next element. */
		cmdb_list_head = cmdb_list_head->link;

		/* We only have to worry about processed type B commands. */
		if (curr_cmdb->is_processed) {
			status = cancel_thread(curr_cmdb->id);
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
		}

		/* Detach and free the element saved in curr_cmdb. */
		curr_cmdb->link = NULL;
		free(curr_cmdb);
	}
	cmdb_list_tail = NULL;



	/*
	 * No need to obtain any locks since all other threads
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



	/* Destroy cmd_mutex. */
	status = pthread_mutex_destroy(&cmd_mutex);
	if (status != 0) {
		data.err.linenum = __LINE__;
		data.err.val = MUTEX_DESTROY_ERR; data.err.msg = MUTEX_DESTROY_ERR_MSG;
		exit_main(data);
	}



	/* Destroy new_cmd_insert_mutex. */
	status = pthread_mutex_destroy(&new_cmd_insert_mutex);
	if (status != 0) {
		data.err.linenum = __LINE__;
		data.err.val = MUTEX_DESTROY_ERR; data.err.msg = MUTEX_DESTROY_ERR_MSG;
		exit_main(data);
	}

	/* Destroy new_cmd_insert_cond_var. */
	status = pthread_cond_destroy(&new_cmd_insert_cond_var);
	if (status != 0) {
		data.err.linenum = __LINE__;
		data.err.val = COND_VAR_DESTROY_ERR; data.err.msg = COND_VAR_DESTROY_ERR_MSG;
		exit_main(data);
	}



	/* Attempt to destroy alarm_rw_bin_sem. */
	if (data.mode > ALARM_RW_BIN_SEM_INIT_FAIL) {
		status = sem_destroy(&alarm_rw_bin_sem);
		if (status != 0) {
			data.err.linenum = __LINE__;
			data.err.val = SEM_DESTROY_ERR; data.err.msg = SEM_DESTROY_ERR_MSG;
			exit_main(data);
		}
	}

	/* Attempt to destroy alarm_r_bin_sem. */
	if (data.mode > ALARM_R_BIN_SEM_INIT_FAIL) {
		status = sem_destroy(&alarm_r_bin_sem);
		if (status != 0) {
			data.err.linenum = __LINE__;
			data.err.val = SEM_DESTROY_ERR; data.err.msg = SEM_DESTROY_ERR_MSG;
			exit_main(data);
		}
	}



	/* Destroy alarm_cancel_mutex. */
	status = pthread_mutex_destroy(&alarm_cancel_mutex);
	if (status != 0) {
		data.err.linenum = __LINE__;
		data.err.val = MUTEX_DESTROY_ERR; data.err.msg = MUTEX_DESTROY_ERR_MSG;
		exit_main(data);
	}

	/* Destroy alarm_cancel_cond_var. */
	status = pthread_cond_destroy(&alarm_cancel_cond_var);
	if (status != 0) {
		data.err.linenum = __LINE__;
		data.err.val = COND_VAR_DESTROY_ERR; data.err.msg = COND_VAR_DESTROY_ERR_MSG;
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
