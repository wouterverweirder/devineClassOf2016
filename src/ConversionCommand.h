//
//  ConversionCommand.h
//  devineClassOf2016
//
//  Created by Wouter Verweirder on 28/06/13.
//
//

#pragma once

#include "ofMain.h"
#include "SysCommand.h"
#include <curl/curl.h>

enum CONVERSION_STATE {
    SETUP_FOLDERS,
    KEY_WITH_AFTER_EFFECTS,
    RESIZE_IMAGE,
    MERGE_LAYERS,
    CONVERT_TO_POLAROID,
    ADD_TYPO,
    UPLOAD_TO_FACEBOOK,
    FINISH_AND_CLEANUP,
    DONE
};

class ConversionCommand : public ofThread {
public:
    ConversionCommand(){}
    ~ConversionCommand(){}
    void execute(string appDir, string sourceImage, string facebookAccessToken, string facebookAlbumId){
        _appDir = appDir;
        _sourceImage = sourceImage;
        _facebookAccessToken = facebookAccessToken;
        _facebookAlbumId = facebookAlbumId;
        stopThread();
        state = SETUP_FOLDERS;
        startThread(false, false);
    }
    
    ofEvent<string> onConversionComplete;
    
private:
    CONVERSION_STATE state;
    string _appDir;
    string _sourceImage;
    string _facebookAccessToken;
    string _facebookAlbumId;
    void threadedFunction(){
        while(isThreadRunning()) {
            if(state == SETUP_FOLDERS) {
                setupFolders();
            } else if (state == KEY_WITH_AFTER_EFFECTS) {
                keyWithAfterEffects();
            } else if (state == RESIZE_IMAGE) {
                resizeImage();
            } else if (state == MERGE_LAYERS) {
                mergeLayers();
            } else if (state == CONVERT_TO_POLAROID) {
                convertToPolaroid();
            } else if (state == ADD_TYPO) {
                addTypo();
            } else if (state == UPLOAD_TO_FACEBOOK) {
                uploadToFacebook();
            } else if (state == FINISH_AND_CLEANUP) {
                finishAndCleanup();
            } else if (state == DONE) {
                break;
            }
            ofSleepMillis(100);
        }
    }
    
    SysCommand cmd;
    
    void setupFolders(){
        ofFile sourceImageFile = ofFile(_sourceImage);
        if(!sourceImageFile.exists()) {
            state = DONE;
            cout << "source image " << sourceImageFile.getAbsolutePath() << " does not exist" << endl;
            return;
        }
        
        string progressFolderPath = "progress/" + sourceImageFile.getBaseName() + "/";
        
        ofDirectory(ofToDataPath(progressFolderPath)).remove(true);
        ofDirectory(ofToDataPath(progressFolderPath)).create();
        
        sourceImageFile.moveTo(progressFolderPath + "source_image.jpg");
        ofFile(ofToDataPath("assets/devine_keying.aep")).copyTo(progressFolderPath + "devine_keying.aep");
        
        state++;
    }
    
    void keyWithAfterEffects(){
        ofFile sourceImageFile = ofFile(_sourceImage);
        string progressFolderPath = "progress/" + sourceImageFile.getBaseName() + "/";
        
        string outputFile = _appDir + "data/" + progressFolderPath + "keying_progress.png";
        string projectFile = _appDir + "data/" +progressFolderPath + "devine_keying.aep";
        
        string command = "/Applications/Adobe\\ After\\ Effects\\ CC/aerender -project '" + projectFile + "' -comp \"polaroid\" -RStemplate \"Best Settings\" -OMtemplate \"keying\" -output '" + outputFile + "'";
        
        string result = cmd.exec((char *)command.c_str());
        ofFile(ofToDataPath(progressFolderPath + "keying_progress.png00000")).renameTo(ofToDataPath(progressFolderPath + "keying_finished.png"));
        printf("keyWithAfterEffects done\n");
        state++;
    }
    
    void resizeImage(){
        ofFile sourceImageFile = ofFile(_sourceImage);
        string progressFolderPath = "progress/" + sourceImageFile.getBaseName() + "/";
        
        ofImage backdropImage;
        backdropImage.setUseTexture(false);
        backdropImage.loadImage("assets/backdrop.jpeg");
        
        ofImage resizedImage;
        resizedImage.setUseTexture(false);
        resizedImage.loadImage(ofToDataPath(progressFolderPath + "keying_finished.png"));
        //resize to height of backdrop
        float scale = backdropImage.getHeight() / resizedImage.getHeight();
        int targetWidth = resizedImage.getWidth() * scale;
        int targetHeight = resizedImage.getHeight() * scale;
        resizedImage.resize(targetWidth, targetHeight);
        //crop image
        resizedImage.crop((targetWidth - backdropImage.getWidth()) * .5, 0, backdropImage.getWidth(), backdropImage.getHeight());
        
        string resizedFileTarget = ofToDataPath(progressFolderPath + "keying_resized.png");
        resizedImage.saveImage(resizedFileTarget);
        
        state++;
    }
    
    void mergeLayers(){
        ofFile sourceImageFile = ofFile(_sourceImage);
        string progressFolderPath = "progress/" + sourceImageFile.getBaseName() + "/";
        
        ofImage a;
        a.setUseTexture(false);
        a.loadImage("assets/backdrop.jpeg");
        a.setImageType(OF_IMAGE_COLOR_ALPHA);
        
        ofImage b;
        b.setUseTexture(false);
        b.loadImage(progressFolderPath + "keying_resized.png");
        b.setImageType(OF_IMAGE_COLOR_ALPHA);
        
        // a and b are both RGBA images
        int totalPixels = a.width * a.height;
        unsigned char * aPixels = a.getPixels();
        unsigned char * bPixels = b.getPixels();
        
        for(int i=0; i<totalPixels; i++) {
            int base = i*4;
            // deal with r,g,b
            float bAlpha = float(bPixels[base+3])/256.0f;
            for (int j=0; j<3; j++ )
            {
                // merge b into a based on b's alpha
                aPixels[base+j] = (aPixels[base+j]*(1.0f-bAlpha) + bPixels[base+j]*bAlpha);
            }
            // full alpha (no transparency)
            aPixels[base+3] = 255;
        }
        
        a.setImageType(OF_IMAGE_COLOR);
        a.saveImage(progressFolderPath + sourceImageFile.getBaseName() + "_merged.jpg");
        
        state++;
    }
    
    void convertToPolaroid(){
        ofFile sourceImageFile = ofFile(_sourceImage);
        string progressFolderPath = "progress/" + sourceImageFile.getBaseName() + "/";
        
        string imagePath = _appDir + "data/" + progressFolderPath + sourceImageFile.getBaseName() + "_merged.jpg";
        Poco::Timestamp originalModified = ofFile(imagePath).getPocoFile().getLastModified();
        
        string command = "open -a '" + _appDir + "data/assets/polaroid_droplet.app/Contents/MacOS/Droplet' '" + imagePath + "'";
        string result = cmd.exec((char *)command.c_str());
        
        Poco::Timestamp lastModified = ofFile(imagePath).getPocoFile().getLastModified();
        while(isThreadRunning() && lastModified == originalModified)
        {
            ofSleepMillis(1000);
            lastModified = ofFile(imagePath).getPocoFile().getLastModified();
        }
        ofFile(imagePath).moveTo(_appDir + "data/" + progressFolderPath + "polaroid.jpg");
        
        state++;
    }
    
    void addTypo(){
        ofFile sourceImageFile = ofFile(_sourceImage);
        string progressFolderPath = "progress/" + sourceImageFile.getBaseName() + "/";
        
        ofImage a;
        a.setUseTexture(false);
        a.loadImage(progressFolderPath + "polaroid.jpg");
        a.setImageType(OF_IMAGE_COLOR_ALPHA);
        
        ofImage b;
        b.setUseTexture(false);
        b.loadImage("assets/typo.png");
        b.setImageType(OF_IMAGE_COLOR_ALPHA);
        
        // a and b are both RGBA images
        int totalPixels = a.width * a.height;
        unsigned char * aPixels = a.getPixels();
        unsigned char * bPixels = b.getPixels();
        
        for(int i=0; i<totalPixels; i++) {
            int base = i*4;
            // deal with r,g,b
            float bAlpha = float(bPixels[base+3])/256.0f;
            for (int j=0; j<3; j++ )
            {
                // merge b into a based on b's alpha
                aPixels[base+j] = (aPixels[base+j]*(1.0f-bAlpha) + bPixels[base+j]*bAlpha);
            }
            // full alpha (no transparency)
            aPixels[base+3] = 255;
        }
        
        a.setImageType(OF_IMAGE_COLOR);
        a.saveImage(progressFolderPath + "polaroid.jpg");
        state++;
    }
    
    void uploadToFacebook(){
        ofFile sourceImageFile = ofFile(_sourceImage);
        string progressFolderPath = "progress/" + sourceImageFile.getBaseName() + "/";
        
        string _imageFileName = _appDir + "data/" + progressFolderPath + "polaroid.jpg";
        string _commentToPost = "Class Of 2016";
        
        struct curl_httppost *post=NULL;
        struct curl_httppost *last=NULL;
        struct curl_slist *headers=NULL;
        
        CURL * easyhandle;
        easyhandle = curl_easy_init();
        curl_easy_setopt(easyhandle, CURLOPT_VERBOSE, 1); // do this to make debugging easier
        
        headers = curl_slist_append(headers, "Content-Type: multipart/form-data");
        
        curl_formadd(&post, &last,
                     CURLFORM_COPYNAME, "access_token",
                     CURLFORM_COPYCONTENTS, _facebookAccessToken.c_str(), CURLFORM_CONTENTHEADER, headers, CURLFORM_END);
        
        curl_formadd(&post, &last,
                     CURLFORM_COPYNAME, "file",
                     CURLFORM_FILE, _imageFileName.c_str(), CURLFORM_CONTENTTYPE, "image/jpg", CURLFORM_END);
        
        /*curl_formadd(&post, &last,
         CURLFORM_COPYNAME, "message",
         CURLFORM_COPYCONTENTS, _commentToPost.c_str(), CURLFORM_CONTENTHEADER, headers, CURLFORM_END);*/
        
        string target = "https://graph.facebook.com/" + _facebookAlbumId + "/photos";
        
        curl_easy_setopt(easyhandle, CURLOPT_POSTFIELDS, "");
        curl_easy_setopt(easyhandle, CURLOPT_POSTFIELDSIZE, 23L);
        curl_easy_setopt(easyhandle, CURLOPT_URL, target.c_str());
        
        /* Set the form info */
        curl_easy_setopt(easyhandle, CURLOPT_HTTPPOST, post);
        
        /* post away! */
        curl_easy_perform(easyhandle);
        
        /* free the post data again */
        curl_formfree(post);
        
        curl_easy_cleanup(easyhandle);
        
        state++;
    }
    
    void finishAndCleanup(){
        //move polaroid to finished folder
        ofFile sourceImageFile = ofFile(_sourceImage);
        string progressFolderPath = "progress/" + sourceImageFile.getBaseName() + "/";
        
        ofFile polaroidFile = ofFile(ofToDataPath(progressFolderPath + "polaroid.jpg"));
        
        string fileName = ofGetTimestampString() + ".jpg";
        
        polaroidFile.moveTo(ofToDataPath("finished/" + fileName));
        
        ofDirectory(ofToDataPath(progressFolderPath)).remove(true);
        
        ofNotifyEvent(onConversionComplete, fileName);
        
        state++;
    }
};