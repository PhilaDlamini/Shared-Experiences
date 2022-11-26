package com.example;

import java.util.Arrays;
import java.util.LinkedList;
import java.util.List;
import java.util.stream.Collectors;
import javafx.geometry.Insets;
import javafx.geometry.Pos;
import javafx.scene.Node;
import javafx.scene.Scene;
import javafx.scene.control.Button;
import javafx.scene.control.ScrollPane;
import javafx.scene.control.TextField;
import javafx.scene.control.ScrollPane.ScrollBarPolicy;
import javafx.scene.layout.Background;
import javafx.scene.layout.BackgroundFill;
import javafx.scene.layout.HBox;
import javafx.scene.layout.VBox;
import javafx.scene.paint.Color;
import javafx.scene.shape.Circle;
import javafx.scene.shape.Rectangle;
import javafx.scene.text.Font;
import javafx.scene.text.Text;
import javafx.util.Duration;
import javafx.scene.media.Media;
import javafx.scene.media.MediaPlayer;
import javafx.scene.media.MediaView;
import javafx.scene.media.MediaPlayer.Status;

public class MoviePlayer {
    private static MediaView mediaView = new MediaView();
    private static MediaPlayer mediaPlayer;

    public static Scene getScreen() {
        return new Scene(screen(), App.SCREEN_W, App.SCREEN_H);
    }

   //Builds the video player 
    private static VBox player() {
    
        System.out.println("movie name is " + App.movieName);
        //Instantiating the Media player class 
        String url = "http://localhost:"  + App.PORT + "/" + App.movieName;
        Media media = new Media(url);
        mediaPlayer = new MediaPlayer(media); 
        mediaPlayer.setOnError(() -> {
            System.out.println("media error "+ mediaPlayer.getError().toString());

            //what do do here? Perhaps have the client rejoin??
        });
        
        mediaPlayer.setOnReady(() -> {
            //Send Downloaded message
            App.write(new char[]{App.DOWNLOADED}, 1);
            mediaView.setMediaPlayer(mediaPlayer);
            
        }); 

        mediaPlayer.setOnEndOfMedia(() -> {
            System.out.println("finished playing media");

            //Send END_MOVIE message
            App.write(new char[]{App.END_MOVIE}, 1);
        });

        mediaView.setFitHeight(App.VIDEO_HEIGHT);
        mediaView.setFitWidth(App.VIDEO_WIDTH);

        Rectangle clip = new Rectangle(
            mediaView.getFitWidth(), mediaView.getFitHeight()
        );
        clip.setArcWidth(50);
        clip.setArcHeight(50);
        mediaView.setClip(clip);

        //The controls 
        Button seekBack = new Button("<");
        seekBack.setOnAction(e -> {
            long seconds = (long) mediaPlayer.getCurrentTime().toSeconds() + 10;
            sendSeek(App.userName, seconds);
        });

        Button seekForward = new Button(">");
        seekForward.setOnAction(e -> {
            long seconds = (long) mediaPlayer.getCurrentTime().toSeconds() - 10;
            sendSeek(App.userName, seconds);
        });

        Button playPause = new Button("||");
        playPause.setOnAction(e -> {

            //Send toggle to server
            sendToggle(App.userName);
        });

        Text movieName = new Text(App.movieName);
        HBox controls = new HBox(seekBack, playPause, seekForward);
        controls.setSpacing(20);
        controls.setAlignment(Pos.CENTER);
        HBox all = new HBox(movieName, controls);
        all.setAlignment(Pos.CENTER_LEFT);
        all.setSpacing(120);

        //The vertical component'
        VBox box = new VBox(mediaView, all);
        box.setSpacing(20);
        return box;
    }

    //Starts playing the media
    public static void start() {
        System.out.println("Movie will start playing at " + App.startDuration + " seconds");
        mediaPlayer.seek(Duration.seconds(App.startDuration));
        mediaPlayer.play();
    }

    //Seeks the mediaPlayer
    public static void seekMovie(long duration) {
        mediaPlayer.seek(Duration.seconds(duration));
    }

    //Toggles player
    public static void togglePlayer() {
        if(mediaPlayer.getStatus() == Status.PLAYING) 
            mediaPlayer.pause();
        else mediaPlayer.play();
    }

    //Displays the mediaplayer
    public static VBox screen() {
        //The user name 
        Circle icon = new Circle(21, Color.GRAY);
        Text text = new Text(App.userName);
        text.setFont(Font.font("Raleway", 14));
        Button button = new Button("Logout");
        button.setOnAction(e -> {
            sendGoodbye(App.userName);
            System.out.println("Logout pressed");
            mediaPlayer.dispose();
            App.switchToScreen(App.LOGIN);
        });

        //Displays user name at top 
        HBox profile = new HBox(icon, text);
        profile.setAlignment(Pos.CENTER);
        profile.setSpacing(10);
        HBox hbox = new HBox(profile, button);
        hbox.setAlignment(Pos.CENTER_LEFT);
        hbox.setSpacing(20); //TODO: this is not the right way to do it

        //The messages
        List<String> messages = new LinkedList<>(
            Arrays.asList(
            "Hey", "OMG I agree!", "So sad to see thing happening tho",
            "One thing I've always said this movie is great. Imagine!", 
            "Hey", "OMG I love this movie!", "So scary", "I love it!",
            "Hey", "OMG I love this movie!", "So scary", "I love it!", 
            "One thing I've always said this movie is great. Imagine!", 
            "Hey", "OMG I love this movie!", "So scary", "I love it!",
            "Hey", "OMG I love this movie!", "So scary", "I love it!", 
            "One thing I've always said this movie is great. Imagine!", 
            "Hey", "OMG I love this movie!", "So scary", "I love it!",
            "Hey", "OMG I love this movie!", "So scary", "I love it!", 
            "One thing I've always said this movie is great. Imagine!", 
            "Hey", "OMG I love this movie!", "So scary", "I love it!",
            "Hey", "OMG I love this movie!", "So scary", "I love it!",  
            "Hey", "Lmao totally agree! This can definetely go better. There's so much to figure out", "So scary", "I love it!")
        );

        HBox vid_mes_area = new HBox(30, player(), messages(messages));

        //The send message area
        TextField tf = new TextField();
        tf.setMinWidth(App.VIDEO_WIDTH);
        tf.setMinHeight(50);
        tf.setPromptText("Type message");
        tf.setFocusTraversable(false);
        tf.setStyle("-fx-border-radius: 20; -fx-background-radius: 20;");

        Button sendMesage = new Button("Send");
        sendMesage.setOnAction(e -> {
            System.out.println("Sending message: " + tf.getText());
        });
        HBox send = new HBox(30, tf, sendMesage);
        send.setAlignment(Pos.CENTER_LEFT);

        //The column of all elements
        VBox vbox = new VBox(10);
        vbox.setAlignment(Pos.TOP_LEFT);
        vbox.getChildren().addAll(hbox, vid_mes_area, send); 
        for(Object n: vbox.getChildren().toArray()) {
            VBox.setMargin((Node) n, new Insets(24, 24, 0, 24));
        }

        vbox.setBackground(new Background(new BackgroundFill(Color.WHITE, null, null)));
        return vbox;
    }

    //Returns all the messages 
    private static ScrollPane messages(List<String> messages) {
        VBox scroll = new VBox();
        scroll.setBackground(new Background(new BackgroundFill(Color.WHITE, null, null))); 
        scroll.getChildren().addAll(
            messages
            .stream()
            .map(m -> {

                //The profile pic and text message 
                Text text = new Text(m);
                text.setWrappingWidth(200);
                return text; 
            })
            .collect(Collectors.toList()));

        ScrollPane pane = new ScrollPane();
        pane.setBackground(new Background(new BackgroundFill(Color.WHITE, null, null))); 
        pane.setStyle("-fx-background-color:transparent;");
        pane.setVbarPolicy(ScrollBarPolicy.NEVER);
        pane.setPrefHeight(App.VIDEO_HEIGHT);
        pane.setFocusTraversable(false);
        pane.setContent(scroll);
        return pane;
    }

    //Sends goobye message to server 
    private static void sendGoodbye(String userName) {
        char[] hello = new char[21];
        hello[0] = App.GOODBYE;

        for(int i = 0; i < userName.length(); i++) {
            hello[i + 1] = userName.charAt(i);
        }

        hello[userName.length() + 1] = '\0';
        App.write(hello, 21);
    }

    //Sends the toggle message to server 
    private static void sendToggle(String userName) {
    char[] toggle = new char[21];
    toggle[0] = App.TOGGLE;

    for(int i = 0; i < userName.length(); i++) {
        toggle[i + 1] = userName.charAt(i);
    }

    toggle[userName.length() + 1] = '\0';
    App.write(toggle, 21);
}

    //Sends the toggle message to server 
    private static void sendSeek(String userName, long s) {

        System.out.println("Sent seek for " + s);

        //Write the type and seconds
        App.write(new char[]{App.SEEK}, 1);
        App.writeLong(s);

        //Write the name
        char[] seek = new char[20];
        for(int i = 0; i < userName.length(); i++) {
            seek[i] = userName.charAt(i);
        }

        seek[userName.length()] = '\0';
        App.write(seek, 20);
    }
}