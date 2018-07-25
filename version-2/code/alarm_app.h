/**************************************************************************
 *
 * Author:
 * 					Ashkan Moatamed
 *
 *
 *
 * alarm_app.h
 *
 *
 *
 * Headers, Macros, Type Definitions, Variable
 * Definitions, and useful Function Prototypes
 * for alarm_app.c
 *
 *************************************************************************/

#ifndef INCLUDE_GUARD_ALARM__APP___H
	#define INCLUDE_GUARD_ALARM__APP___H



	/* Headers */

	#include "alarm_def.h"
	#include <semaphore.h>
	#include <sched.h>



	/* Macros */

	/* Should variables and functions be defined or declared? */
	#ifdef DEFINE_VARIABLES_AND_FUNCTIONS
		/* Define variables and function prototypes. */
		#define EXTERN					/* Nothing */
		/*
		 * A very general form of the SET macro, looks
		 * like the following:
		 *
		 * #define SET(...)				= __VA_ARGS__
		 *
		 * Which requires -std=c99 or higher when compiling.
		 *
		 * But here, since we only ever need to pass a single
		 * argument, we can settle for the following simple
		 * version which does the trick.
		 */
		#define SET(x)					= x
	#else
		/* Declare variables and function prototypes. */
		#define EXTERN					extern
		#define SET(x)					/* Nothing */
	#endif

	/*
	 * Application log file name. Change the text to get a different
	 * file name but to keep everything working perfectly, also make
	 * the same change in the Makefile by changing the LOG variable
	 * value. The macro can also be removed(or commented out) if one
	 * desires to completely remove the part where the application
	 * prompts for the logging and just have it print everything to
	 * the standard output stream(stdout) by default.
	 */
	#define APP_LOG_FILE "App_Log.txt"



	/* Type Definitions */

	/* Enumeration of all cleanup modes of the main thread of the application. */
	typedef enum MainCleanupMode {
		APP_LOG_FILE_LOCATION_FAIL = -1,
		APP_LOG_FILE_OPEN_FAIL = 0,
		ALARM_RW_BIN_SEM_INIT_FAIL = 1,
		ALARM_R_BIN_SEM_INIT_FAIL = 2,
		CMD_THREAD_CREATE_FAIL = 3,
		STD_CLEANUP = 4
	} MCMode;

	/*
	 * Structure encapsulating all of the local variables that
	 * the main thread has used which need to be freed on its
	 * cancellation.
	 */
	typedef struct MainLocalData {
		/* The main thread's cleanup mode. */
		MCMode					mode;

		/* Last occurred error in main. */
		Error					err;

		/*
		 * Flag representing whether the application
		 * log file is separate from stdout or not.
		 */
		bool					separate_log_file;

		/* The ID of the command handler thread. */
		pthread_t				cmd_thread_tid;

		/* Last read line of input. */
		char					*line;
	} MLData;



	/* Variable Definitions */

	/*
	 * Shared data between threads:
	 *
	 * 1. The cmd_mutex used to lock access to the commands singly-linked-list.
	 * 		Since there are only two writers to the list(the main and the command
	 * 		handler threads), then a simple mutex is enough to create synchronization
	 * 		between them.
	 *
	 * 2. The new_cmd_insert_mutex and new_cmd_insert_cond_var which are used when the command
	 * 		handler thread needs to know when a new valid command has been entered by the user
	 * 		and that the main thread has parsed, validated, and inserted it into the global
	 * 		commands list. This only makes the application perform better since the command
	 * 		handler thread will be blocked when there are no new commands to be executed since
	 * 		it will be waiting on the new_cmd_insert_cond_var. The reason why it improves
	 * 		performance is that the command handler thread will no longer attempt to obtain any
	 * 		locks.
	 *
	 * 3. Pointers to the head and tail of the commands singly-linked-list.
	 * 		In the actual implementation, the list has been separated into
	 * 		three sublists. Each sublist only stores commands of one type.
	 * 		For the type B commands list, we also have an extra pointer to the
	 * 		first unprocessed type B command to improve the command handler's
	 * 		processing runtime.
	 *
	 * 4. The alarm_rw_bin_sem used to lock access to the alarms singly-linked-list.
	 * 		This is used to lock access to the alarms list between readers and writers
	 * 		of the list. There is only one writer which is the command handler thread
	 * 		while all other threads are readers of the list. This includes the main and
	 * 		all alarm handler threads.
	 *
	 * 5. The alarm_r_bin_sem used to lock access to the alarms singly-linked-list.
	 * 		This is used to lock access to the alarms list between readers of the list.
	 *
	 * 6. The reader_count which keeps track of how many readers are currently reading
	 * 		the alarms singly-linked-list when the writer is inactive. If however, the
	 * 		writer is active, then the reader_count denotes the number of readers which
	 * 		are waiting to be able to read.
	 *
	 * 7. Pointer to the head of the alarms singly-linked-list.
	 *
	 * 8. The alarm_cancel_mutex and alarm_cancel_cond_var which are used when the command
	 * 		handler thread needs to know when an alarm handler thread has successfully detached
	 * 		an alarm node from its own local list and potentially cancelled itself. This is
	 * 		needed since the command handler thread should be able to safely perform any of
	 * 		the following:
	 * 			1. Replace or cancel an alarm in appropriate type A or C command.
	 * 			2. Free the allocated resources.
	 * 			3. Join with the alarm handler thread if it self terminated.
	 *
	 * 9. Application log file where all application messages are printed to.
	 */
	/* Initialize cmd_mutex. */
	EXTERN pthread_mutex_t cmd_mutex SET(PTHREAD_MUTEX_INITIALIZER);

	/* Initialize new_cmd_insert_mutex. */
	EXTERN pthread_mutex_t new_cmd_insert_mutex SET(PTHREAD_MUTEX_INITIALIZER);
	/* Initialize new_cmd_insert_cond_var. */
	EXTERN pthread_cond_t new_cmd_insert_cond_var SET(PTHREAD_COND_INITIALIZER);

	/* Initialize an empty type A commands list. */
	EXTERN CmdA *cmda_list_head SET(NULL);
	EXTERN CmdA *cmda_list_tail SET(NULL);
	/* Initialize an empty type B commands list. */
	EXTERN CmdB *cmdb_list_head SET(NULL);
	EXTERN CmdB *cmdb_list_tail SET(NULL);
	EXTERN CmdB *cmdb_list_new_elm SET(NULL);
	/* Initialize an empty type C commands list. */
	EXTERN CmdC *cmdc_list_head SET(NULL);
	EXTERN CmdC *cmdc_list_tail SET(NULL);

	EXTERN sem_t alarm_rw_bin_sem; /* Reader-Writer Semaphore. */
	EXTERN sem_t alarm_r_bin_sem; /* Reader Semaphore. */
	EXTERN uint_fast64_t reader_count SET(0); /* Initialize reader_count. */

	/* Initialize an empty alarms list. */
	EXTERN Alarm *alarm_list_head SET(NULL);

	/* Initialize alarm_cancel_mutex. */
	EXTERN pthread_mutex_t alarm_cancel_mutex SET(PTHREAD_MUTEX_INITIALIZER);
	/* Initialize alarm_cancel_cond_var. */
	EXTERN pthread_cond_t alarm_cancel_cond_var SET(PTHREAD_COND_INITIALIZER);

	/* Initialize the application log file. */
	EXTERN FILE *app_log SET(NULL);



	/* Function Prototypes */

	/*
	 * The main thread's cleanup routine.
	 *
	 * Cleanup the main thread by freeing all allocated memory,
	 * cancelling all active threads and in general releasing all
	 * resources acquired during its life cycle.
	 *
	 * Precondition: arg can be safely casted into (MLData *).
	 */
	EXTERN void cleanup_main(void *arg);

	/* End the main thread and print the error message if needed. */
	EXTERN void exit_main(MLData data);



	/*
	 * Obtain a reader lock on the global
	 * alarms list for the calling thread.
	 */
	EXTERN void obtain_alarm_read_lock(MLData *data_ptr);

	/*
	 * Release previously obtained reader lock on
	 * the global alarms list by the calling thread.
	 */
	EXTERN void release_alarm_read_lock(MLData *data_ptr);



	/*
	 * The only invocations of this function should be from the command handler thread.
	 *
	 * Disable cancellation for the caller and then obtain all necessary locks.
	 */
	EXTERN void cmd_handler_obtain_locks(int *old_state_ptr);

	/*
	 * The only invocations of this function should be from the command handler thread.
	 *
	 * Release all necessary locks in the opposite order of obtaining them when
	 * invoking cmd_handler_obtain_locks and then enable cancellation for the
	 * caller. Finally check if there are any pending cancellation requests.
	 */
	EXTERN void cmd_handler_release_locks(int *old_state_ptr);



	/*
	 * The command handler thread routine.
	 *
	 * Precondition: arg can be safely casted into (pthread_t *).
	 *
	 * Returns: arg
	 */
	EXTERN void * cmd_handler(void *arg);

	/*
	 * The only invocations of this function should be from the command handler thread.
	 *
	 * If handler_id is NULL then do nothing and just return 0 to the caller.
	 *
	 * If handler_id is not NULL then join with the alarm handler thread with
	 * ID *handler_id and release all allocated resources and return the alarm
	 * handler thread's message type which is guaranteed to be non-zero.
	 */
	EXTERN uint_fast32_t cmd_handler_join_with_alarm_handler(pthread_t *handler_id);

	/*
	 * The command handler thread cleanup routine.
	 *
	 * Precondition: arg can be safely casted into (pthread_t *).
	 */
	EXTERN void cleanup_cmd_handler(void *arg);



	/*
	 * The alarm handler thread routine.
	 *
	 * Precondition: arg can be safely casted into (uint_fast32_t *).
	 *
	 * Returns: arg
	 */
	EXTERN void * alarm_handler(void *arg);

	/*
	 * The alarm handler thread cleanup routine.
	 *
	 * Precondition: arg can be safely casted into (Alarm *).
	 */
	EXTERN void cleanup_alarm_handler(void *arg);

#endif
