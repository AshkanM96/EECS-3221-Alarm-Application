/**************************************************************************
 *
 * Author:
 * 					Ashkan Moatamed
 *
 *
 *
 * alarm_app.c
 *
 *
 *
 * The main thread reads input from stdin until EOF.
 * The input has to be of one of the following types
 * or it will be ignored as an invalid input.
 *
 *
 * The command types are as follows:
 * Type A: Time MessageType(Type) AlarmMessage
 * Type B: Create_Thread: MessageType(Type)
 * Type C: Terminate: MessageType(Type)
 *
 * Where MessageType, Create_Thread, and Terminate are
 * all reserved keywords by the program.
 *
 * Time is an unsigned number denoting the amount of time
 * to wait between two consecutive prints of the alarm.
 *
 * Type is an unsigned number used to categorize different alarms.
 *
 * And finally AlarmMessage is a non-empty string of maximum length
 * MAX_MSG_LEN. Note that longer strings will be truncated to MAX_MSG_LEN
 * characters. This restriction on MAX_MSG_LEN characters can easily be
 * modified by changing #define MAX_MSG_LEN 50 in alarm_def.h.
 *
 *
 * Type A commands, produce a new alarm to be added to the
 * alarms list for later processing.
 *
 * Type B commands, create a new thread to handle all outstanding
 * alarms of the given type.
 *
 * Type C commands, terminate all threads and remove all messages
 * of the given type.
 *
 *************************************************************************/

/* Define variables and function prototypes specified in alarm_app.h */
#define DEFINE_VARIABLES_AND_FUNCTIONS
#include "alarm_app.h"



/*
 * The main thread ignores all command-line arguments
 * hence its signature is int main(void) instead of:
 * int main(int argc, const char *argv[])
 */
int main(void) {
	/* Save the current thread(main thread)'s ID. */
	const uint_fast64_t id = (uint_fast64_t) pthread_self();



	/* Stores the length of the read line. */
	size_t len = 0;
	/* Stores the next read line of input. */
	char *line = NULL;

	/* Temporary variables used to parse commands. */
	char *tmp_str = NULL;
	size_t i = 0, msg_start = 0, msg_end = 0, msg_max_end = 0;



	/*
	 * Is the application log file separate from the
	 * standard output stream(stdout).
	 *
	 * i.e. separate_log_file == (app_log != stdout)
	 */
	bool separate_log_file = false;



	/*
	 * Alarm pointer used for iterating over the alarms list
	 * and also to store new alarms created by the user.
	 */
	Alarm *curr_alarm = NULL;

	/* The new alarm's wait time. */
	uint_fast32_t wait_time = 0;
	/* Used for safely reading wait_time as a uint_fast32_t number. */
	int_fast64_t l_wait_time = 0;
	/* The new alarm's message type. */
	uint_fast32_t msg_type = 0;
	/* Used for safely reading msg_type as a uint_fast32_t number. */
	int_fast64_t l_msg_type = 0;
	/*
	 * msg has maximum length MAX_MSG_LEN + 1 since it has to be
	 * able to store MAX_MSG_LEN many chars and 1 nullchar('\0').
	 */
	char msg[MAX_MSG_LEN + 1];



	/* Pointer to the head of the threads singly-linked-list. */
	Thread *thread_list_head = NULL;
	/*
	 * Thread pointer used for iterating over the threads list
	 * and also to store new threads created by the user.
	 */
	Thread *curr_thread = NULL;



	/*
	 * Thread pointer used to save the pointer to the previous
	 * node in the local threads list when removing nodes from
	 * the list in Type C command executions.
	 */
	Thread *prev_thread = NULL;
	/*
	 * Alarm pointer used to save the pointer to the previous
	 * node in the global alarms list when removing alarms from
	 * the list in Type C command executions.
	 */
	Alarm *prev_alarm = NULL;



	/* Stores the return status of functions. */
	int status = 0;



	/* Encapsulates all of main thread's local data needed during cleanup. */
	MLData data;



	/*
	 * Set up this thread's cleanup routines to
	 * be called when it is abruptly cancelled.
	 */
	pthread_cleanup_push(cleanup_main, (void *) (&data));



	/*
	 * Initialize data but update each attribute
	 * when corresponding attribute is being updated.
	 */
	data.err.filename = __FILE__; data.err.linenum = 0;
	data.err.val = 0; data.err.msg = "";
	data.separate_log_file = separate_log_file;
	data.line = line;
	data.thread_list_head = thread_list_head;



	app_log = stdout; /* Initialize app_log. */
	#ifdef APP_LOG_FILE
		printf("Do you want to save the application log to a file named %s? (y/n) ", APP_LOG_FILE);
		status = read_line(fgetc, stdin, &len, &line, NULL); data.line = line;
		if (status == -2) {
			/* Cleanup main thread and terminate. */
			data.err.linenum = __LINE__;
			data.err.val = STREAM_ERR; data.err.msg = STREAM_ERR_MSG;
			pthread_exit(&data);
		} else if (status == -1) {
			/* Cleanup main thread and terminate. */
			data.err.linenum = __LINE__;
			data.err.val = ALLOC_STR_ERR; data.err.msg = ALLOC_STR_ERR_MSG;
			pthread_exit(&data);
		} else if (status == 1) {
			/* EOF has been reached therefore proceed to cleanup. */
			data.err.linenum = __LINE__;
			pthread_exit(&data);
		}
		/*
		 * (status == 2) implies that the user entered a very
		 * long line of input and we have only read some of it
		 * but that can still be parsed and processed so continue
		 * on with the processing.
		 */

		/* Parse the user's answer. */
		if ((len == 1) || (len == 3)) {
			if ((strcmp(line, "y") == 0) ||
				(strcmp(line, "Y") == 0) ||
				(strcmp(line, "yes") == 0) ||
				(strcmp(line, "Yes") == 0) ||
				(strcmp(line, "YES") == 0)) {

				/* Attempt to open/create a new application log file. */
				errno = 0;
				app_log = fopen(APP_LOG_FILE, "w+");
				if ((app_log == NULL) || (errno != 0)) {
					app_log = NULL;
					data.err.linenum = __LINE__;
					data.err.val = FOPEN_ERR; data.err.msg = FOPEN_ERR_MSG;
					pthread_exit(&data);
				} /* (app_log != NULL) && (errno == 0) */
				data.separate_log_file = separate_log_file = true;
			}
		}

		printf("\n-------------------------\n\n");
	#endif



	/* Inform the user of some basic application rules. */
	printf("Input should be of one of the following formats:\n");
	printf("Time MessageType(Type) AlarmMessage\n");
	printf("Create_Thread: MessageType(Type)\n");
	printf("Terminate: MessageType(Type)\n");

	printf("\nWhere MessageType, Create_Thread, and Terminate are\n");
	printf("all reserved keywords by the program.\n");

	printf("\nTime is an unsigned number denoting the amount of time\n");
	printf("to wait between two consecutive prints of the alarm.\n");

	printf("\nType is an unsigned number used to categorize different alarms.\n");

	printf("\nAnd finally AlarmMessage is a non-empty string of maximum length %d.\n", MAX_MSG_LEN);
	printf("Note that longer strings will be truncated to %d characters.\n", MAX_MSG_LEN);

	printf("\n\nType A commands, produce a new alarm to be added to the\n");
	printf("alarms list for later processing.\n");

	printf("\nType B commands, create a new thread to handle all\n");
	printf("outstanding alarms of the given type.\n");

	printf("\nType C commands, terminate all threads and remove all\n");
	printf("messages of the given type.\n");

	if (sizeof(uint_fast32_t) < sizeof(int_fast64_t)) {
		printf("\n\nAn unsigned number is an integer in the following range:\n[0, %" \
					PRIuFAST32 "]\n", UINT_FAST32_MAX);
	} else { /* sizeof(uint_fast32_t) >= sizeof(int_fast64_t) */
		printf("\n\nAn unsigned number is an integer in the following range:\n[0, %" \
					PRIdFAST64 "]\n", INT_FAST64_MAX);
	}

	printf("\n\nAll times are given in seconds since the UNIX Epoch.\n\n\n");

	#ifdef APP_LOG_FILE
		if (separate_log_file) {
			printf("Application log messages will be printed to %s\n\n\n", APP_LOG_FILE);
		}
	#endif



	/* Infinite loop to process new commands read from stdin. */
	while (true) {
		/* The application prompt. */
		printf("Alarm> ");

		/* Read the next line of input from stdin. */
		status = read_line(fgetc, stdin, &len, &line, NULL); data.line = line;
		if (status == -2) {
			/* Cleanup main thread and terminate. */
			data.err.linenum = __LINE__;
			data.err.val = STREAM_ERR; data.err.msg = STREAM_ERR_MSG;
			pthread_exit(&data);
		} else if (status == -1) {
			/* Cleanup main thread and terminate. */
			data.err.linenum = __LINE__;
			data.err.val = ALLOC_STR_ERR; data.err.msg = ALLOC_STR_ERR_MSG;
			pthread_exit(&data);
		} else if (status == 1) {
			/* EOF has been reached therefore proceed to cleanup. */
			data.err.linenum = __LINE__;
			pthread_exit(&data);
		}
		/*
		 * (status == 2) implies that the user entered a very
		 * long line of input and we have only read some of it
		 * but that can still be parsed and processed so continue
		 * on with the processing.
		 */



		/* Process the read command stored in line. */
		if (len < 2) {
			/*
			 * Input line has length 0 or 1 which means that
			 * it cannot possibly be a valid command so reset
			 * the line and read the next command.
			 */
			fprintf(stderr, "The read command is invalid since its length is less than 2.\nCommand: |%s|\n", line);
			goto RESET_AND_READ_NEXT_LINE;
		}



		if (sscanf(line, "%" SCNdFAST64 " MessageType(%" SCNdFAST64 ") ", &l_wait_time, &l_msg_type) == 2) {
			/* Type A */

			/* Parse l_wait_time as a uint_fast32_t number. */
			errno = 0;
			wait_time = f64_to_uf32(l_wait_time);
			if (errno != 0) {
				/* l_wait_time is not a valid uint_fast32_t number. */
				fprintf(stderr, "The given type A command is invalid since the given Time is not a valid unsigned number.\nCommand: |%s|\n", line);
				goto RESET_AND_READ_NEXT_LINE;
			} else if (wait_time == 0) {
				/* wait_time should be positive. */
				fprintf(stderr, "The given type A command is invalid since the given Time is 0.\nCommand: |%s|\n", line);
				goto RESET_AND_READ_NEXT_LINE;
			}

			/* Parse l_msg_type as a uint_fast32_t number. */
			errno = 0;
			msg_type = f64_to_uf32(l_msg_type);
			if (errno != 0) {
				/* l_msg_type is not a valid uint_fast32_t number. */
				fprintf(stderr, "The given type A command is invalid since the given Type is not a valid unsigned number.\nCommand: |%s|\n", line);
				goto RESET_AND_READ_NEXT_LINE;
			} else if (msg_type == 0) {
				/* msg_type should be positive. */
				fprintf(stderr, "The given type A command is invalid since the given Type is 0.\nCommand: |%s|\n", line);
				goto RESET_AND_READ_NEXT_LINE;
			}



			/* Allocate and fill tmp_str. */
			tmp_str = MALLOC_ARRAY(char, len);
			if (tmp_str == NULL) {
				/* Cleanup main thread and terminate. */
				data.err.linenum = __LINE__;
				data.err.val = ALLOC_STR_ERR; data.err.msg = ALLOC_STR_ERR_MSG;
				pthread_exit(&data);
			}
			sprintf(tmp_str, "%" PRIuFAST32 " MessageType(%" PRIuFAST32 ") ", wait_time, msg_type);

			/*
			 * Find msg from line using tmp_str's length.
			 *
			 * msg's 1st index == tmp_str's last index + 1
			 * == (strlen(tmp_str) - 1) + 1 == strlen(tmp_str)
			 */
			msg_start = strlen(tmp_str);
			/*
			 * msg ends either MAX_MSG_LEN many more chars after it starts
			 * or it ends where the line ends which happens when the line
			 * is shorter thus eliminating the need for truncation.
			 *
			 * Truncation is done by taking the minimum of the two possible
			 * lengths for msg and ignoring the rest of the chars in line.
			 */
			msg_max_end = msg_start + MAX_MSG_LEN;
			msg_end = MIN(msg_max_end, len);

			/* Copy the alarm message from line into msg. */
			for (i = msg_start; i != msg_end; ++i) {
				msg[i - msg_start] = line[i];
			}
			msg[msg_end - msg_start] = '\0'; /* Null terminate msg. */

			/* Make sure the alarm message is non-empty. */
			if (msg_start == msg_end) {
				free(tmp_str); /* Free memory allocated to tmp_str. */
				fprintf(stderr, "The given AlarmMessage is the empty string.\nCommand: |%s|\n", line);
				goto RESET_AND_READ_NEXT_LINE;
			}

			/*
			 * Validate the command type.
			 *
			 * Make sure line and tmp_str match in the first
			 * msg_start == strlen(tmp_str) many chars.
			 */
			status = 1; /* Assume its validity. */
			for (i = 0; i != msg_start; ++i) {
				if (line[i] != tmp_str[i]) {
					status = 0; /* Not valid. */
					break;
				}
			}
			free(tmp_str); /* Free memory allocated to tmp_str. */
			if (status == 0) {
				/*
				 * Command is invalid since line does not start with the same
				 * string as tmp_str. This happens in one of the following situations:
				 *
				 * 1. line has additional or not enough whitespace
				 * 2. line has redundant zeros
				 * 3. line contains an empty AlarmMessage
				 * 4. line contains at least one number that does not fit into int_fast64_t
				 *
				 * All of the above issues occur in the first msg_start( == strlen(tmp_str))
				 * many chars of line. The reason why these are not checked earlier is that
				 * the scanf family of functions including sscanf, ignore whitespace,
				 * redundant zeros, and numbers too long for a given format which is why we
				 * enforce it manually.
				 */
				fprintf(stderr, "The given type A command is invalid since it differs from the specified format before the first character of the AlarmMessage.\nCommand: |%s|\n", line);
				goto RESET_AND_READ_NEXT_LINE;
			}



			/* Allocate memory for the new alarm node. */
			curr_alarm = MALLOC(Alarm);
			if (curr_alarm == NULL) {
				/* Cleanup main thread and terminate. */
				data.err.linenum = __LINE__;
				data.err.val = ALLOC_ALARM_ERR; data.err.msg = ALLOC_ALARM_ERR_MSG;
				pthread_exit(&data);
			}

			/* Initialize the new alarm node's attributes. */
			curr_alarm->link = NULL; curr_alarm->link_handle = NULL;
			curr_alarm->wait_time = wait_time;
			curr_alarm->msg_type = msg_type;
			strcpy(curr_alarm->msg, msg); /* Set curr_alarm's message. */
			curr_alarm->is_assigned = false;

			/* Lock the mutex. */
			status = pthread_mutex_lock(&mutex);
			if (status != 0) {
				free(curr_alarm); /* Free memory allocated to curr_alarm. */

				/* Cleanup main thread and terminate. */
				data.err.linenum = __LINE__;
				data.err.val = MUTEX_LOCK_ERR; data.err.msg = MUTEX_LOCK_ERR_MSG;
				pthread_exit(&data);
			}

			/*
			 * Critical Section:
			 * Insert the new alarm pointed to by curr_alarm into the global
			 * alarms list in sorted order using the insert_alarm method.
			 */
			insert_alarm(&alarm_list_head, curr_alarm, next_alarm,
						insert_first_alarm, insert_after_alarm);

			/* Print status message informing the user of the internal state. */
			fprintf(app_log, "New alarm with message type = %" PRIuFAST32 \
						" inserted by Main thread with ID = %" PRIuFAST64 \
						" into the alarms list at %" PRIuFAST64 ".\n", msg_type, id, now());

			/* Unlock the mutex. */
			status = pthread_mutex_unlock(&mutex);
			if (status != 0) {
				/*
				 * We do not need to free memory allocated to curr_alarm
				 * since it has been successfully inserted into the global
				 * alarms list which will be freed by the main cleanup.
				 */

				/* Cleanup main thread and terminate. */
				data.err.linenum = __LINE__;
				data.err.val = MUTEX_UNLOCK_ERR; data.err.msg = MUTEX_UNLOCK_ERR_MSG;
				pthread_exit(&data);
			}
		} else if (sscanf(line, "Create_Thread: MessageType(%" SCNdFAST64 ")", &l_msg_type) == 1) {
			/* Type B */

			/* Parse l_msg_type as a uint_fast32_t number. */
			errno = 0;
			msg_type = f64_to_uf32(l_msg_type);
			if (errno != 0) {
				/* l_msg_type is not a valid uint_fast32_t number. */
				fprintf(stderr, "The given type B command is invalid since the given Type is not a valid unsigned number.\nCommand: |%s|\n", line);
				goto RESET_AND_READ_NEXT_LINE;
			} else if (msg_type == 0) {
				/* msg_type should be positive. */
				fprintf(stderr, "The given type B command is invalid since the given Type is 0.\nCommand: |%s|\n", line);
				goto RESET_AND_READ_NEXT_LINE;
			}

			/* Validate the command. */
			status = is_valid_cmd(line, len, 'B', msg_type);
			if (status == -1) {
				/* Cleanup main thread and terminate. */
				data.err.linenum = __LINE__;
				data.err.val = ALLOC_STR_ERR; data.err.msg = ALLOC_STR_ERR_MSG;
				pthread_exit(&data);
			} else if (status == 0) {
				fprintf(stderr, "The given type B command is invalid since it does not conform to the specified format.\nCommand: |%s|\n", line);
				goto RESET_AND_READ_NEXT_LINE;
			}



			/* Allocate memory for the new thread node. */
			curr_thread = MALLOC(Thread);
			if (curr_thread == NULL) {
				/* Cleanup main thread and terminate. */
				data.err.linenum = __LINE__;
				data.err.val = ALLOC_THREAD_ERR; data.err.msg = ALLOC_THREAD_ERR_MSG;
				pthread_exit(&data);
			}

			/* Initialize the new thread node's attributes. */
			curr_thread->link = NULL;
			curr_thread->msg_type = msg_type;
			status = pthread_create(&(curr_thread->id), NULL,
						alarm_handler, (void *) (&msg_type));
			if (status != 0) {
				free(curr_thread); /* Free memory allocated to curr_thread. */

				/* Cleanup main thread and terminate. */
				data.err.linenum = __LINE__;
				data.err.val = THREAD_CREATE_ERR; data.err.msg = THREAD_CREATE_ERR_MSG;
				pthread_exit(&data);
			}

			/* Add the new thread to the beginning of the (local) threads list in O(1). */
			curr_thread->link = thread_list_head;
			data.thread_list_head = thread_list_head = curr_thread;

			/* Print status message informing the user of the internal state. */
			fprintf(app_log, "New Alarm thread with ID = %" PRIuFAST64 \
						" for message type = %" PRIuFAST32 " created by Main thread with ID = %" \
						PRIuFAST64 " at %" PRIuFAST64 ".\n", (uint_fast64_t) curr_thread->id,
						msg_type, id, now());
		} else if (sscanf(line, "Terminate: MessageType(%" SCNdFAST64 ")", &l_msg_type) == 1) {
			/* Type C */

			/* Parse l_msg_type as a uint_fast32_t number. */
			errno = 0;
			msg_type = f64_to_uf32(l_msg_type);
			if (errno != 0) {
				/* l_msg_type is not a valid uint_fast32_t number. */
				fprintf(stderr, "The given type C command is invalid since the given Type is not a valid unsigned number.\nCommand: |%s|\n", line);
				goto RESET_AND_READ_NEXT_LINE;
			} else if (msg_type == 0) {
				/* msg_type should be positive. */
				fprintf(stderr, "The given type C command is invalid since the given Type is 0.\nCommand: |%s|\n", line);
				goto RESET_AND_READ_NEXT_LINE;
			}

			/* Validate the command. */
			status = is_valid_cmd(line, len, 'C', msg_type);
			if (status == -1) {
				/* Cleanup main thread and terminate. */
				data.err.linenum = __LINE__;
				data.err.val = ALLOC_STR_ERR; data.err.msg = ALLOC_STR_ERR_MSG;
				pthread_exit(&data);
			} else if (status == 0) {
				fprintf(stderr, "The given type C command is invalid since it does not conform to the specified format.\nCommand: |%s|\n", line);
				goto RESET_AND_READ_NEXT_LINE;
			}



			/* Cancel all threads handling messages of type msg_type. */
			prev_thread = NULL;
			for (curr_thread = thread_list_head; curr_thread != NULL; /* Update inside. */) {
				if (curr_thread->msg_type == msg_type) {
					/* Attempt to cancel the thread. */
					status = cancel_thread(curr_thread->id);
					if (status != 0) {
						/* Cleanup main thread and terminate. */
						data.err.linenum = __LINE__;
						data.err.val = status;
						if (status == THREAD_JOIN_ERR) {
							data.err.msg = THREAD_JOIN_ERR_MSG;
						} else if (status == THREAD_CANCEL_ERR) {
							data.err.msg = THREAD_CANCEL_ERR_MSG;
						} else { /* (status == THREAD_CANCEL_RETVAL_ERR) */
							data.err.msg = THREAD_CANCEL_RETVAL_ERR_MSG;
						}
						pthread_exit(&data);
					}

					/* Remove the thread from the local threads list. */
					if (curr_thread == thread_list_head) {
						/* The current thread is the first thread in the list. */

						/* Update the head pointer of the list. */
						data.thread_list_head = thread_list_head = thread_list_head->link;

						/* Detach and free the element saved in curr_thread. */
						curr_thread->link = NULL;
						free(curr_thread);

						/* Update curr_thread. */
						curr_thread = thread_list_head;
					} else { /* (curr_thread != thread_list_head) */
						/* The current thread is NOT the first thread in the list. */

						/* Make the prev node point to the next node in the list. */
						prev_thread->link = curr_thread->link;

						/* Detach and free the element saved in curr_thread. */
						curr_thread->link = NULL;
						free(curr_thread);

						/* Update curr_thread. */
						curr_thread = prev_thread->link;
					}
				} else { /* (curr_thread->msg_type != msg_type) */
					/* Move to the next element. */
					prev_thread = curr_thread;
					curr_thread = curr_thread->link;
				}
			}



			/* Lock the mutex. */
			status = pthread_mutex_lock(&mutex);
			if (status != 0) {
				/* Cleanup main thread and terminate. */
				data.err.linenum = __LINE__;
				data.err.val = MUTEX_LOCK_ERR; data.err.msg = MUTEX_LOCK_ERR_MSG;
				pthread_exit(&data);
			}

			/*
			 * Remove all alarms of message type msg_type from
			 * the global alarms list and free the allocated memory.
			 */
			prev_alarm = NULL;
			for (curr_alarm = alarm_list_head; curr_alarm != NULL; /* Update inside. */) {
				if (curr_alarm->msg_type == msg_type) {
					/*
					 * If the alarm was assigned to a thread, then its
					 * link_handle attribute has been set to NULL by
					 * the thread's cleanup routine.
					 *
					 * If the alarm was unassigned then its link_handle
					 * attribute was initialized to NULL and has remained so.
					 *
					 * Therefore, at this point, we only have to worry about
					 * the link attribute which connects nodes in the global
					 * alarms list.
					 */

					/* Remove the alarm from the global alarms list. */
					if (curr_alarm == alarm_list_head) {
						/* The current alarm is the first alarm in the list. */

						/* Update the head pointer of the list. */
						alarm_list_head = alarm_list_head->link;

						/* Detach and free the element saved in curr_alarm. */
						curr_alarm->link = NULL;
						free(curr_alarm);

						/* Update curr_alarm. */
						curr_alarm = alarm_list_head;
					} else { /* (curr_alarm != alarm_list_head) */
						/* The current alarm is NOT the first alarm in the list. */

						/* Make the prev node point to the next node in the list. */
						prev_alarm->link = curr_alarm->link;

						/* Detach and free the element saved in curr_alarm. */
						curr_alarm->link = NULL;
						free(curr_alarm);

						/* Update curr_alarm. */
						curr_alarm = prev_alarm->link;
					}
				} else { /* (curr_alarm->msg_type != msg_type) */
					/* Move to the next element. */
					prev_alarm = curr_alarm;
					curr_alarm = curr_alarm->link;
				}
			}

			/* Unlock the mutex. */
			status = pthread_mutex_unlock(&mutex);
			if (status != 0) {
				/* Cleanup main thread and terminate. */
				data.err.linenum = __LINE__;
				data.err.val = MUTEX_UNLOCK_ERR; data.err.msg = MUTEX_UNLOCK_ERR_MSG;
				pthread_exit(&data);
			}

			/* Print status message informing the user of the internal state. */
			fprintf(app_log, "All alarms and Alarm threads for message type = %" PRIuFAST32 \
						" have been deleted and terminated by Main thread with ID = %" \
						PRIuFAST64 " at %" PRIuFAST64 ".\n", msg_type, id, now());
		} else {
			fprintf(stderr, "The read command is invalid since it does not conform to any of the specified formats.\nCommand: |%s|\n", line);
		}



RESET_AND_READ_NEXT_LINE:
		/* Reset errno. */
		errno = 0;

		/*
		 * Reset all line attributes for the next iteration
		 * of the while loop which reads input commands.
		 */
		len = 0;
		free(line);
		data.line = line = NULL;
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
	 * reached since the above while loop will only be terminated when the
	 * main thread has been abruptly cancelled, it has encountered an error
	 * or when it reaches the End Of File(EOF).
	 */
	pthread_cleanup_pop(1);



	/*
	 * Cleanup main thread and terminate.
	 *
	 * This following will never be reached.
	 */
	data.err.linenum = __LINE__;
	pthread_exit(&data);
	return 0;
}
