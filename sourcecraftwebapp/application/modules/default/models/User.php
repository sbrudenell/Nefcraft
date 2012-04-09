<?php

/**
 * User
 *  
 * @author wilsonmu
 * @version 
 */

require_once 'App_Db_Table_Abstract.php';

class User extends App_Db_Table_Abstract
{
	protected $_use_adapter = "user";
	protected $_name = 'nukevo_users';
	protected $_primary = "user_id";
	protected $_sequence = true;
	
	public function getUser($username)
	{
		return $this->fetchRow($this->select()->where('username = ?', $username));
	}	
}
