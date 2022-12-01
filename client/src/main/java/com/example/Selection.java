package com.example;

import java.util.LinkedList;
import java.util.List;

import javafx.collections.FXCollections;
import javafx.collections.ObservableList;
import javafx.geometry.Pos;
import javafx.scene.*;
import javafx.scene.control.Button;
import javafx.scene.control.ListView;
import javafx.scene.control.SelectionMode;
import javafx.scene.layout.*;
import javafx.scene.paint.Color;

public class Selection {

    /*
     * Builds the login screen
     */
    public static Scene getScreen() {
        ObservableList<String> names = FXCollections.observableArrayList(App.movies);
        ListView<String> listView = new ListView<String>(names);
        listView.getSelectionModel().setSelectionMode(SelectionMode.SINGLE);
        listView.setFocusTraversable(false);
        listView.setMaxWidth(300);
        listView.setStyle("-fx-background-color: #90A4AE; -fx-font-size:14; ");

        Button button = new Button("Vote");
        button.setOnAction(e -> {

            //Get selected item
            char index = (char) listView.getSelectionModel().getSelectedIndex();
            System.out.println("Voted for movie of index " + (char) (index + 48));

            //Send movie selection to server
            App.write(new char[]{App.VOTE, index}, 2);
            System.out.println("Awaiting selected movie info..");
        });

        //Constructs them 
        VBox box = new VBox(listView, button);
        box.setSpacing(50);
        box.setAlignment(Pos.CENTER);
        box.setBackground(new Background(
            new BackgroundFill(Color.WHITE, null, null)
        ));

        return new Scene(box, App.SCREEN_W, App.SCREEN_H);
    }
}
