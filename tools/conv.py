#!/usr/bin/python

import sys
import re


class converter():
	confline=re.compile(r'^\s*#include\s*"config.h"\s*$')
	bline=re.compile(r'^\s*#include\s*<boost/(.*)>\s*$')
	locline=re.compile(r'^(locale/.*|locale.hpp)$')

	included=0
	assigned=0
	lst=[];

	def print_list(self):
		if not self.lst:
			return
		if not self.included:
			print r'#include "config.h"'
			self.included=1
		print '#ifdef CPPCMS_USE_EXTERNAL_BOOST'
		for inc in self.lst:
			print '#   include <boost/%s>' % inc
		print '#else // Internal Boost'
		for inc in self.lst:
			print '#   include <cppcms_boost/%s>' % inc
		if not self.assigned:
			print '    namespace boost = cppcms_boost;'
			self.assigned=1
		print '#endif'
		self.lst=[]
	def run(self):
		for line in sys.stdin:
			if self.confline.match(line):
				self.included=1
			r=self.bline.match(line)
			if r:
				inc=r.group(1)
				if self.locline.match(inc):
					self.print_list()
					print line,
				else:
					self.lst.append(inc)
					
			else:
				self.print_list()
				print line,

		self.print_list()
		
cvt=converter()
cvt.run()
