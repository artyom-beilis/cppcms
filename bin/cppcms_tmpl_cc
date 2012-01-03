#!/usr/bin/env python

############################################################################
#
#  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
#                                                                             
#  See accompanying file COPYING.TXT file for licensing details.
#
############################################################################

import os
import re
import sys

str_match=r'"([^"\\]|\\[^"]|\\")*"'
single_var_param_match=r'(?:-?\d+|"(?:[^"\\]|\\[^"]|\\")*")'
call_param_match=r'(?:\(\)|\((?:' + single_var_param_match + r')(?:,' + single_var_param_match + r')*\))'
variable_match=r"\*?([a-zA-Z][a-zA-Z0-9_]*"+ call_param_match +r"?)(((\.|->)([a-zA-Z][a-zA-Z0-9_]*" + call_param_match + r"?))*)"

def interleave(*args):
    for idx in range(0, max(map(len,args))):
        for arg in args:
            try:
                yield arg[idx]
            except IndexError:
                continue

def output(s):
    global stack
    global file_name
    global line_number
    global output_fd
    output_fd.write('\t' * len(stack) + '#line %d "%s"' % (line_number,file_name)+'\n' + \
        '\t'*len(stack) + s + '\n')

class tmpl_descr:
    def __init__(self,start,size):
        self.start_id=start
        self.param_num=size

class skin_block:
    basic_pattern='skin'
    basic_name='skin'
    pattern=r'^<%\s*skin\s+(\w+)?\s*%>$'
    type='skin'
    def use(self,m):
        global namespace_name
        name = m.group(1)

        if namespace_name == '':
            if name == '':
                error_ext("Skin name is not defined implicitly or explicitly")
            else:
                namespace_name = name
        elif namespace_name != name and name:
            error_exit("Can't use more then one skin name for same skin: %s, %s" % ( namespace_name,name))
        output( "namespace %s {" % namespace_name)
        global stack
        stack.append(self)
    def on_end(self):
        global namespace_name
        output( "} // end of namespace %s" % namespace_name)


def write_class_loader(unsafe = False):
    global class_list
    global namespace_name
    output("namespace {")
    output(" cppcms::views::generator my_generator; ")
    output(" struct loader { ")
    output("  loader() { ")
    output('   my_generator.name("%s");' % namespace_name)
    if  unsafe:
        safe = 'false'
    else:
        safe = 'true'
    for class_def in class_list:
        output( '   my_generator.add_view<%s::%s,%s>("%s",%s);' \
                % (class_def.namespace,class_def.name,class_def.content_name,class_def.name,safe))
    output('    cppcms::views::pool::instance().add(my_generator);')
    output(' }')
    output(' ~loader() {  cppcms::views::pool::instance().remove(my_generator); }')
    output('} a_loader;')
    output('} // anon ')


class html_type:
    basic_pattern='x?html'
    basic_name='xhtml'
    pattern=r'^<%\s*(x)?html\s*%>$'
    def use(self,m):
        global html_type_code
        if m.group(1):
            html_type_code='as_xhtml'
        else:
            html_type_code='as_html'


class view_block:
    pattern=r'^<%\s*view\s+(\w+)\s+uses\s+(:?:?\w+(::\w+)*)(\s+extends\s+(:?:?\w+(::\w+)?))?\s*%>$'
    basic_pattern='view'
    basic_name='view'
    type='view'
    topmost = 0
    def declare(self):
        if self.extends=='' :
            constructor='cppcms::base_view(_s)'
            self.extends='cppcms::base_view'
            self.top_most = 1
        else:
            constructor='%s(_s,_content)' % self.extends;
        output( "struct %s :public %s" % (self.class_name , self.extends ))
        output( "{")
        if self.uses!='' : 
            output( "\t%s &content;" % self.uses)
            output( "\t%s(std::ostream &_s,%s &_content): %s,content(_content)" % ( self.class_name,self.uses,constructor ))
        else:
            output( "\t%s(std::ostream &_s): %s" % ( self.class_name,constructor ))
        output("\t{")
        global spec_gettext
        if self.topmost and spec_gettext:
            output('\t\tout() << cppcms::locale::as::domain("%s");' % spec_gettext )
        output("\t}")

    def use(self,m):
        self.class_name=m.group(1)
        self.uses=m.group(2)
        if m.group(4):
            self.extends=m.group(5)
        else:
            self.extends=''
        self.declare();
        global stack
        if len(stack)!=1 or stack[-1].type!='skin':
            error_exit("You must define view inside skin block only")
        stack.append(self)
        global class_list
        global namespace_name
        class information:
            content_name=self.uses
            name=self.class_name
            namespace=namespace_name
        class_list.append(information())
    def on_end(self):
        output( "}; // end of class %s" % self.class_name)



class template_block:
    pattern=r'^<%\s*template\s+([a-zA-Z]\w*)\s*\(([\w\s,:\&]*)\)\s*%>$'
    basic_pattern = 'template'
    basic_name='template'
    type='template'
    plist=[]
    def create_parameters(self,lst):
        pattern=r'^\s*((:?:?\w+(::\w+)*)\s*(const)?\s*(\&)?\s*(\w+))\s*(,(.*))?$'
        m=re.match(pattern,lst)
        res=[]
        while m:
            global tmpl_seq
            id=m.group(6)
            if id in tmpl_seq:
                error_exit("Duplicate definition of patameter %s" % id)
                for v in self.plist:
                    del tmpl_seq[v] 
                return ""
            tmpl_seq[id]=''
            res.append(m.group(1))
            self.plist.append(id)
            if m.group(8):
                lst=m.group(8)
                m=re.match(pattern,lst)
            else:
                return ','.join(res)
        for v in self.plist:
            del tmpl_seq[v]
        error_exit("Wrong expression %s" % lst)
        
    def use(self,m):
        self.name=m.group(1)
        params=""
        if m.group(2) and not re.match('^\s*$',m.group(2)):
            params=self.create_parameters(m.group(2))
        output( "virtual void %s(%s) {" % (self.name,params) )
        global stack
        if len(stack)==0 or stack[-1].type!='view':
            error_exit("You must define template inside view block only")
        stack.append(self)
        global current_template
        current_template=self.name
        global ignore_inline
        ignore_inline=0

    def on_end(self):
        output( "} // end of template %s" % self.name)
        global ignore_inline
        ignore_inline=1
        global tmpl_seq
        tmpl_seq={}

        

def inline_content(s):
    global ignore_inline
    if not ignore_inline:
        output( 'out()<<"%s";' % to_string(s))

def warning(x):
    global file_name
    global line_number
    sys.stderr.write("Warning: %s in file %s, line %d\n" % (x,file_name,line_number))

def error_exit(x):
    global exit_flag
    global file_name
    global line_number
    sys.stderr.write("Error: %s in file %s, line %d\n" % (x,file_name,line_number))
    exit_flag=1

def to_string(s):
    res=''
    for c in s:
        global stack
        if c=='\n':
            res+="\\n\""+"\n"+"\t"*len(stack)+"\t\""
        elif c=="\t":
            res+="\\t"
        elif c=="\v":
            res+="\\v"
        elif c=="\b":
            res+="\\b"
        elif c=="\r":
            res+="\\r"
        elif c=="\f":
            res+="\\f"
        elif c=="\a":
            res+="\\a"
        elif c=="\\":
            res+="\\\\"
        elif c=="\"":
            res+="\\\""
        elif ord(c)>0 and ord(c)<32:
            res+="%03o" % ord(c)
        else:
            res+=c

    return res


def make_ident(val):
    m=re.match('^'+variable_match+'$',val)
    global tmpl_seq
    if m.group(1) in tmpl_seq:
        return val
    m2=re.match('^\*(.*)$',val)
    if m2:
        return "*content." + m2.group(1)
    else:
        return "content." + val

def print_using_block_start(class_name,variable_name,content_name):
    if content_name:
        content=make_ident(content_name)
        guard=True
    else:
        content ='content'
        guard=False
    output(r'{')
    if guard:
        output(r'  cppcms::base_content::app_guard _g(%s,content);' % content);
    output(r'  %s %s(out(),%s);' % ( class_name, variable_name, content));
    
def print_using_block_end():
    output('}')

class using_block:
    pattern=r'^<%\s*using\s+(?P<class>(?:\w+::)*\w+)(?:\s+with\s+(?P<content>' + variable_match + r'))?\s+as\s+(?P<name>[a-zA-Z_]\w*)\s*%>$' 
    basic_pattern = 'using'
    basic_name = 'using'
    type='using'
    def use(self,m):
        print_using_block_start(m.group('class'),m.group('name'),m.group('content'))
        global stack
        stack.append(self)
    def on_end(self):
        print_using_block_end();


class foreach_block:
    pattern=r'^<%\s*foreach\s+([a-zA-Z]\w*)(\s+as\s+((:?:?\w+)(::\w+)*))?' \
        + r'(?:\s+rowid\s+([a-zA-Z]\w*)(?:\s+from\s+(\d+))?)?' \
        + r'(?:\s+(reverse))?' \
        + '\s+in\s+(' + variable_match +')\s*%>$'
    basic_pattern = 'foreach'
    basic_name = 'foreach'
    type='foreach'
    has_item=0
    has_separator=0
    separator_label=''
    on_first_label=''
    type_name=0
    def use(self,m):
        self.ident=m.group(1)
        self.seq_name=make_ident(m.group(9))
        self.rowid = m.group(6)
        if m.group(7):
            self.rowid_begin = int(m.group(7))
        else:
            self.rowid_begin = 0
        if m.group(8):
            self.reverse = 'r'
        else:
            self.reverse = ''
        self.type_name = m.group(3)
        global tmpl_seq
        if self.ident in tmpl_seq:
            error_exit("Nested sequences with same name %s" % self.ident)
        if self.rowid == self.ident:
            error_exit("Nested sequence and rowid has same name %s" % self.ident)
        if self.rowid and (self.rowid in tmpl_seq):
            error_exit("Nested sequences with same rowid name %s" % self.rowid )
        tmpl_seq[self.ident]='';
        output( "if((%s).%sbegin()!=(%s).%send()) {" % (self.seq_name,self.reverse,self.seq_name,self.reverse) )
        if self.rowid:
            tmpl_seq[self.rowid]='';
            output("    int %s = %s;" % (self.rowid,self.rowid_begin))
        global stack
        stack.append(self)

    def on_end(self):
        if not self.has_item:
            error_exit("foreach without item")

        global tmpl_seq
        del tmpl_seq[self.ident]
        if self.rowid:
            del tmpl_seq[self.rowid]

        output( "}" )
    def prepare_foreach(self):
        if not self.type_name:
            ptr_type = 'CPPCMS_TYPEOF((%(s)s).%(r)sbegin())'
        else:
            ptr_type = self.type_name
        incr = ''
        if self.rowid:
            incr = ',++%s' % self.rowid;
        fmt = "for("+ptr_type+" %(i)s_ptr=(%(s)s).%(r)sbegin(),%(i)s_ptr_end=(%(s)s).%(r)send();%(i)s_ptr!=%(i)s_ptr_end;++%(i)s_ptr%(u)s) {";
        fmt = fmt %  { 's' : self.seq_name, 'i' : self.ident , 'r' : self.reverse, 'u' : incr };
        output(fmt)
        if not self.type_name:
            output( "CPPCMS_TYPEOF(*%s_ptr) &%s=*%s_ptr;" % (self.ident,self.ident,self.ident))
        else:
            output( "std::iterator_traits< %s >::value_type &%s=*%s_ptr;" % (self.type_name,self.ident,self.ident))
        if self.has_separator:
            output( "if(%s_ptr!=(%s).%sbegin()) {" % (self.ident,self.seq_name,self.reverse))
        
        

class separator_block:
    pattern=r'^<%\s*separator\s*%>'
    basic_pattern = 'separator'
    basic_name = 'separator'
    type='separator'
    def use(self,m):
        global stack
        if len(stack)==0 or stack[len(stack)-1].type!='foreach':
            error_exit("separator without foreach")
            return
        foreachb=stack[len(stack)-1]
        if foreachb.has_separator:
            error_exit("two separators for one foreach")
        foreachb.has_separator=1
        foreachb.prepare_foreach()

        

class item_block:
    pattern=r'^<%\s*item\s*%>'
    basic_pattern = 'item'
    basic_name = 'item'
    type='item'
    def use(self,m):
        global stack
        if not stack or stack[-1].type!='foreach':
            error_exit("item without foreach")
            return
        foreachb=stack[-1]
        if foreachb.has_item:
            error_exit("Two items for one foreach");
        if foreachb.has_separator:
            output( "} // end of separator")
        else:
            foreachb.prepare_foreach()
        foreachb.has_item=1
        stack.append(self)
    def on_end(self):
        output( "} // end of item" )

class empty_block:
    pattern=r'^<%\s*empty\s*%>'
    basic_pattern = 'empty'
    basic_name = 'empty'
    type='empty'
    def use(self,m):
        global stack
        if not stack or stack[-1].type!='foreach':
            error_exit("empty without foreach")
            return
        forb=stack.pop()
        if not forb.has_item:
            error_exit("Unexpected empty - item missed?")
        output( " } else {")
        self.ident=forb.ident
        self.rowid=forb.rowid
        stack.append(self)
    def on_end(self):
        output( "} // end of empty")
        global tmpl_seq
        del tmpl_seq[self.ident]
        if self.rowid:
            del tmpl_seq[self.rowid]


class else_block:
    pattern=r'^<%\s*else\s*%>$'
    basic_pattern = 'else'
    basic_name = 'else'
    type='else'
    def on_end(self):
        output("}")
    def use(self,m):
        prev=stack.pop()
        if prev.type!='if' and prev.type!='elif':
            error_exit("elif without if");
        output( "}else{")
        stack.append(self)

class if_block:
    pattern=r'^<%\s*(if|elif)\s+((not\s+|not\s+empty\s+|empty\s+)?('+variable_match+')|\((.+)\)|)\s*%>$'
    basic_pattern = '(if|elif)'
    basic_name = 'if/elif'
    type='if'
    def prepare(self):
        output( "if(%s) {" % self.ident)

    def on_end(self):
        output( "} // endif")

    def use(self,m):
        global stack
        self.type=m.group(1)
        if m.group(4):
            if m.group(4)=='rtl':
                self.ident='(cppcms::locale::translate("LTR").str(out().getloc())=="RTL")'
            else:
                self.ident=make_ident(m.group(4))
            if m.group(3):
                if re.match('.*empty',m.group(3)):
                    self.ident=self.ident + '.empty()'
                if re.match('not.*',m.group(3)):
                    self.ident="!("+self.ident+")"
        else:
            self.ident=m.group(10)
        if self.type == 'if' :
            self.prepare()
            stack.append(self)
        else: # type == elif
            if stack :
                prev=stack.pop()
                if prev.type!='if' and prev.type!='elif':
                    error_exit("elif without if");
                output( "}")
                output( "else")
                self.prepare()
                stack.append(self)
            else:
                error_exit("Unexpeced elif");
# END ifop                
            

class end_block:
    pattern=r'^<%\s*end(\s+(\w+))?\s*%>$';
    basic_pattern = 'end'
    basic_name = 'end'
    def use(self,m):
        global stack
        if not stack:
            error_exit("Unexpeced 'end'");
        else:
            obj=stack.pop();
            if m.group(1):
                if obj.type!=m.group(2):
                    error_exit("End of %s does not match block %s" % (m.group(2) , obj.type));
            obj.on_end()

class error_com:
    pattern=r'^<%(.*)%>$'
    basic_pattern = ''
    basic_name = ''
    def use(self,m):
        error_exit("Invalid statement `%s'" % m.group(1))


class cpp_include_block:
    pattern=r'^<%\s*c\+\+\s+(.*)%>$'
    basic_pattern = 'c\+\+'
    basic_name = 'c++'
    def use(self,m):
        output( m.group(1));


class base_show:
    mark='('+variable_match+r')\s*(\|(.*))?'
    base_pattern='^\s*'+mark + '$'
    def __init__(self,default_filter='escape'):
        self.default_filter=default_filter
    def get_params(self,s):
        pattern='^\s*(('+variable_match+')|('+str_match+')|(\-?\d+(\.\d*)?))\s*(,(.*))?$'
        res=[]
        m=re.match(pattern,s)
        while m:
            if m.group(2):
                res.append(make_ident(m.group(2)))
            elif m.group(8):
                res.append(m.group(8))
            elif m.group(10):
                res.append(m.group(10))
            if m.group(13):
                s=m.group(13)
                m=re.match(pattern,s)
            else:
                return res
        error_exit("Invalid parameters: `%s'" % s )
        return []
    def prepare(self,s):
        m=re.match(self.base_pattern,s)
        if not m:
            error_exit("No variable")
            return [];
        var=make_ident(m.group(1))
        if not m.group(8):
            return "cppcms::filters::%s(%s)" % (self.default_filter, var)
        filters=m.group(8)
        expr='^\s*(ext\s+)?(\w+)\s*(\((([^"\)]|'+str_match + ')*)\))?\s*(\|(.*))?$'
        m=re.match(expr,filters)
        while m:
            if m.group(1):
                func="content."+m.group(2)
            else:
                func="cppcms::filters::" + m.group(2)
            if m.group(3):
                params=','.join([var]+self.get_params(m.group(4)))
            else:
                params=var
            var=func+"("+params+")"
            if m.group(8):
                filters=m.group(8)
                m=re.match(expr,filters)
            else:
                return var
        error_exit("Seems to be a problem in expression %s" % filters)
        return "";

class form_block:
    pattern=r'^<%\s*form\s+(as_p|as_table|as_ul|as_dl|as_space|input|block|begin|end)\s+('\
         + variable_match +')\s*%>$'
    
    basic_pattern  = 'form'
    basic_name = 'form'
    type = 'form'

    def format_input(self,command_type,ident):
   
        global html_type_code

        flags = 'cppcms::form_flags::' + html_type_code;
        output('{ cppcms::form_context _form_context(out(),%s);' % flags)
        render_command = '    (%s).render_input(_form_context);' % ident;

        if command_type=='begin':
            output('    _form_context.widget_part(cppcms::form_context::first_part);')
            output(render_command)
        elif command_type=='end':
            output('    _form_context.widget_part(cppcms::form_context::second_part);')
            output(render_command)
        else:
            output('    _form_context.widget_part(cppcms::form_context::first_part);')
            output(render_command)
            output('    out() << (%s).attributes_string();' % ident)
            output('    _form_context.widget_part(cppcms::form_context::second_part);')
            output(render_command)
        output('}')
    
    def use(self,m):

        ident=make_ident(m.group(2))
        command_type = m.group(1)
        global html_type_code
        if command_type=='input' or command_type=='begin' or command_type=='end' or command_type=='block':
            if command_type != 'block':
                self.format_input(command_type,ident)
            else:
                self.format_input('begin',ident)
                self.ident = ident
                self.command_type = 'end'
                global stack
                stack.append(self)

        else:
            flags = 'cppcms::form_flags::%s,cppcms::form_flags::%s' % ( html_type_code, m.group(1));
            output('{ cppcms::form_context _form_context(out(),%s); (%s).render(_form_context); }' % (flags , ident))

    def on_end(self):
        self.format_input(self.command_type,self.ident)

class render_block:
    pattern=r'^<%\s*render\s+' \
            + r'((?P<fst_str>'+ str_match +r')|(?P<fst_var>' + variable_match + r'))' \
            + r'(\s*,\s*((?P<snd_str>'+ str_match +r')|(?P<snd_var>' + variable_match + r')))?' \
            + r'(\s+with\s+(?P<content>' + variable_match + r'))?\s*%>$'
    basic_pattern = 'render'
    basic_name = 'render'
    def use(self,m):
        if m.group('content'):
            content = make_ident(m.group('content'))
            guard=True
        else:
            content = 'content';
            guard=False
        
        first_str = m.group('fst_str')
        first_var = m.group('fst_var')
        if first_var:
            first = make_ident(first_var)
        else:
            first = first_str
        
        second_str = m.group('snd_str')
        second_var = m.group('snd_var')
        if second_var:
            second = make_ident(second_var)
        else:
            second = second_str

        if first and second:
            template_name = first
            view_name = second
        else:
            global namespace_name
            template_name = '"' + namespace_name + '"'
            view_name = first;
       
        output('{')

        if guard:
            output(r'cppcms::base_content::app_guard _g(%s,content);' % content)

        output(r'cppcms::views::pool::instance().render(%s,%s,out(),%s);' % (template_name,view_name,content))

        output('}')


class filters_show_block(base_show):
    pattern=r'^<%(=)?\s*('+ variable_match + r'\s*(\|.*)?)%>$'
    basic_pattern = '=?'
    basic_name = 'Inline Variable'
    def use(self,m):
        if not m.group(1):
            warning("Variables syntax like <% foo %> is deprecated, use <%= foo %> syntax");
        expr=self.prepare(m.group(2));
        if expr!="":
            output('out()<<%s;' % expr)

def make_format_params(s,default_filter = 'escape'):
    pattern=r'^(([^,\("]|'+str_match+'|\(([^"\)]|'+str_match+')*\))+)(,(.*))?$'
    params=[]
    m=re.match(pattern,s)
    s_orig=s
    while m.group(1):
        res=base_show(default_filter).prepare(m.group(1))
        if res:
            params.append(res)
        if not m.group(6):
            return params
        s=m.group(7)
        m=re.match(pattern,s)
    error_exit("Seems to be wrong parameters list [%s]" % s_orig)
    return []

class cache_block:
    pattern=r'^<%\s*cache\s+((?P<str>'+ \
            str_match +')|(?P<var>'+ variable_match +r'))' + \
            r'(\s+for\s+(?P<time>\d+))?(\s+on\s+miss\s+(?P<callback>[a-zA-Z]\w*)\(\))?' \
            + r'(?P<notriggers>\s+no\s+triggers)?' \
            + r'(?P<norecording>\s+no\s+recording)?' \
            + '\s*%>$'        
    basic_pattern = 'cache'
    basic_name = 'cache'
    type = 'cache'
    def use(self,m):
        if(m.group('str')):
            self.parameter = m.group('str')
        else:
            self.parameter = make_ident(m.group('var'));
        self.notriggers = m.group('notriggers')
        self.norecording = m.group('norecording')
        output('{ std::string _cppcms_temp_val;')
        output('  if(content.app().cache().fetch_frame(%s,_cppcms_temp_val))' % self.parameter);
        output('      out() << _cppcms_temp_val;');
        output('  else {')
        output('    cppcms::copy_filter _cppcms_cache_flt(out());')
        if not self.norecording:
            output('    cppcms::triggers_recorder _cppcms_trig_rec(content.app().cache());')
        # the code below should be the last one 
        if(m.group('callback')):
            output('    '+make_ident(m.group('callback')+'()') + ';')
        self.timeout = m.group('time');
        global stack
        stack.append(self)
    def on_end(self):
        if self.timeout:
            timeout_time = self.timeout
        else:
            timeout_time = '-1'
        if self.norecording:
            recorded = 'std::set<std::string>()'
        else:
            recorded = '_cppcms_trig_rec.detach()'
        if self.notriggers:
            notriggers='true'
        else:
            notriggers='false'
        output('    content.app().cache().store_frame(%s,_cppcms_cache_flt.detach(),%s,%s,%s);' \
                    % (self.parameter,recorded,timeout_time,notriggers))
        output('}} // cache')

class trigger_block:
    pattern=r'^<%\s*trigger\s+((?P<str>'+  str_match +')|(?P<var>'+ variable_match +r'))' + r'\s*%>$'
    basic_pattern = 'trigger'
    basic_name = 'trigger'
    def use(self,m):
        if(m.group('str')):
            parameter = m.group('str')
        else:
            parameter = make_ident(m.group('var'));
        output('content.app().cache().add_trigger(%s);' % parameter)




class ngettext_block:
    pattern=r'^<%\s*ngt\s*('+str_match+')\s*,\s*('+str_match+')\s*,\s*('+variable_match+')\s*(using(.*))?\s*%>$'
    basic_pattern = 'ngt'
    basic_name = 'ngt'
    def use(self,m):
        s1=m.group(1)
        s2=m.group(3)
        idt=make_ident(m.group(5))
        params=[]
        if m.group(11):
            params=make_format_params(m.group(12))
        if not params:
            output( "out()<<cppcms::locale::translate(%s,%s,%s);" % (s1,s2,idt))
        else:
            output( "out()<<cppcms::locale::format(cppcms::locale::translate(%s,%s,%s)) %% (%s);" % (s1,s2,idt, ') % ('.join(params)))
            

class gettext_block:
    pattern=r'^<%\s*gt\s*('+str_match+')\s*(using(.*))?\s*%>$'
    basic_pattern = 'gt'
    basic_name = 'gt'
    def use(self,m):
        s=m.group(1)
        params=[]
        if m.group(3):
            params=make_format_params(m.group(4))
        if not params:
            output( "out()<<cppcms::locale::translate(%s);" % s)
        else:
            output( "out()<<cppcms::locale::format(cppcms::locale::translate(%s)) %% (%s);" % (s , ') % ('.join(params)))

class url_block:
    pattern=r'^<%\s*url\s*('+str_match+')\s*(using(.*))?\s*%>$'
    basic_pattern = 'url'
    basic_name = 'url'
    def use(self,m):
        s=m.group(1)
        params=[]
        if m.group(3):
            params=make_format_params(m.group(4),'urlencode')
        if not params:
            output( "content.app().mapper().map(out(),%s);" % s)
        else:
            output( "content.app().mapper().map(out(),%s, %s);" % (s , ', '.join(params)))

class csrf_block:
    pattern=r'^<%\s*csrf(\s+(token|cookie|script))?\s*%>$'
    basic_pattern = 'csrf'
    basic_name = 'csrf'
    def use(self,m):
        s=m.group(2)

        global html_type_code

        if html_type_code == 'as_xhtml':
            suffix = '/'
        else:
            suffix =''

        if not s:
            output(r'out() << "<input type=\"hidden\" name=\"_csrf\" value=\"" << content.app().session().get_csrf_token() <<"\" %s>\n";' % suffix)
        elif s == 'token':
            output(r'out() << content.app().session().get_csrf_token();')
        elif s == 'cookie':
            output(r'out() << content.app().session().get_csrf_token_cookie_name();')
        else: # script
            script="""
            <script type='text/javascript'>
            <!--
                {
                    var cppcms_cs = document.cookie.indexOf("$=");
                    if(cppcms_cs != -1) {
                        cppcms_cs += '$='.length;
                        var cppcms_ce = document.cookie.indexOf(";",cppcms_cs);
                        if(cppcms_ce == -1) {
                            cppcms_ce = document.cookie.length;
                        }
                        var cppcms_token = document.cookie.substring(cppcms_cs,cppcms_ce);
                        document.write('<input type="hidden" name="_csrf" value="' + cppcms_token + '" %s>');
                    }
                }
            -->
            </script>
            """ % suffix; 
            script = to_string(script).replace('$','"<< content.app().session().get_csrf_token_cookie_name() <<"')
            output('out() << "' + script +'";')


class include_block:
    basic_pattern = 'include'
    basic_name = 'include'
    pattern=r'^<%\s*include\s+([a-zA_Z]\w*(::\w+)?)\s*\(\s*(.*)\)' \
            + r'(?:\s+' \
            +   r'(from\s+(?P<from>\w+)' \
            +    '|' \
            +   r'using\s+(?P<class>(\w+::)*(\w+))(?:\s+with\s+(?P<content>' + variable_match +r'))?' \
            + r'))?' \
            + r'\s*%>$'

    def print_include(self,call,params):
        output( "%s(%s);" % (call , ','.join(params)))
        
    def use(self,m):
        if m.group(3):
            params=base_show().get_params(m.group(3))
        else:
            params=[]
        call=m.group(1)
        if m.group('from'):
            call = m.group('from') + '.' + call
            self.print_include(call,params)
        elif m.group('class'):
            print_using_block_start(m.group('class'),'_using',m.group('content'))
            self.print_include('_using.' + call,params)
            print_using_block_end()
        else:
            self.print_include(call,params)


def fetch_content(content):
    tmp=''
    for row in re.split('\n',content):
        global line_number
        global file_name
        line_number+=1
        l1=re.split(r'<%([^"%]|"([^"\\]|\\[^"]|\\")*"|%[^>])*%>',row)
        n=0
        for l2 in re.finditer(r'<%([^"%]|"([^"\\]|\\[^"]|\\")*"|%[^>])*%>',row):
            yield tmp+l1[n]
            tmp=''
            yield l2.group(0)
            n+=3
        tmp+=l1[n]+'\n'
    yield tmp

def help():
    print( "Usage cppcms_tmpl_cc [-o filename.cpp] [-s skin] [-d domain] file1.tmpl ... \n" \
        "      -o filename.cpp     file name that implements this template\n" \
        "      -s skin             define skin name\n" \
        "      -d domain           setup gettext domain for this template\n" \
        "      -u                  use unsafe static casting instead of dynamic casing for dlls\n" \
        "      -h/--help           show this help message\n")

def main():
    global stack
    all=[]
    indx=1
    global namespace_name
    global output_file
    global exit_flag
    unsafe_build = False
    while indx < len(os.sys.argv):
        if os.sys.argv[indx]=='-s' or os.sys.argv[indx]=='-n':
            if indx+1>=len(os.sys.argv):
                sys.stderr.write("-s (-n) should be followed by skin name\n")
                help()
                exit_flag=1
                return
            else:
                namespace_name=os.sys.argv[indx+1];
                indx+=1
        elif os.sys.argv[indx]=='-o':
            if indx+1>=len(os.sys.argv):
                sys.stderr.write("-o should be followed by output file name\n")
                help()
                exit_flag=1
                return
            else:
                output_file=os.sys.argv[indx+1]
                indx+=1
        elif os.sys.argv[indx]=='-d':
            if indx+1>=len(os.sys.argv):
                sys.stderr.write("-d should followed by gettext domain name\n")
                help()
                exit_flag=1
                return
            else:
                global spec_gettext
                spec_gettext=os.sys.argv[indx+1]
                indx+=1
        elif os.sys.argv[indx]=='-u':
            unsafe_build = True
        elif os.sys.argv[indx]=='--help' or os.sys.argv[indx]=='-h':
            help()
            exit_flag=1
            return
        else:
            all.append(os.sys.argv[indx])
        indx+=1
    if not all:
        sys.stderr.write("No input file names given\n")
        help()
        exit_flag=1
        return
    global output_fd
    if output_file!='':
        output_fd=open(output_file,"w")
    for file in all:
        global file_name
        global line_number
        line_number=0
        file_name=file
        f=open(file,'r')
        content=f.read()
        f.close()
        for x in fetch_content(content):
            if x=='' : continue
            if len(stack)==0:
                if re.match(r"^\s*$",x):
                    continue
                elif not re.match(r"<\%.*\%>",x):
                    error_exit("Content is not allowed outside template blocks")
                    continue
            matched=0
            for c in [\
                    skin_block(), \
                    view_block(), \
                    template_block(), \
                    end_block(), \
                    if_block(), \
                    else_block(), \
                    cpp_include_block(), \
                    gettext_block(),ngettext_block(), \
                    url_block(), \
                    foreach_block(), item_block(), empty_block(),separator_block(), \
                    include_block(), \
                    cache_block(), \
                    trigger_block(), \
                    using_block(), \
                    render_block(), \
                    html_type(), form_block(), csrf_block(), \
                    filters_show_block(), error_com() ]:

                basic_pattern = r'^<%\s*' + c.basic_pattern + r'.*%>$'
                if re.match(basic_pattern,x):
                    m = re.match(c.pattern,x)
                    if m:
                        c.use(m)
                    else:
                        error_exit('Syntax error in command %s : %s' % ( c.basic_name , x))
                    matched=1
                    break;
            if not matched:
                inline_content(x)


        if stack:
            error_exit("Unexpected end of file %s" % file)
    global class_list
    if class_list and exit_flag==0:
        write_class_loader(unsafe_build)

#######################
# MAIN
#######################


html_type_code='as_html'
output_file=''
output_fd=sys.stdout
namespace_name=''
file_name=''
labels_counter=0
tmpl_seq={}
template_parameters={}
templates_map={}
parameters_counter=2
stack=[]
class_list=[]
exit_flag=0
current_template=''
spec_gettext=''
ignore_inline=1

################
main()
################

if output_fd!=sys.stderr:
    output_fd.close()

if exit_flag!=0 and output_file!='':
    try:
        os.unlink(output_file)
    except:
        pass

sys.exit(exit_flag)


# vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
