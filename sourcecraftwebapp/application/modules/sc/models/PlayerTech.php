<?php

/**
 * PlayerTech
 *  
 * @author wilsonmu
 * @version 
 */

require_once 'App_Db_Table_Abstract.php';

class PlayerTech extends App_Db_Table_Abstract
{
	protected $_use_adapter = "sc";
	protected $_name = "sc_player_alias";
	protected $_primary = array("player_ident", "faction");
	protected $_cols = array(
		'player_ident'  => 'player_ident',
		'faction'    	=> 'faction',
		'tech_count'    => 'tech_count',
		'tech_level'    => 'tech_level'
	);
}
