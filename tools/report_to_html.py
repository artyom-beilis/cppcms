#!/usr/bin/env python

import re
import sys
import datetime

dic = { 
	'arm':'<td>GCC-4.1</td><td>Linux 2.6</td><td>armel</td>', 
	#'gcc_mac':'<td>GCC-4.2</td><td>Mac OS X (10.6)</td><td>x86_64</td>', 
	#'gcc_43_32':'<td>GCC-4.3</td><td>Linux 2.6</td><td>x86</td>', 
	'clangcc':'<td>Clang-2.9</td><td>Linux 2.6</td><td>x86_64</td>', 
	'gcc_44':'<td>GCC-4.4</td><td>Linux 2.6</td><td>x86_64</td>', 
	'gcc_44_stlport':'<td>GCC-4.4/STLPort 5.2</td><td>Linux 2.6</td><td>x86_64</td>', 
	#'gcc_45':'<td>GCC-4.5</td><td>Linux 2.6</td><td>x86_64</td>', 
	#'gcc_450x':'<td>GCC-4.5 (C++0x mode)</td><td>Linux 2.6</td><td>x86_64</td>', 
	'intel':'<td>Intel 11.0</td><td>Linux 2.6</td><td>x86_64</td>', 
	'bsd_gcc':'<td>GCC-4.2</td><td>FreeBSD 8.0</td><td>x86</td>', 
	'solaris_gcc':'<td>GCC-3.4</td><td>OpenSolaris 2009.06</td><td>x86</td>',
	'solaris_suncc':'<td>Sun Studio 5.11</td><td>OpenSolaris 2009.06</td><td>x86</td>',
	'win_gcc':'<td>GCC-4.5/MinGW</td><td>Windows XP</td><td>x86</td>', 
	'win_msvc':'<td>MSVC 2008</td><td>Windows XP</td><td>x86</td>', 
	#'win_cyg':'<td>GCC-4.3</td><td>Windows XP/Cygwin</td><td>x86</td>', 
}

r=r'(\w+)\s+-\s*(pass|fail)';

print """
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
    "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
<head>
 <title>Nightly CppCMS Builds and Tests</title>
<head>
<body>
<h1>Nightly CppCMS Builds and Tests</h1>
"""
print datetime.datetime.now().strftime('<h2>Tested at: %Y-%m-%d %H:%M</h2>')

print """
<table cellpadding="3" cellspacing="5" >
<tr><th>Compiler</th><th>Operating System</th><th>Platform</th><th>Status</th></tr>
"""

all={}

for line in sys.stdin:
	m=re.match(r,line)
	if m:
		all[m.group(1)] = m.group(2)

for key,value in dic.items():
	status = 'not tested'
	if key in all:
		status = all[key]
		link='<a href="./nightly-build-report/' + key +'.txt">' + status + '</a>'
	else:
		link=status
	print '<tr>', value, '<td>', link, '</td></tr>'

print """
</table>
</body>
</html>
"""


