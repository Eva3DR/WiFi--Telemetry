import serial

def readC():
	f = open('ap_config.txt', 'r')
	print f.readline()
	f.close()
	
def writeP(current):
	f = open('ports.txt', 'w')
	for n,s in current: 
		f.write(s)
		f.write('\n')
	f.close()
	

def scan():
   # scan for available ports. return a list of tuples (num, name)
   available = []
   for i in range(256):
       try:
           s = serial.Serial(i)
           available.append( (i, s.portstr))
           s.close()
       except serial.SerialException:
           pass
   return available

for n,s in scan(): print "%s" % s
#writeP(scan())
#readC()

