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

LockexHelper::lock();
sleep(1);
LockexHelper::unlock(); // 此行可有可无

