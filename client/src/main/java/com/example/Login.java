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
            Text errMsg = new Text();
            errMsg.setFill(Color.RED);
            
            if(App.userName.length() == 0) {
                errMsg.setText("Please provide user name");
                info.getChildren().removeAll(info.getChildren());
                info.getChildren().addAll(tf, errMsg);
                System.out.println("ERROR: Please provide valid user name");
            }
            else if(App.userName.length() > 20 || App.userName.contains(":")) {
                errMsg.setText("Username ≥ 20 characters and/or contains ':' ");
                info.getChildren().removeAll(info.getChildren());
                info.getChildren().addAll(tf, errMsg);
                System.out.println("ERROR: username should be less than 20 characters and not contain :");
            } else {
                System.out.println("Username is " + App.userName);
                sendHello(App.userName);
            } 
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
        byte[] hello = new byte[21];
        hello[0] = App.HELLO;

        for(int i = 0; i < userName.length(); i++) {
            hello[i + 1] = (byte) userName.charAt(i);
        }

        hello[userName.length() + 1] = '\0';
        App.write(hello, 21);
    }
}
