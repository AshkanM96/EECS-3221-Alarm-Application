/**************************************************************************
 *
 * Author:
 * 					Ashkan Moatamed
 *
 *
 *
 * alarm_def.c
 *
 *
 *
 * Implementation of the Functions
 * defined in alarm_def.h
 *
 *************************************************************************/

#include "alarm_def.h"

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
 * 		5. msg_type != 0
 *
 * Returns:
 * 		1. -1	if there is a memory allocation error
 * 		2.  0	if it is not valid
 * 		3.  1	if it is valid
 */
int is_valid_cmd(const char *line, const size_t len,
			const char cmd_type, const uint_fast32_t msg_type) {

	/* Stores the return value of the current method. */
	int result = 1; /* Assume its validity. */

	/* Stores the expected command string. */
	char *expected = NULL;



	/* Allocate memory for expected. */
	expected = MALLOC_ARRAY(char, len);
	if (expected == NULL) { return -1; }

	if (cmd_type == 'B') {
		sprintf(expected, "Create_Thread: MessageType(%" PRIuFAST32 ")", msg_type);
	} else { /* (cmd_type == 'C') */
		sprintf(expected, "Terminate: MessageType(%" PRIuFAST32 ")", msg_type);
	}

	/* Validate the command and set result if invalid. */
	if (strcmp(line, expected) != 0) {
		/* line and expected are NOT equal. */
		result = 0;
	}

	/* Free memory allocated to expected. */
	free(expected);



	return result;
}



/* Singly Linked List Functions */

/*
 * Get the next Alarm after node in the global alarms list.
 *
 * Precondition: node != NULL
 *
 * Returns: node->link
 */
Alarm * next_alarm(const Alarm *node) {
	return node->link;
}

/*
 * Get the next Alarm after node in the local alarms list.
 *
 * Precondition: node != NULL
 *
 * Returns: node->link_handle
 */
Alarm * next_handled_alarm(const Alarm *node) {
	return node->link_handle;
}

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
void insert_first_alarm(Alarm **head_ptr, Alarm *new_alarm) {
	new_alarm->link = *head_ptr;
	*head_ptr = new_alarm;
}

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
void insert_first_handled_alarm(Alarm **head_ptr, Alarm *new_alarm) {
	new_alarm->link_handle = *head_ptr;
	*head_ptr = new_alarm;
}

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
void insert_after_alarm(Alarm *node, Alarm *new_alarm) {
	new_alarm->link = node->link;
	node->link = new_alarm;
}

/*
 * Insert the Alarm structure pointed to by new_alarm after node
 * by attaching them through setting the link_handle attribute.
 *
 * Preconditions:
 * 		1. node != NULL
 * 		2. new_alarm != NULL
 * 		3. new_alarm->link_handle == NULL
 */
void insert_after_handled_alarm(Alarm *node, Alarm *new_alarm) {
	new_alarm->link_handle = node->link_handle;
	node->link_handle = new_alarm;
}

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
bool is_less_alarm(const Alarm *a, const Alarm *b) {
	/*
	 * Check to see if they both point to
	 * the same Alarm structure in memory.
	 */
	if (a == b) { return false; }

	/* Sort by alarm message types(msg_type) first. */
	if (a->msg_type < b->msg_type) {
		return true;
	} else if (a->msg_type > b->msg_type) {
		/*
		 * a cannot be less than b
		 * if its msg_type is greater.
		 */
		return false;
	}

	/*
	 * Due to above checks, we now know that:
	 * a->msg_type == b->msg_type therefore
	 * just compare their wait times to break
	 * the tie in comparison.
	 */
	return (a->wait_time < b->wait_time);
}

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
 * Preconditions:
 * 		1. head_ptr != NULL
 * 		2. new_alarm != NULL
 * 		3. new_alarm->link == NULL
 * 		4. new_alarm->link_handle == NULL
 * 		5. new_alarm does not point to any of the alarms already in the list
 */
void insert_alarm(Alarm **head_ptr, Alarm *new_alarm,
			Alarm * (*after)(const Alarm *),
			void (*insert_first)(Alarm **, Alarm *),
			void (*insert_after)(Alarm *, Alarm *)) {

	/* Alarm pointers used for iterating over the alarms list. */
	Alarm *curr_alarm = NULL, *next_alarm = NULL;



	/*
	 * Special Case 1:
	 * Check to see if the alarms list is empty.
	 */
	if (*head_ptr == NULL) {
		*head_ptr = new_alarm;
		return;
	}

	/*
	 * Special Case 2:
	 * Check to see if the new_alarm is less than
	 * the current first element in the list which
	 * means that it has to become the new first element.
	 */
	if (is_less_alarm(new_alarm, *head_ptr)) {
		/* (*new_alarm < **head_ptr) */
		(*insert_first)(head_ptr, new_alarm);
		return;
	}



	/* General Case: */

	for (curr_alarm = *head_ptr; (next_alarm = (*after)(curr_alarm)) != NULL; /* Update inside. */) {
		if (is_less_alarm(new_alarm, next_alarm)) {
			/* (*new_alarm < *next_alarm) */
			break;
		}

		/* Move to the next element since (*next_alarm <= *new_alarm). */
		curr_alarm = next_alarm;
	}
	/*
	 * At this point we know that: *curr_alarm <= *new_alarm < *next_alarm
	 * Therefore, we should insert the Alarm structure pointed to by
	 * new_alarm between curr_alarm and next_alarm which is the same
	 * as inserting it after curr_alarm.
	 */
	(*insert_after)(curr_alarm, new_alarm);
}



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
int cancel_thread(pthread_t id) {
	/* Stores the return status of functions. */
	int status = 0;
	/* Stores the return value of the thread. */
	void *retval = NULL;



	/* Issue cancellation request to the thread. */
	status = pthread_cancel(id);
	if (status != 0) { return THREAD_CANCEL_ERR; }

	/*
	 * Join with the thread to check if the
	 * cancellation completed. Joining is the
	 * only way to know that cancellation has
	 * completed or not through checking the
	 * thread's return value stored in retval.
	 */
	status = pthread_join(id, &retval);
	if (status != 0) { return THREAD_JOIN_ERR; }

	/* Check return value against PTHREAD_CANCELED. */
	if (retval != PTHREAD_CANCELED) {
		return THREAD_CANCEL_RETVAL_ERR;
	}



	return 0;
}
