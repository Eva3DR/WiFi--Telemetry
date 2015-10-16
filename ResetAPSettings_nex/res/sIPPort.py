import serial
import sys

com_port = sys.argv[1]
ip = sys.argv[2]
port = sys.argv[3]


command = "AT+CIPPORT=\"" + ip + "\"," + port + "\r\n" 
print com_port
print command
baud = 0
ser = serial.Serial(com_port, 57600, timeout=1)

t_cmd = "AT+TEST\r\n"
t_msg = "HELLO\r\n" # "\r\n"

l = [1200, 2400, 4800, 9600, 57600, 19200, 38400, 111100, 115200, 500000, 921600, 1500000]

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
	print baud 	# for fb
else:
	print "No baud match"
ser.close()












