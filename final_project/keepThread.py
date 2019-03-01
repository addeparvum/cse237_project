import os
import pty
import subprocess

master, slave = pty.openpty()
tokenizer = subprocess.Popen(['./printOutput'], shell=True, stdin=subprocess.PIPE, stdout=slave, close_fds=True)
stdin_handle = tokenizer.stdin
stdout_handle = os.fdopen(master)
x = 1
while x == 1:
    inputNum = int(input("Enter a number: "))
    stdin_handle.write(('%d\n' % inputNum).encode())
   
    if inputNum == 4:
        x = 0
tokenizer.stdin.close()
out = stdout_handle.readline()
print (out)
tokenizer.wait()
print ("All done")