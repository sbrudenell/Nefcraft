<?php
/**
 * A view helper that prints out the department tree. This is done from a 
 * view helper because it needs to be called recursively. 
 * 
 * @version $Id: PrintDepartmentTree.php,v 1.3 2007-12-04 16:54:49 seva Exp $
 */

require_once 'Zend/Controller/Front.php';

class Zend_View_Helper_PrintPlayerList
{

    /**
     * Iterate and build the HTML for the player list
     *
     * @param  array $list
     * @return string
     */
    public function printPlayerList($list)
    {
        if (!is_array($list) || empty($list))
            return '';
        
        $html = '<ul>';
        $baseurl = Zend_Controller_Front::getInstance()->getBaseUrl() . '/player/show/name/';
        foreach ($list as $player) {
            $html .= '<li><a href="' . $baseurl . urlencode($player->name) . '">' . htmlspecialchars($player->name) . '</a>';
            $html .= "</li>\n";
        }
        
        $html .= '</ul>';
        
        return $html;
    }
}