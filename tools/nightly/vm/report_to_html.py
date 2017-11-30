#!/usr/bin/env python

import re
import sys
import datetime
import glob

oses = { 'win7' : 'Windows 7', 'solaris' : 'Solaris 11', 'freebsd' : 'FreeBSD 11.1', 'localhost' : 'Linux Ubuntu 16.04' }


def get_failed_tests(tag):
    f = open('logs/' + tag + '.log','r')
    if not f:
        return ''
    res=[]
    attach=False
    for l in f.readlines():
        if attach:
            res.append(l)
        if l.find('The following tests FAILED')!=-1:
            attach=True
    return '<br/>'.join(res)

def compiler_name(x):
    return x.replace('mingw_','MinGW ').replace('gcc','GCC ').replace('clang','Clang ').replace('msvc','MSVC ').replace('std','/C++')


repo_url=sys.argv[1]
repo_rev=sys.argv[2]

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
<style>
/* Tooltip container */
.tooltip {
    position: relative;
    /*display: inline-block;*/
}

/* Tooltip text */
.tooltip .tooltiptext {
    visibility: hidden;
    width: 500px;
    background-color: yellow;
    color: black;
    text-align: left;
    padding: 5px 5px;
    border-radius: 6px;
    /* Position the tooltip text - see examples below! */
    position: absolute;
    z-index: 1;
}

/* Show the tooltip text when you mouse over the tooltip container */
.tooltip:hover .tooltiptext {
    visibility: visible;
}
</style>
"""
print datetime.datetime.now().strftime('<h2>Tested at: %Y-%m-%d %H:%M</h2>')

print "<p>%s<br/>%s</p>" % (repo_url,repo_rev)

print """
<table cellpadding="3" cellspacing="0" border="1" >
<tr><th width="20%" >Operating System</th><th width="20%" >Compiler</th><th width="20%">Platform</th><th width="20%">Status</th></tr>
"""

test_re = re.compile('logs/(([^-]+)-([^-]+)-([^-]+))-status.txt')

reports=glob.glob('logs/*-status.txt')
reports.sort()

for report in reports:
    m=test_re.match(report)
    tag=m.group(1)
    OS = oses[m.group(2)]
    Compiler = compiler_name(m.group(3))
    Platform = m.group(4)
    status=open(report,'r').readlines()[0][0:-1]
    failed = get_failed_tests(tag)
    if status!='ok':
        print '<tr><td>%s</td><td>%s</td><td>%s</td><td class="tooltip"><a href="./nightly-build-report/%s.txt">%s</a><span class="tooltiptext">%s</span></td></tr>' % (OS,Compiler,Platform,tag,status,failed)
    else:
        print '<tr><td>%s</td><td>%s</td><td>%s</td><td><a href="./nightly-build-report/%s.txt">%s</a></td></tr>' % (OS,Compiler,Platform,tag,status)

print """
</table>
</body>
</html>
"""


