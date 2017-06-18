<?php
if(version_compare(PHP_VERSION, '7.0.0') >= 0)
	include_once('../php7/cppcms.php');
else
	include_once('../php5/cppcms.php');

$pool=CppCMS_SessionPool::from_config('../../reference/config.js');
$session=$pool->session();
$session->load();

$output="";

for($i=1;;$i++) {
    $id = "_" . $i;
    if(!isset($_GET["op" . $id]))
        break;
    $op  = $_GET["op" . $id];
    $key = isset($_GET["key" . $id]) ? $_GET["key" . $id] : "";
    $value = isset($_GET["value" . $id]) ? $_GET["value" . $id] : "";
    $result = "ok";
    if($op=="is_set") {
        $result = isset($session[$key]) ? "yes" : "no";
    }
    elseif($op == "erase")  {
        unset($session[$key]);
    }
    elseif($op == "clear") {
        $session->clear();
    }
    elseif($op == "is_exposed") {
        $result = $session->get_exposed($key) ? "yes" : "no";
    }
    elseif($op == "expose") {
        $session->set_exposed($key,intval($value));
    }
    elseif($op == "get") {
        $result = $session[$key];
    }
    elseif($op == "set") {
        $session[$key]=$value;
    }
    elseif($op == "get_binary") {
        $result = bin2hex($session->get($key));
    }
    elseif($op == "set_binary") {
        $session->set($key,hex2bin($value));
    }
    elseif($op == "get_age") {
        $result = strval($session->get_age());
    }
    elseif($op == "set_age") {
        $session->set_age(intval($value));;
    }
    elseif($op == "default_age") {
        $session->set_default_age();
    }
    elseif($op == "get_expiration") {
        $result = strval($session->get_expiration());
    }
    elseif($op == "set_expiration") {
        $session->set_expiration(intval($value));
    }
    elseif($op == "default_expiration") {
        $session->set_default_expiration();
    }
    elseif($op == "get_on_server") {
        $result = $session->get_on_server() ? "yes" : "no";
    }
    elseif($op == "set_on_server") {
        $session->set_on_server(intval($value));
    }
    elseif($op == "reset_session") {
        $session->reset_session();
    }
    elseif($op == "csrf_token") {
        $result = "t=" . $session->get_csrf_token();
    }
    elseif($op == "keys") {
        $result = "";
        $keys = $session->keys();
        foreach($keys as $p) {
            if($result != "")
                $result = $result . ",";
            $result =$result .  "[" . $p . "]";
        }
    }
    else {
        $result = "invalid op=" . $op;
    }
    $output = $output . strval($i) . ":" . $result . ";";
}

$session->save();
header("Content-Type: text/plain");

echo $output;
