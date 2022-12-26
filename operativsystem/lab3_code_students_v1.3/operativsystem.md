# Föreläsning säkerhetssystem
Växt explosionsartad under 20-30 åren

En säkerhetsbug kallas vulnerability, att nyttja en vulnerability kallas exploit

### Exploits
Can be launched manually or automatically
Virus & worms allow to lunch automatically exploits


### CIA modelen
The security can be seen as three aspects/properties

* Confidentiality - secret data remain secret 
* Integrity - unauthorized users should not be able to modify any data without owner's permission
* Availability, nobody can disturb a system to made it unusable

Threats to CIA properties 
* Confidentiality, exposure of data (leaking)
* Integrity, tempering of data (middle man attack)
* Availability, Denial of service attacker (DoS)

Sophisticated mechanism to guarantee C&I are available, but it is hard to do something about

## OS security: What is and what is not

Is related to attacks that target operating system
Is related to attacks where the operating system plays an important role in enforcing the security policy

SQL injections and application level vulnerabilities are independent of OS and thus not relevant. 

## Passive and active attacks
* Passive attacks try to steal information passively
  * Sniffing traffic
* Activate attacks try to steal information by searching itself

### Cryptography and Hardening 
##### two protection mechanism 
* Cryptography is about protocols and algorithms to prevent an unauthorized party to access information

* Hardening is a restriction of what you can do on a computer to restrict malicious behaviour

### Core questions
 Q1: Is it possible to build a secure system
 
 A1: Yes in theory, if the software is not too large or complicated

   * Smaller and simpler software entails a more secure system
   * Military software with designed with a specific goal in mind for example

Q2: If so, why it is not done?

A2: 

* Users are unwilling to throw away unsecure systems they like to use (e.g. windows osx office; acrobat;mysql)
Flexibility = less secure systems. How many features are we willing to sacrifice?
* Users and vendors like features; features make a system complex and prone to security bugs; features are enemies of security. 
* Security is valuable but the tradeoff is not something most would do. Most systems draw a line between security and features

### Features which make systems weak
* Email
  * If we transmit only ascii text, it is easy to make the system fairly secure (first email systems)
  * If we transmit attachments of any kind we transport potential malicious code that will be executed on the target host

* Web
  * Static html pages are not so unsecure; ascii text is transmitted
  * Dynamic webpages executing javascript and communicating with a server leaves room for corrupt transmissions

### How to build a secure system
We need a security model at the core of the operating system that is simple enough

* That the designer can actually understand it 
* That the sdesigner can resist pressures to deviate from it in order 
  

### TCB trusted computing base
* Trusted systems are places the normal user shouldn't access. It is made to be as secure as possible, but also so small as possible. Think of your Hardware and pagetables, kernel etc

Functions which are a part of TCB includes
process creation
process switiching
memory management
part of the io management

### Reference monitor
Is the key to tyour TCB. Accept all system calls involving security and decide whether they should be processed

### Access control: protection domains
Divide objects in classes deciding what it can do. 
'
each domain has a unique name

A domain is a set of (object, rights) pairs

* Each pair specify an object and a subset of operations that can be performed on it
* rights specify permission to perform one of the operations
  
Allocated based on who needs to know what
# Course summary

Reason to have os: 

* hide complexity and make usage easier
* have a resource manager which use the resources in a secure and fair way

What is a process , a thread

- Process owns the resources 
- Threads handle the execution of a program
- Processes 

PCB (process control block) is used to store information about a thread, which values were in the register, where were we in execution. We store processes here when their time on the cpu is out (a part of what happens under a context switch)

### Process thread
* Ready, can be run
* Running, is running
* Blocked, waiting on smth

### Preemptive non-preemptive

* preemptive, to stop a process from outside of the process
* non-preemptive, the process stops once it seems fit


To have a deadlock, all for conditions need to be met in your program 

Four conditions for deadlocks

1. Mutual exclusions - only one can hold a resource at a time
2. Hold and wait - i can allocate a resource, and hold it whilst waiting on another
3. No preemption - I cannot steal resources from another process
4. Circular wait condition - resource a is allocated to me, i need resource b which is allocated to a process which is waiting for resource a

Strategies use for dealing with dealocks: 

1. Ignore the problem
2. detection and recovery (allow it to happen and then have an algorithm solve it somehow)
3. Dynamic avoidance by careful resource allocation Be careful when allocating a resource
4. Deadlock prevention, negating one of the four conditions

## Memory managment

Adress mappings

- Logical/virtual vs physical memory
- Relocation (converting from logical to physical since all programs believe they start at adress 0)
- Base + limit is the solution
- Paged memory (big thing, divide the logical adress space in pages (usually 4kb) and divide the physical memory into pageframes of same size, allowing you to store pages in the pageframes, requires hardware (MMU/TLB))
- Segmented memory
### Different memory types
* Text 
  * (code, global and static variables), 
* data, 
* heap 
  * (stores dynamically allocated memory, long lived), 
* stack 
  * (local variables, function calls to know where we are) segments (the difference segments of memory, short lived)
### Memory management
* demand paging
* page tables
  * Single and multilevel page tables
  
* page faults (happens when we do not have a page in the primary memory, forcing us to load from disk)
* page eviction (how to handle pagefaults)
  * LRU FIFO, etc
  * LRU approximations, e.g NRU, 2nd chance
* working set
  * The amount of memory used right now

## Filesystems
Allocation methods

- Contiguous
  - everything is stored in a long line
- Linked allocation
  - Stores where the next block is in the block of your program
  - Clunky
- Linked allocation with indexfile (fat)
  - Store all indexes in a block at the start
  - If the fat table is corrupted, all info is lost
- Index nodes

## Input/output hardware and software

What to know

* Programmed i/o polling
* Interrupt driven io
* Interrupts
  * Priority
  * Interrupt vector
  * Interrupt handles
  * Hardware and software interrupts (syscalls)
* DMA - Direct memory
 
Round robin load-in occurs after swap

Ready - running - blocked
Ready - running - ready
blocked - ready

Assume a mechanical disk that rotates with 7500 rpm rotations per minute how long is the average rotiational delay

7500/60 = 125 v/s
1/125 = 8 ms /varv

Rotation delay is 0.5 * 8 ms

Same disk. It has 800 sectors per track and each sector is 512 (0.5 KB) what is the maxmimu data transfer rate in mb/s
800 * 0.5 kb = 400 kb per varv
hur många sekunder tar det att snurra ett varv? 
svar 400kb/8ms = 50 mb/s

Why does the system call interface use an interrupt rather than a procedure call

The only way to access kernel mode is through interrupt, procedures remain in user mode

7.1 Delat icke delat, den ena använder delat minne medan den andra skickar meddelanden mellan processer

