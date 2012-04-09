<?php

/**
 * Races
 *  
 * @author wilsonmu
 * @version 
 */

require_once 'Zend/Db/Table/Abstract.php';

class Races extends App_Db_Table_Abstract
{
	protected $_use_adapter = "sc";
	protected $_name = 'sc_races';
	protected $_primary = array("race_ident");
	protected $_cols = array(
		'race_ident'    	=> 'race_ident',
		'race_name'  		=> 'race_name',
		'long_name'  		=> 'long_name',
		'parent_name'  		=> 'parent_name',
		'faction'  		=> 'faction',
		'type'  		=> 'type',
		'description'  		=> 'description',
		'image'  		=> 'image',
		'required_level'  	=> 'required_level',
		'tech_level'  		=> 'tech_level',
		'add_date'		=> 'add_date'
	);	
	
	public function selectPlayerRaces($ident)
	{
		$db = $this->getAdapter();
		return $db->select()->from(array('sc_races', 'r'),
	                               array('r.race_name', 'r.long_name', 'r.description'))
							->joinLeft(array('sc_player_races', 'pr'),
							   		   'r.player_ident == pr.player_ident',
							           array('pr.xp','pr.level'))
							->where('player_ident = ?', $ident)
							->order('long_name');
	}
}
