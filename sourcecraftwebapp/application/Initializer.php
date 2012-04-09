<?php
/**
 * My new Zend Framework project
 * 
 * @author  
 * @version 
 */

require_once 'Zend/Controller/Front.php';
require_once 'Zend/Config/Ini.php';
require_once 'Zend/Registry.php';
require_once 'Zend/Session/Namespace.php';
require_once 'Zend/Controller/Plugin/Abstract.php';
require_once 'Zend/Controller/Request/Abstract.php';
require_once 'Zend/Controller/Action/HelperBroker.php';
require_once 'Zend/Layout.php';
require_once 'ModularModelsDirectory.php';

/**
 * 
 * Initializes configuration depndeing on the type of environment 
 * (test, development, production, etc.)
 *  
 * This can be used to configure environment variables, databases, 
 * layouts, routers, helpers and more
 *   
 */
class Initializer extends Zend_Controller_Plugin_Abstract
{
    /**
     * @var Zend_Config
     */
    protected $_config;

    /**
     * @var string Current environment
     */
    protected $_env;

    /**
     * @var Zend_Controller_Front
     */
    protected $_front;

    /**
     * @var string Path to application root
     */
    protected $_root;

    /**
     * @var Zend_Session_Namespace
     */
    protected $_session;

    /**
     * Constructor
     *
     * Initialize environment, root path, and configuration.
     * 
     * @param  string $env 
     * @param  string|null $root 
     * @return void
     */
    public function __construct($env, $root = null)
    {
        $this->_setEnv($env);
        if (null === $root) {
            $root = realpath(dirname(__FILE__) . '/../');
        }
        $this->_root = $root;

        $this->initPhpConfig();
        
        $this->_front = Zend_Controller_Front::getInstance();
        
        // set the test environment parameters
        if ($env == 'test') {
			// Enable all errors so we'll know when something goes wrong. 
			error_reporting(E_ALL | E_STRICT);  
			ini_set('display_startup_errors', 1);  
			ini_set('display_errors', 1); 

			$this->_front->throwExceptions(true);  
        }
    }

    /**
     * Initialize environment
     * 
     * @param  string $env 
     * @return void
     */
    protected function _setEnv($env) 
    {
		$this->_env = $env;    	
    }
    

    /**
     * Load Configuration
     * 
     * @return void
     */
    public function initPhpConfig()
    {
		// Load Configuration
		$this->_config = new Zend_Config_Ini($this->_root . '/config/sc_config.ini', 'default');
		Zend_Registry::set('config', $this->_config);
    }
    
    /**
     * Route startup
     * 
     * @return void
     */
    public function routeStartup(Zend_Controller_Request_Abstract $request)
    {
       	$this->initDb();
        $this->initHelpers();
        $this->initView();
        $this->initPlugins();
        $this->initRoutes();
        $this->initControllers();
        $this->initSession();
    }
    
    /**
     * Initialize data bases
     * 
     * @return void
     */
    public function initDb()
    {
		$dbAdapters = array();
		$databases = new Zend_Config_Ini($this->_root . '/config/sc_config.ini', 'databases');
		foreach($databases as $config_name => $db)
		{
			$config = $db->config->toArray();
			$dbAdapters[$config_name] = Zend_Db::factory($db->adapter, $config);
 			if((boolean)$db->default)
 			{
    				Zend_Db_Table::setDefaultAdapter($dbAdapters[$config_name]);
 			}
		}
		Zend_Registry::set('dbAdapters', $dbAdapters);
    }

    /**
     * Initialize action helpers
     * 
     * @return void
     */
    public function initHelpers()
    {
    	// register the default action helpers
    	Zend_Controller_Action_HelperBroker::addPath('../application/default/helpers', 'Zend_Controller_Action_Helper');
    }
    
    /**
     * Initialize view 
     * 
     * @return void
     */
    public function initView()
    {
		// Bootstrap layouts
		Zend_Layout::startMvc(array(
		    'layoutPath' => $this->_root .  '/application/modules/default/layouts',
		    'layout' => 'main'
		));
    	
    }
    
    /**
     * Initialize plugins 
     * 
     * @return void
     */
    public function initPlugins()
    {
	/* Modular Models Directory plugin */
	$this->_front->registerPlugin(new ModularModelsDirectory());
    }
    
    /**
     * Initialize routes
     * 
     * @return void
     */
    public function initRoutes()
    {
    
    }

    /**
     * Initialize Controller paths 
     * 
     * @return void
     */
    public function initControllers()
    {
    	//$this->_front->addControllerDirectory($this->_root . '/application/modules/default/controllers', 'default');
    	//$this->_front->addControllerDirectory($this->_root . '/application/modules/sc/controllers', 'sc');
    	$this->_front->addModuleDirectory($this->_root . '/application/modules');
    }
    
    /**
     * Initialize session
     * 
     * @return void
     */
    public function initSession()
    {
		// Start Session
		$session = new Zend_Session_Namespace('SourceCraft');
		Zend_Registry::set('session', $session);	
    }
}
?>
