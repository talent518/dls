<?php

include dirname(__FILE__) . '/LockexHelper.php';

class Controller {
	function ajaxMessage($message) {
		exit($message);
	}
}

class Yii {
	private static $_app;
	public static function app() {
		if(self::$_app === null) {
			self::$_app = new stdClass();
			self::$_app->controller = new Controller();
			self::$_app->params = array(
				'dlsHost' => '127.0.0.1',
			);
		}
		
		return self::$_app;
	}
}

echo PHP_EOL, '=== +test1 ===', PHP_EOL;
LockexHelper::lock('test1');
sleep(1);
echo PHP_EOL, '=== +test2 ===', PHP_EOL;
LockexHelper::lock('test2');
sleep(1);

echo PHP_EOL, '=== *query ===', PHP_EOL;
LockexHelper::query();
sleep(1);

echo PHP_EOL, '=== -test2 ===', PHP_EOL;
LockexHelper::unlock('test2');
sleep(1);
echo PHP_EOL, '=== -test1 ===', PHP_EOL;
LockexHelper::unlock('test1');
sleep(1);

echo PHP_EOL, '=== +test3 ===', PHP_EOL;
LockexHelper::lock('test3');
sleep(1);
echo PHP_EOL, '=== +test4 ===', PHP_EOL;
LockexHelper::lock('test4');
sleep(5);

echo PHP_EOL, '=== *query ===', PHP_EOL;
LockexHelper::query();
sleep(10);
echo PHP_EOL, '=== *query ===', PHP_EOL;
LockexHelper::query();
sleep(20);
echo PHP_EOL, '=== *query ===', PHP_EOL;
LockexHelper::query();
sleep(30);
echo PHP_EOL, '=== *query ===', PHP_EOL;
LockexHelper::query();
sleep(40);
echo PHP_EOL, '=== *query ===', PHP_EOL;
LockexHelper::query();

