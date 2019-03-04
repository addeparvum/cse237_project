from subprocess import Popen, PIPE
from time import sleep
from fcntl import fcntl, F_GETFL, F_SETFL
from os import O_NONBLOCK, read, fdopen
import pty

master, slave = pty.openpty()

# run the shell as a subprocess:
p = Popen(['./printOutput'],
        stdin = PIPE, stdout = slave, stderr = slave, shell = True, bufsize=1)

# set the O_NONBLOCK flag of p.stdout file descriptor:
stdout = fdopen(master)

flags = fcntl(stdout, F_GETFL) # get current p.stdout flags
fcntl(stdout, F_SETFL, flags | O_NONBLOCK)


# issue command:
x = 1
while x == 1:
    inputNum = int(input("Enter a number: "))
    p.stdin.write(('%d\n' % inputNum).encode())
    if inputNum != 1 and inputNum != 0:
        break
        
    sleep(0.1)
    out = (stdout.read())[:-1]
    print(out)
   
# let the shell output the result:
sleep(0.1)
# get the output
try:
    print("done!")
except OSError:
    # the os throws an exception if there is no data
    print '[No more data]'
