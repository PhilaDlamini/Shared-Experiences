package com.example;

import javafx.application.Application;
import javafx.application.Platform;
import javafx.stage.Stage;
import java.io.*;
import java.net.*;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.LinkedList;

/**
 * JavaFX App
 */
public class App extends Application {
    private static Socket socket;
    private static PrintWriter out;
    private static InputStream in;
    private static Stage stage;
    public static final int SCREEN_W = 800, SCREEN_H = 600, VIDEO_WIDTH = 500,
    VIDEO_HEIGHT = 350;
    public static int PORT;
 
    //The message types 
    public static final int HELLO = 1, MOVIES = 2, VOTE = 3, MOVIE_SELECTED  = 4,
    MOVIE_CONTENT = 5, DOWNLOADED = 6, START = 7, END_MOVIE = 8, GOODBYE = 9, 
    TOGGLE = 10, TOGGLE_MOVIE = 11, SEEK = 12, SEEK_MOVIE = 13, CHAT = 14,
    CHATS = 15;

    //The user information 
    public static String userName;
    public static String movieName = "unknown.mp4";
    public static LinkedList<String> movies = new LinkedList<>();
    public static ArrayList<String> chats = new ArrayList<>();
    public static long startDuration;
    public static File movieFile;

    //The known screens
    public static final char LOGIN = 'L';
    public static final char SELECTION = 'S';
    public static final char MOVIE_PLAYER = 'M';

    
    //Reads the given number of bytes from the socket
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
        return buffer;
    }

    //Writes a long
    public static void writeLong(long l) {
        out.println(l);
        out.flush();
    }

    //Reads in a long 
    public static long readLong() { 
        ByteBuffer buffer = ByteBuffer.allocate(Long.BYTES);
        buffer.put(read(8));
        buffer.flip();
        return buffer.getLong();
    }

    //Writes the specified number of bytes
    public static void write(char[] data, int n) {
        out.write(data, 0, n);
        out.flush();
    }

    //Switches to the specified screen
    public static void switchToScreen(char screen) {
        Platform.runLater(() -> {
            switch(screen) {
                case LOGIN:
                    stage.setScene(Login.getScreen());
                    break;
                
                case SELECTION:
                    stage.setScene(Selection.getScreen());
                    break;
                
                case MOVIE_PLAYER:
                    stage.setScene(MoviePlayer.getScreen());
                    break;
    
                default:
                    System.out.println("Unknown screen");
                    break;
            }    
        });
    }

    @Override
    public void start(Stage stage) {
        App.stage = stage;
        stage.setOnCloseRequest(e -> MoviePlayer.disposeMedia());
        stage.setOnHidden(e -> MoviePlayer.disposeMedia());
		stage.setTitle("NextChat");
        stage.setScene(Login.getScreen());
        stage.setResizable(false);
		stage.show();
    }

    public static void main(String[] args) {
        //Connect with server
        PORT = Integer.parseInt(args[0]);
        try {
            socket = new Socket(InetAddress.getByName("localhost").getHostAddress(), PORT);
            out = new PrintWriter(socket.getOutputStream(), true);
            in = socket.getInputStream();
        } catch (Exception e) {
            System.out.println("Client encountered error: " + e.getMessage());
            System.exit(-1);
        } 

        //Continously reads input and processes it
        Thread t = new Thread(new Control());
        t.start();

        //Launch the application
        Application.launch(args);

    }

    //Updates the movie file 
    public static void updateMovie(byte[] movie) {
        try {
            if(movieFile != null) {
                movieFile.delete();
            }

            movieFile = File.createTempFile("VACA", ".mp4");
            movieFile.deleteOnExit();
            OutputStream o = new FileOutputStream(movieFile);
            o.write(movie);
            o.close();
            
        } catch (IOException e) {
            System.out.println("ERR updating movie file " + e.getMessage());
        }
    }

}