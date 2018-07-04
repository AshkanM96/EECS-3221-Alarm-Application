/*
 * Author:
 * 					Ashkan Moatamed
 *
 *
 *
 * std_utilities.h
 *
 *
 *
 * Standard Headers, Type Definitions, Generic Macros,
 * and useful Function Prototypes for numbers, memory
 * allocation, input/output(IO), time, and much more.
 */

#ifndef INCLUDE_GUARD_STD__UTILITIES___H
	#define INCLUDE_GUARD_STD__UTILITIES___H



	/* Standard Headers */

	#include <unistd.h>
	#include <stddef.h>
	#include <stdbool.h>
	#include <inttypes.h>
	#include <limits.h>
	#include <errno.h>
	#include <stdlib.h>
	#include <stdio.h>
	#include <string.h>
	#include <ctype.h>
	#include <time.h>



	/* Type Definitions */

	/* Structure encapsulating errors. */
	typedef struct {
		/* Name of file where error has occurred. */
		const char				*filename;
		/* Line number where error has occurred. */
		int						linenum;

		/* Error value. */
		int						val;
		/* Error message. */
		const char				*msg;
	} Error;



	/* Number Macros and Function Prototypes */

	/* Generic Macro used to find the absolute value of a number. */
	#define ABS(x) ((x) < 0 ? -1 * (x) : (x))

	/* Generic Macro used to find the sign of a number. */
	#define SGN(X) ((x) == 0 ? 0 : ((x) < 0 ? -1 : 1))

	/* Generic Macro used to find the minimum of two numbers. */
	#define MIN(x, y) ((x) < (y) ? (x) : (y))

	/* Generic Macro used to find the maximum of two numbers. */
	#define MAX(x, y) ((x) < (y) ? (y) : (x))

	/* Returns: Greatest common divisor of x and y. */
	uint_fast64_t gcd(uint_fast64_t x, uint_fast64_t y);

	/* Returns: Least common multiple of x and y. */
	uint_fast64_t lcm(uint_fast64_t x, uint_fast64_t y);

	/* Generic Macro used to find the gcd of two numbers. */
	#define GCD(x, y) gcd((uint_fast64_t) x, (uint_fast64_t) y)

	/* Generic Macro used to find the lcm of two numbers. */
	#define LCM(x, y) lcm((uint_fast64_t) x, (uint_fast64_t) y)

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
	int_fast8_t f64_to_f8(const int_fast64_t i);

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
	int_fast16_t f64_to_f16(const int_fast64_t i);

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
	int_fast32_t f64_to_f32(const int_fast64_t i);

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
	uint_fast8_t f64_to_uf8(const int_fast64_t i);

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
	uint_fast16_t f64_to_uf16(const int_fast64_t i);

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
	uint_fast32_t f64_to_uf32(const int_fast64_t i);

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
	uint_fast64_t f64_to_uf64(const int_fast64_t i);

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
	uint_fast8_t uf64_to_uf8(const uint_fast64_t u);

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
	uint_fast16_t uf64_to_uf16(const uint_fast64_t u);

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
	uint_fast32_t uf64_to_uf32(const uint_fast64_t u);

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
	int_fast8_t uf64_to_f8(const uint_fast64_t u);

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
	int_fast16_t uf64_to_f16(const uint_fast64_t u);

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
	int_fast32_t uf64_to_f32(const uint_fast64_t u);

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
	int_fast64_t uf64_to_f64(const uint_fast64_t u);

	/*
	 * Parse the given character as a single digit.
	 *
	 * Returns: The parsed digit if valid and -1 if not.
	 */
	int ctoi(const char c);

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
	uint_fast64_t str_to_uf64(const char *s);



	/* Memory Allocation Macros and Function Prototypes */

	/* Generic Macro used to allocate memory for a single object. */
	#define MALLOC(type) ((type *) malloc(sizeof(type)))

	/* Generic Macro used to allocate memory for an array of objects. */
	#define MALLOC_ARRAY(type, size) \
		((type *) malloc(sizeof(type) * ((size_t) (size))))

	/*
	 * realloc_safe, reallocates memory for an array pointed to
	 * by ptr with no memory leak by freeing previously allocated
	 * memory in case of failure.
	 *
	 * Preconditions: Same as void * realloc(void *ptr, size_t size)
	 */
	void * realloc_safe(void *ptr, size_t size);

	/*
	 * Generic Macro used to reallocate memory for an array of
	 * objects using the realloc_safe user-defined method.
	 */
	#define REALLOC_ARRAY(type, ptr, size) \
		((type *) realloc_safe((ptr), sizeof(type) * ((size_t) (size))))



	/* IO Macros and Function Prototypes */

	#define INITIAL_LINE_CAPACITY 10

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
	int read_line(int (*read_char)(FILE *), FILE *stream, size_t *len_ptr, char **line_ptr);



	/* Time Functions */

	/*
	 * Returns: Current time by invoking time(NULL)
	 * and casting the result into uint_fast64_t.
	 */
	uint_fast64_t now(void);



	/* Error Macros and Function Prototypes */

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
	void print_err(const char *filename, const int linenum, const int val, const char *msg);

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
	void print_errno(const char *filename, const int linenum, const char *msg);

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
	void print_error(const Error err);

	/*
	 * The "do {" ... "} while (false);" bracketing around the macros
	 * allows them to be used as if they were function calls, even in
	 * contexts where a trailing ";" would generate a null statement.
	 * For example:
	 *
	 * 		if (errno != 0) {
	 * 			EXIT_ERRNO("message");
	 *		} else if (status != 0) {
	 *			EXIT_ERR("message", status);
	 *		} else {
	 *			return status;
	 *		}
	 *
	 * The above would not compile if EXIT_ERRNO or EXIT_ERR were macros
	 * ending with "}", because C does not expect a ";" to follow "}".
	 * However, C does expect a ";" following ")" in the do...while construct.
	 */
	#define EXIT_ERR(msg, val) do { \
			print_err(__FILE__, __LINE__, val, msg); \
			exit(val); \
		} while (false)

	#define EXIT_ERRNO(msg) do { \
			print_errno(__FILE__, __LINE__, msg); \
			exit(errno); \
		} while (false)

#endif
