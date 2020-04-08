# OS Lab3: Networked Spell Checker
tuh26432@temple.edu

## Overview
This program is designed to allow one server connect to multiple computers and allow them to check spelling of words. The server responds after each request with an answer whether the word is spelled right or now. The spellchecker can only take one word from each user at a time. This program utilizes multi-threading to ensure that everything runs accurately and efficiently.

## Program design

This program is made of three main parts. The three parts are split up using threads. This makes it more efficient and even more accurate. The three parts are the main thread, the 'worker thread' and the log thread. 

The main thread loads in the default dictionary or the selected dictionary by the user that starts the server. The main thread also creates the other threads. These other threads are the multiple worker threads and the log thread. The main thread is also responsible for initializing network connection and waiting for clients to connect. Finally it is the main thread's job to place the socket descriptor on the connection queue. 
