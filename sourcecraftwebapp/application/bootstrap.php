<?php
/**
 * My new Zend Framework project
 * 
 * @author  
 * @version 
 */
set_include_path('.' . PATH_SEPARATOR . '../../library' . PATH_SEPARATOR .
                 '../../application/modules/default/models/' . PATH_SEPARATOR . 
                 '../../application/modules/sc/models/' . PATH_SEPARATOR .
                 get_include_path());

require_once 'Initializer.php';
require_once "Zend/Loader.php"; 
require_once "Zend/Loader/Autoloader.php"; 
require_once "Zend/Application/Module/Autoloader.php"; 

require_once 'App_Controller_Router_Route_RequestVars.php';

// Set up autoload.
//Zend_Loader::registerAutoload();
//$autoloader = Zend_Loader_Autoloader::getInstance();
$autoloader = new Zend_Application_Module_Autoloader(array(
            				'namespace' => 'Default',
            				'basePath'  => dirname(__FILE__),
));
 
// Prepare the front controller. 
$frontController = Zend_Controller_Front::getInstance(); 

if (strlen($base_url) > 0) {
	$frontController->setBaseUrl($base_url);
}

// Use the RequestVars router
$router = $frontController->getRouter();
$router->addRoute('RequestVars', new App_Controller_Router_Route_RequestVars());

// Change to 'production' parameter under production environemtn
$frontController->registerPlugin(new Initializer('test'));    

// Dispatch the request using the front controller. 
$frontController->dispatch(); 

?>

