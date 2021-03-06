import RPi.GPIO as GPIO
import SimpleHTTPServer
from subprocess import Popen, PIPE
from time import sleep
from fcntl import fcntl, F_GETFL, F_SETFL
from os import O_NONBLOCK, read, fdopen, popen
import pty
import SocketServer

host_name = '0.0.0.0'
host_port = 80

master, slave = pty.openpty()

# run the shell as a subprocess:
p = Popen(['./run.out'],
        stdin = PIPE, stdout = slave, stderr = slave, shell = True, bufsize=1)

# set the O_NONBLOCK flag of p.stdout file descriptor:
stdout = fdopen(master)

flags = fcntl(stdout, F_GETFL) # get current p.stdout flags
fcntl(stdout, F_SETFL, flags | O_NONBLOCK)
   

class MyServer(SimpleHTTPServer.SimpleHTTPRequestHandler):

    def do_HEAD(self):
     
        self.send_response(200)
        self.send_header('Content-type', 'text/html')
        self.end_headers()

    def _redirect(self, path):
        self.send_response(303)
        self.send_header('Content-type', 'text/html')
        self.send_header('Location', path)
        self.end_headers()

    def do_GET(self):
        
        html = '''
           <html>
           <body style="width:960px; margin: 20px auto;">
           <h1>Welcome to my Raspberry Pi</h1>
           <p>Box Output:{}</p>
           <form action="/" method="POST">
               Lock Box:
               <input type="submit" name="submit" value="Lock">
               <input type="submit" name="submit" value="Unlock">
               Open Box:
               <input type="submit" name="submit" value="Open">
               <input type="submit" name="submit" value="Close">
           </form>
           
           </body>
           </html>
        '''

        p.stdin.write(('4\n').encode())
        sleep(0.5)
        temp = (stdout.read())
       # temp = popen("/opt/vc/bin/vcgencmd measure_temp").read()
        self.do_HEAD()
        self.wfile.write(html.format(temp).encode("utf-8"))

    def do_POST(self):
       
        content_length = int(self.headers['Content-Length'])  # Get the size of data
        post_data = self.rfile.read(content_length).decode("utf-8")  # Get the data
        post_data = post_data.split("=")[1]  # Only keep the value


        if post_data == 'Open':
            p.stdin.write(('0\n').encode())
            sleep(0.1)
            out = (stdout.read())[:-1]
        
        else if post_data == 'Close':
            p.stdin.write(('1\n').encode())
            sleep(0.1)
            out = (stdout.read())[:-1]
            print(out)

        else if post_data == 'Unlock':
            p.stdin.write(('2\n').encode())
            
        else if post_data == 'Lock':
            p.stdin.write(('3\n').encode())
          

                    
        #        p.stop()
        print("LED is {}".format(post_data))
        self._redirect('/')  # Redirect back to the root url


if __name__ == '__main__':
    http_server = SocketServer.TCPServer((host_name, host_port), MyServer)
    print("Server Starts - %s:%s" % (host_name, host_port))

    try:
        http_server.serve_forever()
    except KeyboardInterrupt:
        http_server.server_close()
