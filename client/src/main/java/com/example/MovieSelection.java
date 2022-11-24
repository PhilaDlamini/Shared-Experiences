package com.example;

import java.io.IOException;
import java.util.Arrays;
import java.util.LinkedList;
import java.util.List;
import java.util.concurrent.TimeUnit;

import javafx.application.Platform;
import javafx.collections.FXCollections;
import javafx.collections.ObservableList;
import javafx.geometry.HPos;
import javafx.geometry.Orientation;
import javafx.geometry.Pos;
import javafx.scene.*;
import javafx.scene.control.Button;
import javafx.scene.control.ListView;
import javafx.scene.control.MultipleSelectionModel;
import javafx.scene.control.ProgressBar;
import javafx.scene.control.SelectionMode;
import javafx.scene.control.TextField;
import javafx.scene.layout.*;
import javafx.scene.paint.Color;
import javafx.scene.text.Text;
import javafx.stage.Stage;

public class MovieSelection {

    private static boolean selected = false;
    private static boolean showProgressBar = false;
    private static String selectedMovieName = "";

    /*
     * Builds the login screen
     */
    public static Scene getScreen(Stage primaryStage, String[] movies, String userName) {
        ObservableList<String> names = FXCollections.observableArrayList(movies);
        ListView<String> listView = new ListView<String>(names);
        listView.getSelectionModel().setSelectionMode(SelectionMode.SINGLE);
        listView.setFocusTraversable(false);

        Button button = new Button("Vote");
        button.setOnAction(e -> {

            //Get selected item
            char index = (char) listView.getSelectionModel().getSelectedIndex();

            System.out.println("Selected item " + selectedMovieName + " is of index " + (char) (index + 48));

            //Send movie selection to server
            App.out.write(new char[]{App.VOTE, index});
            App.out.flush();

            //Move to progress screen
            showProgressBar = true;
            primaryStage.setScene(MovieSelection.getScreen(primaryStage, movies, userName));

            // //Await for movie selection. When done, go to movie player
            Platform.runLater(new Runnable() {
                @Override
                public void run() {
                    System.out.println("Awaiting selected movie info..");
                    byte[] buffer = App.read(2);
                       
                    //Move on the the movie player
                    System.out.println("Selected movie: " + buffer[1]);
                    selectedMovieName = movies[buffer[1]]; 

                    System.out.println("MovieSelection: userName " + userName + " , movieName: " + selectedMovieName);
                    primaryStage.setScene(Home.getScreen(primaryStage, userName, selectedMovieName));

                }
            });
           
        });

        /*TODO: progessbar does not update while waiting for movie selection 
         *see (https://stackoverflow.com/questions/27090426/progressbar-in-javafx-does-not-update-in-onaction-block)
         */

        //Constructs them 
        VBox box = new VBox(listView, button);
        box.setSpacing(100);
        box.setAlignment(Pos.CENTER);
        box.setBackground(new Background(
            new BackgroundFill(Color.WHITE, null, null)
        ));

        Scene selectScene = new Scene(box, App.SCREEN_W, App.SCREEN_H);

        //The progress bar scene
        ProgressBar bar = new ProgressBar();
        bar.setPrefHeight(20);
        bar.setPrefWidth(App.SCREEN_W - 100);
        Text a = new Text(selected ? "Loading.." : "Waiting for movie selection ");

        bar.setProgress(ProgressBar.INDETERMINATE_PROGRESS);
        VBox b = new VBox(bar, a);
        b.setSpacing(20);
        b.setAlignment(Pos.CENTER);
        VBox c;
        Text selectedMovie = new Text("Movie selected: " + selectedMovieName);
        if(selected) c = new VBox(b, selectedMovie);
        else c = new VBox(b);
        c.setSpacing(100);
        c.setAlignment(Pos.CENTER);

        Scene progressScene = new Scene(c, App.SCREEN_W, App.SCREEN_H);
        return showProgressBar ? progressScene : selectScene;
    }
}
