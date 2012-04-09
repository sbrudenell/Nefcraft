<?php

require_once('Zend/Controller/Plugin/Abstract.php');

class ModularModelsDirectory extends Zend_Controller_Plugin_Abstract
{
    public function preDispatch(Zend_Controller_Request_Abstract $request)
    {
        $moduleName = $request->getModuleName();
        set_include_path(get_include_path() .
                         PATH_SEPARATOR . '../../application/modules/' . $moduleName . '/models/');
    }
} 
?>
