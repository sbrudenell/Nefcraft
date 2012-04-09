<?php

/**
 * Upgrades
 *  
 * @author wilsonmu
 * @version 
 */

require_once 'App_Db_Table_Abstract.php';

class Upgrades extends App_Db_Table_Abstract
{
	protected $_use_adapter = "sc";
	protected $_name = 'upgrades';
	protected $_primary = array("race_ident", "upgrade");
	protected $_cols = array(
		'race_ident'    => 'race_ident',
		'upgrade'  	=> 'upgrade',
		'category'      => 'category',
		'upgrade_name'  => 'upgrade_name',
		'long_name'  	=> 'long_name',
		'description'	=> 'description',
		'add_date'	=> 'add_date'
	);	
}
