<?php

/**
 * User
 *  
 * @author wilsonmu
 * @version 
 */

require_once 'App_Db_Table_Abstract.php';

class SteamId extends App_Db_Table_Abstract
{
	protected $_use_adapter = "user";
	protected $_name = 'nukevo_bbxdata_data';
	protected $_primary = "xdata_value";
	protected $_sequence = true;
	
	public function getRank($steamid)
	{
		return $this->fetchRow($this->select()->where('field_id = 10 and xdata_value = ?', $steamid));
	}	
}
