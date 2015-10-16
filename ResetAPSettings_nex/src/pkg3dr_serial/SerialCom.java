/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package pkg3dr_serial;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.net.URI;
import java.util.ArrayList;
import pkg3dr_extract.UnpackResources;

/**
 *
 * @author elc
 */
public class SerialCom {
   
    public static String [] getPorts() {
        ArrayList<String> portList = new ArrayList<String>();
        Process p;
        String line;
        
        final URI uri;
        final URI exe;

        try {
            uri = UnpackResources.getJarURI();
            exe = UnpackResources.getFile(uri, "serialT.py");
            System.out.println(exe);
        } catch (Exception ex) {
            System.out.println("Error al desempacar serialT.py");
            System.out.println(ex);
            return null;
        }

        String urlStr = exe.toString();
        int idx = urlStr.indexOf(":/");
        if(idx == -1) {
            return null;
        }
        urlStr = urlStr.substring(idx+2);
        System.out.println("urlStr:" + urlStr);
        
        String [] cmd = {"python", urlStr};
        
	try{
            p = Runtime.getRuntime().exec(cmd);
            BufferedReader input =
            new BufferedReader(new InputStreamReader(p.getInputStream()));
            while ((line = input.readLine()) != null) {
                portList.add(line);
                System.out.println(line);
            }
            input.close();
            p.waitFor();
            p.destroy();
	} catch (Exception e) {
            System.out.println("Error al ejecutar serialT.py");
            System.out.println(e);
        }
       
        String [] portListArr = new String[portList.size()];
        portListArr = portList.toArray(portListArr);

        return portListArr;
    }
    
    public static String writePortUpdate_SSIDPSW(String comPort, String [] args) {
        if(args.length != 2) {
            return "Invalid args";
        }
        String cmd = comPort + " " + args[0] + " " + args[1];
        System.out.println(cmd); 
        return writePort(comPort, "swrite.py", cmd);
      
    }

    public static String writePortUpdate_IPPORT(String comPort, String [] args) {
        if(args.length != 2) {
            return "Invalid args";
        }
        String cmd = comPort + " " + args[0] + " " + args[1];
        System.out.println(cmd); 
        return writePort(comPort, "sIPPort.py", cmd);
    }

    private static String writePort(String comPort, String script, String commArgs) {
        Process p;
        String line = "";
        String status = "Done.";
        
        final URI uri;
        final URI exe;

        try {
            uri = UnpackResources.getJarURI();
            exe = UnpackResources.getFile(uri, script);
            System.out.println(exe);
        } catch (Exception ex) {
            System.out.println("Error al desempacar " + script);
            System.out.println(ex);
            return "Error getting resource";
        }
        
        String urlStr = exe.toString();
        int idx = urlStr.indexOf(":/");
        if(idx == -1) {
            return null;
        }
        urlStr = urlStr.substring(idx+2);
        System.out.println(urlStr);

        String cmd = "python "+ urlStr + " " + commArgs; 
        try{
            p = Runtime.getRuntime().exec(cmd); 
            BufferedReader input =
            new BufferedReader(new InputStreamReader(p.getInputStream()));
            while ((line = input.readLine()) != null) {
                System.out.println(line);
                if(line.equals("Baudrate")) { 
                    status += " Baudrate found: " + input.readLine();
                }
            }
            input.close();
            p.waitFor();
            p.destroy();
        } catch (Exception e) {
            System.out.println("Error al ejecutar " + script);
            System.out.println(e);
            status = e.toString();
        }
        return status;
    }    

}
