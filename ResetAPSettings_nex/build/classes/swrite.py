import serial
import sys

port = sys.argv[1]
ap_name = sys.argv[2]
psw = sys.argv[3]

command = "AT+CWSAP=\"" + ap_name + "\",\"" + psw + "\"\r\n" 
print port
print command
baud = 0
ser = serial.Serial(port, 57600, timeout=1)

t_cmd = "AT+TEST\r\n"
t_msg = "HELLO\r\n" # "\r\n"

l = [57600, 1200, 2400, 4800, 9600, 19200, 38400, 111100, 115200, 500000, 921600, 1500000]

for t in l:
	print t
	ser.setBaudrate(t)
	ser.write(t_cmd)
	data = ser.read(7)
	print data
	if data == t_msg:
		baud = t
		break
	

if baud != 0:
	x = ser.write(command)
	data = ser.readline()
	print "Baudrate"
	print baud  # for fb
else:
	print "No baud match"
ser.close()












