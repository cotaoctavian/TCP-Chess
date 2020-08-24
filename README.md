# TCP-Chess
This is a [Chess](https://en.wikipedia.org/wiki/Chess) game I've done as a project for Computer Networks.

The project uses TCP/IP communication protocol and is able to manage more games at the same time.

To be able to play it you have to own Linux as an operating system.

# How to compile:
  * g++ server.c -o name1
  * g++ client.c -o name2

# How to run:
  - firstly, you need to start the server: **./name1**
  - secondly, connect 2 clients to the server using the command: **./name2 127.0.0.1 2728** 
   > you can connect more than 2 clients to the server, if the number of clients is an even number they will be grouped up in pairs, having their own game.
   
   > if a client does not have another client as a rival then it will be put on hold.
   
# How to play!
  - you have to pick the position of the piece you want to move and the position where you would like to move the piece. 
    **Example:** b4 c7
  - if you want to make a move such as **promotion** or **castling** you have to add another letter after the positions.
  - for **promotion** you have to write the piece you want your pawn to transform into
  - for **castling** you have to write F.
  
 # That's it! Have fun!
