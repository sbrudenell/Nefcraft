<?php

/**
 * App_Db_Table_Abstract
 *  
 * @author wilsonmu
 * @version 
 */

require_once 'Zend/Db/Table/Abstract.php';

abstract class App_Db_Table_Abstract extends Zend_Db_Table_Abstract
{
	function App_Db_Table_Abstract($config = null)
	{
		if(isset($this->_use_adapter))
		{
			$dbAdapters = Zend_Registry::get('dbAdapters');
			$config = ($dbAdapters[$this->_use_adapter]);
		}
		return parent::__construct($config);
	}
}

