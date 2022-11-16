package com.example;

import java.io.File;
import java.util.Arrays;
import java.util.Collection;
import java.util.LinkedList;
import java.util.List;
import java.util.stream.Collector;
import java.util.stream.Collectors;

import javafx.application.Application;
import javafx.geometry.Insets;
import javafx.geometry.Orientation;
import javafx.geometry.Pos;
import javafx.scene.Group;
import javafx.scene.Node;
import javafx.scene.Scene;
import javafx.scene.control.Button;
import javafx.scene.control.ContentDisplay;
import javafx.scene.control.ScrollPane;
import javafx.scene.control.TextField;
import javafx.scene.control.ScrollPane.ScrollBarPolicy;
import javafx.scene.image.Image;
import javafx.scene.image.ImageView;
import javafx.scene.layout.Background;
import javafx.scene.layout.BackgroundFill;
import javafx.scene.layout.Border;
import javafx.scene.layout.BorderStroke;
import javafx.scene.layout.CornerRadii;
import javafx.scene.layout.FlowPane;
import javafx.scene.layout.HBox;
import javafx.scene.layout.VBox;
import javafx.scene.paint.Color;
import javafx.scene.shape.Circle;
import javafx.scene.shape.Rectangle;
import javafx.scene.text.Font;
import javafx.scene.text.Text;
import javafx.stage.Stage;
import javafx.util.Duration;
import javafx.scene.media.Media;
import javafx.scene.media.MediaPlayer;
import javafx.scene.media.MediaView;
import javafx.scene.media.MediaPlayer.Status;

public class Home {

    public static Scene getScreen(Stage primarStage, String userName) {

        //The user name 
        Circle icon = new Circle(21, Color.GRAY);
        Text text = new Text(userName);
        text.setFont(Font.font("Raleway", 14));
        Button button = new Button("Logout");
        button.setOnAction(e -> {
            primarStage.setScene(Login.getScreen(primarStage));
            System.out.println("Button pressed");
        });

        //Displays user name at top 
        HBox profile = new HBox(icon, text);
        profile.setAlignment(Pos.CENTER);
        profile.setSpacing(10);
        HBox hbox = new HBox(profile, button);
        hbox.setAlignment(Pos.CENTER_LEFT);
        hbox.setSpacing(1150); //TODO: this is not the right way to do it

        //The video player (giant box for now)
        Rectangle video = new Rectangle(App.VIDEO_WIDTH, App.VIDEO_HEIGHT, Color.DIMGRAY);
        video.setArcWidth(50);
        video.setArcHeight(50);


        //The messages
        List<String> messages = new LinkedList<>(
            Arrays.asList(
            "Hey", "OMG I agree!", "So sad to see thing happening tho",
            "One thing I've always said this movie is great. Imagine!", 
            "Hey", "OMG I love this movie!", "So scary", "I love it!",
            "Hey", "OMG I love this movie!", "So scary", "I love it!", 
            "Hey", "Lmao totally agree! This can definetely go better. There's so much to figure out", "So scary", "I love it!")
        );

        HBox vid_mes_area = new HBox(30, getVideoPlayer(), scrollView(messages));

        //The send message area
        TextField tf = new TextField();
        tf.setMinWidth(App.VIDEO_WIDTH);
        tf.setMinHeight(150);
        tf.setPromptText("Type message");
        tf.setFocusTraversable(false);
        tf.setStyle("-fx-border-radius: 20; -fx-background-radius: 20;");


        Image image = new Image("file:///C:/Users/phila/cs112/client/demo/src/main/java/com/example/send.png");
        ImageView img = new ImageView(image);
        img.setFitHeight(32);
        img.setFitWidth(32);
        Button sendMesage = new Button("Send");
        // sendMesage.setContentDisplay(ContentDisplay.GRAPHIC_ONLY);
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
        Scene scene = new Scene(vbox);
        return scene;
    }

    //Returns the scrooll view of messages 
    private static ScrollPane scrollView(List<String> messages) {
        VBox scroll = new VBox();
        scroll.setBackground(new Background(new BackgroundFill(Color.WHITE, null, null))); 
        scroll.getChildren().addAll(
            messages
            .stream()
            .map(m -> {

                //The profile pic and text message 
                Circle profile = new Circle(20, Color.DIMGREY);
                Text text = new Text(m);
                HBox box = new HBox(10, profile, text);
                box.setAlignment(Pos.CENTER_LEFT);
                for(Object n: box.getChildren().toArray()) {
                    HBox.setMargin((Node) n, new Insets(10, 0, 0, 0));
                }
                return box; 
            })
            .collect(Collectors.toList()));

        ScrollPane pane = new ScrollPane();
        pane.setBackground(new Background(new BackgroundFill(Color.WHITE, null, null))); 
        pane.setStyle("-fx-background-color:transparent;");
        pane.setVbarPolicy(ScrollBarPolicy.NEVER);
        pane.setPrefWidth(500);
        pane.setPrefHeight(App.VIDEO_HEIGHT);
        pane.setFocusTraversable(false);
        pane.setContent(scroll);
        return pane;
    }

    //Builds the video player 
    public static VBox getVideoPlayer() {
    
        //Instantiating the Media player class 
        Media media =  new Media("file:///C:/Users/phila/cs112/client/demo/src/main/java/com/example/mkbhd.mp4");    
        MediaPlayer mediaPlayer = new MediaPlayer(media); 
        mediaPlayer.setOnError(()->
            System.out.println("media error"+mediaPlayer.getError().toString())
        );
        mediaPlayer.setOnReady(() -> System.out.println("REady to play")); 
        mediaPlayer.setAutoPlay(true);
        MediaView mediaView = new MediaView(mediaPlayer);  
        mediaView.setFitHeight(App.VIDEO_HEIGHT - 100);
        mediaView.setFitWidth(App.VIDEO_WIDTH);

        Rectangle clip = new Rectangle(
            mediaView.getFitWidth(), mediaView.getFitHeight()
        );
        clip.setArcWidth(50);
        clip.setArcHeight(50);
        mediaView.setClip(clip);
        Duration tensec = new Duration(5000);

        //The controls 
        Button seekBack = new Button("<");
        seekBack.setOnAction(e -> {
            mediaPlayer.seek(mediaPlayer.getCurrentTime().subtract(tensec));
        });

        Button seekForward = new Button(">");
        seekForward.setOnAction(e -> {
            mediaPlayer.seek(mediaPlayer.getCurrentTime().add(tensec));
        });

        Button playPause = new Button("||");
        playPause.setOnAction(e -> {
            if(mediaPlayer.getStatus() == Status.PLAYING) 
                mediaPlayer.pause();
            else mediaPlayer.play();
        });

        HBox controls = new HBox(seekBack, playPause, seekForward);
        controls.setSpacing(20);
        controls.setAlignment(Pos.CENTER);

        //The vertical component'
        VBox box = new VBox(mediaView, controls);
        box.setSpacing(20);
        return box;
    }
}