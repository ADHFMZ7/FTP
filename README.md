# FTP

## Names

- Ahmad Aldasouqi - ahmadaldasouqi@csu.fullerton.edu
- Wayne Muse - waynemuse@csu.fullerton.edu
- Kourosh Alasti - kalasti@csu.fullerton.edu
- Francisco Godoy - fgodoy0@csu.fullerton.edu
- Christian Bonilla - chrisbo@csu.fullerton.edu

## User Guide
Program requires GNU Readline.

1. Use make command from src directory to compile the code. This creates a new bin directory.
2. cd into bin/server and run `./server <port>`
3. cd into bin/client command and use `./client <ip> <port>`. Use the ip the server is on (127.0.0.1 if on same host). Use the same port as in the previous step.
4. You can now run commands from the client.

## Example

### Server
```
~/Documents/school/FTP/src/bin/server - (main) > ./server 8081
Server started on 127.0.0.1:8081
server: waiting for connections...
server: got connection from ::ffff:127.0.0.1
Receiving file: test1.txt
recv error: Undefined error: 0
File transfer complete: test1.txt
server: got connection from ::ffff:127.0.0.1
Receiving file: test1.txt
Segment received - Length: 29, Sequence Number: 0
Segment received - Length: 0, Sequence Number: 2
EOF marker received
File transfer complete: test1.txt
Sending file: test2.txt
Read 30 bytes; seq number: 0
Sent EOF marker
File transfer complete: test2.txt
```

### Client
```
(web) ~/Documents/school/FTP/src/bin/client - (main) > ./client 127.0.0.1 8081
IP: 127.0.0.1
Connected to server
ftp> put test1.txt
Using chunk size: 16328 bytes
Read 29 bytes; seq number: 0
Sent EOF marker
File transfer complete: test1.txt
ftp> ls
test1.txt
test2.txt
server
ftp> get test2.txt
getting filename: test2.txt
Segment received - Length: 30, Sequence Number: 0
Segment received - Length: 0, Sequence Number: 1
File transfer complete: test2.txt
```

## Protocol Design

### Data packet
This packet is used when file data is being transmitted. The first two byte field holds the length of the payload in the current segment in byte and the second two byte field holds the sequence number of the current file transmission.

```
0                32                64  
+--------+--------+--------+--------+  
|      Length     |    seq. number  |  
+--------+--------+--------+--------+  
|            data octets            |  
+---------------- ...                   
```

### EOF packet
This packet designates the end of file transmission. It is the previous header with a length field of zero.

### Error packet
An error packet is sent to stop the transmission. It has a length and sequence number field of zero.



### PUT
```
> put [filename]
```

1. Client sends PUT message and begins sending data packets.
2. Server starts receiving until EOF packet is received.

### GET
```
> get [filename]
```

1. Client sends PUT message.
2. Server starts sending data packets until end of file is reached. An EOF packet is then sent.
3. Client receives the packets until an EOF packet is sent.

### LS
```
> ls
```

1. Client sends LS message to server.
2. Server responds with list of files in the active directory.
