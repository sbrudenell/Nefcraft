<?php

/**
 * Groups
 *  
 * @author wilsonmu
 * @version 
 */

require_once 'Zend/Db/Table/Abstract.php';

class Groups extends Zend_Db_Table_Abstract
{
	protected $_use_adapter = "admin";
	protected $_name = 'sm_groups';
	protected $_primary = "id";
	protected $_sequence = true;
	protected $_cols = array(
		'id'  			 => 'id',
		'flags'   		 => 'flags',
		'name'   		 => 'name',
		'immunity_level' => 'immunity_level'
	);
}
