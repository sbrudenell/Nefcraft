<?php

/**
 * Factions
 *  
 * @author wilsonmu
 * @version 
 */

require_once 'Zend/Db/Table/Abstract.php';

class Factions extends App_Db_Table_Abstract
{
	protected $_use_adapter = "sc";
	protected $_name = 'sc_factions';
	protected $_primary = array("faction");
	protected $_cols = array(
		'faction'    		=> 'faction',
		'name'  		=> 'name',
		'long_name'  		=> 'long_name',
		'description'  		=> 'description',
		'image'  		=> 'image',
		'add_date'		=> 'add_date'
	);	
	
	public function selectPlayerFactions($ident)
	{
		$db = $this->getAdapter();
		return $db->select()->from(array('sc_factions', 'f'),
	                               array('r.faction_name', 'r.long_name', 'r.description'))
							->joinLeft(array('sc_player_tech', 'pt'),
							   		 'f.faction == pt.faction',
							           array('pt.tech_count','pt.tech_level'))
							->where('player_ident = ?', $ident)
							->order('long_name');
	}
}
