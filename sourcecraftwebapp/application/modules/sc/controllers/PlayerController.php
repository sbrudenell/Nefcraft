<?php

/**
 * PlayerController
 * 
 * @author
 * @version 
 */

require_once 'Zend/Controller/Action.php';

require_once 'Player.php';

class Sc_PlayerController extends Zend_Controller_Action
{
    /**
     * @var Zend_Session_Namespace
     */
    protected $session = null;

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
    
	/**
	 * The default action - show the home page
	 */
	public function indexAction()
	{
		$this->_forward('list');
	}
	
	public function findAction()
	{
		$ident = $this->getRequest()->getParam('ident');
		$view = $this->initView();
				
		$player_table = new Player();
		$rowset = $player_table->find($ident);
		if ($rowset)
		{
			$view->player = $rowset->current();
			$view->alias = $player_table->getAlias($view->player->player_ident);
			$view->races = $player_table->getRaces($view->player->player_ident);
			$view->upgrades = $player_table->getUpgrades($view->player->player_ident);
			
			//$race_table = new Races();
			//$paginator = Zend_Paginator::factory($race_table->selectPlayerRaces($ident));
			//$paginator->setItemCountPerPage(1);
			//$paginator->setCurrentPageNumber($this->_getParam('page'));
			//$paginator->setView($view);
			//$this->view->paginator = $paginator;
			//$this->render();
		}
		else
		{
			$view->error = 'Player # "' . $ident . ' was not found';
		}
		$this->render();
	}
	
	public function showAction()
	{
		$steamid = $this->getRequest()->getParam('steamid');
		if ($steamid)
		{
			$player_table = new Player();
			$view = $this->initView();
			$player = $player_table->getPlayerForSteamid($steamid);
			if ($player)
			{
				$view->player = $player;
				$view->alias = $player_table->getAlias($view->player->player_ident);
				$view->races = $player_table->getRaces($player->player_ident);
				$view->upgrades = $player_table->getUpgrades($view->player->player_ident);
			}
			else
			{
				$view->error = 'No player with a steamid of ' . $steamid . ' was found';
			}
			$this->render();
		}
		else
		{
			$name = $this->getRequest()->getParam('pname');
			if ($name)
			{
				$player_table = new Player();
				$rowset = $player_table->getPlayerForName($name);
				if (count($rowset))
				{
					$view = $this->initView();
					$view->player = $rowset->current();
					$view->alias = $player_table->getAlias($view->player->player_ident);
					$view->races = $player_table->getRaces($view->player->player_ident);
					$view->upgrades = $player_table->getUpgrades($view->player->player_ident);
					$this->render();
				}
				else
				{
					$view = $this->initView();
					$view->error = 'No player with a name of ' . $name . ' was found';
					$this->render();
				}
			}
			else
			{
				$user = $this->getRequest()->getParam('user');
				if ($user)
				{
					$player_table = new Player();
					$player = $player_table->getPlayerForUsername($user);
					if ($player)
					{
						$view = $this->initView();
						$view->player = $player;
						$view->alias = $player_table->getAlias($view->player->player_ident);
						$view->races = $player_table->getRaces($player->player_ident);
						$view->upgrades = $player_table->getUpgrades($view->player->player_ident);
						$this->render();
					}
					else
					{
						$this->_forward('show', 'player', 'sc',
					                	array('pname' => '%' . $user . '%'));
					}
				}
				else
				{
					$this->_forward('list');
				}
			}
		}
	}
	
	public function matchAction()
	{
		$player_table = new Player();
		$name = $this->getRequest()->getParam('pname');
		if ($name)
		{
			$rowset = $player_table->getPlayersMatchingName('%' . $name . '%');
			$count = count($rowset);
			if ($count > 1)
			{
				$view = $this->initView();
				$paginator = Zend_Paginator::factory($rowset);
				$paginator->setItemCountPerPage(10);
				$paginator->setCurrentPageNumber($this->_getParam('page'));
				$paginator->setView($view);
				$this->view->paginator = $paginator;
				$this->render();
			}
			elseif ($count == 1)
			{
				$this->_forward('show', 'player', 'sc',
				               	array('pname' => $name));
			}
			else
			{
				$view = $this->initView();
				$view->error = 'No player matching ' . $name . ' was found';
				$this->render();
			}
		}
		else
		{
			$this->_forward('list');
		}
	}
	
	public function listAction()
	{
		$player_table = new Player();
		$view = $this->initView();
		$paginator = Zend_Paginator::factory($player_table->select());
		$paginator->setItemCountPerPage(10);
		$paginator->setCurrentPageNumber($this->_getParam('page'));
		$paginator->setView($view);
		$this->view->paginator = $paginator;
		$this->render();
	}
}
?>

