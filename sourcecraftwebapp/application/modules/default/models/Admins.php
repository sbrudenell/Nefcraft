<?php

/**
 * Admins
 *  
 * @author wilsonmu
 * @version 
 */

require_once ('App_Db_Table_Abstract.php');

class Admin extends App_Db_Table_Abstract
{
	protected $_use_adapter = "admin";
	protected $_name = 'sm_admins';
	protected $_primary = "id";
	protected $_sequence = true;
	protected $_cols = array(
		'id'  		=> 'id',
		'authtype'  => 'steamid',
		'identity'  => 'identity',
		'password'  => 'password',
		'flags'   	=> 'flags',
		'name'   	=> 'name',
		'immunity' 	=> 'immunity'
	);
}
