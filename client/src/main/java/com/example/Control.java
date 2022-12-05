package com.example;
import java.io.*;
import java.util.*;

/*
 * The main control thread
 */
public class Control implements Runnable {
    
    public Thread t;
    
    Control () {
        t = new Thread(this);
        t.start();
    }

    public void run() {

        while(!Thread.interrupted()) {
            int type = App.read(1)[0];

            switch(type) {

                case App.MOVIES:

                    //Read the list and go to movie selection
                    System.out.println("Got movie list");
                    String[] movies = new String(App.read(800)).split("\0");
                    
                    //Removies the ".mp4" extension
                    LinkedList<String> rem = new LinkedList<>();
                    for(String name: movies) {
                        String a = name.replaceAll(".mp4", "");
                        rem.add(a);
                    }   
                    App.movies = rem;

                    //Print them out
                    for(String movie: App.movies)
                        System.out.println(movie);

                    //Go to movie selection
                    App.switchToScreen(App.SELECTION);
                    break;

                case App.MOVIE_SELECTED:
                    int selection = App.read(1)[0];
                    System.out.println("Winning movie: " + selection);
                    App.movieName = App.movies.get(selection); 
                    break;
                
                case App.IMAGE:
                    System.out.println("Got image!");
                    String sender = new String(App.read(20));
                    long size = App.readLong();
                    System.out.println("Image was " + size + " bytes");
                    byte[] data = App.read((int) size); //Again, this will be a problem
                    
                    //Create the file
                    File imgFile = null;
                    try {
                        imgFile = File.createTempFile("IMAGE", "");
                        imgFile.deleteOnExit();
                        OutputStream o = new FileOutputStream(imgFile);
                        o.write(data);
                        o.close();
                    } catch (IOException e) {
                        System.out.println("Encountered err saving image " + e.getMessage());
                    }

                    App.images.add(new ImageInfo(sender, imgFile));
                    System.out.println("Got image! Sender was " + sender);
                    break;

                case App.MOVIE_CONTENT:
                    
                    //TODO: what if movie bytes are more than INT_MAX (mkbhd video gives problem)
                    byte[] movie = App.read((int) App.readLong());
                    System.out.println("Movie Content Byte Array Length: " + movie.length);
                    App.updateMovie(movie);
                    App.switchToScreen(App.MOVIE_PLAYER);
                    break;

                case App.START:
                    System.out.println("Got start signal from server!");

                    //Read in the start duration
                    App.startDuration = App.readLong();
                    MoviePlayer.start();
                    break;

                case App.TOGGLE_MOVIE:
                    System.out.println("Got toggle from server");
                    MoviePlayer.togglePlayer();
                    break;

                case App.SEEK_MOVIE:
                    System.out.println("Got seek from server");
                    MoviePlayer.seekMovie(App.readLong());
                    break;

                case App.CHATS:
                    System.out.println("Got chats");

                    //Read incoming chats
                    long len = App.readLong();
                    System.out.println("Size of message was " + len);

                    if(len != 0) {
                        String[] incoming = new String(App.read((int)len)).split("\0");
                        System.out.println("arr len: " + incoming.length);
                    
                        //Append these messages to the old ones
                        Collections.addAll(App.chats, incoming);
                    }
                
                    MoviePlayer.updateChats();
                    break;

                default:
                    System.out.println("Unknown message type in control");
                    break;
            }
        }
    }
}
