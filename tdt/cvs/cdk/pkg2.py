#!/usr/bin/env python
# Usage: /path/to/file.ipk hostname

import sys
import telnetlib

pkg = sys.argv[1]

print "install '%s' through telnet" % pkg

HOST = sys.argv[2]
user = '' #'admin'
password = '' #'admin'
ftpuser = 'root'
ftppass = ''

prompt = ":~# "

fname = pkg.split('/')[-1]
import os
cmd = 'wput -u -nc %s ftp://%s:%s@%s/../tmp/%s' % (pkg, ftpuser, ftppass, HOST, fname)
print "executing", cmd
os.system(cmd)

print "connecting to", HOST,
tn = telnetlib.Telnet(HOST)
print "ok"

if user:
    print "waiting for login"
    tn.read_until("login: ")
    tn.write(user + "\n")
if password:
    print "waiting for password"
    tn.read_until("Password: ")
    tn.write(password + "\n")

def read():
	while 1:
		d = tn.read_some()
		sys.stdout.write(d)
		if d.endswith(prompt):
			break

tn.read_until(prompt)
print "start commands"
tn.write("ipkg install --force-downgrade /tmp/%s\n" % fname)
read()
tn.write("exit\n")

print tn.read_all()
