/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package pkg3dr_socket;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketException;

/**
 *
 * @author elc
 */
public class APCommunication {

    public static void send(String server, int port, String msg) throws SocketException, IOException {
        DatagramSocket clientSocket = new DatagramSocket();
        int localPort = clientSocket.getLocalPort();
        System.out.println("Puerto local: " + localPort);
        InetAddress IPAddress = InetAddress.getByName(server); 
        byte[] sendData = new byte[1024];
        sendData = msg.getBytes();
        DatagramPacket sendPacket = new DatagramPacket(sendData, sendData.length, IPAddress, port); 
        clientSocket.send(sendPacket);
    }

}
