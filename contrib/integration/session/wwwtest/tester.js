function report(id,st,msg)
{
	var okfail = st ? '<span style="color:#00AA00">OK</span>' : '<span style="color:red">FAIL</span>';
	document.getElementById('log').innerHTML += '<tr><td>' + id + '</td><td>' + okfail + '</td><td>' + msg + '</td></tr>';
}

function get_cookie(name){
    var pattern = RegExp(name + "=.[^;]*")
    matched = document.cookie.match(pattern)
    if(matched){
        var cookie = matched[0].split('=')
        return cookie[1]
    }
    return '';
}

function reset_report()
{
	document.getElementById('log').innerHTML   = '';
}
function wait_please(how)
{
	document.getElementById('wait').innerHTML = 'Please wait - manual delay of ' + how + ' seconds in progress';
}
function wait_done()
{
	document.getElementById('wait').innerHTML = ''
}
function run_test(baseurl,test,next_test)
{
	wait_done();
	var url = baseurl + '?';
	for(var i=0;i<test.request.length;i++) {
		if(i!=0)
			url = url + '&';
		var st = test.request[i];
		var id = i+1;
		var tmp = 'op_' + id + '=' + st.op;
		if(typeof st.key != 'undefined')
			tmp = tmp + '&key_' + id + '=' + st.key;
		if(typeof st.value != 'undefined')
			tmp = tmp + '&value_' + id + '=' + st.value;
		url = url + tmp;
	}
	var xhr = new  XMLHttpRequest();
	//console.log(url);
	xhr.open('GET',url,true);
	xhr.onreadystatechange = function(e) {
		if(xhr.readyState == 4) {
			var text = xhr.responseText;
			if(test.response instanceof RegExp ? text.match(test.response) : text == test.response) {
				if(test.callback == undefined)
					report(test.id,true,'');
				else {
					var res = test.callback();
					if(res == null)
						report(test.id,true,'');
					else
						report(test.id,false,res);
				}
			}
			else {
				total_tests_failed ++;
				report(test.id,false, text + ' != ' + test.response);
			}
			var next = function() {
				next_test(baseurl);
			};
			if(test.delay == undefined)
				next();
			else {
				wait_please(test.delay);
				setTimeout(next,test.delay * 1000);
			}
		}
	};
	xhr.send();
}


function run_test_set(url,set,id)
{
	if(id == 0) {
		total_tests_failed  = 0;
	}
	if(id >= set.length) {
		report('SUMMARY',total_tests_failed == 0,'Failed tests: ' + total_tests_failed);
		return;
	}
	run_test(url,set[id],function(url) {
		run_test_set(url,set,id+1);
	});
}

function tester(url)
{
	reset_report();	
	var all_tests =  [
		{
			id:'init',
			request: [{op:'clear'} ],
			response : '1:ok;'
		},	
		{
			id:'set',
			request: [{op:'is_set',key:'x'},{op:'set',key:'x',value:'val'} ],
			response : '1:no;2:ok;'
		},	
		{
			id:'get',
			request: [{op:'is_set',key:'x'},{op:'get',key:'x'}],
			response : '1:yes;2:val;'
		},
		{
			id:'erase',
			request:[{op:'erase',key:'x'}],
			response : '1:ok;'
		},
		{
			id:'is_get false',
			request: [{op:'is_set',key:'x'}],
			response : '1:no;'
		},
		{
			id:'set binary',
			request: [{op:'set',key:'a',value:'test'},{op:'set_binary',key:'b',value:'54455354'},{op:'set_binary',key:'c',value:'0001'} ],
			response : '1:ok;2:ok;3:ok;'
		},
		{
			id:'get binary',
			request: [{op:'get_binary',key:'a'},{op:'get',key:'b'},{op:'get_binary',key:'c'} ],
			response : '1:74657374;2:TEST;3:0001;'
		},
		{
			id:'expire set',
			request:[{op:'get_age'},{op:'set_age',value:20},{op:'get_age'},{op:'default_age'},{op:'get_age'},{op:'set_age',value:'3'},{op:'get',key:'a'},{op:'get_age'}],
			response: "1:604800;2:ok;3:20;4:ok;5:604800;6:ok;7:test;8:3;",
			delay: 5,
		},
		{
			id:'expired',
			request:[{op:'get_age'},{op:'is_set',key:'a'}],
			response: "1:604800;2:no;",
		},
		{
			id:'expiration',
			request:[{op:'set_expiration',value:'0'},{op:'get_expiration'},{op:'default_expiration'},{op:'get_expiration'},{op:'set_expiration',value:'0'},{op:'set_age',value:'8'},{op:'set',key:'x',value:'test'}],
			response:'1:ok;2:0;3:ok;4:1;5:ok;6:ok;7:ok;',
			delay:6,
		},
		{
			id:'expiration a',
			request:[{op:'get_expiration'},{op:'get',key:'x'}],
			response:'1:0;2:test;',
			delay:5,
		},
		{
			id:'expiration b',
			request:[{op:'get_expiration'},{op:'is_set',key:'x'}],
			response:'1:1;2:no;',
            callback:function() {
                document.cookie ='cppcms_session_foo=test; path=/'
                return null;
            }
		},
		{
			id:'expose',
			request:[{op:'set',value:'test',key:'x'},{op:'expose','value':1,'key':'x'},{op:'is_exposed',key:'x'},
				 {op:'set',value:'ttt',key:'y'},{op:'is_exposed',key:'y'}],
			response:'1:ok;2:ok;3:yes;4:ok;5:no;',
			callback:function() {
				if(get_cookie('cppcms_session_foo')!='')
					return 'invalid foo cookie';
				if(get_cookie('cppcms_session_x')!='test')
					return 'invalid x cookie';
				if(get_cookie('cppcms_session_y')!='')
					return 'invalid_y cookie';
				return null;
			}
		},
		{
			id:'exposed 2',
			request: [{op:'is_exposed',key:'x'},{op:'expose',key:'x',value:'0'}],
			response: '1:yes;2:ok;',
			callback: function() {
				if(get_cookie('cppcms_session_x')!='')
					return 'x exists';
				return null;
			},
		},
		{
			id:'client server',
			request:[{op:'get_on_server'},{op:'set_on_server','value':1},{op:'get_on_server'}],
			response:'1:no;2:ok;3:yes;',
			callback: function() {
				var sid = get_cookie('cppcms_session');
				if(sid[0]!='I')
					return 'not ID session ' + sid;
				global_cppcms_session_id = sid;
				return null;
			},
		},
		{
			id:'reset',
			request:[{op:'get_on_server'},{op:'reset_session'}],
			response:'1:yes;2:ok;',
			callback: function() {
				var sid = get_cookie('cppcms_session');
				if(sid[0]!='I')
					return 'not ID session ' + sid;
				if(typeof global_cppcms_session_id === 'undefined')
					return null;
				if(sid === global_cppcms_session_id)
					return 'session id ' + global_cppcms_session_id + ' had not change';
				return null;
			},
		},
		{
			id:'back client',
			request:[{op:'set_on_server',value:'0'}],
			response:'1:ok;',
			callback: function() {
				var sid = get_cookie('cppcms_session');
				if(sid[0]!='C')
					return 'not cliend session ' + sid;
				return null;
			}
		},
		{
			id:'keys',
			request:[{op:'keys'}],
			response:'1:[x],[y];'
		},
		{
			id:'csrf',
			request:[{op:'csrf_token'}],
			response: /^1:t=.+;$/,
		},
		///////////////////////////////////////////////////////
		{
			id:'last',
			request: [{op:'set',key:'x',value:'val'} ],
			response : '1:ok;'
		}

	];
	run_test_set(url,all_tests,0);
}

