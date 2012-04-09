<?php

/**
 * IndexController - The default controller class
 * 
 * @author
 * @version 
 */

require_once 'Zend/Controller/Action.php';

class Sc_IndexController extends Zend_Controller_Action 
{
	/**
	 * Overriding the init method to also load the session from the registry
	 *
	 */
	public function init()
	{
		parent::init();
		$this->session = Zend_Registry::get('session');
	}

	public function initView()
	{
		$view = parent::initView();
		if (isset($this->session))
		{
			$view->session = $this->session;        
		}
		return $view;
	}

	public function indexAction()
	{
		if ($this->getRequest()->getMethod() != 'POST')
		{
			// Not a POST request, show form
			$view = $this->initView();
			$this->render();
		}
		else
		{
			$name = $this->getRequest()->getParam('pname');
			if ($name)
			{
				$this->_redirect('/sc/player/match/pname/' . urlencode($name));
			}
			else
			{
				$steamid = $this->getRequest()->getParam('steamid');
				if ($steamid)
				{
					$this->_forward('show', 'player', 'sc',
							array('steamid' => $steamid));
				}
				else
				{
					$view = $this->initView();
					$this->render();
				}
			}
		}
	}
}
