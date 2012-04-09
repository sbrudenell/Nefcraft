<?php

/** Zend_Controller_Router_Exception */
require_once 'Zend/Controller/Router/Exception.php';

/** Zend_Controller_Router_Route_Interface */
require_once 'Zend/Controller/Router/Route/Interface.php';

/**
 * Route
 *
 * @package    App_Controller
 * @subpackage Router
 * @copyright  Copyright (c) 2008 Rob Allen (rob@akrabat.com)
 */
class App_Controller_Router_Route_RequestVars implements Zend_Controller_Router_Route_Interface
{
    protected $_current = array();

    /**
     * Instantiates route based on passed Zend_Config structure
     */
    public static function getInstance(Zend_Config $config)
    {
        return new self();
    }

    /**
     * Matches a user submitted path with a previously defined route.
     * Assigns and returns an array of defaults on a successful match.
     *
     * @param string Path used to match against this routing map
     * @return array|false An array of assigned values or a false on a mismatch
     */
    public function match($path)
    {
        $frontController = Zend_Controller_Front::getInstance();
        $request = $frontController->getRequest();
        /* @var $request Zend_Controller_Request_Http */
        
        $baseUrl = $request->getBaseUrl();
        if (strpos($baseUrl, 'index.php') !== false) {
            $url = str_replace('index.php', '', $baseUrl);
            $request->setBaseUrl($url);
        }
        
        $params = $request->getParams();
        
        if (array_key_exists('module', $params)
                || array_key_exists('controller', $params)
                || array_key_exists('action', $params)) {
            
            $module = $request->getParam('module', $frontController->getDefaultModule());
            $controller = $request->getParam('controller', $frontController->getDefaultControllerName());
            $action = $request->getParam('action', $frontController->getDefaultAction());

            $result = array('module' => $module, 
                'controller' => $controller, 
                'action' => $action, 
                );
            $this->_current = $result;
            return $result;
        }
        return false;
    }

    /**
     * Generates a URL path that can be used in URL creation, redirection, etc. 
     * 
     * May be passed user params to override ones from URI, Request or even defaults. 
     * If passed parameter has a value of null, it's URL variable will be reset to
     * default. 
     * 
     * If null is passed as a route name assemble will use the current Route or 'default'
     * if current is not yet set.
     * 
     * Reset is used to signal that all parameters should be reset to it's defaults. 
     * Ignoring all URL specified values. User specified params still get precedence.
     * 
     * Encode tells to url encode resulting path parts.     
     *
     * @param  array $userParams Options passed by a user used to override parameters
     * @param  mixed $name The name of a Route to use
     * @param  bool $reset Whether to reset to the route defaults ignoring URL params
     * @param  bool $encode Tells to encode URL parts on output
     * @throws Zend_Controller_Router_Exception
     * @return string Resulting URL path
     */
    public function assemble($userParams = array(), $name = null, $reset = false, $encode = true)
    {
        $frontController = Zend_Controller_Front::getInstance();
        
        if(!array_key_exists('module', $userParams) && !$reset 
            && array_key_exists('module', $this->_current)
            && $this->_current['module'] != $frontController->getDefaultModule()) {
            $userParams = array_merge(array('module'=>$this->_current['module']), $userParams);
        }
        if(!array_key_exists('controller', $userParams) && !$reset 
            && array_key_exists('controller', $this->_current) 
            && $this->_current['controller'] != $frontController->getDefaultControllerName()) {
            $userParams = array_merge(array('controller'=>$this->_current['controller']), $userParams);
        }
        if(!array_key_exists('action', $userParams) && !$reset 
            && array_key_exists('action', $this->_current)
            && $this->_current['action'] != $frontController->getDefaultAction()) {
            $userParams = array_merge(array('action'=>$this->_current['action']), $userParams);
        }
        
        $url = '';
        if(!empty($userParams)) {
            $urlParts = array();
            foreach($userParams as $key=>$value) {
                $urlParts[] = $key . '=' . $value;
            }
            $url = '?' . implode('&', $urlParts);
        }

        return $url;
    }
}


