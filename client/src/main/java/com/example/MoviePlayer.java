package com.example;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOError;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.nio.ByteBuffer;
import java.nio.file.Files;
import java.util.ArrayList;
import java.util.stream.Collectors;
import javafx.application.Platform;
import javafx.concurrent.Task;
import javafx.geometry.Insets;
import javafx.geometry.Pos;
import javafx.scene.Node;
import javafx.scene.Scene;
import javafx.scene.control.Button;
import javafx.scene.control.ProgressBar;
import javafx.scene.control.ScrollPane;
import javafx.scene.control.TextField;
import javafx.scene.control.ScrollPane.ScrollBarPolicy;
import javafx.scene.image.Image;
import javafx.scene.image.ImageView;
import javafx.scene.layout.Background;
import javafx.scene.layout.BackgroundFill;
import javafx.scene.layout.Border;
import javafx.scene.layout.BorderStroke;
import javafx.scene.layout.HBox;
import javafx.scene.layout.StackPane;
import javafx.scene.layout.VBox;
import javafx.scene.paint.Color;
import javafx.scene.shape.Circle;
import javafx.scene.shape.Rectangle;
import javafx.scene.text.Font;
import javafx.scene.text.FontWeight;
import javafx.scene.text.Text;
import javafx.stage.FileChooser;
import javafx.stage.FileChooser.ExtensionFilter;
import javafx.util.Duration;
import javafx.scene.media.Media;
import javafx.scene.media.MediaPlayer;
import javafx.scene.media.MediaView;
import javafx.scene.media.MediaPlayer.Status;

public class MoviePlayer {
    private static VBox scroll;
    private static MediaView mediaView = new MediaView();
    private static ScrollPane pane;
    private static MediaPlayer mediaPlayer;
    private static TextField textInput;
    private static boolean imageSelected = false;
    private static File imageFile;

    //The progress task 
    private static Task<Void> progressTask;
    
    public static Scene getScreen() {
        Scene scene = new Scene(screen(), App.SCREEN_W, App.SCREEN_H);
        scene.setFill(Color.WHITE);
        return scene;
    }

   //Builds the video player 
    private static VBox player() {
        System.out.println("movie name is " + App.movieName);
        
        //Instantiating the Media player class 
        final Media media = new Media(App.movieFile.toURI().toString());
        mediaPlayer = new MediaPlayer(media); 
        mediaPlayer.setOnError(() -> {
            System.out.println("media error "+ mediaPlayer.getError().toString());

            //Try this?? seeking too soon??
            // mediaPlayer = new MediaPlayer(media);
            // mediaPlayer.setOnPlaying(() ->{
            //     mediaPlayer.seek(Duration.seconds(App.startDuration)); //seeking too soon?        
            //     mediaPlayer.setOnPlaying(null);
            // });
            // mediaView.setMediaPlayer(mediaPlayer);
            // mediaPlayer.play();
            

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
            mediaPlayer.dispose();
        });

        mediaView.setFitHeight(App.VIDEO_HEIGHT);
        mediaView.setFitWidth(App.VIDEO_WIDTH);

        Rectangle clip = new Rectangle(
            mediaView.getFitWidth(), mediaView.getFitHeight()
        );
        clip.setArcWidth(30);
        clip.setArcHeight(30);
        mediaView.setClip(clip);
        //TODO: figure out why this clip does not work for bottom

        //The controls 
        Button seekBack = new Button("<");
        seekBack.setOnAction(e -> {
            long seconds = (long) mediaPlayer.getCurrentTime().toSeconds() - 10;
            sendSeek(App.userName, Math.max(0, seconds));
        });

        Button seekForward = new Button(">");
        seekForward.setOnAction(e -> {
            long seconds = (long) mediaPlayer.getCurrentTime().toSeconds() + 10;
            System.out.println("sent seek for: " + seconds);
            sendSeek(App.userName, seconds);
            
        });

        Button playPause = new Button("||");
        playPause.setOnAction(e -> {
            sendToggle(App.userName);
        });

        //moviename and controls
        Text movieName = new Text(App.movieName);
        HBox controls = new HBox(seekBack, playPause, seekForward);
        controls.setSpacing(20);
        controls.setAlignment(Pos.CENTER);
        HBox nameAndControls = new HBox(movieName, controls);
        nameAndControls.setAlignment(Pos.CENTER_LEFT);
        nameAndControls.setSpacing(120);

        //mediaview and progress
        //The media progress
        final ProgressBar mediaProgress = new ProgressBar(0.0);
        mediaProgress.setPrefWidth(App.VIDEO_WIDTH);
        mediaProgress.setPrefHeight(10);

        progressTask = new Task<Void>() {

            @Override
            protected Void call() throws Exception {
                while (mediaPlayer.getStatus() != Status.DISPOSED) {
                    double curr = mediaPlayer.getCurrentTime().toSeconds();
                    double end = media.getDuration().toSeconds();
                    updateProgress(curr, end);
                }

                return null;
            }
        };
        mediaProgress.progressProperty().bind(progressTask.progressProperty());
        VBox mediaAndProgress = new VBox(mediaView, mediaProgress);
        mediaAndProgress.setAlignment(Pos.CENTER_LEFT);

        //The vertical component'
        VBox box = new VBox(mediaAndProgress, nameAndControls);
        box.setSpacing(10);
        return box;
    }

    //Starts playing the media
    public static void start() {
        System.out.println("Movie will start playing at " + App.startDuration + " seconds");
        mediaPlayer.seek(Duration.seconds(App.startDuration));      
        mediaPlayer.play();

        //Start the progress thread
        final Thread thread = new Thread(progressTask);
        thread.setDaemon(true);
        thread.start();
    }

    //Seeks the mediaPlayer
    public static void seekMovie(long duration) {
        System.out.println("Movie will seek to " + duration);
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
        Circle icon = new Circle(21, Color.DARKCYAN);
        Text letter = new Text("" + (char) App.userName.charAt(0));
        letter.setFont(Font.font("Verdana", FontWeight.BOLD,null, 16));
        letter.setFill(Color.WHITE);
        StackPane pane = new StackPane(icon, letter);
        pane.setAlignment(Pos.CENTER);
        Text text = new Text(App.userName);
        text.setFont(Font.font("Raleway", 14));
        Button button = new Button("Logout");
        button.setOnAction(e -> {
            App.chats = new ArrayList<>();
            sendGoodbye(App.userName);
            System.out.println("Logout pressed");
            mediaPlayer.dispose();
            App.close();
        });

        //Displays user name at top 
        HBox profile = new HBox(pane, text);
        profile.setAlignment(Pos.CENTER);
        profile.setSpacing(10);
        HBox hbox = new HBox(profile, button);
        hbox.setAlignment(Pos.CENTER_LEFT);
        hbox.setSpacing(50); //TODO: this is not the right way to do it

        //The messages
        HBox vid_mes_area = new HBox(15, player(), messages());
        vid_mes_area.setBackground(new Background(new BackgroundFill(Color.WHITE, null, null)));

        //The send message area
        textInput = new TextField();
        textInput.setFont(Font.font(13));
        textInput.setMinWidth(App.VIDEO_WIDTH);
        textInput.setMinHeight(50);
        textInput.setPromptText("Type message");
        textInput.setFocusTraversable(false);
        textInput.setStyle("-fx-border-radius: 20; -fx-background-radius: 20;");
        textInput.setPadding(new Insets(0, 40, 0, 8));
        StackPane inputs = new StackPane(textInput, imagePicker());
        inputs.setAlignment(Pos.CENTER_RIGHT);
        
        Button sendMesage = new Button("Send");
        sendMesage.setOnAction(e -> {
            System.out.println("Sending message: " + textInput.getText());

            //Ensure message is less than 400 char
            String message = textInput.getText();
            int n = message.length();
            if(n > 0 && n < 400) {
                sendChat(App.userName, message);
            } else System.out.println("Chat not set: message size == 0 or >= 400 chars");

            //Also send the image 
            if(imageSelected)
                sendImage();
            
            //Reset the field
            textInput.setText("");
        });

        HBox send = new HBox(15, inputs, sendMesage);
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

    //Picks an image 
    private static HBox imagePicker() {

        //The file chooser
        final FileChooser chooser = new FileChooser();

        chooser.getExtensionFilters().addAll(
            new ExtensionFilter("Image Files", "*.png", "*.jpg", "*.heif"));
        chooser.setTitle("Select Image");


        //The circle
        Circle circle = new Circle(10, Color.web("#78909C"));
        Text plus = new Text("+");
        plus.setFont(Font.font("Verdana", FontWeight.BOLD,null, 16));
        plus.setFill(Color.WHITE);
        Text cross = new Text("x");
        cross.setFont(Font.font("Verdana", FontWeight.BOLD,null, 13));
        cross.setFill(Color.WHITE);

        // The pane of all things
        HBox row = new HBox();
        row.setSpacing(8);
        row.setAlignment(Pos.CENTER);
        row.setPadding(new Insets(0, 8, 0, 0));
        row.setMaxWidth(42);

        //Holds the image
        ImageView image = new ImageView();
        image.setFitHeight(24);
        image.setFitWidth(24);
        image.setOnMouseClicked(e -> {
            //TODO: expand image for view to see 
        });

        StackPane pane = new StackPane(circle, plus);
        pane.setAlignment(Pos.CENTER);
        pane.setMaxWidth(circle.getRadius());
        pane.setMaxHeight(circle.getRadius());
        pane.setOnMouseClicked(e -> {
            System.out.println("Moused pressed!");

            if(!imageSelected) {
                imageFile = chooser.showOpenDialog(App.getStage());
            
                if(imageFile != null) {

                    //Displaly it
                    image.setImage(new Image(imageFile.toURI().toString()));
                    row.getChildren().add(0, image);

                    //Change to cross
                    pane.getChildren().remove(plus);
                    pane.getChildren().add(cross);
                    imageSelected = true;
                    textInput.setPadding(new Insets(0, 64, 0, 8));

                    System.out.println("Image selected!");
                }

            } else {

                //remove the image and switch to the plus
                row.getChildren().remove(image);
                pane.getChildren().remove(cross);
                pane.getChildren().add(plus);
                textInput.setPadding(new Insets(0, 40, 0, 8));
                imageSelected = false;
                System.out.println("Image removed");
            }
        });
        row.getChildren().addAll(pane);


        return row;
    } 

    //Returns all the messages 
    private static ScrollPane messages() {
        scroll = new VBox();
        scroll.setAlignment(Pos.CENTER);
        scroll.setBackground(new Background(new BackgroundFill(Color.WHITE, null, null))); 
        updateChats();

        pane = new ScrollPane();
        pane.setStyle("-fx-background: rgb(255,255,255);\n -fx-background-color: rgb(255,255,255)");
        pane.setVbarPolicy(ScrollBarPolicy.NEVER);
        pane.setPrefHeight(App.VIDEO_HEIGHT);
        pane.setFocusTraversable(false);
        pane.setContent(scroll);
        pane.setVvalue(pane.getVmax());
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

    //Sends the chat message 
    private static void sendChat(String sender, String message) {
        char[] data = new char[421];
        data[0] = App.CHAT;

        //Copy over the name
        for(int i = 0; i < sender.length(); i++)
            data[i + 1] = sender.charAt(i);  
        data[sender.length() + 1] = '\0';

        //copy over the message
        for(int i = 0; i < message.length(); i++) 
            data[i + 21] = message.charAt(i);
        data[message.length() + 21] = '\0';
        
        App.write(data, 421);

    }

    //Sends the toggle message to server 
    private static void sendSeek(String userName, long s) {

        //The data to write
        char[] data = new char[29];
        data[0] = App.SEEK;

        //Put the long
        ByteBuffer buffer = ByteBuffer.allocate(Long.BYTES);
        buffer.putLong(0, s);
        buffer.flip();
        String arr = new String(buffer.array());

        for(int i = 0; i < 8; i++)
            data[i + 1] = arr.charAt(i);
        
        //Put the name in as well
        for(int i = 0; i < userName.length(); i++) {
            data[i + 9] = userName.charAt(i);
        }

        data[userName.length() + 9] = '\0';
        App.write(data, 29);
    }

    public static void updateChats() {
        if(scroll == null) 
            return;

        Platform.runLater(() -> {
        scroll.getChildren().removeAll(scroll.getChildren());
        scroll.getChildren().addAll(
            App.chats
            .stream()
            .map(m -> {
                System.out.println("Curr message " + m);

                if(m.length() > 2) {
                    String[] message = m.split(":");
                    String person = message[0];
                    String chat = message[1];
                    boolean controls = 
                        chat.equals("PAUSED MOVIE") ||
                        chat.equals("RESUMED MOVIE") ||
                        chat.equals("LEFT") ||
                        chat.equals("SEEKED MOVIE");

                    Text name = new Text(person + ":");
                    name.setFill(controls ? Color.web("#EF5350") : Color.web("#263238"));
                    name.setFont(Font.font("Verdana", FontWeight.BOLD,null, 13));
    
                    Text c = new Text(chat);
                    c.setFont(Font.font("Verdana", 12));
                    c.setWrappingWidth(150);
                    if(controls) c.setFill(Color.web("#EF5350"));
                    HBox box = new HBox(name, c);
                    box.setSpacing(5);
                    return box; 
                } else {

                    //This is an image
                    ImageInfo info = App.images.get(App.imageIndex++);
                    ImageView img = new ImageView(info.getURL());
                    img.setPreserveRatio(true);
                    img.setFitHeight(250);
                    img.setFitWidth(250);
                    Rectangle clip = new Rectangle(img.getFitWidth(), img.getFitHeight());
                    clip.setArcWidth(30);
                    clip.setArcHeight(30);
                    img.setClip(clip);
                    
                    Text name = new Text(info.getSender());
                    name.setFill(Color.web("#263238"));
                    name.setFont(Font.font("Verdana", FontWeight.BOLD,null, 13));
    
                    VBox box = new VBox(name, img);
                    box.setSpacing(2);
                    box.setPadding(new Insets(8, 0, 0, 0));
                    return box;

                }
            })
            .collect(Collectors.toList()));
        });

        //Scroll all the way to the bottom
        if(pane != null)
            pane.setVvalue(pane.getVmax());

        System.out.println("Updated chats!");
    }

    //When the stage is being closed
    public static void disposeMedia() {
        if(mediaPlayer != null) {
            mediaPlayer.stop();
            mediaPlayer.dispose();
        }
    }

    //Sends the image 
    private static void sendImage() {

        try {
            //The image bytes
            byte[] img = Files.readAllBytes(imageFile.toPath());
            System.out.println("Actual image size " + Files.size(imageFile.toPath()));

            //Put it all together 
            char[] a = new char[29];
            a[0] = App.IMAGE;

            //Put in the name
            for(int i = 0; i < App.userName.length(); i++) 
                a[i + 1] = App.userName.charAt(i);
            a[App.userName.length() + 1] = '\0';

            //Put the long
            ByteBuffer buffer = ByteBuffer.allocate(Long.BYTES);
            buffer.putLong(0, (long) img.length);
            buffer.flip();
            byte[] f = buffer.array();

            for(int i = 0; i < 8; i++)
                a[i + 21] = (char) f[i];

            // //Put the bytes
            // for(int i = 0; i < img.length; i++) 
            //     data[i + 29] = (char) img[i];

            //Write it out
            System.out.println("data arr len is " + a.length);
            App.write(a, 29);
            
        } catch(IOException e) {
            e.printStackTrace();
        }
    }
}