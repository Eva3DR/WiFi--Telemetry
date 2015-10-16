/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package pkg3dr_gui;

import java.io.File;
import javax.swing.filechooser.FileFilter;

import pkg3dr_util.Utils;


/**
 *
 * @author elc
 */
public class FileExtensionFilter extends FileFilter{

    @Override
    public boolean accept(File pathname) {
        if(pathname.isDirectory())
            return true;
        
        String ext = Utils.getExtension(pathname);
        if(ext != null) {
            if(ext.equals("bin"))
                return true;
        }
        return false;
    }

    @Override
    public String getDescription() {
        return ".bin";
    }
    
}
