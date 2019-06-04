# SHM_MANAGER
A System-V SHM interface for Python. Offers functionalities for shared memory segments creation and handling, file-like methods (i.e: read, readline, write) and the possibility to transfer PyStringObjects, saving time from memory copies.

## Example

Process 1:
```
>>> from shm_manager import *
>>> segment1 = shm_manager(123456, 100)
>>> segment1.write("Hello world!")
```

`ipcs` content after running process 1

```
------ Message Queues --------
key        msqid      owner      perms      used-bytes   messages    

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      
0x00000000 262144     sergio     600        134217728  2          dest         
0x00000000 360449     sergio     600        524288     2          dest         
0x00000000 393218     sergio     600        524288     2          dest         
0x00000000 622595     sergio     600        524288     2          dest         
0x00000000 524292     sergio     600        6422528    2          dest         
0x00000000 491525     sergio     600        6422528    2          dest         
0x00000000 655366     sergio     600        524288     2          dest         
0x00000000 688135     sergio     600        524288     2          dest         
0x00000000 1933320    sergio     600        67108864   2          dest         
0x00000000 1081353    sergio     600        524288     2          dest         
0x00000000 3014666    sergio     600        524288     2          dest         
0x00000000 3047435    sergio     600        16777216   2          dest         
0x0001e240 3637260    sergio     600        100        1                       
0x00000000 1900560    sergio     600        524288     2          dest  
```

Process 2:
```
>>> from shm_manager import *
>>> segment1 = shm_manager(123456, 100, 0600)
>>> segment1.read()
'Hello world!\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
```
