#!/usr/bin/env python

import sys
import telnetlib

pkg = sys.argv[1]

print "reinstall '%s' through telnet" % pkg

HOST = sys.argv[2]
user = '' #'admin'
password = '' #'admin'

prompt = ":~# "

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
tn.write("ipkg update\n")
read()
tn.write("ipkg remove --force-depends %s\n" % pkg)
read()
tn.write("ipkg install %s\n" % pkg)
read()
tn.write("exit\n")

print tn.read_all()
