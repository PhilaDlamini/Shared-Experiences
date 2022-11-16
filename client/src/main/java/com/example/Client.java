package com.example;
import java.net.*;
import java.io.*;
import java.util.*;
import java.awt.image.*;
import org.bytedeco.javacv.FFmpegFrameGrabber;
import org.bytedeco.javacv.Frame;
import org.bytedeco.javacv.FrameGrabber;

import javafx.application.Application;
import javafx.scene.*;
import javafx.scene.image.ImageView;
import javafx.scene.layout.VBox;
import javafx.stage.Stage;

public class Client extends Application {

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

    public void readMessage() throws Exception{
        System.out.println("About to read data");
        int c = in.read();
        // int length = 0;

        while(in.ready()) {
            // length += line.length();
            System.out.print((char)c);
            c = in.read(); 
        }

        // System.out.println(length + " Should have finished reading data");
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


    public static void streamImageToView(ImageView view, int port, int socketBacklog, Consumer<Grabber> grabberSettings) {
        try ( 
            final FrameGrabber grabber = new FFmpegFrameGrabber(clientSocket.getInputStream());
        ) {
            grabberSettings.accept(grabber);
            grabber.start();
            while (!Thread.interrupted()) {
                final Frame frame = grabber.grab();
                if (frame == null) {
                    continue;
                }
                final BufferedImage bufferedImage = converter.convert(frame);
                if (bufferedImage != null) {
                    Platform.runLater(() -> {
                        SwingFXUtils.toFXImage(bufferedImage, view.getImage());
                        view.setImage(view.getImage()); 
                        // might not be required. Forces repaint
                    });
                }
            } catch (IOException ex) {
                // same as before
            }
        } 
    }

    @Override
    public void start(Stage primaryStage) throws Exception {
        ImageView image = new ImageView();
        VBox box = new VBox(image);
        Scene scene = new Scene(box, 500, 500);

        primaryStage.setTitle("NextChat");
		primaryStage.setScene(scene);
		primaryStage.show();
    }

    public static void main(String[] args) throws Exception{
        Application.launch(args);
        // Client client = new Client();
        // client.startConnection("localhost", 9036);
        // client.readMessage();
    }
    
}
