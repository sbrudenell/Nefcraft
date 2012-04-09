<?php

/**
 * PlayerUpgrades
 *  
 * @author wilsonmu
 * @version 
 */

require_once 'App_Db_Table_Abstract.php';

class PlayerRaces extends App_Db_Table_Abstract
{
	protected $_use_adapter = "sc";
	protected $_name = "sc_player_upgrades";
	protected $_primary = array("player_ident", "race_ident", "upgrade");
	protected $_cols = array(
		'player_ident'  => 'player_ident',
		'race_ident'    => 'race_ident',
		'upgrade'       => 'upgrade',
		'upgrade_level' => 'upgrade_level'
	);	
}
