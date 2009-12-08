#!/usr/bin/env python
#
# Copyright Artyom Beilis 2009. Use, modification and
# distribution is subject to the Boost Software License, Version
# 1.0. (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)
#


import re
import sys
import os
import os.path
import StringIO

class SyntaxError(Exception):
    def __init__(self,message):
        self.message=message

class tockenizer():
    def __init__(self,input):
        self.input=input
    def chr_end(self,line,pos,c):
        size=len(line)
        while pos<size:
            if line[pos:pos+2]=="\\\\" or line[pos:pos+2]=='\\'+c:
                pos+=2
            elif line[pos]==c:
                return pos+1
            else:
                pos+=1
        return -1
    def tockens(self):
        line=self.input.readline()
        while line:
            size=len(line)
            i=0
            while i<size:
                if line[i].isalpha() or line[i]=='_':
                    n=1
                    while i+n<size and (line[i+n].isalpha() or line[i+n]=='_' or line[i+n].isdigit()):
                        n+=1
                    yield line[i:i+n]
                    i+=n
                elif line[i:i+2]==r'//':
                    yield line[i:]
                    i=size
                elif line[i:i+2]==r'/*':
                    end=line.find(r'*/',i)
                    if end!=-1:
                        yield line[i:end+2]
                        i=end+2
                    else:
                        res=line[i:]
                        line=self.input.readline()
                        while line:
                            end=line.find(r'*/')
                            if end==-1:
                                res+=line
                                line=self.input.readline()
                            else:
                                res+=line[:end+2]
                                size=len(line)
                                i=end+2;
                                yield res
                                break
                        size=len(line)
                elif line[i]=='"' or line[i]=="'":
                    c=line[i]
                    end=self.chr_end(line,i+1,c)
                    if end!=-1:
                        yield line[i:end]
                        i=end
                    else:
                        res=line[i:]
                        line=self.input.readline()
                        while line:
                            end=self.chr_end(line,0,c)
                            if end==-1:
                                res+=line
                                line=self.input.readline()
                            else:
                                res+=line[:end]
                                size=len(line)
                                i=end;
                                yield res
                                break
                        size=len(line)
                elif i+1==size and line[i:i+1]=='\\\n':
                    yield '\\\n'
                    i+=2
                else:
                    yield line[i]
                    i+=1
            line=self.input.readline()


class renamer():
    fname=re.compile(r'^(\w+)$')
    ident=re.compile(r'^[a-zA-Z_][a-zA-Z_0-9]*$')
    def __init__(self,input,output,namespace,newdir=''):
        self.input=input
        self.output=output
        self.namespace=namespace
        self.newdir=newdir
    def process_tocken(self,tocken):
        if self.ident.match(tocken):
            tocken=tocken.replace('BOOST',namespace.upper())
            tocken=tocken.replace('boost',namespace)
        self.output.write(tocken)
    def process_all(self,lst):
        for tocken in lst:
            self.process_tocken(tocken)
    def convert_path(self,lst):
        self.output.write('<'+self.newdir)
        self.process_all(lst[2:])
        
    def rename(self):
        parser=tockenizer(self.input)
        state='Normal'
        substate=None
        lst=[]
        inc=re.compile(r'^"boost(/.*)"$')
        for tocken in parser.tockens():
            lst.append(tocken)
            if state=='Normal' and tocken=='<':
                state='<'
                continue
            elif state=='<' and tocken=='boost':
                state='boost'
                continue
            elif state=='boost' and tocken=='/':
                state='/'
                continue
            elif state=='/' and self.fname.match(tocken):
                state='dir'
                continue
            elif state=='dir' and tocken=='/':
                state='/'
                continue
            elif state=='dir' and tocken=='.':
                state='.'
                continue
            elif state=='.' and (tocken=='ipp' or tocken=='h' or tocken=='hpp'):
                state='hpp'
                continue
            elif state=='dir' and tocken=='>':
                self.convert_path(lst)
                lst=[]
            elif state=='hpp' and tocken=='>':
                self.convert_path(lst)
                lst=[]
            elif state=='Normal' and inc.match(tocken):
                m=inc.match(tocken)
                lst[0]='"'+self.newdir+m.group(1)+'"'
            state='Normal'
            self.process_all(lst)
            lst=[]
def is_cpp(name):
    for suffix in ['.hpp','.h','.ipp','.cpp','.c','.inl','inc','.SUNWCCh','.cxx','.cc' ]:
        if name.endswith(suffix):
            return True
    if os.path.basename(os.path.dirname(name)) in ['tr1','cpp_c_headers']:
        return True
    return False

def is_ign(name):
    ign=['.vcproj', '.sln', '.v2', '.html', '.cmake', '.txt', '.qbk',\
         '.mak', '.sh', '.pl', '.r', '.css', '.png', '.doc', '.vsprops','.mcp'\
         '.xml','.xsd','.jam','.htm','.bat','.xml','.dtd','.zip',\
         '.gif','.sty','.pdf','.csh','.w','.fig','.graffle','.jpg',\
         '.dot','.cfg','.dimacs','.expected','.dat','.js','.py','.svg','.jpeg','.mml',\
         '.input','.flex','.hdf','.manifest','.xsl','.m4','.rst','.rsa','.pyste',\
         '.ok','.err1','.err2','.err3','.mini','.db','.toyxml','.quickbook','.gold',\
         '.cmd','.toc','.pem','.xls','.rsp','.reno','.output','.log','.in','.am']
    for suffix in ign:
        if name.endswith(suffix):
            return True
    if os.path.basename(os.path.dirname(name)) in ['doc','boehm_gc','debian']:
        return True
    name=os.path.basename(name)
    if name in ['Jamfile', 'Makefile','Jamroot','INSTALL','README','LICENSE','Thumbs.db','TODO','NEWS','configure','sublibs','Changelog']:
        return True
    return False

def rename_one(name,namespace,newdir):
    if is_cpp(name):
        print "Processing file %s" % name
        fin=file(name)
        buffer=StringIO.StringIO()
        ren=renamer(fin,buffer,namespace,newdir)
        ren.rename()
        fin.close()
        fout=file(name,'w')
        buffer.seek(0)
        line=buffer.readline()
        while line:
            fout.write(line)
            line=buffer.readline()
        fout.close()
    elif is_ign(name):
        pass
    else:
        print "Warning!!!!!!!!!! Unlnown file type %s" % name
        print "--------------------Ignoring----------------"
        rep=file('warning.log','a')
        rep.write('Unlnown file type %s\n' % name)
        rep.close()

def rename_recursively(dir,namespace,newdir):
    for root,dirs,files in os.walk(dir):
        for file in files:
            rename_one(os.path.join(root,file),namespace,newdir)

if __name__=='__main__':
    if len(sys.argv)<3:
        print "Usage rename.py path newnamespace"
        print "for example: rename.py boost_1_39_0 mybst"
        sys.exit(1)
    path=sys.argv[1]
    namespace=sys.argv[2]
    if namespace.lower()!=namespace:
        print "Namespace should be lowercase"
        sys.exit(1)
    newdir=namespace
    ren=rename_recursively(path,namespace,newdir)
    boost_dir=os.path.join(path,'boost')
    new_dir=os.path.join(path,newdir)
    if os.path.isdir(boost_dir):
        os.rename(boost_dir,new_dir)

