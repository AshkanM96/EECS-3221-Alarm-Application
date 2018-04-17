/*
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
 */

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
 * 		5. n != 0
 *
 * Returns:
 * 		1. -1	if there is a memory allocation error
 * 		2.  0	if it is not valid
 * 		3.  1	if it is valid
 */
int is_valid_cmd(const char *line, const size_t len,
			const char cmd_type, const uint_fast32_t n) {

	/* Stores the return value of the current method. */
	int result = 1; /* Assume its validity. */

	/* Stores the expected command string. */
	char *expected = NULL;



	/* Allocate memory for expected. */
	expected = MALLOC_ARRAY(char, len);
	if (expected == NULL) { return -1; }

	if (cmd_type == 'B') {
		sprintf(expected, "Create_Thread: MessageType(%" PRIuFAST32 ")", n);
	} else { /* (cmd_type == 'C') */
		sprintf(expected, "Cancel: Message(%" PRIuFAST32 ")", n);
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
	if (a == b) { return true; }

	/*
	 * Alarm message numbers are used to
	 * uniquely identify different alarms
	 * (i.e., primary key) therefore just
	 * compare their msg_num attributes to
	 * determine their relative order.
	 */
	return (a->msg_num < b->msg_num);
}

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
bool is_equal_alarm(const Alarm *a, const Alarm *b) {
	/*
	 * Check to see if they both point to
	 * the same Alarm structure in memory.
	 */
	if (a == b) { return true; }

	/*
	 * Alarm message numbers are used to
	 * uniquely identify different alarms
	 * (i.e., primary key) therefore just
	 * check equality of their msg_num
	 * attributes to determine their equality.
	 */
	return (a->msg_num == b->msg_num);
}

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
pthread_t * replace_alarm(Alarm *existing_alarm, Alarm *new_alarm) {
	/* Stores the return value of the current method. */
	pthread_t *result = NULL;



	if (existing_alarm->is_assigned) {
		/*
		 * We only have to worry about the
		 * replacement of an assigned alarm.
		 */
		existing_alarm->is_replaced = true;

		if (existing_alarm->link_handle == NULL) {
			/*
			 * We know that the alarm handler thread is
			 * going to cancel itself since the current
			 * alarm is being handled(is_assigned) and no
			 * other alarm is being handled(link_handle == NULL)
			 * by this alarm handler thread.
			 */
			result = existing_alarm->handler_id;
		}
	}



	/* Copy necessary information from new_alarm into existing_alarm. */

	/*
	 * link needs to remain unchanged.
	 *
	 * link_handle will be reset by handling thread if needed.
	 */
	existing_alarm->wait_time = new_alarm->wait_time;
	existing_alarm->msg_type = new_alarm->msg_type;
	/*
	 * msg_num's are the same as a result of
	 * is_equal_alarm(existing_alarm, new_alarm)
	 * returning true.
	 */
	strcpy(existing_alarm->msg, new_alarm->msg); /* Set existing_alarm's message. */
	/*
	 * is_assigned and handler_id will be reset by handling thread if needed.
	 *
	 * is_replaced has been set in the above if needed.
	 *
	 * is_cancelled needs to remain unchanged(false).
	 */



	/* Free memory allocated to new_alarm. */
	free(new_alarm);



	return result;
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
			pthread_mutex_t *mutex_ptr) {

	/* Stores the return value of the current method. */
	pthread_t *result = NULL;



	/* Alarm pointers used for iterating over the alarms list. */
	Alarm *curr_alarm = NULL, *next_alarm = NULL;



	/*
	 * Special Case 1:
	 * Check to see if the alarms list is empty.
	 */
	if (*head_ptr == NULL) {
		*head_ptr = new_alarm;
		return NULL;
	}

	/*
	 * Special Case 2:
	 * Check to see if the new_alarm is equal to the
	 * current first element in the list which means
	 * that it has to replace the current first element.
	 */
	if (global_list && is_equal_alarm(new_alarm, *head_ptr)) {
		/* (*new_alarm == **head_ptr) */
		result = replace_alarm(*head_ptr, new_alarm);
		/*
		 * Check if is_replaced flag has been set denoting
		 * that the alarm needs to be reset by the handling
		 * thread. Here it is the same as checking is_assigned.
		 */
		if ((*head_ptr)->is_replaced) {
			/*
			 * Block this thread(command handler) until another thread(alarm handler) signals
			 * the conditional variable pointed to by cond_var_ptr. While this thread is
			 * blocked, the mutex is released, then re-aquired before this thread is woken
			 * up and the call returns.
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
			while ((*head_ptr)->is_assigned) {
				errno = pthread_cond_wait(cond_var_ptr, mutex_ptr);
				if (errno != 0) {
					EXIT_ERR(COND_VAR_WAIT_ERR_MSG, COND_VAR_WAIT_ERR);
				}
			} /* (!(*head_ptr)->is_assigned) */
			(*head_ptr)->is_replaced = false;
		}
		return result;
	}

	/*
	 * Special Case 3:
	 * Check to see if the new_alarm is less than
	 * the current first element in the list which
	 * means that it has to become the new first element.
	 */
	if (is_less_alarm(new_alarm, *head_ptr)) {
		/* (*new_alarm < **head_ptr) */
		(*insert_first)(head_ptr, new_alarm);
		return NULL;
	}



	/* General Case: */

	for (curr_alarm = *head_ptr; (next_alarm = (*after)(curr_alarm)) != NULL; /* Update inside. */) {
		if (global_list && is_equal_alarm(new_alarm, next_alarm)) {
			/* (*new_alarm == *next_alarm) */
			result = replace_alarm(next_alarm, new_alarm);
			/*
			 * Check if is_replaced flag has been set denoting
			 * that the alarm needs to be reset by the handling
			 * thread. Here it is the same as checking is_assigned.
			 */
			if (next_alarm->is_replaced) {
				/*
				 * Block this thread(command handler) until another thread(alarm handler) signals
				 * the conditional variable pointed to by cond_var_ptr. While this thread is
				 * blocked, the mutex is released, then re-aquired before this thread is woken
				 * up and the call returns.
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
				while (next_alarm->is_assigned) {
					errno = pthread_cond_wait(cond_var_ptr, mutex_ptr);
					if (errno != 0) {
						EXIT_ERR(COND_VAR_WAIT_ERR_MSG, COND_VAR_WAIT_ERR);
					}
				} /* (!next_alarm->is_assigned) */
				next_alarm->is_replaced = false;
			}
			return result;
		} else if (is_less_alarm(new_alarm, next_alarm)) {
			/* (*new_alarm < *next_alarm) */
			break;
		}

		/* Move to the next element since (*next_alarm < *new_alarm). */
		curr_alarm = next_alarm;
	}
	/*
	 * At this point we know that: *curr_alarm < *new_alarm < *next_alarm
	 * Therefore, we should insert the Alarm structure pointed to by
	 * new_alarm between curr_alarm and next_alarm which is the same
	 * as inserting it after curr_alarm.
	 */
	(*insert_after)(curr_alarm, new_alarm);
	return NULL;
}



/* Time Functions */

/*
 * Returns: Current time by invoking time(NULL)
 * and casting the result into uint_fast64_t.
 */
uint_fast64_t now(void) {
	return ((uint_fast64_t) time(NULL));
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
