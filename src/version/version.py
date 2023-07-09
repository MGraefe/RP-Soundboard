#!/usr/bin/python
# -*- coding: utf-8 -*-

# Copyright (c) 2023 Marius Gräfe
# Script to generate version.h from git output


import re
import sys
import subprocess

productName = 'ts3sb'; # Set product name here


def replaceVersion():
	s = subprocess.check_output(['git', 'describe', '--tags']).decode();
	
	pattern = '([0-9]+)\\.([0-9]+)\\.([0-9]+)\\.([0-9]+)';

	reObj = re.search(pattern, s);
	if reObj == None:
		print('Pattern not found');
		return 1;

	groups = list(reObj.groups());

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
	
	return 0;


sys.exit(replaceVersion());
