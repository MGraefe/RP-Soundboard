#!/usr/bin/python
# -*- coding: utf-8 -*-

# Copyright (c) 2016 Marius Gräfe
# Script to generate version.h from version.txt
# version.txt should be a simple text document with
# four dot separated numbers in it (e.g. "1,2,3,1234")
# Options:
#   -inc	Increment version (fourth number by default)
#   -major	Increment first number if -inc is given
#   -minor	Increment second number if -inc is given
#   -revision   Increment third number if -inc is given


import re
import sys

productName = 'ts3sb'; # Set product name here


def findPatternNum():
	if '-major' in sys.argv:
		return 1;
	if '-minor' in sys.argv:
		return 2;
	if '-revision' in sys.argv:
		return 3;
	return 4;

def replaceVersion():
	incVersion = '-inc' in sys.argv;
	f = open('version.txt', 'r');
	if f == None:
		print('File not found');
		return 1;
		
	s = f.read();
	f.close();
	
	pattern = '([0-9]+)\\.([0-9]+)\\.([0-9]+)\\.([0-9]+)';

	reObj = re.search(pattern, s);
	if reObj == None:
		print('Pattern not found');
		return 1;

	patternNum = findPatternNum();
	groups = list(reObj.groups());
	if incVersion:
		groups[patternNum-1] = str(int(groups[patternNum-1]) + 1);

	productNameCap = productName.upper();
	
	sh = \
	'//CHANGES WILL BE OVERWRITTEN BY version.py\n\n' \
	'#ifndef ' + productName + '_all__version_H__\n' \
	'#define ' + productName + '_all__version_H__\n\n' \
	'#define ' + productNameCap + '_VERSION ' + ','.join(groups) + '\n' + \
	'#define ' + productNameCap + '_VERSION_S "' + '.'.join(groups) + '"\n' + \
	'#define ' + productNameCap + '_VERSION_MAJOR ' + groups[0] + '\n' + \
	'#define ' + productNameCap + '_VERSION_MINOR ' + groups[1] + '\n' + \
	'#define ' + productNameCap + '_VERSION_REVISION ' + groups[2] + '\n' + \
	'#define ' + productNameCap + '_VERSION_BUILD ' + groups[3] + '\n' + \
	'\n#endif\n';
	
	fh = open('version.h', 'w');
	
	fh.write(sh);
	fh.close();

	if incVersion:
		ft = open('version.txt', 'w');
		ft.write('.'.join(groups));
		ft.close();
	
	return 0;


sys.exit(replaceVersion());
