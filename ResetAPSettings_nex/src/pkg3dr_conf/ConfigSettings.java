/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package pkg3dr_conf;

/**
 *
 * @author elc
 */
public class ConfigSettings {
    
    public static String getUpdateSSIDPSWCommand(String AP_name, String AP_psw) {
        return "AT+CWSAP=\"" + AP_name + "\",\"" + AP_psw + "\"\r\n";
    }
    
    public static String getChangeBaudCommand(String baud) {
        return "AT+UART=" + baud + "\r\n";
    }
    
}
