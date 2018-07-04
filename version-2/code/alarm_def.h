/*
 * Author:
 * 					Ashkan Moatamed
 *
 *
 *
 * alarm_def.h
 *
 *
 *
 * Headers, Macros, Type Definitions, and
 * Function Prototypes for a multi-threaded
 * alarm application.
 */

#ifndef INCLUDE_GUARD_ALARM__DEF___H
	#define INCLUDE_GUARD_ALARM__DEF___H



	/* Headers */

	#include <pthread.h>
	#include "std_utilities.h"



	/* Macros */

	/*
	 * The maximum length of a message. Note that messages longer than
	 * the maximum length will be truncated to MAX_MSG_LEN chars.
	 */
	#define MAX_MSG_LEN 50

	/*
	 * Alarm threads wait the following number of seconds between
	 * two consecutive updates of their local alarms list.
	 */
	#define ALARM_THREAD_UPDATE_PERIOD 10

	/*
	 * The following consists of possible errors that
	 * can occur during the application execution.
	 */

	/* Negative values imply that errno is set. */

	/* String memory allocation error. */
	#define ALLOC_STR_ERR -1
	#define ALLOC_STR_ERR_MSG "String memory allocation error"

	/* Alarm memory allocation error. */
	#define ALLOC_ALARM_ERR -2
	#define ALLOC_ALARM_ERR_MSG "Alarm memory allocation error"

	/* Thread memory allocation error. */
	#define ALLOC_THREAD_ERR -3
	#define ALLOC_THREAD_ERR_MSG "Thread memory allocation error"

	/* CPU yield error. */
	#define CPU_YIELD_ERR -4
	#define CPU_YIELD_ERR_MSG "CPU yield error"

	/* File opening error. */
	#define FOPEN_ERR -5
	#define FOPEN_ERR_MSG "File opening error"

	/* File closing error. */
	#define FCLOSE_ERR -6
	#define FCLOSE_ERR_MSG "File closing error"

	/* File flushing error. */
	#define FFLUSH_ERR -7
	#define FFLUSH_ERR_MSG "File flushing error"

	/* Semaphore wait error. */
	#define SEM_WAIT_ERR -8
	#define SEM_WAIT_ERR_MSG "Semaphore wait error"

	/* Semaphore signal error. */
	#define SEM_SIGNAL_ERR -9
	#define SEM_SIGNAL_ERR_MSG "Semaphore signal error"

	/* Semaphore initialization error. */
	#define SEM_INIT_ERR -10
	#define SEM_INIT_ERR_MSG "Semaphore initialization error"

	/* Semaphore destroy error. */
	#define SEM_DESTROY_ERR -11
	#define SEM_DESTROY_ERR_MSG "Semaphore destroy error"

	/* Type A command memory allocation error. */
	#define ALLOC_CMDA_ERR -12
	#define ALLOC_CMDA_ERR_MSG "Type A command memory allocation error"

	/* Type B command memory allocation error. */
	#define ALLOC_CMDB_ERR -13
	#define ALLOC_CMDB_ERR_MSG "Type B command memory allocation error"

	/* Type C command memory allocation error. */
	#define ALLOC_CMDC_ERR -14
	#define ALLOC_CMDC_ERR_MSG "Type C command memory allocation error"

	/* Positive values imply that errno is NOT set. */

	/* Mutex lock error. */
	#define MUTEX_LOCK_ERR 1
	#define MUTEX_LOCK_ERR_MSG "Mutex lock error"

	/* Mutex unlock error. */
	#define MUTEX_UNLOCK_ERR 2
	#define MUTEX_UNLOCK_ERR_MSG "Mutex unlock error"

	/* Mutex trylock error. */
	#define MUTEX_TRYLOCK_ERR 3
	#define MUTEX_TRYLOCK_ERR_MSG "Mutex trylock error"

	/* Mutex destroy error. */
	#define MUTEX_DESTROY_ERR 4
	#define MUTEX_DESTROY_ERR_MSG "Mutex destroy error"

	/* Conditional variable destroy error. */
	#define COND_VAR_DESTROY_ERR 5
	#define COND_VAR_DESTROY_ERR_MSG "Conditional variable destroy error"

	/* Thread creation error. */
	#define THREAD_CREATE_ERR 6
	#define THREAD_CREATE_ERR_MSG "Thread creation error"

	/* Thread cancellation error caused when pthread_cancel fails. */
	#define THREAD_CANCEL_ERR 7
	#define THREAD_CANCEL_ERR_MSG "Thread cancellation error"

	/* Thread join error. */
	#define THREAD_JOIN_ERR 8
	#define THREAD_JOIN_ERR_MSG "Thread join error"

	/* Cancelled thread return value error. */
	#define THREAD_CANCEL_RETVAL_ERR 9
	#define THREAD_CANCEL_RETVAL_ERR_MSG "Cancelled thread return value error"

	/* Thread cancellation enabling error. */
	#define CANCELLATION_ENABLE_ERR 10
	#define CANCELLATION_ENABLE_ERR_MSG "Thread cancellation enabling error"

	/* Thread cancellation disabling error. */
	#define CANCELLATION_DISABLE_ERR 11
	#define CANCELLATION_DISABLE_ERR_MSG "Thread cancellation disabling error"

	/* Thread cancellation set type error. */
	#define CANCELLATION_SET_TYPE_ERR 12
	#define CANCELLATION_SET_TYPE_ERR_MSG "Thread cancellation set type error"

	/* Conditional variable wait error. */
	#define COND_VAR_WAIT_ERR 13
	#define COND_VAR_WAIT_ERR_MSG "Conditional variable wait error"

	/* Conditional variable signal error. */
	#define COND_VAR_SIGNAL_ERR 14
	#define COND_VAR_SIGNAL_ERR_MSG "Conditional variable signal error"

	/* Stream error. */
	#define STREAM_ERR 15
	#define STREAM_ERR_MSG "Stream error"



	/* Type Definitions */

	/* Structure encapsulating each alarm as a node in a singly-linked-list. */
	typedef struct AlarmNode {
		/* Pointer to the next alarm node in the global alarms list. */
		struct AlarmNode		*link;
		/*
		 * Pointer to the next alarm node in the local alarms list
		 * being handled by some handling thread.
		 */
		struct AlarmNode		*link_handle;

		/*
		 * The amount of time to wait in seconds between two
		 * consecutive prints of the current alarm's message.
		 */
		uint_fast32_t			wait_time;

		/* The alarm's message type used to categorize different alarms. */
		uint_fast32_t			msg_type;
		/* The alarm's message number used to uniquely identify different alarms. */
		uint_fast32_t			msg_num;

		/*
		 * The alarm's message of maximum length MAX_MSG_LEN.
		 * However msg is defined to be of length MAX_MSG_LEN + 1
		 * since it needs to be able to store MAX_MSG_LEN chars
		 * which represent the message but also 1 nullchar('\0')
		 * at the end to terminate the string.
		 */
		char					msg[MAX_MSG_LEN + 1];

		/* The state of the alarm either ASSIGNED(true) or UNASSIGNED(false). */
		bool					is_assigned;
		/*
		 * Pointer to the thread ID of the alarm_handler thread which
		 * is handling this alarm and NULL if no such thread exists.
		 *
		 * Threads are responsible to maintain the following invariants:
		 * 		1. is_assigned == (handler_id != NULL)
		 * 		2. (handler_id != NULL) implies (*handler_id is a valid thread ID)
		 */
		pthread_t				*handler_id;

		/* Is the current alarm being replaced or not? */
		bool					is_replaced;
		/* Is the current alarm being cancelled or not? */
		bool					is_cancelled;
	} Alarm;

	/* Structure encapsulating each pthread as a node in a singly-linked-list. */
	typedef struct ThreadNode {
		/* Pointer to the next thread node in the threads list. */
		struct ThreadNode		*link;

		/* The alarm message type that the thread is handling. */
		uint_fast32_t			msg_type;
		/* The thread's identifier(ID) returned by pthread_create(). */
		pthread_t				id;
	} Thread;

	/* Structure encapsulating each type A command as a node in a singly-linked-list. */
	typedef struct CmdNodeA {
		/* Pointer to the next type A command node in the global commands list. */
		struct CmdNodeA			*link;

		/*
		 * The amount of time to wait in seconds between two
		 * consecutive prints of the current alarm's message.
		 */
		uint_fast32_t			wait_time;

		/* The alarm's message type used to categorize different alarms. */
		uint_fast32_t			msg_type;
		/* The alarm's message number used to uniquely identify different alarms. */
		uint_fast32_t			msg_num;

		/*
		 * The alarm's message of maximum length MAX_MSG_LEN.
		 * However msg is defined to be of length MAX_MSG_LEN + 1
		 * since it needs to be able to store MAX_MSG_LEN chars
		 * which represent the message but also 1 nullchar('\0')
		 * at the end to terminate the string.
		 */
		char					msg[MAX_MSG_LEN + 1];
	} CmdA;

	/* Structure encapsulating each type B command as a node in a singly-linked-list. */
	typedef struct CmdNodeB {
		/* Pointer to the next type B command node in the global commands list. */
		struct CmdNodeB			*link;

		/* The alarm's message type used to categorize different alarms. */
		uint_fast32_t			msg_type;
		/* The thread's identifier(ID) returned by pthread_create(). */
		pthread_t				id;

		/* The state of the command either PROCESSED(true) or UNPROCESSED(false). */
		bool					is_processed;
	} CmdB;

	/* Structure encapsulating each type C command as a node in a singly-linked-list. */
	typedef struct CmdNodeC {
		/* Pointer to the next type C command node in the global commands list. */
		struct CmdNodeC			*link;

		/* The alarm's message number used to uniquely identify different alarms. */
		uint_fast32_t			msg_num;
	} CmdC;



	/* Function Prototypes */

	/* Command Validation Functions */

	/*
	 * Check to see if the command stored in the given string(line) of the
	 * given length(len) is a valid command of the given type(cmd_type).
	 *
	 * Preconditions:
	 * 		1. line != NULL
	 * 		2. line is a valid C string(i.e., null-terminated)
	 * 		3. strlen(line) == len
	 * 		4. (cmd_type == 'B') || (cmd_type == 'C')
	 * 		5. n != 0
	 *
	 * Returns:
	 * 		1. -1	if there is a memory allocation error
	 * 		2.  0	if it is not valid
	 * 		3.  1	if it is valid
	 */
	int is_valid_cmd(const char *line, const size_t len,
				const char cmd_type, const uint_fast32_t n);



	/* Singly Linked List Functions */

	/*
	 * Get the next Alarm after node in the global alarms list.
	 *
	 * Precondition: node != NULL
	 *
	 * Returns: node->link
	 */
	Alarm * next_alarm(const Alarm *node);

	/*
	 * Get the next Alarm after node in the local alarms list.
	 *
	 * Precondition: node != NULL
	 *
	 * Returns: node->link_handle
	 */
	Alarm * next_handled_alarm(const Alarm *node);

	/*
	 * Insert the Alarm structure pointed to by new_alarm as the
	 * new first element of the given singly-linked-list. The list is
	 * accessed and/or modified through *head_ptr which points to the
	 * head of the list.
	 *
	 * Attaching is done by setting the link attribute.
	 *
	 * Preconditions:
	 * 		1. head_ptr != NULL
	 * 		2. new_alarm != NULL
	 * 		3. *head_ptr != new_alarm
	 * 		4. new_alarm->link == NULL
	 * 		5. new_alarm->link_handle == NULL
	 */
	void insert_first_alarm(Alarm **head_ptr, Alarm *new_alarm);

	/*
	 * Insert the Alarm structure pointed to by new_alarm as the
	 * new first element of the given singly-linked-list. The list is
	 * accessed and/or modified through *head_ptr which points to the
	 * head of the list.
	 *
	 * Attaching is done by setting the link_handle attribute.
	 *
	 * Preconditions:
	 * 		1. head_ptr != NULL
	 * 		2. new_alarm != NULL
	 * 		3. *head_ptr != new_alarm
	 * 		4. new_alarm->link_handle == NULL
	 */
	void insert_first_handled_alarm(Alarm **head_ptr, Alarm *new_alarm);

	/*
	 * Insert the Alarm structure pointed to by new_alarm after node
	 * by attaching them through setting the link attribute.
	 *
	 * Preconditions:
	 * 		1. node != NULL
	 * 		2. new_alarm != NULL
	 * 		3. new_alarm->link == NULL
	 * 		4. new_alarm->link_handle == NULL
	 */
	void insert_after_alarm(Alarm *node, Alarm *new_alarm);

	/*
	 * Insert the Alarm structure pointed to by new_alarm after node
	 * by attaching them through setting the link_handle attribute.
	 *
	 * Preconditions:
	 * 		1. node != NULL
	 * 		2. new_alarm != NULL
	 * 		3. new_alarm->link_handle == NULL
	 */
	void insert_after_handled_alarm(Alarm *node, Alarm *new_alarm);

	/*
	 * Compare the Alarm structure pointed to by a
	 * against the Alarm structure pointed to by b
	 * to determine relative order.
	 *
	 * Preconditions:
	 * 		1. a != NULL
	 * 		2. b != NULL
	 *
	 * Returns:
	 * 		1. false
	 * 					if *a >= *b
	 * 		2. true
	 * 					if *a <  *b
	 */
	bool is_less_alarm(const Alarm *a, const Alarm *b);

	/*
	 * Compare the Alarm structure pointed to by a
	 * against the Alarm structure pointed to by b
	 * to determine equality.
	 *
	 * Preconditions:
	 * 		1. a != NULL
	 * 		2. b != NULL
	 *
	 * Returns:
	 * 		1. false
	 * 					if *a != *b
	 * 		2. true
	 * 					if *a == *b
	 */
	bool is_equal_alarm(const Alarm *a, const Alarm *b);

	/*
	 * replace_alarm, copies the necessary information from the Alarm structure
	 * pointed to by new_alarm into the existing Alarm structure in the alarms
	 * list pointed to by existing_alarm. It will then free the memory allocated
	 * to the Alarm structure pointed to by new_alarm by calling void free(void *ptr).
	 *
	 * Preconditions:
	 * 		1. existing_alarm != NULL
	 * 		2. new_alarm != NULL
	 * 		3. new_alarm->link == NULL
	 * 		4. new_alarm->link_handle == NULL
	 * 		5. existing_alarm != new_alarm
	 * 		6. is_equal_alarm(existing_alarm, new_alarm)
	 * 		7. new_alarm can be safely freed by using free(new_alarm)
	 *
	 * Returns:
	 * 		Pointer to the thread ID of the alarm handling thread that
	 * 		was handling the Alarm structure pointed to by existing_alarm
	 * 		only if the thread ended up cancelling itself. If no such
	 * 		thread exists then the function will return NULL.
	 */
	pthread_t * replace_alarm(Alarm *existing_alarm, Alarm *new_alarm);

	/*
	 * insert_alarm, inserts a new Alarm structure pointed to by new_alarm
	 * into the given alarms singly-linked-list. The list is accessed and/or
	 * modified through *head_ptr which points to the head of the list.
	 *
	 * It iterates through the list using the given function pointed to
	 * by after and attaches two nodes using the given function pointed
	 * to by insert_after.
	 *
	 * Furthermore, it maintains the sorted order of the list by using
	 * the is_less_alarm function to compare alarm nodes. (Insertion Sort)
	 *
	 * It also maintains the uniqueness of alarms by treating their message
	 * numbers(msg_num) as the sole primary key as specified by the following
	 * function: is_equal_alarm
	 *
	 * It will replace an existing alarm with the same message number as the
	 * new alarm's message number using the replace_alarm function.
	 *
	 * When the command thread calls this function, it is possible for an
	 * alarm to be replaced. In this case, it will wait on the given conditional
	 * variable and mutex which gives the responsible alarm handler thread
	 * the chance to safely detach the old alarm from its own handled list and
	 * then send a signal signifying that it has performed this action so that
	 * the replacement can proceed as required.
	 *
	 * However, when an alarm handler thread calls this function, only link_handle
	 * attributes are being modified which means that the alarm is not a new
	 * structure as a whole, it is just being added to that thread's local list
	 * which has been embedded into the global list.
	 *
	 * Preconditions:
	 * 		1.  head_ptr != NULL
	 * 		2.  new_alarm != NULL
	 * 		3.  new_alarm->link == NULL
	 * 		4.  new_alarm->link_handle == NULL
	 * 		5.  new_alarm does not point to any of the alarms already in the list
	 * 		6.  new_alarm can be safely freed by using free(new_alarm)
	 * 		7.  global_list  == (command thread calling)
	 * 		8.  !global_list == (alarm handler thread calling)
	 * 		9.  global_list  == (cond_var_ptr != NULL)
	 * 		10. global_list  == (mutex_ptr != NULL)
	 * 		11. (mutex_ptr != NULL) implies (*mutex_ptr is locked by caller)
	 *
	 * Returns: replace_alarm's return when appropriate and NULL otherwise.
	 */
	pthread_t * insert_alarm(Alarm **head_ptr, Alarm *new_alarm,
				Alarm * (*after)(const Alarm *),
				void (*insert_first)(Alarm **, Alarm *),
				void (*insert_after)(Alarm *, Alarm *),
				bool global_list, pthread_cond_t *cond_var_ptr,
				pthread_mutex_t *mutex_ptr);



	/* Thread Functions */

	/*
	 * Cancel the thread with the given ID.
	 *
	 * Precondition: id is a valid thread ID.
	 *
	 * Returns:
	 * 		1. THREAD_CANCEL_ERR
	 * 										if pthread_cancel fails
	 * 		2. THREAD_JOIN_ERR
	 * 										if pthread_join fails
	 * 		3. THREAD_CANCEL_RETVAL_ERR
	 * 										if PTHREAD_CANCELED is NOT
	 * 										returned by the thread
	 * 		4. 0
	 * 										on success
	 */
	int cancel_thread(pthread_t id);

#endif
