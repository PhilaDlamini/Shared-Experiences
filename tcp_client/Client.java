package tcp_client;
import java.net.*;
import java.io.*;

public class Client {

    private Socket clientSocket;
    private PrintWriter out;
    private BufferedReader in;

    public void startConnection(String ip, int port) {
        try {
            clientSocket = new Socket(ip, port);
            out = new PrintWriter(clientSocket.getOutputStream(), true);
            in = new BufferedReader(new InputStreamReader(clientSocket.getInputStream()));
        } catch (Exception e) {
            e.printStackTrace();
        }
       
    }

    public String readMessage() {
        String n = null;
        try{
            n =  in.readLine();
        } catch (IOException e) {
            e.printStackTrace();
        }
        
        return n;
    }

    public void stopConnection() {
        try {
            in.close();
            out.close();
            clientSocket.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public static void main(String[] args) {
        Client client = new Client();
        client.startConnection("127.0.0.1", 9030);
    }
    
}
