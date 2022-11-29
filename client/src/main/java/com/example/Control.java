package com.example;
import java.util.*;
import javafx.application.Platform;
import javafx.scene.media.MediaPlayer;

/*
 * The main control thread
 */
public class Control implements Runnable {
    public void run() {

        while(true) {
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
                
                case App.MOVIE_CONTENT:
                    byte[] movie = App.read((int) App.readLong()); //TODO: what if movie bytes are more than INT_MAX
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
                    System.out.print("Got toggle from server");
                    MoviePlayer.togglePlayer();
                    break;

                case App.SEEK_MOVIE:
                    System.out.println("Got seek from server");
                    MoviePlayer.seekMovie(App.readLong());
                    break;

                case App.CHATS:

                    //Read incoming chats
                    long len = App.readLong();
                    String[] incoming = new String(App.read((int)len )).split("\0");
                    
                    //Append these messages to the old ones
                    Collections.addAll(App.chats, incoming);
                    MoviePlayer.updateChats();
                    break;

                default:
                    System.out.println("Unknown message type in control");
                    break;
            }
        }
    }
}
