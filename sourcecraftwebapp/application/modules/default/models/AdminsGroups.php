<?php

/**
 * AdminsGroups
 *  
 * @author wilsonmu
 * @version 
 */

require_once ('App_Db_Table_Abstract.php');

class AdminsGroups extends App_Db_Table_Abstract
{
	protected $_use_adapter = "admin";
	protected $_name = 'sm_admins_groups';
	protected $_primary = array("admin_id", "group_id");
	protected $_sequence = false;
	protected $_cols = array(
		'admin_id'  	=> 'admin_id',
		'group_id'  	=> 'group_id',
		'inherit_order' => 'inherit_order'
	);
}
