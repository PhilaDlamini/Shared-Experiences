package com.example;

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
                    App.movies = new String(App.read(800)).split("\0");
                    
                    for(String movie: App.movies)
                        System.out.println(movie);

                    //Go to movie selection
                    App.switchToScreen(App.SELECTION);
                    break;

                case App.MOVIE_SELECTED:
                    int selection = App.read(1)[0];
                    System.out.println("Winning movie: " + selection);
                    App.movieName = App.movies[selection]; 
                    break;
                
                case App.MOVIE_CONTENT:
                    //Switch to movie screen
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
                    long duration = App.readLong();
                    MoviePlayer.seekMovie(duration);
                    break;

                default:
                    System.out.println("Unknown message type in control");
                    break;
            }
        }
    }
}
