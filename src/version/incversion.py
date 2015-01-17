#!/usr/bin/python
# -*- coding: utf-8 -*-


import re
import sys

productName = 'ts3sb';

def findPatternNum():
	if '-major' in sys.argv:
		return 1;
	if '-minor' in sys.argv:
		return 2;
	if '-revision' in sys.argv:
		return 3;
	return 4;

def replaceVersion():
	f = open('version.txt', 'r');
	if f == None:
		print('File not found');
		return 1;
		
	s = f.read();
	f.close();
	
	pattern = '([0-9]+),([0-9]+),([0-9]+),([0-9]+)'

	reObj = re.search(pattern, s);
	if reObj == None:
		print('Pattern not found');
		return 1;

	patternNum = findPatternNum();
	groups = list(reObj.groups());
	groups[patternNum-1] = str(int(groups[patternNum-1]) + 1);

	ns = re.sub(pattern, ','.join(groups), s);
	productNameCap = productName.upper();
	
	sh = \
	'//CHANGES WILL BE OVERWRITTEN BY incversion.py\n\n' \
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
	ft = open('version.txt', 'w');
	ft.write(','.join(groups));
	ft.close();
	
	return 0;


sys.exit(replaceVersion());
