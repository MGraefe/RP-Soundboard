#!/usr/bin/python
# -*- coding: utf-8 -*-

import re
import sys
import os.path
import datetime
import os
import codecs


# GLOBAL SETTINGS

rootfolder = 'src'
ignores = ('.svn', 'editor/resource.h', 'starter/resource.h')
encoding = 'iso-8859-15'
extensions = ('.h', '.cpp', '.glsl', '.rps')

header = u"""\
//----------------------------------
// RP Soundboard Source Code
// Copyright (c) """ + datetime.datetime.now().strftime('%Y') + u""" Marius Graefe
// All rights reserved
// Contact: rp_soundboard@mgraefe.de
//----------------------------------"""



# END GLOBAL SETTINGS

class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    
def printErr(s):
	if os.name == 'nt':
		print(s)
	else:
		print(bcolors.FAIL + s + bcolors.ENDC)

def printWar(s):
	if os.name == 'nt':
		print(s)
	else:
		print(bcolors.WARNING + s + bcolors.ENDC)

def printOk(s):
	if os.name == 'nt':
		print(s)
	else:
		print(bcolors.OKGREEN + s + bcolors.ENDC)

def isCRLF(s):
	return s.find('\r\n') != -1


def getHeaderGuardName(headerFn):
	return re.sub('_h__', '_H__', re.sub(r"[^a-zA-Z0-9]", "_", re.sub(r"[/\\]", "__", headerFn)) + "__")


#replace header guards
def replaceHeaderGuard(fc, filename, prefix=''):
	if os.path.splitext(filename)[1] != '.h':
		return fc
	#search #ifndef
	mifn = re.search(r'#ifndef\s+(\w+_[hH]_{0,2})', fc)
	if mifn == None:
		return fc
	#search #define
	mdef = re.search(r'#define\s+(' + mifn.group(1) + ')', fc)
	if mdef == None:
		return fc
	if mifn.end(0) > mdef.start(0):
		printWar('Bogus file: ' + filename)
		return fc
	pragma = ''
	if fc.find('#pragma once') == -1:
		pragma = '#pragma once' + ('\r\n' if isCRLF(fc) else '\n')
	#search #endif
	mcmt = re.search(r'#endif\s+//\s+(' + mifn.group(1) + ')', fc)
	ng = prefix + getHeaderGuardName(filename)
	rep = fc[0:mifn.start(0)] + pragma + fc[mifn.start(0):mifn.start(1)] + ng + \
		fc[mifn.end(1):mdef.start(1)] + ng + \
		((fc[mdef.end(1):]) if mcmt == None else (fc[mdef.end(1):mcmt.start(1)] + ng + fc[mcmt.end(1):]));
	return rep


#replace the file's top comment's filename
def replaceFilenameComment(fc, filename):
	ext = os.path.splitext(filename)[1]
	if not ext in extensions:
		printErr('replaceFirstLine: invalid extension ' + ext)
		return
	mcmt = re.search(r'\w*//\s*[\w\-\\/\.]+((' + ')|('.join(extensions) + '))', fc)
	if mcmt != None and mcmt.start(0) == 0:
		rep = u'// ' + filename + fc[mcmt.end(0):]
	else:
		rep = u'// ' + filename + ('\r\n' if isCRLF(fc) else '\n') + fc;
	return rep


#replace the header
def replaceHeader(fc, filename):
	mcmt = re.search(r'//\-{34}\r?\n(//.+\r?\n){4}//\-{34}', fc);
	lheader = (header.replace('\n', '\r\n')) if isCRLF(fc) else (header)
	if mcmt == None:
		return lheader + ('\r\n\r\n' if isCRLF(fc) else '\n\n') + fc
	else:
		return fc[0:mcmt.start(0)] + lheader + fc[mcmt.end(0):]


#do all replacements in a file
def replaceFile(fn, enc='utf-8'):
	f = codecs.open(fn, 'r', enc)
	if f == None:
		printErr('error opening file ' + fn)
		return False
	fc = f.read()
	f.close()
	fc = replaceHeader(fc, fn)
	fc = replaceFilenameComment(fc, fn)
	fc = replaceHeaderGuard(fc, fn, 'rpsb')
	try:
		f = codecs.open(fn, 'w', enc)
		f.write(fc)
		f.close()
		printOk(fn)
		return True
	except:
		printErr('Could not write to file ' + fn)
		return False


def isIgnored(fn, ignos):
	for igno in ignos:
		if fn.find(igno) != -1:
			return True
	return False


def handleFileEntry(fn, ignos, enc):
	ret = True
	for p in os.listdir(fn):
		pp = fn + '/' + p
		if os.path.isdir(pp):
			ret = handleFileEntry(pp, ignos, enc)
		else:
			ext = os.path.splitext(pp)[1]
			if (ext in extensions) and (not isIgnored(pp, ignos)):
				if not replaceFile(pp, enc):
					ret = False
	return ret


print("Source code header manager")
print("Copyright (c) 2015 Marius Graefe")
err = False
if not handleFileEntry(rootfolder, ignores, encoding):
	err = True;
#for f in os.listdir(rootfolder):
#	if os.path.isdir(f):
#		if not handleFileEntry(f, ignores, encoding):
#			err = True;
print('All done.')
if err:
	print('Some errors occured.')



