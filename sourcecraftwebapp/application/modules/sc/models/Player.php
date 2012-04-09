<?php

require_once ('App_Db_Table_Abstract.php');

class Player extends App_Db_Table_Abstract
{
	protected $_use_adapter = "sc";
	protected $_name = "sc_players";
	protected $_primary = "player_ident";
	protected $_sequence = true;
	protected $_cols = array(
		'player_ident'  => 'player_ident',
		'steamid'       => 'steamid',
		'name'        	=> 'name',
		'race_ident'    => 'race_ident',
		'crystals'   	=> 'crystals',
		'vespene'   	=> 'vespene',
		'overall_level' => 'overall_level',
		'settings' 	=> 'settings',
		'last_update' 	=> 'last_update',
		'username'	=> 'username'
	);

	public function getPlayerForSteamid($steamid)
	{
		return $this->fetchRow($this->select()->where('steamid = ?', $steamid));
	}

	public function getPlayerForUsername($username)
	{
		return $this->fetchRow($this->select()->where('username = ?', $username));
	}

	public function getPlayerForName($name)
	{
		return $this->fetchAll($this->select()->where('name = ?', $name));
	}

	public function getPlayersMatchingName($name)
	{
		$db = $this->getAdapter();

		$select_players = $db->select();
		$select_players->from(array('p' => 'sc_players'),
		       		      array('player_ident', 'steamid', 'overall_level', 'crystals', 'vespene', 'name'));
		$select_players->where('name like ?', $name);

		$select_aliases = $db->select();
		$select_aliases->from(array('p' => 'sc_players'),
		       		      array('player_ident', 'steamid', 'overall_level', 'crystals', 'vespene'));
		$select_aliases->join(array('pa' => 'sc_player_alias'),
				      "pa.player_ident = p.player_ident",
				      "name");
		$select_aliases->where('pa.name like ?', $name);

		$select_union = $db->select();
		$select_union->union(array($select_players, $select_aliases));
		$select_union->order('name');
		return $db->fetchAll($select_union);
	}

	public function getAlias($ident)
	{
		$db = $this->getAdapter();
		$select = $db->select();
		$select->from(array('pa' => 'sc_player_alias'),
			array('steamid', 'name', 'last_used',
		   	      'date' => "DATE_FORMAT(last_used, '%m/%d/%y')"));
		$select->where('pa.player_ident = ?', $ident);
		$select->order(array('last_used','name'));
		//$stmt = $db->query($select);
		//return $stmt->fetchAll();
		return $db->fetchAll($select);
	}

	public function getRaces($ident)
	{
		$db = $this->getAdapter();
		$select = $db->select();
		$select->from(array('p' => 'sc_player_races'),
			      array('xp', 'level', 'race_ident'));
		$select->where('p.player_ident = ?', $ident);
		$select->join(array('r' => 'sc_races'),
			      'p.race_ident = r.race_ident',
			      array('r.long_name', 'r.faction', 'r.type', 'r.description', 'r.image'));
		$select->join(array('f' => 'sc_factions'),
			      'f.faction = r.faction',
			      array('faction_name' => 'f.long_name'));
		$select->order('faction_name','r.long_name');
		//$stmt = $db->query($select);
		//return $stmt->fetchAll();
		return $db->fetchAll($select);
	}

	public function getUpgrades($ident)
	{
		$db = $this->getAdapter();
		$select = $db->select();
		$select->from(array('pu' => 'sc_player_upgrades'),
		              array('pu.race_ident', 'pu.upgrade', 'pu.upgrade_level'));
		$select->where('pu.player_ident = ?', $ident);
		$select->join(array('u' => 'sc_upgrades'),
		              'u.race_ident = pu.race_ident and u.upgrade = pu.upgrade',
		              array('u.long_name', 'u.description'));
		$select->order(array('pu.race_ident', 'pu.upgrade'));
		//$stmt = $db->query($select);
		//return $stmt->fetchAll();
		return $db->fetchAll($select);
	}	
}

?>
