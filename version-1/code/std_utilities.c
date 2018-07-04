/*
 * Author:
 * 					Ashkan Moatamed
 *
 *
 *
 * std_utilities.c
 *
 *
 *
 * Implementation of the Functions
 * defined in std_utilities.h
 */

#include "std_utilities.h"

/* Number Functions */

/* Returns: Greatest common divisor of x and y. */
uint_fast64_t gcd(uint_fast64_t x, uint_fast64_t y) {
	uint_fast64_t min = 0, max = 0;
	uint_fast64_t rem = 0; /* Stores (max mod min). */



	if (x == y) {
		/*
		 * gcd(x, x) == x or gcd(y, y) == y
		 *
		 * Note that we have implicitly
		 * defined gcd(0, 0) == 0 here.
		 */
		return y;
	} else if ((x == 1) || (y == 1)) {
		/*
		 * We know that exactly one of
		 * x and y is 1 therefore we have:
		 *
		 * gcd(1, y) == 1 and gcd(x, 1) == 1
		 *
		 * Note that even if both were 1,
		 * the result would still be 1 as
		 * returned by the first case.
		 */
		return 1;
	} else if (x == 0) {
		/*
		 * We know that x != y and x == 0 therefore:
		 *
		 * gcd(0, y) == y
		 */
		return y;
	} else if (y == 0) {
		/*
		 * We know that x != y and y == 0 therefore:
		 *
		 * gcd(x, 0) == x
		 */
		return x;
	}



	/* Find min and max from x and y. */
	if (x < y) {
		min = x; max = y;
	} else { /* x >= y */
		min = y; max = x;
	}

	/* Euclid's algorithm: */
	while (min != 0) {
		/* Find remainder upon integer division. */
		rem = max % min;

		/*
		 * Update max then min using the following:
		 * gcd(min, max) == gcd(rem, min)
		 */
		max = min;
		min = rem;
	}
	return max;
}

/* Returns: Least common multiple of x and y. */
uint_fast64_t lcm(uint_fast64_t x, uint_fast64_t y) {
	if ((x == 0) || (y == 0)) {
		/* lcm(0, y) == 0 == lcm(x, 0) */
		return 0;
	} /* (x != 0) && (y != 0) */
	return (x / gcd(x, y)) * y;
}

/*
 * Parse the given number of type int_fast64_t as
 * a number of type int_fast8_t.
 *
 * The function sets errno to -1 if the int_fast64_t
 * number does not represent a valid int_fast8_t number
 * and returns 0.
 *
 * Precondition: errno == 0
 *
 * Returns: The int_fast8_t number if valid and 0 if not
 * but also sets errno to -1 when invalid to distinguish
 * between a valid parsing of 0 and an invalid return.
 */
int_fast8_t f64_to_f8(const int_fast64_t i) {
	/* Check if i is in [INT_FAST8_MIN, INT_FAST8_MAX]. */
	if ((INT_FAST8_MIN <= i) && (i <= INT_FAST8_MAX)) {
		return ((int_fast8_t) i);
	}

	/* i is not a valid int_fast8_t. */
	errno = -1;
	return 0;
}

/*
 * Parse the given number of type int_fast64_t as
 * a number of type int_fast16_t.
 *
 * The function sets errno to -1 if the int_fast64_t
 * number does not represent a valid int_fast16_t number
 * and returns 0.
 *
 * Precondition: errno == 0
 *
 * Returns: The int_fast16_t number if valid and 0 if not
 * but also sets errno to -1 when invalid to distinguish
 * between a valid parsing of 0 and an invalid return.
 */
int_fast16_t f64_to_f16(const int_fast64_t i) {
	/* Check if i is in [INT_FAST16_MIN, INT_FAST16_MAX]. */
	if ((INT_FAST16_MIN <= i) && (i <= INT_FAST16_MAX)) {
		return ((int_fast16_t) i);
	}

	/* i is not a valid int_fast16_t. */
	errno = -1;
	return 0;
}

/*
 * Parse the given number of type int_fast64_t as
 * a number of type int_fast32_t.
 *
 * The function sets errno to -1 if the int_fast64_t
 * number does not represent a valid int_fast32_t number
 * and returns 0.
 *
 * Precondition: errno == 0
 *
 * Returns: The int_fast32_t number if valid and 0 if not
 * but also sets errno to -1 when invalid to distinguish
 * between a valid parsing of 0 and an invalid return.
 */
int_fast32_t f64_to_f32(const int_fast64_t i) {
	/* Check if i is in [INT_FAST32_MIN, INT_FAST32_MAX]. */
	if ((INT_FAST32_MIN <= i) && (i <= INT_FAST32_MAX)) {
		return ((int_fast32_t) i);
	}

	/* i is not a valid int_fast32_t. */
	errno = -1;
	return 0;
}

/*
 * Parse the given number of type int_fast64_t as
 * a number of type uint_fast8_t.
 *
 * The function sets errno to -1 if the int_fast64_t
 * number does not represent a valid uint_fast8_t number
 * and returns 0.
 *
 * Precondition: errno == 0
 *
 * Returns: The uint_fast8_t number if valid and 0 if not
 * but also sets errno to -1 when invalid to distinguish
 * between a valid parsing of 0 and an invalid return.
 */
uint_fast8_t f64_to_uf8(const int_fast64_t i) {
	const uint_fast64_t u = f64_to_uf64(i);
	return (errno == 0 ? uf64_to_uf8(u) : 0);
}

/*
 * Parse the given number of type int_fast64_t as
 * a number of type uint_fast16_t.
 *
 * The function sets errno to -1 if the int_fast64_t
 * number does not represent a valid uint_fast16_t number
 * and returns 0.
 *
 * Precondition: errno == 0
 *
 * Returns: The uint_fast16_t number if valid and 0 if not
 * but also sets errno to -1 when invalid to distinguish
 * between a valid parsing of 0 and an invalid return.
 */
uint_fast16_t f64_to_uf16(const int_fast64_t i) {
	const uint_fast64_t u = f64_to_uf64(i);
	return (errno == 0 ? uf64_to_uf16(u) : 0);
}

/*
 * Parse the given number of type int_fast64_t as
 * a number of type uint_fast32_t.
 *
 * The function sets errno to -1 if the int_fast64_t
 * number does not represent a valid uint_fast32_t number
 * and returns 0.
 *
 * Precondition: errno == 0
 *
 * Returns: The uint_fast32_t number if valid and 0 if not
 * but also sets errno to -1 when invalid to distinguish
 * between a valid parsing of 0 and an invalid return.
 */
uint_fast32_t f64_to_uf32(const int_fast64_t i) {
	const uint_fast64_t u = f64_to_uf64(i);
	return (errno == 0 ? uf64_to_uf32(u) : 0);
}

/*
 * Parse the given number of type int_fast64_t as
 * a number of type uint_fast64_t.
 *
 * The function sets errno to -1 if the int_fast64_t
 * number does not represent a valid uint_fast64_t number
 * and returns 0.
 *
 * Precondition: errno == 0
 *
 * Returns: The uint_fast64_t number if valid and 0 if not
 * but also sets errno to -1 when invalid to distinguish
 * between a valid parsing of 0 and an invalid return.
 */
uint_fast64_t f64_to_uf64(const int_fast64_t i) {
	/* Check if i is in [0, UINT_FAST64_MAX]. */
	if (i >= 0) {
		if (sizeof(int_fast64_t) <= sizeof(uint_fast64_t)) {
			/*
			 * uint_fast64_t has at least as many bytes as
			 * int_fast64_t which means that a number of type
			 * int_fast64_t can NEVER have a larger value
			 * than UINT_FAST64_MAX due to being signed and
			 * having less bytes used to represent it.
			 */
			return ((uint_fast64_t) i);
		} else {
			/* (sizeof(int_fast64_t) > sizeof(uint_fast64_t)) */
			if (i <= ((int_fast64_t) UINT_FAST64_MAX)) {
				return ((uint_fast64_t) i);
			}
		}
	}

	/* i is not a valid uint_fast64_t. */
	errno = -1;
	return 0;
}

/*
 * Parse the given number of type uint_fast64_t as
 * a number of type uint_fast8_t.
 *
 * The function sets errno to -1 if the uint_fast64_t
 * number does not represent a valid uint_fast8_t number
 * and returns 0.
 *
 * Precondition: errno == 0
 *
 * Returns: The uint_fast8_t number if valid and 0 if not
 * but also sets errno to -1 when invalid to distinguish
 * between a valid parsing of 0 and an invalid return.
 */
uint_fast8_t uf64_to_uf8(const uint_fast64_t u) {
	/* Check if u is in [0, UINT_FAST8_MAX]. */
	if (u <= UINT_FAST8_MAX) {
		return ((uint_fast8_t) u);
	}

	/* u is not a valid uint_fast8_t. */
	errno = -1;
	return 0;
}

/*
 * Parse the given number of type uint_fast64_t as
 * a number of type uint_fast16_t.
 *
 * The function sets errno to -1 if the uint_fast64_t
 * number does not represent a valid uint_fast16_t number
 * and returns 0.
 *
 * Precondition: errno == 0
 *
 * Returns: The uint_fast16_t number if valid and 0 if not
 * but also sets errno to -1 when invalid to distinguish
 * between a valid parsing of 0 and an invalid return.
 */
uint_fast16_t uf64_to_uf16(const uint_fast64_t u) {
	/* Check if u is in [0, UINT_FAST16_MAX]. */
	if (u <= UINT_FAST16_MAX) {
		return ((uint_fast16_t) u);
	}

	/* u is not a valid uint_fast16_t. */
	errno = -1;
	return 0;
}

/*
 * Parse the given number of type uint_fast64_t as
 * a number of type uint_fast32_t.
 *
 * The function sets errno to -1 if the uint_fast64_t
 * number does not represent a valid uint_fast32_t number
 * and returns 0.
 *
 * Precondition: errno == 0
 *
 * Returns: The uint_fast32_t number if valid and 0 if not
 * but also sets errno to -1 when invalid to distinguish
 * between a valid parsing of 0 and an invalid return.
 */
uint_fast32_t uf64_to_uf32(const uint_fast64_t u) {
	/* Check if u is in [0, UINT_FAST32_MAX]. */
	if (u <= UINT_FAST32_MAX) {
		return ((uint_fast32_t) u);
	}

	/* u is not a valid uint_fast32_t. */
	errno = -1;
	return 0;
}

/*
 * Parse the given number of type uint_fast64_t as
 * a number of type int_fast8_t.
 *
 * The function sets errno to -1 if the uint_fast64_t
 * number does not represent a valid int_fast8_t number
 * and returns 0.
 *
 * Precondition: errno == 0
 *
 * Returns: The int_fast8_t number if valid and 0 if not
 * but also sets errno to -1 when invalid to distinguish
 * between a valid parsing of 0 and an invalid return.
 */
int_fast8_t uf64_to_f8(const uint_fast64_t u) {
	const int_fast64_t i = uf64_to_f64(u);
	return (errno == 0 ? f64_to_f8(i) : 0);
}

/*
 * Parse the given number of type uint_fast64_t as
 * a number of type int_fast16_t.
 *
 * The function sets errno to -1 if the uint_fast64_t
 * number does not represent a valid int_fast16_t number
 * and returns 0.
 *
 * Precondition: errno == 0
 *
 * Returns: The int_fast16_t number if valid and 0 if not
 * but also sets errno to -1 when invalid to distinguish
 * between a valid parsing of 0 and an invalid return.
 */
int_fast16_t uf64_to_f16(const uint_fast64_t u) {
	const int_fast64_t i = uf64_to_f64(u);
	return (errno == 0 ? f64_to_f16(i) : 0);
}

/*
 * Parse the given number of type uint_fast64_t as
 * a number of type int_fast32_t.
 *
 * The function sets errno to -1 if the uint_fast64_t
 * number does not represent a valid int_fast32_t number
 * and returns 0.
 *
 * Precondition: errno == 0
 *
 * Returns: The int_fast32_t number if valid and 0 if not
 * but also sets errno to -1 when invalid to distinguish
 * between a valid parsing of 0 and an invalid return.
 */
int_fast32_t uf64_to_f32(const uint_fast64_t u) {
	const int_fast64_t i = uf64_to_f64(u);
	return (errno == 0 ? f64_to_f32(i) : 0);
}

/*
 * Parse the given number of type uint_fast64_t as
 * a number of type int_fast64_t.
 *
 * The function sets errno to -1 if the uint_fast64_t
 * number does not represent a valid int_fast64_t number
 * and returns 0.
 *
 * Precondition: errno == 0
 *
 * Returns: The int_fast64_t number if valid and 0 if not
 * but also sets errno to -1 when invalid to distinguish
 * between a valid parsing of 0 and an invalid return.
 */
int_fast64_t uf64_to_f64(const uint_fast64_t u) {
	/* Check if u is in [INT_FAST64_MIN, INT_FAST64_MAX]. */
	if (sizeof(uint_fast64_t) < sizeof(int_fast64_t)) {
		/*
		 * uint_fast64_t is being represented by less
		 * bytes than int_fast64_t which means that a
		 * number of type uint_fast64_t can NEVER have
		 * a larger value than INT_FAST64_MAX due to the
		 * extra bytes used to represent int_fast64_t.
		 */
		return ((int_fast64_t) u);
	} else {
		/* (sizeof(uint_fast64_t) >= sizeof(int_fast64_t)) */
		if (u <= ((uint_fast64_t) INT_FAST64_MAX)) {
			return ((int_fast64_t) u);
		}
	}

	/* u is not a valid int_fast64_t. */
	errno = -1;
	return 0;
}

/*
 * Parse the given character as a single digit.
 *
 * Returns: The parsed digit if valid and -1 if not.
 */
int ctoi(const char c) {
	if (isdigit(c) == 0) {
		/* c is not valid a digit(i.e., 0 through 9). */
		return -1;
	}
	return ((int) (c - '0'));
}

/*
 * Parse the given string as a number of type
 * uint_fast64_t.
 *
 * The function sets errno to -1 if the string
 * does not represent a valid uint_fast64_t number
 * and returns 0.
 *
 * The function set errno to 1 if the string
 * represents a number larger than UINT_FAST64_MAX
 * and returns the parsed value which has wrapped
 * around back to 0 (potentially multiple) times.
 *
 * Preconditions:
 * 		1. errno == 0
 * 		2. s != NULL
 * 		3. s is a valid C string(i.e., null-terminated)
 *
 * Returns: The uint_fast64_t number if valid and 0 if not
 * but also sets errno to -1 when invalid to distinguish
 * between a valid parsing of 0 and an invalid return.
 */
uint_fast64_t str_to_uf64(const char *s) {
	/* Loop variables. */
	size_t l = 0, i = 0;
	/* Stores the parsed number when valid. */
	uint_fast64_t result = 0;
	/*
	 * The following two variables are used
	 * during the string parsing to determine
	 * if result has overflowed and wrapped
	 * around back to 0.
	 */
	bool first_digit = false;
	uint_fast64_t old_result = 0;



	/*
	 * Loop through the string and check
	 * that all of its characters are digits.
	 */
	for (l = 0; s[l] != '\0'; l++) {
		if (isdigit(s[l]) == 0) {
			/* s[l] is not valid a digit(i.e., 0 through 9). */
			errno = -1;
			return 0;
		}
	}
	/*
	 * Note that at this point, we have already eliminated the
	 * possibility of the string representing a negative number
	 * by enforcing that every single one of its characters to be
	 * a digit.
	 *
	 * This is due to the fact that negative numbers would have
	 * to start with '-' which is not a digit thus being eliminated
	 * in the first iteration of the above for loop.
	 */

	/*
	 * At this point we know that the length of the
	 * string is just stored in l since we looped till
	 * the terminating null character.
	 */
	if (l == 0) {
		/*
		 * The empty string cannot be a valid
		 * uint_fast64_t number hence check (l == 0).
		 */
		errno = -1;
		return 0;
	}
	l--; /* l is now the last index(i.e., l == strlen(s) - 1). */



	/* Attempt to parse the string as a uint_fast64_t number. */
	first_digit = true; /* Parsing first digit so set flag to true. */
	old_result = result = 0;
	for (i = 0; i <= l; i++) {
		result += (unsigned) ctoi(s[l - i]); /* s[l - i] is guaranteed to be valid. */
		result *= 10;
		/*
		 * Note that the value of result may reach the maximum and wrap
		 * around back to 0. We know this from the standard which states:
		 * "A computation involving unsigned operands can never overflow,
		 * because a result that cannot be represented by the resulting
		 * unsigned integer type is reduced modulo the number that is one
		 * greater than the largest value that can be represented by the
		 * resulting type."
		 */

		/* Check for overflow and wrapping. */
		if (errno == 0) {
			if ((!first_digit) && (result <= old_result)) {
				/* Overflow and wrapping has occurred. */
				errno = 1;
			}
			first_digit = false; /* Parsing remaining digits so set flag to false. */
			old_result = result;
		}
	}

	/* Return the parsed value. */
	return result;
}



/* Memory Allocation Functions */

/*
 * realloc_safe, reallocates memory for an array pointed to
 * by ptr with no memory leak by freeing previously allocated
 * memory in case of failure.
 *
 * Preconditions: Same as void * realloc(void *ptr, size_t size)
 */
void * realloc_safe(void *ptr, size_t size) {
	void *tmp = realloc(ptr, size);
	if (tmp == NULL) { free(ptr); }
	return (ptr = tmp);
}



/* IO Functions */

/*
 * Read the next line of input from the given stream
 * reading one character at a time using the given
 * function pointed to by read_char.
 *
 * The function will save the read line of input
 * in *line_ptr and store its length in *len_ptr.
 *
 * Memory is managed in a leak free way meaning that
 * if there is a memory allocation error, the function
 * will free all allocated memory. It also means that
 * when there isn't an error, *line_ptr can be freed
 * safely by a call to void free(void *ptr).
 *
 * Preconditions:
 * 		1. stream is valid as required by int fgetc(FILE *stream)
 * 		2. *line_ptr == NULL
 * 		3. *len_ptr == 0
 *
 * Returns:
 * 		1. -2	if there is a stream error
 * 		2. -1	if there is a memory allocation error
 * 		3.  0	on success
 * 		4.  1	if EOF is reached
 * 		5.  2	if enough capacity cannot be allocated for *line_ptr
 */
int read_line(int (*read_char)(FILE *), FILE *stream, size_t *len_ptr, char **line_ptr) {
	/* c stores the next read char from the given stream. */
	int c = '\0';

	/* The capacity of the line stored in *line_ptr. */
	size_t capacity = INITIAL_LINE_CAPACITY;



	/* Initialize *len_ptr and *line_ptr. */
	*len_ptr = 0;
	*line_ptr = MALLOC_ARRAY(char, capacity);
	if (*line_ptr == NULL) { return -1; }

	/* Infinite loop reading the next char from the given stream. */
	while (true) {
		c = (*read_char)(stream);
		if (feof(stream) != 0) {
			/*
			 * EOF has been reached so null terminate
			 * the line but then return 1 only if no
			 * chars have been read.
			 */
			(*line_ptr)[*len_ptr] = '\0';
			return (*len_ptr == 0 ? 1 : 0);
		} else if ((ferror(stream) != 0) || (c == EOF)) {
			/*
			 * EOF is returned by a char reading
			 * function on EOF itself or on error
			 * hence check (c == EOF).
			 */
			return -2;
		} else if (c == '\n') {
			/*
			 * A full line has been read so
			 * null terminate it and break
			 * out of the while loop.
			 */
			(*line_ptr)[*len_ptr] = '\0';
			return 0;
		} else {
			/* Store the read char stored in c, in the line. */
			(*line_ptr)[(*len_ptr)++] = (char) c;
			if (*len_ptr == capacity) {
				/*
				 * Double the capacity of the line for
				 * amortized constant running time.
				 */
				capacity *= 2;

				/*
				 * Note that the value of capacity may reach the maximum and wrap
				 * around back to 0. We know this from the standard which states:
				 * "A computation involving unsigned operands can never overflow,
				 * because a result that cannot be represented by the resulting
				 * unsigned integer type is reduced modulo the number that is one
				 * greater than the largest value that can be represented by the
				 * resulting type."
				 */
				if (capacity <= *len_ptr) {
					return 2;
				}

				/* Reallocate memory. */
				*line_ptr = REALLOC_ARRAY(char, *line_ptr, capacity);
				if (*line_ptr == NULL) { return -1; }
			}
		}
	}



	/* This return will never be reached. */
	return 0;
}



/* Time Functions */

/*
 * Returns: Current time by invoking time(NULL)
 * and casting the result into uint_fast64_t.
 */
uint_fast64_t now(void) {
	return ((uint_fast64_t) time(NULL));
}



/* Error Functions */

/*
 * Print the error with value val and message msg that has
 * occurred in file with name filename on line numbered by
 * linenum if val != 0.
 *
 * Preconditions:
 * 		1. filename != NULL
 * 		2. filename is a valid C string(i.e., null-terminated)
 * 		3. linenum > 0
 * 		4. msg != NULL
 * 		5. msg is a valid C string(i.e., null-terminated)
 */
void print_err(const char *filename, const int linenum, const int val, const char *msg) {
	if (val != 0) {
		fprintf(stderr, "%s in \"%s\" on line %d: error value = %d\n",
					msg, filename, linenum, val);
	}
}

/*
 * Print the error with value errno and message msg that has
 * occurred in file with name filename on line numbered by
 * linenum if errno != 0.
 *
 * Preconditions:
 * 		1. filename != NULL
 * 		2. filename is a valid C string(i.e., null-terminated)
 * 		3. linenum > 0
 * 		4. msg != NULL
 * 		5. msg is a valid C string(i.e., null-terminated)
 */
void print_errno(const char *filename, const int linenum, const char *msg) {
	if (errno != 0) {
		fprintf(stderr, "%s in \"%s\" on line %d: errno = %d: %s\n",
					msg, filename, linenum, errno, strerror(errno));
	}
}

/*
 * Print the error with value err.val and message err.msg that
 * has occurred in file with name err.filename on line numbered
 * by err.linenum if err.val != 0.
 *
 * If err.val < 0 then it's the same as calling print_errno with
 * all of its arguments and else (err.val > 0) then it's the same
 * as calling print_err with all of its arguments.
 *
 * Negative values imply that errno is set. Positive values imply
 * that errno is NOT set.
 *
 * Preconditions:
 * 		1. err.filename != NULL
 * 		2. err.filename is a valid C string(i.e., null-terminated)
 * 		3. err.linenum > 0
 * 		4. err.msg != NULL
 * 		5. err.msg is a valid C string(i.e., null-terminated)
 */
void print_error(const Error err) {
	if (err.val < 0) {
		print_errno(err.filename, err.linenum, err.msg);
	} else if (err.val > 0) {
		print_err(err.filename, err.linenum, err.val, err.msg);
	} /* (err.val == 0) */
}
