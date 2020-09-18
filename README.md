# Chat Server using Socket Programming

The chat application is based on socket programming. It is capable of connecting several clients at the same time where each client can do following task:
1. send : A client is capable to send messages to server which are further received by other clients.
2. receive : A client can receive messages from server.
3. upload : A client can upload files on server
4. download: A client can download files from server.
5. exit : A client can notify the server that it wants to disconnect.


## Getting Started

Multiple clients are handled by SELECT() command of LINUX.

Once a client disconnects the server can replace its position with new client connections.

Reference : https://beej.us/guide/bgnet/html//#selectman

Server folder acts as a disk(storage)  for the server.
Each client folder act as its disk storage.

After the connection is established, clients can choose from a menu what action she/he wants to take.
As per the selected option the server takes the action.

The max_client variable is the maximum clients that can connect with the server.
We tested our system for 4 clients and it was set to 30. But it can easily be increased to
value required by the user.

## Usage

Server needs to started as (for example take 9999 as the port number)

```
cd server
./server 9999
```

Example for two clients
```
cd client1
./client 127.0.0.1 9999
```

```
cd client2
./client 127.0.0.1 9999
```
## License
Copyright (c) 2020 Ashish yadav
