# Compilation

```bash
$ make
```

# Notes on code execution

In one terminal, execute the server code by:

## Server

```bash
$ ./s 4444
# format ./s [port no]
```

## Client

For each client, open a new terminal, and run:

```bash
$ ./c 127.0.0.1 4444
# format ./c [server ip] [server port no]

```

# Replay demo

For best results, please run when terminal is >= 93 characters wide. This will replay the demo in real time.
The replay will show an execution of the server code for 1.5 minutes.

```bash
$ scriptreplay --timing=script_log scriptfile
```

For the 6 instances of clients tested, execution is seen below:

```
❯ ./c 127.0.0.1 4444
# Connection established! type msg after the prompt 'C>' 
C>hello s
Scanned: hello s
C>quit
Scanned: quit
# exiting
```

```
❯ ./c 127.0.0.1 4444
# Connection established! type msg after the prompt 'C>' 
C>quit this?
Scanned: quit this?
C>yes 
Scanned: yes 
C>quit
Scanned: quit
# exiting
```

```
❯ ./c 127.0.0.1 4444
# Connection established! type msg after the prompt 'C>' 
C>new client
Scanned: new client
C>will someone quit?
Scanned: will someone quit?
C>quit
Scanned: quit
# exiting
```

```
❯ ./c 127.0.0.1 4444
# Connection established! type msg after the prompt 'C>' 
C>quit
Scanned: quit
# exiting
```

```
❯ ./c 127.0.0.1 4444
# Connection established! type msg after the prompt 'C>' 
C>last one in
Scanned: last one in
C>quit
Scanned: quit
# exiting
```

```
❯ ./c 127.0.0.1 4444
# Server refused connection
❯ ./c 127.0.0.1 4444
# Connection established! type msg after the prompt 'C>' 
C>yay! can join now!
Scanned: yay! can join now!
C>bye
Scanned: bye
C>quit
Scanned: quit
# exiting
```

Screenshot -

![](ss.png)

---
