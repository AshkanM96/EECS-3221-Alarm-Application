## EECS-3221-Alarm-Application

This is just a repository of a multithreaded alarm application written in *ANSI C* that uses the *pthreads* library to handle the multithreading aspects of the application.



There are two versions of the application, each with their own set of specifications.

Version 1 treats all threads as writers to a single list and has no restrictions other than basic parse-validity of the input (i.e., satisfying the specified formats).

Version 2 introduces the concept of readers and writers but also divides the central part of the application into two separate threads. This results in the need for more synchronization constructs such as conditional variables and semaphores. This version also adds quite a few restrictions on the input on top of just being parse-valid (i.e., satisfying the specified formats).



For more information, refer to the specification.txt of each version.
