# Protocol design

0        4        8        12       16 
+--------+--------+--------+--------+ 
|      Length     |    seq. number  | 
+--------+--------+--------+--------+ 
|            data octets            |
+---------------- ...                 


## LS

1. Client sends LS message.
2. Server responds with length in segments, followed by sequence number, then data.
3. Client receives the n segments and sends an acknowledgement.


## GET



## PUT
