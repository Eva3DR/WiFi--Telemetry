/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package pkg3dr_serial;

/**
 *
 * @author elc
 */
public class APSettings {
    public static String updateSSID_PSW(String comPort, String AP_name, String AP_psw) {
        String [] settings = {AP_name, AP_psw};
        return SerialCom.writePortUpdate_SSIDPSW(comPort, settings);
    }
    public static String updateIP_Port(String comPort, String ip, String port) {
    	String [] settings = {ip, port};
        return SerialCom.writePortUpdate_IPPORT(comPort, settings);
    }
}
