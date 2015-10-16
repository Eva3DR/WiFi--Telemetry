/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package pkg3dr_util;

import java.io.File;

/**
 *
 * @author elc
 */
public class Utils {
    
    public static String getExtension(File f) {
        String name = f.getName();
        int idx = name.lastIndexOf(".");
        if(idx != -1)
            name = name.substring(idx+1, name.length());
        
        return name;
    }
}
