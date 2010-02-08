#!/usr/bin/env python
# coding=utf-8
import httplib
import sys

def test(name,A,B):
    if A != B:
        print "Error :" + name 
        print "-----Actual--"
        print A,"---Expected--"
        print B,"-------------"
        sys.exit(1)
    else:
        print "Ok:"+name



h=httplib.HTTPConnection('localhost:8080');
h.request('GET','/test')
r=h.getresponse()
body=r.read();
ref_body = \
"""\
non loaded<br>
<form action="/test" method="post" >
<p>text&nbsp;<span class="cppcms_form_input"><input type="text" name="_1"  ></span></p>
<p>textarea&nbsp;<span class="cppcms_form_input"><textarea name="_2" ></textarea></span></p>
<p>int&nbsp;<span class="cppcms_form_input"><input type="text" name="_3" value=""  ></span></p>
<p>double&nbsp;<span class="cppcms_form_input"><input type="text" name="_4" value=""  ></span></p>
<p>pass&nbsp;<span class="cppcms_form_input"><input type="password" name="_5"  ></span></p>
<p>pass2&nbsp;<span class="cppcms_form_input"><input type="password" name="_6"  ></span></p>
<p>yes or not&nbsp;<span class="cppcms_form_input"><input type="text" name="_7"  ></span></p>
<p>E-Mail&nbsp;<span class="cppcms_form_input"><input type="text" name="_8"  ></span></p>
<p>Checkbox&nbsp;<span class="cppcms_form_input"><input type="checkbox" name="_9" value="y"  ></span></p>
<p>Select Multiple&nbsp;<span class="cppcms_form_input"><select multiple name="_10"  >
<option value="0" selected >a</option>
<option value="1" selected >b</option>
<option value="2" >c</option>
<option value="id1" >tr1</option>
</select></span></p>
<p>Select&nbsp;<span class="cppcms_form_input"><select name="_11"  >
<option value="0" >a</option>
<option value="1" >b</option>
<option value="2" >c</option>
<option value="id2" selected >tr2</option>
</select></span></p>
<p>Radio&nbsp;<span class="cppcms_form_input"><div class="cppcms_radio"  >
<input type="radio" value="0" name="_12" checked > x<br>
<input type="radio" value="1" name="_12" > y<br>
<input type="radio" value="id3" name="_12" > tr3<br>
</div></span></p>
<p>Submit&nbsp;<span class="cppcms_form_input"><input type="submit" name="_13" value="Button"  ></span></p>
</form>
"""

test("/test",body,ref_body)

def test_valid(name,params,ans,url='/non_empty'):
    h=httplib.HTTPConnection('localhost:8080');
    h.request('GET','/test' + url + '?' + params)
    r=h.getresponse()
    test(name,r.read()[:len(ans)],ans)

test_valid('non_empty1','_1=1&_2=1&_3=1&_4=1&_5=1&_6=1&_7=yes&_8=a@a&_9=10&_10=1&_11=1&_12=1&_13=1','valid')
test_valid('non_empty2','_1=&_2=1&_3=1&_4=1&_5=1&_6=1&_7=yes&_8=a@a&_9=10&_10=1&_11=1&_12=1&_13=1','invalid')
test_valid('non_empty3','_1=1&_2=&_3=1&_4=1&_5=1&_6=1&_7=yes&_8=a@a&_9=10&_10=1&_11=1&_12=1&_13=1','invalid')
test_valid('non_empty4','_1=1&_2=1&_3=&_4=1&_5=1&_6=1&_7=yes&_8=a@a&_9=10&_10=1&_11=1&_12=1&_13=1','invalid')
test_valid('non_empty5','_1=1&_2=1&_3=1&_4=1&_5=&_6=1&_7=yes&_8=a@a&_9=10&_10=1&_11=1&_12=1&_13=1','invalid')
test_valid('non_empty6','_1=1&_2=1&_3=1&_4=1&_5=1&_6=&_7=yes&_8=a@a&_9=10&_10=1&_11=1&_12=1&_13=1','invalid')
test_valid('non_empty7','_1=1&_2=1&_3=1&_4=1&_5=1&_6=1&_7=&_8=a@a&_9=10&_10=1&_11=1&_12=1&_13=1','invalid')
test_valid('non_empty8','_1=1&_2=1&_3=1&_4=1&_5=1&_6=1&_7=yes&_8=&_9=10&_10=1&_11=1&_12=1&_13=1','invalid')
test_valid('non_empty9','_1=1&_2=1&_3=1&_4=1&_5=1&_6=1&_7=yes&_8=a@a&_9=&_10=1&_11=1&_12=1&_13=1','valid') # checkbox ok
test_valid('non_empty10','_1=1&_2=1&_3=1&_4=1&_5=1&_6=1&_7=yes&_8=a@a&_9=10&_10=&_11=1&_12=1&_13=1','invalid')
test_valid('non_empty11','_1=1&_2=1&_3=1&_4=1&_5=1&_6=1&_7=yes&_8=a@a&_9=10&_10=1&_11=&_12=1&_13=1','invalid')
test_valid('non_empty12','_1=1&_2=1&_3=1&_4=1&_5=1&_6=1&_7=yes&_8=a@a&_9=10&_10=1&_11=1&_12=&_13=1','invalid')
test_valid('non_empty12','_1=1&_2=1&_3=1&_4=1&_5=1&_6=1&_7=yes&_8=a@a&_9=10&_10=1&_11=1&_12=1&_13=','valid') # Submit ok
test_valid('empty','_1=&_2=&_3=&_4=&_5=&_6=&_7=yes&_8=a@a&_9=&_10=&_11=&_12=&_13=','valid','') # Empty ok only regex, email fails
test_valid('empty1','_1=&_2=&_3=&_4=&_5=&_6=&_7=yes&_8=&_9=&_10=&_11=&_12=&_13=','invalid','') # Empty ok only regex, email fails
test_valid('empty2','_1=&_2=&_3=&_4=&_5=&_6=&_7=&_8=a@a&_9=&_10=&_11=&_12=&_13=','invalid','') # Empty ok only regex, email fails


h=httplib.HTTPConnection('localhost:8080');
h.request('GET','/test/sub')
r=h.getresponse()
body=r.read();
ref_body = \
"""\
<p>pass&nbsp;<span class="cppcms_form_input"><input type="password" name="_5"  ></span></p>
<p>pass2&nbsp;<span class="cppcms_form_input"><input type="password" name="_6"  ></span></p>
<p>yes or not&nbsp;<span class="cppcms_form_input"><input type="text" name="_7"  ></span></p>
<p>E-Mail&nbsp;<span class="cppcms_form_input"><input type="text" name="_8"  ></span></p>
<p>Checkbox&nbsp;<span class="cppcms_form_input"><input type="checkbox" name="_9" value="y"  ></span></p>
<p>Select Multiple&nbsp;<span class="cppcms_form_input"><select multiple name="_10"  >
<option value="0" selected >a</option>
<option value="1" selected >b</option>
<option value="2" >c</option>
<option value="id1" >tr1</option>
</select></span></p>
"""
test("subset",body,ref_body)

def test_valid(name,url,params,ans):
    def get():
        h=httplib.HTTPConnection('localhost:8080');
        h.request('GET','/test' + url + '?' + params)
        r=h.getresponse()
        test(name+' GET',r.read(),ans)
    def post():
        h=httplib.HTTPConnection('localhost:8080');
        headers = {"Content-type": "application/x-www-form-urlencoded"}
        h.request('POST','/test' + url,params,headers)
        r=h.getresponse()
        test(name+' POST',r.read(),ans)
    get()
    post()


test_valid('text','/text','_1=','invalid\n')
test_valid('text1','/text','_1=x','invalid\nx')
test_valid('text2','/text','_1=xx','valid\nxx')
test_valid('text3','/text','_1=xxxxx','valid\nxxxxx')
test_valid('text4','/text','_1=xxxxxx','invalid\nxxxxxx')
test_valid('text5','/text','_1=%d7%a9%d6%b8%d7%9c%d7%95%d7%9d','valid\nשָלום')
test_valid('text6','/text','_1=%d7%a9%d7%9c','valid\nשל')
test_valid('text7','/text','_1=%FF%FF','invalid\n\xFF\xFF')
test_valid('text8','/text','_1=%01%01','invalid\n\x01\x01')

test_valid('number','/number','_1=','invalid\n')
test_valid('number1','/number','_1=10','valid\n10')
test_valid('number2','/number','_1=10.0','valid\n10')
test_valid('number3','/number','_1=10.0e+','invalid\n')
test_valid('number5','/number','_1=10.0e1','valid\n100')
test_valid('number6','/number','_1=10.0x','invalid\n')
test_valid('number7','/number','_1=A10.0','invalid\n')
test_valid('number8','/number','_1=0','invalid\n0')
test_valid('number9','/number','_1=1000','invalid\n1000')
test_valid('number10','/number','_1=10A','invalid\n')


test_valid('pass1','/pass','_1=&_2=','invalid\n')
test_valid('pass2','/pass','_1=x&_2=x','valid\n')
test_valid('pass3','/pass','_1=x1&_2=x2','invalid\n')

test_valid('checkbox1','/checkbox','_1=n','valid\n0')
test_valid('checkbox2','/checkbox','_1=y','valid\n1')

test_valid('sm1','/sm','foo=bar','invalid\n0 0 0 0 \n\n')
test_valid('sm2','/sm','_1=1&_1=0','valid\n1 1 0 0 \n0 1 \n')
test_valid('sm3','/sm','_1=1&_1=id1','valid\n0 1 0 1 \n1 id1 \n')
test_valid('sm4','/sm','_1=0&_1=1&_1=2','invalid\n1 1 1 0 \n0 1 2 \n')


test_valid('select1','/select','foo=bar','invalid\n-1 ')
test_valid('select2','/select','_1=0','valid\n0 0')
test_valid('select3','/select','_1=0&_1=1','invalid\n-1 ')
test_valid('select4','/select','_1=10','invalid\n-1 ')

test_valid('radio1','/radio','foo=bar','invalid\n-1 ')
test_valid('radio2','/radio','_1=0','valid\n0 0')
test_valid('radio3','/radio','_1=0&_1=1','invalid\n-1 ')
test_valid('radio4','/radio','_1=10','invalid\n-1 ')


test_valid('submit1','/submit','_1=1','valid\n1')
test_valid('submit2','/submit','_2=1','valid\n0')
body='<p><label for="submit_id">message:</label> <span class="cppcms_form_error">error</span> <span class="cppcms_form_input"><input type="submit" id="submit_id" name="submit_name" value="test"  ></span><span class="cppcms_form_help">help</span></p>\n'
test_valid('submit3','/submitl','',body)

