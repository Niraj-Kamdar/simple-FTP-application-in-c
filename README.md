# simple-FTP-application-in-c

You can download, upload and list files from FTP server with standard FTP command GET, PUT and LIST. It also contains additional analysis code for efficiency of the server.

## Problem Statement

### Exercise-2a: FTP using socket programming:
Design an application of FTP using TCP. GET method is given to you using that Implement PUT
and LIST methods in FTP. Design your own message structure. Use appropriate data structure.

### Exercise-2b: Multithreaded server:
Extend the application you have designed in Exercise 2a such that server can handle multiple
clients in parallel. Use multithreading.

### Exercise-2c:
Implement a scenario in which there is one server (PC1) which can handle multiple clients (using
multithreading implemented in exercise 2b) and one client (PC2). Server is supposed to send a file of
50Mb for each request. Client (PC2) can send multiple requests (by generating multiple threads) for
the same file.
* Find the time required to transfer file for each request of client.
* Change the number of requests generated from 1to 10. Create a table for time required to
transfer file to any one client(fixed) for different number of requests.
* Plot a graph of time required to transfer file for any one client(fixed) vs. different number of
requests.

### Exercise 2d:
Implement a scenario in which two clients are supposed to access the same file at the server. Provide a locking
mechanism which prevents them to access the file at the same time.

### Exercise 3:
Goal of this exercise is to implement a scenario where all the clients of the server receives the file at the same
time. (Such as online game or stock market etc)
- Implement a scenario in which there is one server and multiple clients.
Periodic measurement of RTT
    1. Server sends echo message to all clients
    2. for each client i
        1)  measure the timestamp when echo message is sent ( T<sub>si</sub>)
        2)  Measure the timestamp when reply is received ( T<sub>ri</sub> )
        3)  RTT<sub>i</sub> = T<sub>ri</sub> − T<sub>si</sub>
    3. ΔT = max (RTT<sub>i</sub>)

- Clients send request to get a file(50Mb) from server. Server sends the file with timestamp( T<sub>ms</sub> ) and
ΔT . When client receives file(at time T<sub>mr</sub>) it holds file till ( ΔT -( T<sub>ms</sub> - T<sub>mr</sub> )) time. After that it
displays file.
