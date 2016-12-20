<?php

abstract class LockexHelper {

	private static $sockfd;

	public static function lock() {
		if (is_resource(self::$sockfd)) {
			$reads = array(
				self::$sockfd 
			);
			socket_select($reads, $writes, $expects, 0, 100) and @socket_read(self::$sockfd, 1024);
			return;
		}
		
		self::$sockfd = @socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
		if (self::$sockfd === false) {
			self::errorMessage('创建socket管道失败！' . socket_last_error());
		}
		
		$address = (isset(Yii::app()->params['dlsHost']) ? Yii::app()->params['dlsHost'] : include (Yii::getPathOfAlias('application.config.dbhost') . '.php'));
		
		@socket_connect(self::$sockfd, $address, 1111) or self::errorMessage('连接到锁服务器失败！错误为：' . socket_last_error(self::$sockfd));
		
		do {
			$ret = @socket_read(self::$sockfd, 1);
		} while ( $ret !== false && ! $ret );
		
		if ($ret === false) {
			self::errorMessage('获得锁失败！错误为：' . socket_last_error(self::$sockfd));
		}
		
		register_shutdown_function(__CLASS__ . '::unlock');
	}

	public static function unlock() {
		@socket_shutdown(self::$sockfd, 2);
		@socket_close(self::$sockfd);
	}

	private static function errorMessage($message) {
		if (class_exists('Controller', false)) {
			Yii::app()->controller->ajaxMessage($message, false);
		} else {
			exit($message);
		}
	}

}
