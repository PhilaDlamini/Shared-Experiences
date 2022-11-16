package com.example;

import javafx.application.Application;
import javafx.scene.Parent;
import javafx.scene.Scene;
import javafx.scene.layout.StackPane;
import javafx.stage.Stage;
import java.io.IOException;

/**
 * JavaFX App
 */
public class App extends Application {

    public static final int VIDEO_WIDTH = 600;
    public static final int VIDEO_HEIGHT = 400;

    @Override
    public void start(final Stage primaryStage) throws IOException {
		primaryStage.setTitle("NextChat");
        primaryStage.setMaximized(true);
		primaryStage.setScene(Login.getScreen(primaryStage));
		primaryStage.show();
    }

    public static void main(String[] args) {
        Application.launch(args);
    }

}