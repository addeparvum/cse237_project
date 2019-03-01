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
p = Popen(['./printOutput'],
        stdin = PIPE, stdout = slave, stderr = slave, shell = True, bufsize=1)

# set the O_NONBLOCK flag of p.stdout file descriptor:
stdout = fdopen(master)

flags = fcntl(stdout, F_GETFL) # get current p.stdout flags
fcntl(stdout, F_SETFL, flags | O_NONBLOCK)


# issue command:

p.stdin.write(('0\n').encode())
    
sleep(0.1)
out = (stdout.readline())[:-1]
print(out)
   

class MyServer(SimpleHTTPServer.SimpleHTTPRequestHandler):
    """ A special implementation of BaseHTTPRequestHander for reading data from
        and control GPIO of a Raspberry Pi
    """

    def do_HEAD(self):
        """ do_HEAD() can be tested use curl command
            'curl -I http://server-ip-address:port'
        """
        self.send_response(200)
        self.send_header('Content-type', 'text/html')
        self.end_headers()

    def _redirect(self, path):
        self.send_response(303)
        self.send_header('Content-type', 'text/html')
        self.send_header('Location', path)
        self.end_headers()

    def do_GET(self):
        """ do_GET() can be tested using curl command
            'curl http://server-ip-address:port'
        """
        html = '''
           <html>
           <body style="width:960px; margin: 20px auto;">
           <h1>Welcome to my Raspberry Pi</h1>
           <p>Current GPU temperature is {}</p>
           <form action="/" method="POST">
               Turn LED :
               <input type="submit" name="submit" value="On">
               <input type="submit" name="submit" value="Off">
           </form>
           </body>
           </html>
        '''
        temp = popen("/opt/vc/bin/vcgencmd measure_temp").read()
        self.do_HEAD()
        self.wfile.write(html.format(temp[5:]).encode("utf-8"))

    def do_POST(self):
        """ do_POST() can be tested using curl command
            'curl -d "submit=On" http://server-ip-address:port'
        """
        content_length = int(self.headers['Content-Length'])  # Get the size of data
        post_data = self.rfile.read(content_length).decode("utf-8")  # Get the data
        post_data = post_data.split("=")[1]  # Only keep the value

        # GPIO setup
      #  GPIO.setmode(GPIO.BOARD)
      #  GPIO.setwarnings(False)
      #  GPIO.setup(12, GPIO.OUT)
      #  p = GPIO.PWM(12, 50)
      #  p.start(7.5)

        if post_data == 'On':
            p.stdin.write(('0\n').encode())
            sleep(0.1)
            out = (stdout.read())[:-1]
            print(out)
       #     p.ChangeDutyCycle(7.5)
       #     time.sleep(1)
       #     p.ChangeDutyCycle(2.5)
       #     time.sleep(1)
       #     p.ChangeDutyCycle(12.5)
       #     time.sleep(1)
        else:
            p.stdin.write(('1\n').encode())
            sleep(0.1)
            out = (stdout.read())[:-1]
            print(out)
                    
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
