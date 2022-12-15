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