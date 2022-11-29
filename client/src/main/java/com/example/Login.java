package com.example;

import javafx.geometry.Pos;
import javafx.scene.*;
import javafx.scene.control.Button;
import javafx.scene.control.TextField;
import javafx.scene.layout.*;
import javafx.scene.paint.Color;
import javafx.scene.text.Text;

public class Login {

    private static VBox info;

    //The error message to show to users 
    private static Text errMsg = new Text("Username ≥ 20 characters and/or contains ':' ");
    
    public static Scene getScreen() {

        Text text = new Text("NextChat");
        text.setStyle("    -fx-font: 70px Tahoma;    ");

        //Text field and button 
        TextField tf = new TextField();
        tf.setPromptText("Username");
        tf.setFocusTraversable(false);
        tf.setMaxWidth(250);
        info = new VBox(tf);
        info.setAlignment(Pos.TOP_LEFT);
        info.setSpacing(5);
        VBox wrapper = new VBox(info);
        wrapper.setAlignment(Pos.CENTER);
        wrapper.setMaxWidth(250);

        Button button = new Button("Join");
        button.setFocusTraversable(false);
        button.setOnAction(e -> {
            App.userName = tf.getText();

            if(App.userName.length() < 20 && !App.userName.contains(":")) {
                //Ensure name is less than 20 chars
                System.out.println("Username is " + App.userName);
                sendHello(App.userName);
            } else {
                System.out.println("ERROR logging in: username should be less than 20 characters and not contain :");
                
                //Show error message
                errMsg.setFill(Color.RED);
                info.getChildren().addAll(errMsg);
            } //TODO: show this on the screen
        });

        //Constructs them 
        VBox box = new VBox(text, wrapper, button);
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
        App.write(hello, 21);
    }
}
