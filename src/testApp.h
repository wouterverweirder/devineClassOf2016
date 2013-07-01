#pragma once

#include "ofMain.h"
#include "WebView.h"
#include "ofxXmlSettings.h"
#include "ofxUI.h"
#include "CanonSDKWrapper.h"
#include "ConversionCommand.h"
#include <deque>
#include "ofxFensterManager.h"
#include "ResultWindow.h"

class testApp : public ofxFensterListener{

public:
	void setup();
	void update();
	void draw();
    void exit();
    
    void windowResized(int w, int h);
    
    string appDir;
    
    ResultWindow resultWindow;
    
    // WEBVIEW FOR FACEBOOK LOGIN
    WebView webView;
    ofxXmlSettings appSettings;
    string facebookAlbumId;
    string facebookAccessToken;
    void onWebViewBeginLoading(const WebViewBeginLoadingEvent& event);
    // END WEBVIEW FOR FACEBOOK LOGIN
    
    // GUI
    ofxUICanvas *gui;
    ofxUILabelButton *takePictureButton;
    ofxUILabelButton *removePictureButton;
    ofxUILabelButton *usePictureButton;
    void guiEvent(ofxUIEventArgs &e);
    // END GUI
    
    // CANON
    CanonSDKWrapper wrapper;
    void onFileAdded(string &fileName);
    deque<string> cameraImages;
    ofImage cameraImage;
    void cameraImageChanged();
    // END CANON
    
    // CONVERSION
    ConversionCommand conversionCommand;
    void onConversionComplete(string& fileName);
    // END CONVERSION
    
    // DIRECTORIES
    ofDirectory toConvertDirectory;
    ofDirectory progressDirectory;
    ofDirectory finishedDirectory;
    int numFilesToConvert;
    bool checkConvertDirectory;
    // END DIRECTORIES
		
};
