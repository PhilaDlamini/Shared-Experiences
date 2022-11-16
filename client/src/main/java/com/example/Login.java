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

    public static Scene getScreen(Stage primarStage) {

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

            String userName = tf.getText();
            System.out.println("User is " + userName);

            //Go to the home screen
            primarStage.setScene(Home.getScreen(primarStage, userName));
        });

        //Constructs them 
        VBox box = new VBox(text, tf, button);
        box.setSpacing(100);
        box.setAlignment(Pos.CENTER);
        box.setBackground(new Background(new BackgroundFill(Color.WHITE, null, null)));

        Scene scene = new Scene(box);
        return scene;
    }
}
