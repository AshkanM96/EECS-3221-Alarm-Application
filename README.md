## EECS-3221
Multithreaded alarm application

This is just a repository of an alarm application written in *C* that uses the *pthreads* library to handle the multithreading aspects of the application.



There are two versions of the application, each with their own set of specifications.

Version 1 treats all threads as writers to a single list and practically has no restrictions.

Version 2 introduces the concept of readers and writers but also divides the central part of the application into two separate threads. This results in the need for more synchronization constructs such as conditional variables and semaphores. This version also adds quite a few restrictions on the input on top of just being parse-valid(satisfying the specified formats).



For more information, refer to the specification.txt under each version.
