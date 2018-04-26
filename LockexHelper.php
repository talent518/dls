<?php

abstract class LockexHelper {

	private static $sockfd;

	private static $keys = array();

	public static function lock ( $key = 'defaultLock' ) {
		if ( self::$sockfd === null ) {
			self::$sockfd = @socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
			if ( self::$sockfd === false ) {
				self::errorMessage('创建socket管道失败！' . socket_strerror(socket_last_error()));
			}
			
			$address = ( isset(Yii::app()->params['dlsHost']) ? Yii::app()->params['dlsHost'] : include ( Yii::getPathOfAlias('application.config.dbhost') . '.php' ) );
			
			@socket_connect(self::$sockfd, $address, 1112) or self::errorMessage('连接到锁服务器失败！错误为：' . socket_strerror(socket_last_error(self::$sockfd)));
			
			register_shutdown_function(__CLASS__ . '::close');
		}
		
		if ( isset(self::$keys[$key]) ) {
			self::$keys[$key] ++ ;
			$reads = array(
				self::$sockfd
			);
			if(socket_select($reads, $writes, $expects, 0, 100)>0) {
				echo @socket_read(self::$sockfd, 1024), PHP_EOL;
			}
		} else {
			self::$keys[$key] = 1;
			
			$ret = @socket_write(self::$sockfd, '+' . $key . chr(0));
			
			if ( $ret === false ) {
				self::errorMessage('加锁失败！错误为：' . socket_strerror(socket_last_error(self::$sockfd)));
			}
			
			do {
				echo $ret = @socket_read(self::$sockfd, 1), PHP_EOL;
			} while ( $ret !== false && $ret !== '' && ! $ret );
			
			if ( $ret === false ) {
				self::errorMessage('加锁失败！错误为：' . socket_strerror(socket_last_error(self::$sockfd)));
			}
		}
	}

	public static function unlock ( $key = 'defaultLock' ) {
		if ( isset(self::$keys[$key]) ) {
			self::$keys[$key] -- ;
			if ( self::$keys[$key] === 0 ) {
				$ret = @socket_write(self::$sockfd, '-' . $key . chr(0));
				
				if ( $ret === false ) {
					self::errorMessage('解锁失败！错误为：' . socket_strerror(socket_last_error(self::$sockfd)));
				}
				
				echo $ret = @socket_read(self::$sockfd, 1), PHP_EOL;
				
				if ( $ret === false ) {
					self::errorMessage('解锁失败！错误为：' . socket_strerror(socket_last_error(self::$sockfd)));
				}
				
				unset(self::$keys[$key]);
			}
		}
	}

	public static function query () {
		if (  ! empty(self::$keys) ) {
			$ret = @socket_write(self::$sockfd, '*' . chr(0));
			
			if ( $ret === false ) {
				self::errorMessage('查询锁失败！错误为：' . socket_strerror(socket_last_error(self::$sockfd)));
			}
			
			do {
				echo $ret = @socket_read(self::$sockfd, 4096), PHP_EOL;
			} while($ret !== false && $ret !== '' && substr($ret,-1) !== "\n");
		}
	}

	public static function close () {
		@socket_shutdown(self::$sockfd, 2);
		@socket_close(self::$sockfd);
		
		self::$sockfd = null;
		self::$keys = array();
	}

	private static function errorMessage ( $message ) {
		if ( class_exists('Controller', false) ) {
			Yii::app()->controller->ajaxMessage($message, false);
		} else {
			exit($message);
		}
	}

}
