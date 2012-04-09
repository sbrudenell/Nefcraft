<?php

/**
 * Members
 *  
 * @author wilsonmu
 * @version 
 */

require_once 'App_Db_Table_Abstract.php';

class Members extends App_Db_Table_Abstract
{
	protected $_use_adapter = "member";
	protected $_name = 'members';
	protected $_primary = "uid";
	protected $_sequence = true;
	protected $_cols = array(
		'uid'  		=> 'uid',
		'name'        	=> 'name',
		'position'      => 'position',
		'adminstatus'   => 'adminstatus',
		'steamid'       => 'steamid',
		'fbstatus'   	=> 'fbstatus',
		'isactive'   	=> 'isactive'
	);
	
	public function getMember($username)
	{
		return $this->fetchRow($this->select()->where('name = ?', "-=|JFH|=-" . $username));
	}	
}
