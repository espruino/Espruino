RTOS for Espruino
===
With RTOS we get a new world for Espruino. Before I forget to mention, actual sources are on very early state. Usually Espruino is very well tested, for this new functions, hmm, let me say, hmm, its more like a proof of concept.
To get RTOS running for Espruino on ESP32 you have to define RTOS=1 before starting make.
###General
RTOS is a shortcut for RealTime Operating System. Realtime OS are welknown in the world, usually they support running more than one task. In big systems, number of tasks can be very high. Obviously, this needs a lot of memory and some control of tasks. Words like priorities, queues, semaphores and more belong to this world.
Lets take a first look into this world to see, what Espruino can do with that.
###Tasks
From outside tasks are seperate applications running on a computer at the same time. They could work together, or totally independent from all others.
Ususally in the world of micro computers, we have one CPU only. Ok, ESP32 has two of them, but even with 2 CPUs, there must be a way to have several application running at the same time.
Its a good point to switch wording now from application to task. A task can be a full sized application like Espruino. But it can also be a simple function like reading input from serial and prepare it for other tasks. Even something like a blinker can be a task.
One of the magic words for tasks is priority. Based on priority, the OS decides how often a task gets time to work. This will not really work with an endless loop.
Next magic words are interrupts. Interrupts like a timer will suspend running task, make a decision based on priority which task should get some time and resumes that task.
###Queues
Lets take a closer look to the concept of the uart reading task mentioned earlier. This task reads data and prepares the data for later use. Tasks like this are often used, but working for themselve sounds senseless. Reading keystrokes from a console for example without doing anything is like talking to a wall.
Good for us, we have a solution and this is called Espruino. Espruino uses uart info to run complex javascript functions. But how could the console task tell Espruino about incomin keystrokes.
The way RTOS supports this is called a queue. One task, in our case the console task writes data into this queue and another task reads data from this queue.
###Task interaction###
RTOSfree supports a lot of functions to interact between tasks. Queues have already been mentioned in previous chapter. Other options are semaphores and notification.
Notification are an option to awake a task really quick. Each task has an area of memory which is available for function calls from other tasks. Simple action is, that one task is waiting for "something", and another task is telling that "something" happened.
###API calls
RTOS supports a lot of functions to work with queues and tasks on a low level. Good part of low level is, you can do a lot of special things. Bad part is, you have to do everything on this low level.
Usually you combine low level calls to easy to use utilities. As a first step we have some functions in targets/esp32/rtosutil.c. Take a short look to the header file to get more information around the functions supported.
###Espruino commands
Rtosutil supports functions on level of C. To use them in Espruino, they need a wrapper. And this wrapper is named jswrap_rtos.c. For each command you can see the wrapper data in the source. BTW, Espruino uses these lines to create its own documentation.
###Examples in Javascript
have to be done

