# COP4520_pa3

To compile and run both programs do: "g++ file_name.cpp" and then "./a"

To alter constants for the algorithms, change the const variables at the top of each program to adjust accordingly

For the first problem with the minotaur and his birthday party, the function is using threads for each servant to add presents to the concurrent linked-list and write thank you notes concurrently. Mutexes are used for synchronization to ensure thread safety. The main function initializes the presents queue, creates servant threads, waits for them to finish, and then checks if all presents have been acknowledged with thank you notes.

For the second problem with the temperature reading, it uses a concurrent program that efficiently collects temperature readings from the 8 sensors, stores them in shared memory, and compiles a report at the end of every hour. Overall, it uses 8 threads, each representing a sensor, to read temperatures at regular intervals. These threads will write the readings to the shared memory region. There was an issue where the report would need to be generated faster than sensors would read the temps, but adjusting the time lengths where each sensor would grab a temperate every 100ms and the report generated every second, allowed this to happen in an appropriate time. 



