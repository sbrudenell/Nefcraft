<?php

/**
 * User
 *  
 * @author wilsonmu
 * @version 
 */

require_once 'App_Db_Table_Abstract.php';

class Donators extends App_Db_Table_Abstract
{
	protected $_use_adapter = "user";
	protected $_name = 'nukevo_donators';
	protected $_primary = "uid";
	protected $_sequence = true;

	public function getDonatorByName($uname)
	{
		return $this->fetchRow($this->select()->where('uname = ?', $uname));
	}

	public function getDonatorById($uid)
	{
		return $this->fetchRow($this->select()->where('uid = ?', $uid));
	}	
}
