#encoding=utf-8
import telnetlib
import sys
import os
import thread
import time
import signal

def timeout_deal():
	time.sleep(5)
	os.kill(os.getpid(),signal.SIGINT);

def do_telnet(Host):
    tn = telnetlib.Telnet(Host, port=23, timeout=5)
    result = tn.read_until('login: ')
    filename = "/tmp/content/telnet_"+Host
    fp = open(filename,'w')
    fp.write(result)
    tn.close()

if __name__=='__main__':
	Host = sys.argv[1]
	thread.start_new_thread(timeout_deal,())
	do_telnet(Host)