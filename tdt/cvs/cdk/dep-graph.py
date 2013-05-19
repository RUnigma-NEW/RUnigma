#!/usr/bin/env python2

import sys
import os
import re

# Usage ./dep-graph.py [target to study] [makefile dump]


makefile_name = "Makefile.dump"
if len(sys.argv) > 2:
	makefile_name = sys.argv[2]
else:
	os.system("export LANG=en_US; make -p > %s" % makefile_name)

rootname = ""
if len(sys.argv) > 1:
	rootname = sys.argv[1]
	
fdot = open('dep.dot', 'w')


f = open(makefile_name)

targ = {}

fdot.write("digraph G{")
fdot.write("""
#	nodesep=0.2;
ranksep=1;
	charset="latin1";
	rankdir=LR;
#	fixedsize=true;
	concentrate=true;
	node [style="rounded,filled", width=0, height=0, shape=box, fillcolor="#E5E5E5"]
""")


def debug(s):
	sys.stderr.write(s+"\n")

def process(s):
	s = s.strip()
	if s.find("Patches") > -1 or s.find("Archive") > -1 or s.startswith('root/'):
		return ''
	l = s.split('/')
	if s.find(".deps") < 0:
		if len(l) < 2:
			s = l[-1]
		else:
			s = l[-2] +'/' + l[-1]
	else:
		s = l[-1]
	if s.startswith("max-") or s.startswith("min-") or s.startswith("std-"):
		return ''
	if s == "|":
		return ''
	return s
	
	
started = False

def uniq(l):
 return list(set(l))

while True:
	line = f.readline()
	if not line:
		break
	if line.startswith("# Files"):
		started = True
	if line.startswith("# Not a target:"):
		f.readline()
		continue
	if not started:
		continue
	if line.startswith("#") or line.startswith("\t"):
		continue
	print "line: " + line.strip()
	l = line.split(":")
	if len(l) < 2:
		continue
	name = process(l[0])
	if not name:
		continue
	deps = []
	if len(l) > 1:
		deps = l[1].split(' ')
	d = []
	for i in range(len(deps)):
		deps[i] = process(deps[i])
		if not deps[i]:
			continue
		d += [deps[i]]
	deps = uniq(d)
	print "adding %s: %s" % (name, ' '.join(deps))
	if name in targ:
		print "ERROR: %s duplicate" % name
	targ[name] = deps

targ2 = {}


def unite(suffix, x):
	if x.endswith(suffix):
		k = x.replace(suffix,'')
		try:
			print "s", k, targ[k]
		except:
			pass
		if not k in targ:
			print "ERROR: unite of %s failed" % k
			return False
		targ[k] += targ[x]
		c = k + '.do_compile'
		while c in targ[k]:
			targ[k].remove(c)
		c = k + '.do_prepare'
		while c in targ[k]:
			targ[k].remove(c)
		targ2[k] = uniq(targ[k])
		targ[k] = uniq(targ[k])
		print "a", k, targ[k]
		return True
	return False

for x in targ:
	if unite('.do_prepare',x):
		continue
	if unite('.do_compile',x):
		continue
	print "o", x, targ[x]
	l = targ[x]
	for i in range(len(l)):
		if l[i].endswith('.do_prepare'):
			l[i] = l[i].replace('.do_prepare', '')
		elif l[i].endswith('.do_compile'):
			l[i] = l[i].replace('.do_compile', '')
		elif l[i].find('.version_') > -1:
			print 'VERSION', l[i][:l[i].find('.version_')]
			l[i] = l[i][:l[i].find('.version_')]
	targ[x] = l
	targ2[x] = targ[x]

targ = targ2
del targ2

"""
key = ".PHONY"
walk = []
while True:
	if (not key in targ) or len(targ[key]) == 0:
		print "rm", key
		if key in targ: del targ[key]
		if len(walk) == 0:
			break
		else:
			parent = walk.pop()
			targ[parent].remove(key)
			key = parent
	else:
		walk += [key]
		key = targ[key][0]
"""

def DFS(start, do_cmd):
	curr = start
	walk = []
	last = {}
	while True:
		#find next
		child = []
		idx = 0
		if curr in targ:
			child = targ[curr]
		if curr in last:
			idx = child.index(last[curr])
			idx += 1
		#print curr, child
		if idx >= len(child):
			# go up
			do_cmd(curr, walk)
			if len(walk) == 0:
				break
			curr = walk.pop()
		else:
			# go next
			ne = child[idx]
			if ne in walk:
				print "ERROR: broke loop at", ne
				targ[curr].remove(ne)
				continue
			last[curr] = ne
			walk.append(curr)
			curr = ne

def print_cmd(curr, walk):
	if walk:
		fdot.write(' "%s" -> "%s" ;\n' % (walk[-1], curr))

if rootname:
	DFS(rootname, print_cmd)
else:
	for x in targ:
		for y in targ[x]:
			fdot.write(' "%s" -> "%s" ;\n' % (y, x))

fdot.write("}")
fdot.close()

print "Drawing graph...."
cmd = "cat dep.dot |grep -v '.version_' |tred |dot -Tsvg -o dot.svg"
print "exec:", cmd
os.system(cmd)
print "output is in dot.svg"
