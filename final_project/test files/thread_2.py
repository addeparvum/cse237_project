from subprocess import Popen, PIPE, STDOUT
import pty
import os

cmd = './printOutput'

master, slave = pty.openpty()

p = Popen(cmd, shell=True, stdin=PIPE, stdout=slave, stderr=slave, close_fds=True)
stdout = os.fdopen(master)
p.stdin.write(('0\n').encode())
p.stdin.write(('1\n').encode())
p.stdin.write(('0\n').encode())
p.stdin.write(('3\n').encode())

p.stdin.close()
print stdout.readline()
#print stdout.readline()