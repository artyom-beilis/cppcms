<?php
include_once('cppcms.php');

$pool=CppCMS_SessionPool::from_config('cppcms-config.js');
$session=$pool->session();
$session->load();
$x=0;
if($session->is_set('x')) {
	$x=$session['x'];
}

$x=intval($x)+1;
$session['x']=$x;
$session->save();

echo "x=$x\n";
