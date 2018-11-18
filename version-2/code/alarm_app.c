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
 * Type A: Time Message(Type, Number) AlarmMessage
 * Type B: Create_Thread: MessageType(Type)
 * Type C: Cancel: Message(Number)
 *
 * Where Message, Create_Thread, MessageType, and Cancel are
 * all reserved keywords by the program.
 *
 * Time is an unsigned number denoting the amount of time
 * to wait between two consecutive prints of the alarm.
 *
 * Type is an unsigned number used to categorize different alarms.
 *
 * Number is an unsigned number used to uniquely identify different
 * alarms. Note that a new alarm with a given Number will replace an
 * existing alarm with that Number since Number is the primary key.
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
 * alarms of the given type if no such thread already exists.
 *
 * Type C commands, terminate a single message with the given number.
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
	const pthread_t tid = pthread_self();
	const uint_fast64_t id = (uint_fast64_t) tid;



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



	/* The command handler thread's ID. */
	pthread_t cmd_thread_tid;



	/*
	 * Has the user been informed of the internal state or not?
	 *
	 * This flag is used in processing commands to determine whether
	 * a status message has been printed informing the user of the
	 * internal state or not.
	 */
	bool is_user_informed = false;
	/*
	 * Is there at least one alarm in the global alarms list(or in the type
	 * A commands list waiting to be added to the alarms list) of the given
	 * message type or with the given message number?
	 *
	 * This flag is used in processing type B and C commands to determine
	 * whether at least one alarm of the given message type or with the
	 * given message number has been found or not.
	 */
	bool alarm_exists = false;

	/*
	 * Command type A pointers used for iterating over the type A
	 * commands list and also to store new type A commands which
	 * are created by the user.
	 */
	CmdA *curr_cmda = NULL, *new_cmda = NULL;
	/* Alarm pointer used for iterating over the alarms list. */
	Alarm *curr_alarm = NULL;
	/*
	 * Command type B pointers used for iterating over the type B
	 * commands list and also to store new type B commands which
	 * are created by the user.
	 */
	CmdB *curr_cmdb = NULL, *new_cmdb = NULL;
	/*
	 * Command type C pointers used for iterating over the type C
	 * commands list and also to store new type C commands which
	 * are created by the user.
	 */
	CmdC *curr_cmdc = NULL, *new_cmdc = NULL;

	/* The read wait time in the new command. */
	uint_fast32_t wait_time = 0;
	/* Used for safely reading wait_time as a uint_fast32_t number. */
	int_fast64_t l_wait_time = 0;
	/* The read message type in the new command. */
	uint_fast32_t msg_type = 0;
	/* Used for safely reading msg_type as a uint_fast32_t number. */
	int_fast64_t l_msg_type = 0;
	/* The read message number in the new command. */
	uint_fast32_t msg_num = 0;
	/* Used for safely reading msg_num as a uint_fast32_t number. */
	int_fast64_t l_msg_num = 0;
	/*
	 * msg has maximum length MAX_MSG_LEN + 1 since it has to be
	 * able to store MAX_MSG_LEN many chars and 1 nullchar('\0').
	 */
	char msg[MAX_MSG_LEN + 1];



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
	data.mode = STD_CLEANUP;
	data.err.filename = __FILE__; data.err.linenum = 0;
	data.err.val = 0; data.err.msg = "";
	data.separate_log_file = separate_log_file;
	data.cmd_thread_tid = cmd_thread_tid;
	data.line = line;



	app_log = stdout; /* Initialize app_log. */
	#ifdef APP_LOG_FILE
		printf("Do you want to save the application log to a file named %s? (y/n) ", APP_LOG_FILE);
		status = read_line(fgetc, stdin, &len, &line, NULL); data.line = line;
		if (status == -2) {
			/* Cleanup main thread and terminate. */
			data.mode = APP_LOG_FILE_LOCATION_FAIL;
			data.err.linenum = __LINE__;
			data.err.val = STREAM_ERR; data.err.msg = STREAM_ERR_MSG;
			pthread_exit(&data);
		} else if (status == -1) {
			/* Cleanup main thread and terminate. */
			data.mode = APP_LOG_FILE_LOCATION_FAIL;
			data.err.linenum = __LINE__;
			data.err.val = ALLOC_STR_ERR; data.err.msg = ALLOC_STR_ERR_MSG;
			pthread_exit(&data);
		} else if (status == 1) {
			/* EOF has been reached therefore proceed to cleanup. */
			data.mode = APP_LOG_FILE_LOCATION_FAIL;
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
					data.mode = APP_LOG_FILE_OPEN_FAIL;
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
	printf("Time Message(Type, Number) AlarmMessage\n");
	printf("Create_Thread: MessageType(Type)\n");
	printf("Cancel: Message(Number)\n");

	printf("\nWhere Message, Create_Thread, MessageType, and Cancel are\n");
	printf("all reserved keywords by the program.\n");

	printf("\nTime is an unsigned number denoting the amount of time\n");
	printf("to wait between two consecutive prints of the alarm.\n");

	printf("\nType is an unsigned number used to categorize different alarms.\n");

	printf("\nNumber is an unsigned number used to uniquely identify different\n");
	printf("alarms. Note that a new alarm with a given Number will replace an\n");
	printf("existing alarm with that Number since Number is the primary key.\n");

	printf("\nAnd finally AlarmMessage is a non-empty string of maximum length %d.\n", MAX_MSG_LEN);
	printf("Note that longer strings will be truncated to %d characters.\n", MAX_MSG_LEN);

	printf("\n\nType A commands, produce a new alarm to be added to the\n");
	printf("alarms list for later processing.\n");

	printf("\nType B commands, create a new thread to handle all outstanding\n");
	printf("alarms of the given type if no such thread already exists.\n");

	printf("\nType C commands, terminate a single message with the given number.\n");

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



	/*
	 * Initialize alarm_rw_bin_sem and alarm_r_bin_sem semaphores
	 * with value(third argument) 1 which is why they are called
	 * binary semaphores(a.k.a. mutexes).
	 *
	 * The second argument to sem_init(pshared) indicates
	 * whether this semaphore is to be shared between the
	 * threads of a single process, or between multiple
	 * processes.
	 *
	 * If pshared is zero, the the semaphore is only shared
	 * between the threads of a single process, and should
	 * be located at some address that is visible to all
	 * threads(e.g., a global variable, or a variable
	 * allocated dynamically on the heap).
	 *
	 * If pshared is nonzero, then the semaphore is shared
	 * between processes, and should be located in a region
	 * of shared memory(see shm_open, mmap, and shmget).
	 * Since a child created by fork inherits its parent's
	 * memory mappings, it can also access the semaphore. Any
	 * process that can access the shared memory region can
	 * operate on the semaphore using sem_post, sem_wait, etc.
	 */
	status = sem_init(&alarm_rw_bin_sem, 0, 1);
	if (status != 0) {
		data.mode = ALARM_RW_BIN_SEM_INIT_FAIL;
		data.err.linenum = __LINE__;
		data.err.val = SEM_INIT_ERR; data.err.msg = SEM_INIT_ERR_MSG;
		pthread_exit(&data);
	}
	status = sem_init(&alarm_r_bin_sem, 0, 1);
	if (status != 0) {
		data.mode = ALARM_R_BIN_SEM_INIT_FAIL;
		data.err.linenum = __LINE__;
		data.err.val = SEM_INIT_ERR; data.err.msg = SEM_INIT_ERR_MSG;
		pthread_exit(&data);
	}



	/* Create the command handler thread. */
	status = pthread_create(&cmd_thread_tid, NULL,
				cmd_handler, (void *) (&cmd_thread_tid));
	data.cmd_thread_tid = cmd_thread_tid;
	if (status != 0) {
		data.mode = CMD_THREAD_CREATE_FAIL;
		data.err.linenum = __LINE__;
		data.err.val = THREAD_CREATE_ERR; data.err.msg = THREAD_CREATE_ERR_MSG;
		pthread_exit(&data);
	}



	/*
	 * At this point, we must have the following: data.mode == STD_CLEANUP
	 *
	 * assert(data.mode == STD_CLEANUP)
	 *
	 *
	 *
	 * Infinite loop to process new commands read from stdin.
	 */
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



		if (sscanf(line, "%" SCNdFAST64 " Message(%" SCNdFAST64 ", %" SCNdFAST64 ") ", &l_wait_time, &l_msg_type, &l_msg_num) == 3) {
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

			/* Parse l_msg_num as a uint_fast32_t number. */
			errno = 0;
			msg_num = f64_to_uf32(l_msg_num);
			if (errno != 0) {
				/* l_msg_num is not a valid uint_fast32_t number. */
				fprintf(stderr, "The given type A command is invalid since the given Number is not a valid unsigned number.\nCommand: |%s|\n", line);
				goto RESET_AND_READ_NEXT_LINE;
			} else if (msg_num == 0) {
				/* msg_num should be positive. */
				fprintf(stderr, "The given type A command is invalid since the given Number is 0.\nCommand: |%s|\n", line);
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
			sprintf(tmp_str, "%" PRIuFAST32 " Message(%" PRIuFAST32 ", %" PRIuFAST32 ") ", wait_time, msg_type, msg_num);

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



			/* Obtain reader lock. */
			obtain_alarm_read_lock(&data);

			/*
			 * Critical Section:
			 * Read the global alarms list to determine if the new
			 * alarm is going to replace an existing alarm or not.
			 */
			for (curr_alarm = alarm_list_head; curr_alarm != NULL; curr_alarm = curr_alarm->link) {
				if (curr_alarm->msg_num == msg_num) {
					/* Print status message informing the user of the internal state. */
					fprintf(app_log, "An alarm with message number = %" PRIuFAST32 \
								" already exists in the alarms list which will be replaced.\n", msg_num);

					/* Set the user informed flag. */
					is_user_informed = true;

					/* Terminate the searching for loop. */
					break;
				} else if (curr_alarm->msg_num > msg_num) {
					/*
					 * The alarms list is sorted by message numbers so
					 * if the current alarm that we are looking at, has
					 * a larger message number than the one we are looking
					 * for, then it is simply not possible to find it.
					 */
					break;
				}
			}

			/* Release reader lock. */
			release_alarm_read_lock(&data);



			/* Lock cmd_mutex. */
			status = pthread_mutex_lock(&cmd_mutex);
			if (status != 0) {
				/* Cleanup main thread and terminate. */
				data.err.linenum = __LINE__;
				data.err.val = MUTEX_LOCK_ERR; data.err.msg = MUTEX_LOCK_ERR_MSG;
				pthread_exit(&data);
			}

			/*
			 * Critical Section:
			 * Read the global commands list to determine if the
			 * new type A command specifies a new alarm that is
			 * going to replace an existing alarm or not.
			 */
			for (curr_cmda = cmda_list_head; curr_cmda != NULL; curr_cmda = curr_cmda->link) {
				if (curr_cmda->msg_num == msg_num) {
					if (!is_user_informed) {
						/* Print status message informing the user of the internal state. */
						fprintf(app_log, "A type A command with message number = %" PRIuFAST32 \
									" already exists in the commands list which will be replaced.\n", msg_num);

						/* Set the user informed flag. */
						is_user_informed = true;
					}

					/*
					 * Replace the type A command. When type A commands are
					 * processed by the command handler thread, they are
					 * immediately removed from the commands list and the
					 * allocated memory is freed. Therefore, if we have found
					 * a command in the list with the same message number, it
					 * means that it has not yet been processed but it is going
					 * to be replaced by the new command. Therefore, we can just
					 * replace the command pointed to be curr_cmda with the new
					 * command to arrive at the same spot that we would have
					 * otherwise arrived.
					 */
					curr_cmda->wait_time = wait_time;
					curr_cmda->msg_type = msg_type;
					strcpy(curr_cmda->msg, msg); /* Set curr_cmda's message. */

					/* Terminate the searching for loop. */
					break;
				}
			}

			if (curr_cmda == NULL) {
				/*
				 * The only way that curr_cmda can be NULL at this
				 * point is if the commands list was empty or the if
				 * condition in the above loop(checking message numbers)
				 * always evaluated to false which implies that the new
				 * command is indeed a new command to be added to the list.
				 */

				/* Allocate memory for the new command A node. */
				new_cmda = MALLOC(CmdA);
				if (new_cmda == NULL) {
					/* Cleanup main thread and terminate. */
					data.err.linenum = __LINE__;
					data.err.val = ALLOC_CMDA_ERR; data.err.msg = ALLOC_CMDA_ERR_MSG;
					pthread_exit(&data);
				}

				/* Initialize the new command A node's attributes. */
				new_cmda->link = NULL;
				new_cmda->wait_time = wait_time;
				new_cmda->msg_type = msg_type;
				new_cmda->msg_num = msg_num;
				strcpy(new_cmda->msg, msg); /* Set new_cmda's message. */

				/* Insert the new type A command at the end of the global commands list in O(1). */
				if (cmda_list_head == NULL) {
					cmda_list_head = new_cmda;
				} else { /* (cmda_list_head != NULL) */
					cmda_list_tail->link = new_cmda;
				}
				cmda_list_tail = new_cmda;

				/* Print status message informing the user of the internal state. */
				if (!is_user_informed) {
					fprintf(app_log, "New type A command with message type = %" PRIuFAST32 \
								" and message number = %" PRIuFAST32 " inserted by Main thread with ID = %" \
								PRIuFAST64 " into the commands list at %" PRIuFAST64 ".\n", msg_type, msg_num, id, now());
				}
			}

			/* Unlock cmd_mutex. */
			status = pthread_mutex_unlock(&cmd_mutex);
			if (status != 0) {
				/*
				 * We do not need to free memory allocated to new_cmda if
				 * it was allocated in the above. The reason is that it has
				 * been successfully inserted into the global commands list
				 * if it was indeed allocated and thus it will be freed by
				 * the main cleanup.
				 */

				/* Cleanup main thread and terminate. */
				data.err.linenum = __LINE__;
				data.err.val = MUTEX_UNLOCK_ERR; data.err.msg = MUTEX_UNLOCK_ERR_MSG;
				pthread_exit(&data);
			}

			/* Signal new_cmd_insert_cond_var signifying new type A command insertion. */
			status = pthread_cond_signal(&new_cmd_insert_cond_var);
			if (status != 0) {
				/* Cleanup main thread and terminate. */
				data.err.linenum = __LINE__;
				data.err.val = COND_VAR_SIGNAL_ERR; data.err.msg = COND_VAR_SIGNAL_ERR_MSG;
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



			/* Obtain reader lock. */
			obtain_alarm_read_lock(&data);

			/*
			 * Critical Section:
			 * Read the global alarms list to determine if there is
			 * at least one alarm of the given message type or not.
			 */
			for (curr_alarm = alarm_list_head; curr_alarm != NULL; curr_alarm = curr_alarm->link) {
				if (curr_alarm->msg_type == msg_type) {
					/* Set the alarm exists flag. */
					alarm_exists = true;

					/* Terminate the searching for loop. */
					break;
				}
			}

			/* Release reader lock. */
			release_alarm_read_lock(&data);



			/* Lock cmd_mutex. */
			status = pthread_mutex_lock(&cmd_mutex);
			if (status != 0) {
				/* Cleanup main thread and terminate. */
				data.err.linenum = __LINE__;
				data.err.val = MUTEX_LOCK_ERR; data.err.msg = MUTEX_LOCK_ERR_MSG;
				pthread_exit(&data);
			}

			/*
			 * The command handler thread first processes type A
			 * commands. This means that even if there are no alarms
			 * in the global alarms list of the given message type
			 * right now, there may be some in the list after the type
			 * A commands are processed and the alarms list has been
			 * repopulated by the command handler thread.
			 */
			if (!alarm_exists) {
				/*
				 * Critical Section Part 1:
				 * Read the global commands list to determine if there is
				 * at least one type A command of the given message type
				 * or not.
				 */
				for (curr_cmda = cmda_list_head; curr_cmda != NULL; curr_cmda = curr_cmda->link) {
					if (curr_cmda->msg_type == msg_type) {
						/* Set the alarm exists flag. */
						alarm_exists = true;

						/* Terminate the searching for loop. */
						break;
					}
				}
			}

			if (alarm_exists) {
				/*
				 * Critical Section Part 2:
				 * Read the global commands list to determine if there is
				 * at least one type B command of the given message type
				 * or not.
				 */
				for (curr_cmdb = cmdb_list_head; curr_cmdb != NULL; curr_cmdb = curr_cmdb->link) {
					if (curr_cmdb->msg_type == msg_type) {
						/* Print status message informing the user of the internal state. */
						if (curr_cmdb->is_processed) {
							printf("The given type B command requests a new Alarm thread with message type = %" \
										PRIuFAST32 " but there is already one such thread with ID = %" PRIuFAST64 \
										".\n", msg_type, (uint_fast64_t) curr_cmdb->id);
						} else { /* (!curr_cmdb->is_processed) */
							printf("The given type B command requests a new Alarm thread with message type = %" \
										PRIuFAST32 " but there is already one such request in the commands list.\n",
										msg_type);
						}

						/* Terminate the searching for loop. */
						break;
					}
				}

				if (curr_cmdb == NULL) {
					/*
					 * The only way that curr_cmdb can be NULL at this
					 * point is if the commands list was empty or the if
					 * condition in the above loop(checking message types)
					 * always evaluated to false which implies that the new
					 * command is indeed a new command to be added to the list.
					 */

					/* Allocate memory for the new command B node. */
					new_cmdb = MALLOC(CmdB);
					if (new_cmdb == NULL) {
						/* Cleanup main thread and terminate. */
						data.err.linenum = __LINE__;
						data.err.val = ALLOC_CMDB_ERR; data.err.msg = ALLOC_CMDB_ERR_MSG;
						pthread_exit(&data);
					}

					/* Initialize the new command B node's attributes. */
					new_cmdb->link = NULL;
					new_cmdb->msg_type = msg_type;
					/* id will be initialized by the command handler thread. */
					new_cmdb->is_processed = false;

					/* Insert the new type B command at the end of the global commands list in O(1). */
					if (cmdb_list_head == NULL) {
						cmdb_list_head = new_cmdb;
					} else { /* (cmdb_list_head != NULL) */
						cmdb_list_tail->link = new_cmdb;
					}
					cmdb_list_tail = new_cmdb;
					/* Update type B commands list new element pointer. */
					if (cmdb_list_new_elm == NULL) {
						cmdb_list_new_elm = cmdb_list_tail;
					}

					/* Print status message informing the user of the internal state. */
					fprintf(app_log, "New type B command with message type = %" PRIuFAST32 \
								" inserted by Main thread with ID = %" PRIuFAST64 \
								" into the commands list at %" PRIuFAST64 ".\n", msg_type, id, now());
				}
			} else { /* (!alarm_exists) */
				/*
				 * At this point we know that there are no alarms
				 * in the global alarms list of the given message
				 * type nor will there be as of now since we also
				 * know that there are no type A commands of the
				 * given message type.
				 */
				printf("The given type B command requests a new Alarm thread with message type = %" \
							PRIuFAST32 " but there are no alarms of this type.\n", msg_type);
			}

			/* Unlock cmd_mutex. */
			status = pthread_mutex_unlock(&cmd_mutex);
			if (status != 0) {
				/*
				 * We do not need to free memory allocated to new_cmdb if
				 * it was allocated in the above. The reason is that it has
				 * been successfully inserted into the global commands list
				 * if it was indeed allocated and thus it will be freed by
				 * the main cleanup.
				 */

				/* Cleanup main thread and terminate. */
				data.err.linenum = __LINE__;
				data.err.val = MUTEX_UNLOCK_ERR; data.err.msg = MUTEX_UNLOCK_ERR_MSG;
				pthread_exit(&data);
			}

			/* Signal new_cmd_insert_cond_var signifying new type B command insertion. */
			status = pthread_cond_signal(&new_cmd_insert_cond_var);
			if (status != 0) {
				/* Cleanup main thread and terminate. */
				data.err.linenum = __LINE__;
				data.err.val = COND_VAR_SIGNAL_ERR; data.err.msg = COND_VAR_SIGNAL_ERR_MSG;
				pthread_exit(&data);
			}
		} else if (sscanf(line, "Cancel: Message(%" SCNdFAST64 ")", &l_msg_num) == 1) {
			/* Type C */

			/* Parse l_msg_num as a uint_fast32_t number. */
			errno = 0;
			msg_num = f64_to_uf32(l_msg_num);
			if (errno != 0) {
				/* l_msg_num is not a valid uint_fast32_t number. */
				fprintf(stderr, "The given type C command is invalid since the given Number is not a valid unsigned number.\nCommand: |%s|\n", line);
				goto RESET_AND_READ_NEXT_LINE;
			} else if (msg_num == 0) {
				/* msg_num should be positive. */
				fprintf(stderr, "The given type C command is invalid since the given Number is 0.\nCommand: |%s|\n", line);
				goto RESET_AND_READ_NEXT_LINE;
			}

			/* Validate the command. */
			status = is_valid_cmd(line, len, 'C', msg_num);
			if (status == -1) {
				/* Cleanup main thread and terminate. */
				data.err.linenum = __LINE__;
				data.err.val = ALLOC_STR_ERR; data.err.msg = ALLOC_STR_ERR_MSG;
				pthread_exit(&data);
			} else if (status == 0) {
				fprintf(stderr, "The given type C command is invalid since it does not conform to the specified format.\nCommand: |%s|\n", line);
				goto RESET_AND_READ_NEXT_LINE;
			}



			/* Obtain reader lock. */
			obtain_alarm_read_lock(&data);

			/*
			 * Critical Section:
			 * Read the global alarms list to determine if there
			 * is an alarm with the given message number or not.
			 */
			for (curr_alarm = alarm_list_head; curr_alarm != NULL; curr_alarm = curr_alarm->link) {
				if (curr_alarm->msg_num == msg_num) {
					/* Set the alarm exists flag. */
					alarm_exists = true;

					/* Terminate the searching for loop. */
					break;
				} else if (curr_alarm->msg_num > msg_num) {
					/*
					 * The alarms list is sorted by message numbers so
					 * if the current alarm that we are looking at, has
					 * a larger message number than the one we are looking
					 * for, then it is simply not possible to find it.
					 */
					break;
				}
			}

			/* Release reader lock. */
			release_alarm_read_lock(&data);



			/* Lock cmd_mutex. */
			status = pthread_mutex_lock(&cmd_mutex);
			if (status != 0) {
				/* Cleanup main thread and terminate. */
				data.err.linenum = __LINE__;
				data.err.val = MUTEX_LOCK_ERR; data.err.msg = MUTEX_LOCK_ERR_MSG;
				pthread_exit(&data);
			}

			/*
			 * The command handler thread first processes type A
			 * commands. This means that even if there are no alarms
			 * in the global alarms list of the given message type
			 * right now, there may be some in the list after the type
			 * A commands are processed and the alarms list has been
			 * repopulated by the command handler thread.
			 */
			if (!alarm_exists) {
				/*
				 * Critical Section Part 1:
				 * Read the global commands list to determine if there is
				 * a type A command with the given message number or not.
				 */
				for (curr_cmda = cmda_list_head; curr_cmda != NULL; curr_cmda = curr_cmda->link) {
					if (curr_cmda->msg_num == msg_num) {
						/* Set the alarm exists flag. */
						alarm_exists = true;

						/* Terminate the searching for loop. */
						break;
					}
				}
			}

			if (alarm_exists) {
				/*
				 * Critical Section Part 2:
				 * Read the global commands list to determine if there is
				 * at least one type C command with the given message number
				 * or not.
				 */
				for (curr_cmdc = cmdc_list_head; curr_cmdc != NULL; curr_cmdc = curr_cmdc->link) {
					if (curr_cmdc->msg_num == msg_num) {
						/* Print status message informing the user of the internal state. */
						printf("The given type C command requests the cancellation of an alarm with message number = %" \
									PRIuFAST32 " but there is already one such request in the commands list.\n", msg_num);

						/* Terminate the searching for loop. */
						break;
					}
				}

				if (curr_cmdc == NULL) {
					/*
					 * The only way that curr_cmdc can be NULL at this
					 * point is if the commands list was empty or the if
					 * condition in the above loop(checking message numbers)
					 * always evaluated to false which implies that the new
					 * command is indeed a new command to be added to the list.
					 */

					/* Allocate memory for the new command C node. */
					new_cmdc = MALLOC(CmdC);
					if (new_cmdc == NULL) {
						/* Cleanup main thread and terminate. */
						data.err.linenum = __LINE__;
						data.err.val = ALLOC_CMDC_ERR; data.err.msg = ALLOC_CMDC_ERR_MSG;
						pthread_exit(&data);
					}

					/* Initialize the new command C node's attributes. */
					new_cmdc->link = NULL;
					new_cmdc->msg_num = msg_num;

					/* Insert the new type C command at the end of the global commands list in O(1). */
					if (cmdc_list_head == NULL) {
						cmdc_list_head = new_cmdc;
					} else { /* (cmdc_list_head != NULL) */
						cmdc_list_tail->link = new_cmdc;
					}
					cmdc_list_tail = new_cmdc;

					/* Print status message informing the user of the internal state. */
					fprintf(app_log, "New type C command with message number = %" PRIuFAST32 \
								" inserted by Main thread with ID = %" PRIuFAST64 \
								" into the commands list at %" PRIuFAST64 ".\n", msg_num, id, now());
				}
			} else { /* (!alarm_exists) */
				/*
				 * At this point we know that there are no alarms
				 * in the global alarms list with the given message
				 * number nor will there be as of now since we also
				 * know that there are no type A commands with the
				 * given message number.
				 */
				printf("The given type C command requests the cancellation of an alarm with message number = %" \
							PRIuFAST32 " but there are no alarms with this message number.\n", msg_num);
			}

			/* Unlock cmd_mutex. */
			status = pthread_mutex_unlock(&cmd_mutex);
			if (status != 0) {
				/*
				 * We do not need to free memory allocated to new_cmdc if
				 * it was allocated in the above. The reason is that it has
				 * been successfully inserted into the global commands list
				 * if it was indeed allocated and thus it will be freed by
				 * the main cleanup.
				 */

				/* Cleanup main thread and terminate. */
				data.err.linenum = __LINE__;
				data.err.val = MUTEX_UNLOCK_ERR; data.err.msg = MUTEX_UNLOCK_ERR_MSG;
				pthread_exit(&data);
			}

			/* Signal new_cmd_insert_cond_var signifying new type C command insertion. */
			status = pthread_cond_signal(&new_cmd_insert_cond_var);
			if (status != 0) {
				/* Cleanup main thread and terminate. */
				data.err.linenum = __LINE__;
				data.err.val = COND_VAR_SIGNAL_ERR; data.err.msg = COND_VAR_SIGNAL_ERR_MSG;
				pthread_exit(&data);
			}
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

		/* Reset the user informed flag. */
		is_user_informed = false;
		/* Reset the alarm exists flag. */
		alarm_exists = false;
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
