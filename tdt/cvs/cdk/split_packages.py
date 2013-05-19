#!/usr/bin/env python

import os

#######################################################################
#Main functions and constants

global bb_data
bb_data = {}

def str_check(s):
	if not isinstance(s, str):
		raise("bb_data got non str " + s)

def bb_set(var, val, *args):
	str_check(var)
	bb_data[var] = val

def bb_get(var, *args):
	str_check(var)
	try:
		return bb_data[var]
	except KeyError:
		return ""

def bb_checkset(var, val):
	str_check(var)
	if not bb_data.has_key(var):
		bb_set(var, val)

DATAS_STR = 'PKGV PKGR DESCRIPTION SECTION PRIORITY MAINTAINER LICENSE PACKAGE_ARCH HOMEPAGE RDEPENDS RREPLACES RCONFLICTS SRC_URI FILES NAME preinst postinst prerm postrm conffiles'
DATAS = DATAS_STR.split()

#######################################################################
#Load enviroment variables

parent_pkg = os.environ['PARENT_PK']
bb_set('PARENT_PK', os.environ['PARENT_PK'])

DEFAULT_DATAS = [
	['MAINTAINER', 'Ar-P team'],
	['PACKAGE_ARCH', 'sh4'],
	['FILES', '/'],
	['SECTION', 'base'],
	['PRIORITY', 'optional'],
	['LICENSE', 'unknown'],
	['HOMEPAGE', 'unknown'],
	['preinst', ''],
	['postinst', ''],
	['prerm', ''],
	['postrm', ''],
	['conffiles', '']]

for x in DEFAULT_DATAS:
	bb_checkset('%s_%s' % (x[0], parent_pkg), x[1])

bb_checkset('NAME_' + parent_pkg, parent_pkg)

global work_dir
work_dir = os.getcwd()
print "Building in", work_dir

if os.environ.has_key('PACKAGES_' + parent_pkg):
	pkg_list = os.environ['PACKAGES_' + parent_pkg]
else:
	pkg_list = parent_pkg


bb_set('PACKAGES', pkg_list)
bb_set('PACKAGES_' + parent_pkg, pkg_list)
import re

extdatas = map("^{0}_.*".format, DATAS)
regexp = '|'.join(extdatas)
reg = re.compile(regexp)

for x in os.environ.keys():
	if reg.match(x):
		bb_set(x, os.environ[x])

#print bb_data

#install DEST_DIR
global pkgs_dir
pkgs_dir = os.environ['packagingtmpdir']
print "searching for files in", pkgs_dir

#where this script will build packages
global build_dir
build_dir = os.environ['IPKGBUILDDIR']
print "temporary files goes in", build_dir



#######################################################################
#######################################################################

#Copied from bitbake (portage based) (c)
import bb

if not os.path.isdir(pkgs_dir):
	bb.mkdirhier(pkgs_dir)

def install_files(files, root):
	import glob, errno, re,os
	
	for file in files:
		if os.path.isabs(file):
			file = '.' + file
		if not os.path.islink(file):
			if os.path.isdir(file):
				newfiles =  [ os.path.join(file,x) for x in os.listdir(file) ]
				if newfiles:
					files += newfiles
					continue
		globbed = glob.glob(file)
		if globbed:
			if [ file ] != globbed:
				if not file in globbed:
					files += globbed
					continue
				else:
					globbed.remove(file)
					files += globbed
		if (not os.path.islink(file)) and (not os.path.exists(file)):
			continue
		if file[-4:] == '.pyc':
			continue
		if file in seen:
			continue
		seen.append(file)
		if os.path.isdir(file) and not os.path.islink(file):
			bb.mkdirhier(os.path.join(root,file))
			os.chmod(os.path.join(root,file), os.stat(file).st_mode)
			continue
		fpath = os.path.join(root,file)
		dpath = os.path.dirname(fpath)
		bb.mkdirhier(dpath)
		ret = bb.copyfile(file, fpath)
		if ret is False:
			raise("File population failed when copying %s to %s" % (file, fpath))

def legitimize_package_name(s):
	"""
	Make sure package names are legitimate strings
	"""
	import re

	def fixutf(m):
		cp = m.group(1)
		if cp:
			return ('\u%s' % cp).decode('unicode_escape').encode('utf-8')

	# Handle unicode codepoints encoded as <U0123>, as in glibc locale files.
	s = re.sub('<U([0-9A-Fa-f]{1,4})>', fixutf, s)

	# Remaining package name validity fixes
	return s.lower().replace('_', '-').replace('@', '+').replace(',', '+').replace('/', '-')

def do_split_packages(d, root, file_regex, output_pattern, description, postinst=None, recursive=False, hook=None, extra_depends=None, aux_files_pattern=None, postrm=None, allow_dirs=False, prepend=False, match_path=False, aux_files_pattern_verbatim=None, allow_links=False):
	"""
	Used in .bb files to split up dynamically generated subpackages of a 
	given package, usually plugins or modules.
	"""

	dvar = pkgs_dir

	packages = bb_get('PACKAGES').split()

	if not recursive:
		objs = os.listdir(dvar + root)
	else:
		objs = []
		for walkroot, dirs, files in os.walk(dvar + root):
			for file in files:
				relpath = os.path.join(walkroot, file).replace(dvar + root + '/', '', 1)
				if relpath:
					objs.append(relpath)
					
	for o in objs:
		import re, stat
		if match_path:
			m = re.match(file_regex, o)
		else:
			m = re.match(file_regex, os.path.basename(o))
		
		if not m:
			continue
		f = os.path.join(dvar + root, o)
		mode = os.lstat(f).st_mode
		if not (stat.S_ISREG(mode) or (allow_links and stat.S_ISLNK(mode)) or (allow_dirs and stat.S_ISDIR(mode))):
			continue
		on = legitimize_package_name(m.group(1))
		pname = output_pattern % on
		pkg = pname.replace('-', '_')
		if not pkg in packages:
			if prepend:
				packages = [pkg] + packages
			else:
				packages.append(pkg)
			the_files = [os.path.join(root, o)]
			if aux_files_pattern:
				if type(aux_files_pattern) is list:
					for fp in aux_files_pattern:
						the_files.append(fp % on)	
				else:
					the_files.append(aux_files_pattern % on)
			if aux_files_pattern_verbatim:
				if type(aux_files_pattern_verbatim) is list:
					for fp in aux_files_pattern_verbatim:
						the_files.append(fp % m.group(1))	
				else:
					the_files.append(aux_files_pattern_verbatim % m.group(1))
			
			bb_set('FILES_' + pkg, " ".join(the_files))
			bb_set('NAME_' + pkg, pname)
			
			if extra_depends != '':
				the_depends = bb_get('RDEPENDS_' + pkg, d, True)
				if the_depends:
					the_depends = '%s %s' % (the_depends, extra_depends)
				else:
					the_depends = extra_depends
				
				bb_set('RDEPENDS_' + pkg, the_depends, d)
			
			bb_set('DESCRIPTION_' + pkg, description % on, d)
			
			if postinst:
				bb_set('postinst_' + pkg, postinst, d)
			if postrm:
				bb_set('postrm_' + pkg, postrm, d)
		else:			
			oldfiles = bb_get('FILES_' + pkg, d, True)			
			if not oldfiles:
				raise("Package '%s' exists but has no files" % pkg)
			bb_set('FILES_' + pkg, oldfiles + " " + os.path.join(root, o), d)
		
		if callable(hook):
			hook(f, pkg, file_regex, output_pattern, m.group(1))

	bb_set('PACKAGES', ' '.join(packages), d)

#End of bitbake code

#######################################################################
#######################################################################

#Use tricky files staff from bitbake(portage based), because it is usefull.
#But keep packaging simple

def read_control_file(fname):
	src = open(fname).read()
	for line in src.split("\n"):
		if line.startswith('Package: '):
			full_package = line[9:].replace('-', '_')
			bb_set('NAME_' + full_package, line[9:])
		if line.startswith('Depends: '):
			bb_set('RDEPENDS_' + full_package, ' '.join(line[9:].split(', ')))
		if line.startswith('Description: '):
			bb_set('DESCRIPTION_' + full_package, line[13:])
		if line.startswith('Replaces: '):
			bb_set('RREPLACES_' + full_package, ' '.join(line[10:].split(', ')))
		if line.startswith('Conflicts: '):
			bb_set('RCONFLICTS_' + full_package, ' '.join(line[11:].split(', ')))
		if line.startswith('Maintainer: '):
			bb_set('MAINTAINER_' + full_package, line[12:])
		if line.startswith('Version: '):
			ll = line[9:].split('-')
			bb_set('PKGV_' + full_package, ll[0])
			if len(ll) > 1:
				bb_set('PKGR_' + full_package, ll[1])
		if line.startswith('Section: '):
			bb_set('SECTION_' + full_package, line[9:])
		if line.startswith('Priority: '):
			bb_set('PRIORITY_' + full_package, line[10:])
		if line.startswith('License: '):
			bb_set('LICENSE_' + full_package, line[9:])
		if line.startswith('Architecture: '):
			bb_set('PACKAGE_ARCH_' + full_package, line[14:])
		if line.startswith('Homepage: '):
			bb_set('HOMEPAGE_' + full_package, line[10:])
		if line.startswith('Source: '):
			bb_set('SRC_URI_' + full_package, line[8:])
	return full_package

def write_control_file(fdir, full_package):
	s = ''
	p = []
	fname = pjoin(fdir, 'control')
	def ext(param):
		return "%s_%s" % (param, full_package)
	pkgv = bb_get(ext('PKGV'))
	pkgr = bb_get(ext('PKGR'))
	if pkgr:
		bb_set(ext('PKGF'), '%s-%s' % (pkgv, pkgr))
	else:
		bb_set(ext('PKGF'), pkgv)
	p.append(["Package: %s\n", ['NAME']])
	p.append(["Version: %s\n", ['PKGF']])
	p.append(["Description: %s\n", ['DESCRIPTION']])
	p.append(["Section: %s\n", ['SECTION']])
	p.append(["Priority: %s\n", ['PRIORITY']])
	p.append(["Maintainer: %s\n", ['MAINTAINER']])
	p.append(["License: %s\n", ['LICENSE']])
	p.append(["Architecture: %s\n", ['PACKAGE_ARCH']])
	p.append(["Homepage: %s\n", ['HOMEPAGE']])
	p.append(["Depends: %s\n", ['RDEPENDS']])
	p.append(["Replaces: %s\n", ['RREPLACES']])
	p.append(["Conflicts: %s\n", ['RCONFLICTS']])
	p.append(["Source: %s\n", ['SRC_URI']])
	
	for l in p:
		var = bb_get(ext(l[1][0]))
		if l[1][0] == 'NAME':
			var = var.replace('_', '-').lower()
		elif l[1][0] in ['RDEPENDS', 'RREPLACES', 'RCONFLICTS']:
			var = var.replace(' ',',').replace('_', '-')
		if not var: continue
		var = l[0] % var
		s += var
	print 'Write control file to', fname
	open(fname, 'w').write(s)
	
	scr = ['preinst', 'postinst', 'prerm', 'postrm', 'conffiles']
	for s in scr:
		script = bb_get(ext(s))
		if script:
			fd = open(pjoin(fdir, s), 'w')
			fd.write(script)
			fd.close()
			if s != 'conffiles':
				os.chmod(pjoin(fdir, s), 0755)

def pjoin(*args):
	#TODO: make it more clean. remove '/' dublicates. Do it with re, it would be faster..
	return '/'.join(args)

def do_finish():
	bb.mkdirhier(build_dir)
	os.chdir(pkgs_dir)
	
	global seen
	seen = []

	for p in bb_get('PACKAGES').split():
		pname = bb_get('NAME_%s' % p)
		print "Package: %s (%s)" % (p, pname)
		print "Description: ", bb_data['DESCRIPTION_'+p]
		files = bb_data['FILES_'+p].split(" ")
		
		pack_dir = pjoin(build_dir,p)
		bb.mkdirhier(pack_dir)
		
		install_files(files, pack_dir)
		
		for data in DATAS:
			if not bb_get(data+'_'+p):
				if data == 'NAME':
					bb_set(data+'_'+p, p)
				else:
					bb_set(data+'_'+p, bb_get(data+'_'+parent_pkg))
		
		bb.mkdirhier(pjoin(pack_dir, 'CONTROL'))
		write_control_file(pjoin(pack_dir, 'CONTROL'), p)


if __name__ == "__main__":
	do_finish()