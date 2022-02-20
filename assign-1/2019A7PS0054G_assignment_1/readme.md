# Compilation 

```bash
$ make
$ ./zombie.o
```
***

# Notes on code execution

In order to create a zombie process, type ```y``` or ```Y```  in the prompt. Any other character 
can be input for avoiding creation of zombie process.
Once we chose execution for zombie, the process will sleep for 15s.

In that time, we can check if our process is ```defunct``` in the process list by executing the following in a new terminal-

```bash
$ watch -n 0.5 'ps -ef | grep "zombie.o" '
```

Once we see in the output, something similar to - 

```
tanmay     32349   32303  0 09:25 pts/3    00:00:00 [zombie.o] <defunct>
```
we can identify the zombie process created.

***

# Replay demo
For best results, please run when terminal is >= 75 characters wide.
This will replay the demo in real time.

```bash
$ scriptreplay --timing=script_log scriptfile
```

***

## Notes on the logic used

```
Zomibe?	action		
y/Y		SIGCHILD catch
any		ignore (default)
```
