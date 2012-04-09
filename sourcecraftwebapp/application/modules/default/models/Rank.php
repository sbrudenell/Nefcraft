<?php

/**
 * User
 *  
 * @author wilsonmu
 * @version 
 */

require_once 'App_Db_Table_Abstract.php';

class Rank extends App_Db_Table_Abstract
{
	protected $_use_adapter = "user";
	protected $_name = 'nukevo_bbranks';
	protected $_primary = "rank_id";
	protected $_sequence = true;
	
	public function getRank($rank)
	{
		return $this->fetchRow($this->select()->where('rank_id = ?', $rank));
	}	
}
