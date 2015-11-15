<?php
include 'cppcms_api.php';

class CppCMS_SessioBase {
	protected $api = null;
	protected function check()
	{
		$code = $this->api->error();
		if($code == 0)
			return;
		$msg = $this->api->error_clear();
		switch($code) {
		case cppcms_api::ERROR_INVALID_ARGUMENT;
			throw new InvalidArgumentException($msg);
		case cppcms_api::ERROR_LOGIC;
			throw new LogicException($msg);
		default:
			throw new RuntimeException($msg);
		}
	}
};

class CppCMS_SessionPool extends CppCMS_SessioBase {
	private function __construct()
	{
		$this->api = new CppCMSAPIPool;
	}
	public static function from_config($path)
	{
		$pool=new CppCMS_SessionPool();
		$pool->api->init($path);
		$pool->check();
		return $pool;
	}
	public static function from_json($json)
	{
		$pool=new CppCMS_SessionPool();
		$pool->api->init_from_json($json);
		$pool->check();
		return $pool;
	}
	public function session()
	{
		return new CppCMS_Session($this);
	}
}

class CppCMS_Cookie {
	private $c=null;
	public function __construct($c)
	{
		$this->c=$c;
	}

	function header() { return $this->c->header(); }
	function header_content() { return $this->c->header_content(); }
	function name() { return $this->c->name(); }
	function value() { return $this->c->value(); }
	function path() { return $this->c->path(); }
	function domain() { return $this->c->domain(); }
	function max_age_defined() { return $this->c->max_age_defined() != 0; }
	function max_age() { return $this->c->max_age(); }
	function expires_defined() { return $this->c->expires_defined() != 0; }
	function expires() { return $this->c->expires(); }
	function is_secure() { return $this->c->is_secure(); }
	function __toString() { return $this->header(); }
}


class CppCMS_Session extends CppCMS_SessioBase implements ArrayAccess {

	public function __construct($pool)
	{
		$this->api = new CppCMSAPISession;
		$this->api->init($pool->api);
		$this->check();
	}
	function clear() { $this->api->clear(); $this->check(); }

	function is_set($key) { $r = $this->api->is_set($key)!=0; $this->check(); return $r; }
	function erase($key) { $this->api->erase($key); $this->check(); }
	function set($key,$value) { 
		$this->api->set_binary_as_hex($key,bin2hex($value)); 
		$this->check(); 
	}
	function get($key) { 
		$r=$this->api->get_binary_as_hex($key); 
		$this->check(); 
		if($r==null)
			return null;
		return hex2bin($r); 
	}

	/// ArrayAccess API
	function offsetExists($key) { return $this->is_set($key); }
	function offsetUnset($key) { $this->erase($key); }
	function offsetGet($key) { return $this->get($key); }
	function offsetSet($key,$value) { $this->set($key,$value); }

	function get_exposed($key) { $r=$this->api->get_exposed($key)!=0; $this->check(); return $r; }
	function set_exposed($key,$is_exposed) { $this->api->set_exposed($key,$is_exposed?1:0 ); $this->check(); }

	function keys() {
		$r=array();
		$k=$this->api->get_first_key();
		while($k!=null) {
			$r[]=$k;
			$k=$this->api->get_next_key();
		}
		$this->check();
		return $r;
	}

	function get_csrf_token() { $r=$this->api->get_csrf_token(); $this->check(); return $r; }

	function reset_session() { $this->api->reset_session(); $this->check(); }
	function set_default_age() { $this->api->set_default_age(); $this->check(); }
	function set_age($t) { $this->api->set_age($t); $this->check(); }
	function get_age() { $r=$this->api->get_age(); $this->check(); return $r; }
	function set_default_expiration() { $this->api->set_default_expiration(); $this->check(); }
	function set_expiration($t) { $this->api->set_expiration($t); $this->check(); }
	function get_expiration() { $r = $this->api->get_expiration(); $this->check(); return $r; }
	function set_on_server($is_on_server) { $this->api->set_on_server($is_on_server ? 1 : 0); $this->check(); }
	function get_on_server() { $r = $this->api->get_on_server(); $this->check(); return $r!=0; }
	function get_session_cookie_name() { $r=$this->api->get_session_cookie_name(); $this->check(); return $r; }

	function load($session_cookie_value = null) {
		if($session_cookie_value == null) {
			$name = $this->get_session_cookie_name();
			if(isset($_COOKIE[$name])) {
				$session_cookie_value = $_COOKIE[$name];
			}
		}
		if($session_cookie_value == null) 
			$session_cookie_value = "";
		$this->api->load($session_cookie_value);
		$this->check();
	}

	function cookies() {
		$r=array();
		$c=$this->api->cookie_first();
		$this->check();
		while($c!=null) {
			$r[]=new CppCMS_Cookie($c);
			$c=$this->api->cookie_next();
			$this->check();
		}
		return $r;
	}

	function save($set_headers = true) {
		$this->api->save();
		$this->check();
		if($set_headers) {
			$c=$this->api->cookie_first();
			$this->check();
			while($c!=null) {
				header($c->header(),false);
				$c=$this->api->cookie_next();
				$this->check();
			}
		}
	}

}


// Code Test
if (!count(debug_backtrace()))
{
	$pool = CppCMS_SessionPool::from_config('cppcms-config.js');
	$session = $pool->session();
	$session->load();
	$session['x']='yes';
	$session['y']=13.4;
	$session->set_exposed('y',true);
	$session->save(false);
	$cookies = $session->cookies();
	$state='';
	foreach($cookies as $c) {
		echo $c,"\n";
		if($c->name() == $session->get_session_cookie_name())
			$state = $c->value();
	}
	$session = $pool->session();
	$session->load($state);
	$session->set_exposed('y',false);
	echo implode(', ',$session->keys()),"\n";
	$session->save(false);
	$cookies = $session->cookies();
	foreach($cookies as $c) {
		echo $c,"\n";
		if($c->name() == $session->get_session_cookie_name())
			$state = $c->value();
	}
}
