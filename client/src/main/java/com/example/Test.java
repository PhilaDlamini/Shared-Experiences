package com.example;
import javafx.application.Application;
import javafx.concurrent.Task;
import javafx.geometry.Insets;
import javafx.scene.Scene;
import javafx.scene.control.ProgressBar;
import javafx.scene.layout.StackPane;
import javafx.stage.Stage;

/*Used for trying out things */
public class Test extends Application {
    private static final double EPSILON = 0.0000005;
    @Override
    public void start(Stage stage) throws Exception {
        final Task<Void> task = new Task<Void>() {
            final int N_ITERATIONS = 4400;

            @Override
            protected Void call() throws Exception {
                for (int i = 0; i < N_ITERATIONS; i++) {
                    updateProgress(i + 1, N_ITERATIONS);
                    // sleep is used to simulate doing some work which takes some time....
                    Thread.sleep(10);
                }

                return null;
            }
        };

        final ProgressBar progress = new ProgressBar();
        progress.progressProperty().bind(
                task.progressProperty()
        );
        // color the bar green when the work is complete.
        progress.progressProperty().addListener(observable -> {
            if (progress.getProgress() >= 1 - EPSILON) {
                progress.setStyle("-fx-accent: forestgreen;");
            }
        });

        // layout the app
        final StackPane layout = new StackPane(progress);
        layout.setPadding(new Insets(10));
        stage.setScene(new Scene(layout));
        stage.show();

        final Thread thread = new Thread(task, "task-thread");
        thread.setDaemon(true);
        thread.start();
    }

    public static void main(String[] args) {
        launch(args);
    }
}
