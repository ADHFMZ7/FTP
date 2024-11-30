# Protocol design


## Data packet
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
An error is 



## PUT

```
> put \[filename]
```

1. Client sends PUT message.
2. Server responds with length of payload in bytes, followed by sequence number, then data payload.
3. Client receives the packets until an EOF packet is sent.

## GET

```
> get \[filename]
```

1. Client sends GET message

## LS


