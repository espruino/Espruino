from optparse import OptionParser
import serial
import xmodem
import os, sys, time
import logging
import pyprind



logging.basicConfig()
parser = OptionParser(usage="python %prog [options]")
parser.add_option("-f", dest="bin_path", help="path of bin to be upload")
parser.add_option("-c", dest="com_port", help="COM port, can be COM1, COM2, ..., COMx")
parser.add_option("-d", action="store_true", dest="debug")
(opt, args) = parser.parse_args()

debug = opt.debug

if not opt.bin_path or not opt.com_port:
    print >> sys.stderr, "\nError: Invalid parameter!! Please specify COM port and bin.\n"
    parser.print_help()
    sys.exit(-1)

if not os.path.exists(opt.bin_path):
    print >> sys.stderr, "\nError: File [ %s ] not found !!!\n" % (opt.bin_path)
    parser.print_help()
    sys.exit(-1)

#s = serial.Serial(opt.com_port, 115200)
#s.rts = 0;
s = serial.Serial()

def resetIntoBootloader():
    
    s.baudrate = 115200
    s.port = opt.com_port
    s.timeout = 1
    #s.rts = 0;
    s.open()

    #Discharge RTS
    s.setRTS(True)
    time.sleep(0.01)

    #Pull down only DTR
    #time.sleep(0.01)
    s.setRTS(False)
    pass


print >> sys.stderr, "Please push the Reset button"


error_count = 0
c_count = 0
retry = 0
start_time = time.time()
resetIntoBootloader()
while 1:
    c = s.read()
    if debug:
        print >> sys.stderr, "Read: "+c.encode('hex')
        pass
    if c == "C":
        c_count  = c_count +1
    if c!=0 and c!="C":
        error_count = error_count +1
    if c_count>3:
        print >> sys.stderr, "Reset button pushed, start uploading user bin"
        break
        pass
    if error_count>30:
        print "Error - Not Reading the start flag" 
        retry  = retry +1
        error_count = 0
        c_count = 0
        start_time = time.time()
        s.close()
        resetIntoBootloader()
    if time.time() - start_time > 3.0:
        print "Error - Timeout" 
        retry  = retry +1
        error_count = 0
        c_count = 0
        start_time = time.time()
        s.close()
        resetIntoBootloader()
        pass
    if retry>3:
        print "Exiting"
        exit()
        pass


#statinfo = os.stat('./bootloader.bin')
#bar = pyprind.ProgBar(statinfo.st_size/128+1)

def getc(size, timeout=1):
    return s.read(size)

#def putc(data, timeout=1):
#    bar.update()
#    return s.write(data)

def putc_user(data, timeout=1):
    bar_user.update()
    return s.write(data)

def pgupdate(read, total):
    print "\r%d/%d bytes (%2.f%%) ..." % (read, total, read*100/total)

#m = xmodem.XMODEM(getc, putc)
#
#stream = open('./bootloader.bin', 'rb')
#m.send(stream)
#s.baudrate = 115200
#
#print >> sys.stderr, "Bootloader uploaded, start uploading user bin"
#time.sleep(1)
#if opt.ldr:
#    s.write("1\r")
#    pass
#if opt.n9:
#    s.write("3\r")
#    pass
#if opt.cm4:
#    s.write("2\r")
#    pass
#s.flush()
#s.flushInput()
#
flag = 1
while flag:
    c = s.read()
    if c =='C':
        flag = 0
        pass
    pass
s.flush()
s.flushInput()

statinfo_bin = os.stat(opt.bin_path)
bar_user = pyprind.ProgBar(statinfo_bin.st_size/1024+2)
stream = open(opt.bin_path, 'rb')
m = xmodem.XMODEM1k(getc, putc_user)
m.send(stream)

print >> sys.stderr, "Program uploaded, starting the program"
time.sleep(1)
#s.write("C\r")
s.flush()
s.flushInput()
