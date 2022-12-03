package com.example;

import java.io.File;

public class ImageInfo {
    private String sender;
    private File imageFile;

    public ImageInfo(String sender, File file) {
        this.sender = sender;
        this.imageFile = file;
    }

    public String getSender() {
        return sender;
    }

    public String getURL() {
        return imageFile.toURI().toString();
    }

}
