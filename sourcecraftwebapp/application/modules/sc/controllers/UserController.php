<?php

/**
 * SourceCraft user controller - Handling user related actions - currently log in and
 * log out.
 * 
 */

require_once 'Zend/Controller/Action.php';
require_once 'Zend/Registry.php';
require_once 'Zend/Session.php';
require_once 'Zend/Auth/Adapter/DbTable.php';

require_once 'Player.php';

class Sc_UserController extends Zend_Controller_Action
{
    /**
     * User session
     *
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
     * Default action - if logged in, log out. If logged out, log in.
     *
     */
    public function indexAction()
    {
        if ($this->session->authenticated) {
            $this->_forward('logout');
        } else {
            $this->_forward('login');
        }
    }

    /**
     * Log in - show the login form or handle a login request
     * 
     * @todo Implement real authentication
     */
    public function loginAction()
    {
        if ($this->getRequest()->getMethod() != 'POST')
        {
            // Not a POST request, show log-in form
            $view = $this->initView();
        	$this->render();
        }
        else
        {
            // Handle log-in form
            $username = $this->getRequest()->getParam('user');
            if ($username)
            	$password = $this->getRequest()->getParam('password');
            else
            {
            	$username = $this->getRequest()->getParam('suser');
            	$password = $this->getRequest()->getParam('spassword');
            }
            
	    // setup Zend_Auth adapter for a database table
	    $dbAdapters = Zend_Registry::get('dbAdapters');
	    $authAdapter = new Zend_Auth_Adapter_DbTable($dbAdapters['user'],
							 'nukevo_users',
							 'username',
							 'user_password',
							 'MD5(?)');

	    // Set the input credential values to authenticate against
	    $authAdapter->setIdentity($username);
	    $authAdapter->setCredential($password);

	    // do the authentication
	    $auth = Zend_Auth::getInstance();
	    $result = $auth->authenticate($authAdapter);
	    if ($result->isValid())
	    {
		    // success: store database row to auth's storage
		    // system. (Not the password though!)
		    $data = $authAdapter->getResultRowObject(null, 'password');
		    $auth->getStorage()->write($data);

		    Zend_Session::regenerateId();
		    $this->session->logged_in = true;
		    $this->session->username = $username;

		    $player_table = new Player();
		    $player = $player_table->getPlayerForUsername($username);
		    if ($player)
		    {
			    $this->session->steamid = $player->steamid;
		    }
		    else
		    {
			    $member_table = new Members();
			    $member = $member_table->getMember($user);
			    if ($member)
			    {
				    $this->session->steamid = 'STEAM_' . $member->steamid;

				    // Update player record's username
				    $player = $player_table->getPlayerForSteamid($this->session->steamid);
				    if ($player)
				    {
					    $where = $table->getAdapter()->quoteInto('steamid = ?', $this->session->steamid);
					    $player_table->update( array( 'username' => $username), $where);
				    }
			    }
		    }

		    //$this->_forward('profile');
		    $this->_redirect('/sc/player/show/user/' . $username);
	    }
	    else // Wrong user name / password
	    {
		    $view = $this->initView();
		    $view->user = $username;
		    $view->error = 'Wrong user name or password, please try again';                
		    $this->render();
	    }
	}
    }

    /**
     * Log out - delete user information and clear the session, then redirect to
     * the log in page.
     */
    public function logoutAction()
    {
	Zend_Auth::getInstance()->clearIdentity();
        $this->session->admin = false;        
    	$this->session->logged_in = false;
        $this->session->username = false;
        $this->session->steamid = false;
        $this->_redirect('/sc/user/login');
    }

    /**
     * Profile - show the user profile screen
     * 
     * @todo Implement real profile screen
     */
    public function profileAction()
    {
        $user = $this->session->username;
        if ($user)
        {
            $this->_redirect('/sc/player/show/user/' . $user);
        	//$this->_forward('show', 'player', 'sc', array('user' => $user));
        }
        else
        {
        	$this->_forward('index','index', 'sc');
        }
    }
}
