<?php
/**
 * Maurina connector for PHP.
 *
 * @version 1.5
 * @author Álvaro Calleja <alvaro.calleja@gmail.com>
 * @link ttp://www.maurina.org
 * @license GPL
 * @license http://opensource.org/licenses/gpl-license.php GNU Public License
 *
 */

/**
 * Maurina main class.
 *
 * @package Maurina
 * 
 * When instantiated this class sends to the Maurina console the contents of
 * the $_REQUEST, $_SESSION and $_COOKIES (if defined) global variables.
 *
 * It also captures any errors and send its contents to the console. You can 
 * configure the error reporting level below.
 *
 * Any user defined messages are sent via the log method.
 *
 * Simple usage:
 *
 * include ('Maurina.php');
 * $M = new Maurina();
 * $M->log('This is a user defined message');
 * 
 * This class requires PHP5.
 *
 * -- Changelog --
 * 
 * 1.5 Fixed Github issue #5
 * 1.4 Changed tags for console version 1.2
 * 1.3 Added packet counter to prevent overflows (see issue #1 in Github)
 * 1.2 Added the Cookies tab
 * 1.1 Fixed a notice when the calling script does not use sessions.
 * 1.0 Initial release.
 */
class Maurina
{
	/* CONFIGURE HERE THE ERROR REPORTING LEVEL */

	private $_ERROR             = true;
	private $_WARNING           = true;
	private $_PARSE             = true;
	private $_NOTICE            = true;
	private $_CORE_ERROR        = true;
	private $_CORE_WARNING      = true;
	private $_COMPILE_ERROR     = true;
	private $_COMPILE_WARNING   = true;
	private $_USER_ERROR        = true;
	private $_USER_WARNING      = true;
	private $_USER_NOTICE       = true;
	private $_STRICT            = true;
	private $_RECOVERABLE_ERROR = true;
	private $_DEPRECATED        = true;
	private $_USER_DEPRECATED   = true;

	/********************************************/
	
	const TYPE_USER    = 1;
	const TYPE_ERRORS  = 2;
	const TYPE_REQUEST = 3;
	const TYPE_SESSION = 4;
	const TYPE_COOKIES = 5;

	const MAX_MSG_SIZE = 5000;

	private $serverIp = '127.0.0.1';
	private $serverPort = 1947;
	private $tabCaptions = array('&User', '&Errors', '&Request', '&Session',
								 '&Cookies');

	private $numPacketsSent = 0;
	private $numPacketsBeforePause = 50;

	function __construct($serverIp = '', $serverPort = '', $tabCaptions = '')
	{
		if ($serverIp != '')
			$this->serverIp = $serverIp;
		if ($serverPort != '')
			$this->serverPort = $serverPort;
		if (count($tabCaptions) > 1)
			$this->tabCaptions = $tabCaptions;

		set_error_handler(array($this, "errorHandler"));
		register_shutdown_function(array($this, "shutdown"));

		if (count($_REQUEST) > 0)
		{
			$data = $this->formatDump(print_r($_REQUEST, true));
			$this->sendLog(Maurina::TYPE_REQUEST, $data);
		}
		
		if (isset($_SESSION))
			if (count($_SESSION) > 0)
			{
				$data = $this->formatDump(print_r($_SESSION, true));
				$this->sendLog(Maurina::TYPE_SESSION, $data);
			}

		if (isset($_COOKIE))
			if (count($_COOKIE) > 0)
			{
				$data = $this->formatDump(print_r($_COOKIE, true));
				$this->sendLog(Maurina::TYPE_COOKIES, $data);
			}
	}

	public function log($message)
	{
		$type = gettype($message);
		if ($type == 'array' || $type == 'object')
			$message = '<br />' . $this->formatDump(print_r($message, 1));
		else if ($type == 'boolean')
			$message = ($message) ? 'true' : 'false';
		else if ($type == 'NULL')
			$message = 'NULL';
		else $message = htmlentities($message);
		$message = nl2br($message);

		foreach (str_split($message, Maurina::MAX_MSG_SIZE) as $packet)
  			$this->sendLog(Maurina::TYPE_USER, $packet, false);
	}

	public function errorHandler($errorNumber, $errorMsg, $errorFile,
								 $errorLine)
	{
		$type = '';
		switch ($errorNumber)
		{
			case 1     : $type = 'E_ERROR'; 
						 if (!$this->_ERROR) return;
						 break;
			case 2     : $type = 'E_WARNING';
						 if (!$this->_WARNING) return;
						 break;
			case 4     : $type = 'E_PARSE';
						 if (!$this->_PARSE) return;
						 break;
			case 8     : $type = 'E_NOTICE';
						 if (!$this->_NOTICE) return;
						 break;
			case 16    : $type = 'E_CORE_ERROR';
						 if (!$this->_CORE_ERROR) return;
						 break;
			case 32    : $type = 'E_CORE_WARNING';
						 if (!$this->_CORE_WARNING) return;
						 break;
			case 64    : $type = 'E_COMPILE_ERROR';
						 if (!$this->_COMPILE_ERROR) return;
						 break;
			case 128   : $type = 'E_COMPILE_WARNING';
						 if (!$this->_COMPILE_WARNING) return;
						 break;
			case 256   : $type = 'E_USER_ERROR';
						 if (!$this->_USER_ERROR) return;
						 break;
			case 512   : $type = 'E_USER_WARNING';
						 if (!$this->_USER_WARNING) return;
						 break;
			case 1024  : $type = 'E_USER_NOTICE';
						 if (!$this->_USER_NOTICE) return;
						 break;
			case 2048  : $type = 'E_STRICT';
						 if (!$this->_STRICT) return;
						 break;
			case 4096  : $type = 'E_RECOVERABLE_ERROR';
						 if (!$this->_RECOVERABLE_ERROR) return;
						 break;
			case 8192  : $type = 'E_DEPRECATED';
						 if (!$this->_DEPRECATED) return;
						 break;
			case 16384 : $type = 'E_USER_DEPRECATED';
						 if (!$this->_USER_DEPRECATED) return;
						 break;
			case 32767 : $type = 'E_ALL'; break;
		}
		$message  = "<span style='color:#ff9e9e'>[$type] ";
		$message .= "Line $errorLine in $errorFile";
		$message .= "</span><br /><span style='color:#fffa9e'><em>";
		if (file_exists($errorFile))
		{
			$line = $this->getLineFromFile($errorFile, $errorLine);
			$message .= htmlentities(trim($line));
		}
		$message .= "</em></span><br />$errorMsg<br />";

		$this->sendLog(Maurina::TYPE_ERRORS, $message);
	}

	public function shutdown()
	{
		if ($error = error_get_last())
        {
        	$this->errorHandler($error['type'], $error['message'],
        						$error['file'], $error['line']);
		}
	}

	private function sendLog($type, $message, $showTime = false)
	{
		$socket = socket_create(AF_INET, SOCK_DGRAM, SOL_UDP);
		$data = array('tabs' => $this->tabCaptions,
					  'log1' => '',
					  'log2' => '',
					  'log3' => '',
					  'log4' => '',
					  'log5' => '');
		
		if ($showTime)
		{
			$time = "<time>[".date('H:i:s') . ']</time> ';
			$message = $time . $message;
		}

		switch ($type)
		{
			case Maurina::TYPE_USER    : $data['log1'] = $message; break;
			case Maurina::TYPE_ERRORS  : $data['log2'] = $message; break;
			case Maurina::TYPE_REQUEST : $data['log3'] = $message; break;
			case Maurina::TYPE_SESSION : $data['log4'] = $message; break;
			case Maurina::TYPE_COOKIES : $data['log5'] = $message; break;
		}

		$data = json_encode($data);

		// Insert pause to prevent overflows
		if (++$this->numPacketsSent % $this->numPacketsBeforePause == 0)
			usleep(100000);
		
		socket_sendto($socket, $data, strlen($data), 0, $this->serverIp,
					  $this->serverPort);
		socket_close($socket);
	}

	private function getLineFromFile($file, $lineNumber)
	{
		$f = fopen($file, 'r');
		$count = 1;
		$line = null;
		while (($line = fgets($f)) !== false)
		{
			if ($count == $lineNumber)
				break;
			++$count;
		}
		return $line;
	}

	private function formatDump($data)
	{
		$tmp = explode("\n", $data);
		array_shift($tmp);array_shift($tmp);
		array_pop($tmp);array_pop($tmp);
		foreach ($tmp as $k => $v)
		{
			$p = explode(' => ', $v);
			$p[0] = str_replace('[', '', $p[0]);
			$p[0] = str_replace(']', '', $p[0]);
			$html = ' <b style="color:#9ee7ff">'.trim($p[0]).'</b> :&nbsp;&nbsp;'.$p[1];
			$tmp[$k] = $html;
		}
		$tmp = trim(implode('<br /><br />', $tmp)).'<br />';

		return $tmp;
	}
}
