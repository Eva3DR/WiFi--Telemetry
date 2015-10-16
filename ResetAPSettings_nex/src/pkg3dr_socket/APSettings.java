/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package pkg3dr_socket;

import pkg3dr_conf.ConfigSettings;

/**
 *
 * @author elc
 */
public class APSettings {
    
    public static String update_SSIDPSW(String ip, int port, String AP_name, String AP_psw) {
        String command = ConfigSettings.getUpdateSSIDPSWCommand(AP_name, AP_psw);
        String status = "Done.";
                
        try {
            APCommunication.send(ip, port, command);  
        } catch(Exception ex) {
            status = ex.toString();
        }
        return status;
    }
    
    public static String changeBaud(String ip, int port, String baud) {
        String command = ConfigSettings.getChangeBaudCommand(baud);
        String status = "Done.";
        System.out.println("CMD:" + command); 
        try {
            APCommunication.send(ip, port, command);  
        } catch(Exception ex) {
            status = ex.toString();
        }
        return status;
    }
  
}
