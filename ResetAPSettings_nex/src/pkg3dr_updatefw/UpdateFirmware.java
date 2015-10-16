/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package pkg3dr_updatefw;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.net.URI;
import pkg3dr_extract.UnpackResources;

/**
 *
 * @author elc
 */
public class UpdateFirmware {
    
    public static String update(String com, String binPath) {
        Process p;
        String status = "Done.";
        
        final URI uri;
        final URI esp_exe;

        try {
            uri = UnpackResources.getJarURI();
            esp_exe = UnpackResources.getFile(uri, "esptool.py");
            System.out.println(esp_exe);
        } catch (Exception ex) {
            System.out.println("Error al desempacar esptool.py, binarios");
            System.out.println(ex);
            return null;
        }

        String urlEsp = esp_exe.toString();
        
        int idx = urlEsp.indexOf(":/");
        if(idx == -1) {
            return null;
        }
        urlEsp = urlEsp.substring(idx+2);
	String cmd = "python " + urlEsp+" -p " + com + " -b 115200 write_flash -ff 40m -fm qio -fs 4m 0x00000 " + binPath + "0x00000.bin 0x40000 "+ binPath + "0x40000.bin";
        System.out.println("CMD:" + cmd + ".");

        String line = "";
        try{
            p = Runtime.getRuntime().exec(cmd);
            
            BufferedReader input =
                new BufferedReader(new InputStreamReader(p.getInputStream()));
            while ((line = input.readLine()) != null) {
                System.out.println(line);
            }
            input.close();
            
            p.waitFor();
            p.destroy();
	} catch (Exception e) {
            System.out.println("Error al ejecutar esptool.py");
            System.out.println(e);
            status = e.toString();
        }
        
        return status;
    }
}
















