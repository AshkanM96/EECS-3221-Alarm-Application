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

	/*
	 * Structure encapsulating all of the local variables that
	 * the main thread has used which need to be freed on its
	 * cancellation.
	 */
	typedef struct MainLocalData {
		/* Last occurred error in main. */
		Error					err;

		/*
		 * Flag representing whether the application
		 * log file is separate from stdout or not.
		 */
		bool					separate_log_file;

		/* Last read line of input. */
		char					*line;

		/* Pointer to the head of the threads singly-linked-list. */
		Thread					*thread_list_head;
	} MLData;



	/* Variable Definitions */

	/*
	 * Shared data between threads:
	 *
	 * 1. The mutex used to lock access to the alarms singly-linked-list.
	 *
	 * 2. Pointer to the head of the alarms singly-linked-list.
	 *
	 * 3. Application log file where all application messages are printed to.
	 */
	/* Initialize the mutex. */
	EXTERN pthread_mutex_t mutex SET(PTHREAD_MUTEX_INITIALIZER);

	/* Initialize an empty alarms list. */
	EXTERN Alarm *alarm_list_head SET(NULL);

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
