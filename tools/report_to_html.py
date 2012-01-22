#!/usr/bin/env python

import re
import sys
import datetime

dic = [ \
	[ 'gcc_44', '<td>Linux 2.6.32</td><td>GCC-4.4</td><td>x86_64</td>' ] , 
	[ 'gcc_44_stlport', '<td>Linux 2.6.32</td><td>GCC-4.4/STLPort 5.2</td><td>x86_64</td>'],
	[ 'clangcc', '<td>Linux 2.6.32</td><td>Clang-2.9</td><td>x86_64</td>'],
	[ 'intel', '<td>Linux 2.6.32</td><td>Intel 12.1</td><td>x86_64</td>'], 
	[ 'arm', '<td>Linux 2.6.32</td><td>GCC-4.1</td><td>armel</td>'],
	[ 'bsd_gcc', '<td>FreeBSD 8.0</td><td>GCC-4.2</td><td>x86</td>'],
	[ 'solaris_gcc', '<td>OpenSolaris 2009.06</td><td>GCC-3.4</td><td>x86</td>'],
	[ 'solaris_suncc', '<td>OpenSolaris 2009.06</td><td>Sun Studio 5.11</td><td>x86</td>'],
	[ 'win_gcc', '<td>Windows XP SP2</td><td>GCC-4.5</td><td>x86</td>'],
	[ 'win_msvc', '<td>Windows XP SP2</td><td>MSVC 2008</td><td>x86</td>'],
]

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
<table cellpadding="3" cellspacing="0" border="1" >
<tr><th width="20%" >Operating System</th><th width="20%" >Compiler</th><th width="20%">Platform</th><th width="20%">Status</th></tr>
"""

all={}

for line in sys.stdin:
	m=re.match(r,line)
	if m:
		all[m.group(1)] = m.group(2)

for key,value in dic:
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


