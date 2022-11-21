package com.example;

import javafx.application.Application;
import javafx.scene.Parent;
import javafx.scene.Scene;
import javafx.scene.layout.StackPane;
import javafx.stage.Stage;
import java.io.*;
import java.net.*;
import java.nio.ByteBuffer;

/**
 * JavaFX App
 */
public class App extends Application {
    private static Socket socket;
    public static PrintWriter out;
    public static InputStream in;
    public static final int SCREEN_W = 800;
    public static final int SCREEN_H = 600;
    public static final int VIDEO_WIDTH = 500;
    public static final int VIDEO_HEIGHT = 300;

    //The message types 
    public static final int HELLO = 1;
    public static final int MOVIES = 2;
    public static final int VOTE = 3;
    public static final int MOVIE_SELECTED  = 4;
    public static final int MOVIE_CONTENT = 5;
    public static final int DOWNLOADED = 6;
    public static final int START = 7;
    public static final int END_MOVIE = 8;
    public static final int GOODBYE = 9;
    public static final int ERROR = 10;
    
    @Override
    public void start(final Stage primaryStage) {
		primaryStage.setTitle("NextChat");
		primaryStage.setScene(Login.getScreen(primaryStage));
		primaryStage.show();
    }

    public static void main(String[] args) {
        try {
            socket = new Socket(InetAddress.getByName("localhost").getHostAddress(), Integer.parseInt(args[0]));
            out = new PrintWriter(socket.getOutputStream(), true);
            in = socket.getInputStream();
        } catch (Exception e) {
            e.printStackTrace();
        }

        Application.launch(args);
    }

    //Writes to the socket
    public static byte[] read(int n) {
        byte[] buffer = new byte[n];
        int read = 0;

        while(read < n) {
            try {
                read += in.read(buffer, read, n - read);
            } catch (IOException e) {
                e.printStackTrace();
            }
        }

        System.out.println("Read " + read + " bytes from server");
        return buffer;
    }

    //Reads in a long 
    public static long readLong() { 
        ByteBuffer buffer = ByteBuffer.allocate(Long.BYTES);
        buffer.put(read(8));
        buffer.flip();
        return buffer.getLong();
    }

}