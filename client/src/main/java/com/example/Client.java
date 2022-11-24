package com.example;
import java.net.*;
import java.io.*;
import java.util.*;

import javafx.application.Application;
import javafx.scene.*;
import javafx.scene.image.ImageView;
import javafx.scene.layout.GridPane;
import javafx.scene.layout.VBox;
import javafx.scene.media.Media;
import javafx.scene.media.MediaPlayer;
import javafx.scene.media.MediaView;
import javafx.stage.Stage;

public class Client extends Application {

    @Override
    public void start(Stage primaryStage) throws Exception {
        GridPane root = new GridPane();
        Scene scene = new Scene(root);

        primaryStage.setScene(scene);
        primaryStage.setMaximized(true);
        primaryStage.show();

        Media media = new Media("http://localhost:9033/video.mp4");
        MediaPlayer mediaPlayer = new MediaPlayer(media);
        mediaPlayer.setAutoPlay(true);

        MediaView mediaView = new MediaView(mediaPlayer);
        mediaView.setFitHeight(500);
        mediaView.setFitWidth(500);

        root.getChildren().add(mediaView);
        mediaPlayer.play();
    }

    public static void main(String[] args) throws Exception{
       
        Application.launch(args);
    }
    
}
