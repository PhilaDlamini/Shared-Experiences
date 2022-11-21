package com.example;

import javafx.geometry.HPos;
import javafx.geometry.Orientation;
import javafx.geometry.Pos;
import javafx.scene.*;
import javafx.scene.control.Button;
import javafx.scene.control.TextField;
import javafx.scene.layout.*;
import javafx.scene.paint.Color;
import javafx.scene.text.Text;
import javafx.stage.Stage;

public class Login {

    private static String userName;

    public static Scene getScreen(Stage stage) {

        Text text = new Text("NextChat");
        text.setStyle("    -fx-font: 70px Tahoma;    ");

        //Text field and button 
        TextField tf = new TextField();
        tf.setPromptText("Username");
        tf.setFocusTraversable(false);
        tf.setMaxWidth(250);

        Button button = new Button("Join");
        button.setFocusTraversable(false);
        button.setOnAction(e -> {
            userName = tf.getText();

            //Ensure name is less than 20 chars
            System.out.println("User is " + userName);

            //Send username
            sendHello(userName);
            processHello(stage);
        });

        //Constructs them 
        VBox box = new VBox(text, tf, button);
        box.setSpacing(80);
        box.setAlignment(Pos.CENTER);
        box.setBackground(new Background(new BackgroundFill(Color.WHITE, null, null)));

        Scene scene = new Scene(box, App.SCREEN_W, App.SCREEN_H);
        return scene;
    }

    //Send hello message to server 
    private static void sendHello(String userName) {
        char[] hello = new char[21];
        hello[0] = App.HELLO;

        for(int i = 0; i < userName.length(); i++) {
            hello[i + 1] = userName.charAt(i);
        }

        hello[userName.length() + 1] = '\0';
        App.out.write(hello, 0, 21);
        App.out.flush();
    }

    //Process response
    private static void processHello(Stage stage) {
        
        //Read in response 
        int response = App.read(1)[0];

        if(response == App.MOVIES) {

            //Get the movie list 
            String[] movies = new String(App.read(800)).split("\0");

            //Print out for good measure
            for(String movie: movies)
                System.out.println(movie);

            stage.setScene(MovieSelection.getScreen(stage, movies, userName));

        } else if (response == App.MOVIE_CONTENT) {

            //Go to the movie screen 
        } else if (response == App.ERROR) {

        }
    }
}
