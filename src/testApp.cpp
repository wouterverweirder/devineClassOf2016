#include "testApp.h"
#include "FBConsts.h"
#include "GetAccessTokenCommand.h"

void testApp::setup(){
    ofSetFrameRate(60);
    ofSetVerticalSync(true);
    
    appDir = ofFilePath::getCurrentExeDir();
    for(int i = 0; i < 3; i++) {
        appDir = ofFilePath::getEnclosingDirectory(appDir);
    }
    
    appSettings.loadFile("fbsettings.xml");
    facebookAlbumId = appSettings.getValue("FACEBOOK:ALBUMID", FACEBOOK_ALBUM_ID); //prive album
    facebookAccessToken = appSettings.getValue("FACEBOOK:ACCESSTOKEN", "");
    
    ofxFenster* win=ofxFensterManager::get()->createFenster(400, 300, 640, 480, OF_WINDOW);
    win->addListener(&resultWindow);
    win->setWindowTitle("Result");
    resultWindow.setup();
    
    gui = new ofxUICanvas();
    gui->setAutoDraw(false);
    gui->setDrawBack(false);
    gui->addSpacer(0.0, 5.0);
    takePictureButton = new ofxUILabelButton("TAKE PICTURE", false, 200, 40);
    gui->addWidgetDown(takePictureButton);
    
    gui->addSpacer(0.0, 390.0);
    
    removePictureButton = new ofxUILabelButton("REMOVE PICTURE", false, 200, 40);
    usePictureButton = new ofxUILabelButton("USE PICTURE", false, 200, 40);
    
    gui->addWidgetDown(removePictureButton);
    gui->addWidgetRight(usePictureButton);
    
    ofAddListener(gui->newGUIEvent,this,&testApp::guiEvent);
    
    gui->autoSizeToFitWidgets();
    
    toConvertDirectory = ofDirectory(ofToDataPath("to-convert/"));
    if(!toConvertDirectory.exists()) toConvertDirectory.create();
    
    progressDirectory = ofDirectory(ofToDataPath("progress/"));
    if(!progressDirectory.exists()) progressDirectory.create();
    
    finishedDirectory = ofDirectory(ofToDataPath("finished/"));
    if(!finishedDirectory.exists()) finishedDirectory.create();
    
    checkConvertDirectory = false;
    ofAddListener(conversionCommand.onConversionComplete, this, &testApp::onConversionComplete);
    
    //webview
    webView.allocate(480, 640, GL_RGBA);
    ofAddListener(webView.beginLoading, this, &testApp::onWebViewBeginLoading);
    if("" == facebookAccessToken){
        string url = FACEBOOK_AUTH_CODE_TARGET_URL + FACEBOOK_APP_ID + FACEBOOK_AUTH_CODE_REDIRECT_URL + FACEBOOK_AUTH_CODE_PERMISSION_SCOPE;
        webView.loadURL(url);
        webView.visible = true;
    } else {
        webView.visible = false;
    }
    //end webview
    
    ofAddListener(wrapper.onFileAdded, this, &testApp::onFileAdded);
    cameraImageChanged();
}

void testApp::onWebViewBeginLoading(const WebViewBeginLoadingEvent& event){
    if (ofIsStringInString(event.url, FACEBOOK_AUTH_CODE_RETURNING_GARBAGE)){
        webView.visible = false;
        GetAccessTokenCommand cmd;
        facebookAccessToken = cmd.execute(event.url);
    }
}

void testApp::guiEvent(ofxUIEventArgs &e)
{
	string name = e.widget->getName();
    
    if("mouseReleased" == e.type) {
        if("TAKE PICTURE" == name){
            if(!wrapper.isStarted()){
                wrapper.start(); //restart if camera was disconnected
            }
            wrapper.takePicture();
        }else if("USE PICTURE" == name || "REMOVE PICTURE" == name){
            if(cameraImages.size() > 0) {
                string cameraImageName = cameraImages.front();
                ofFile cameraImageFile = ofFile(cameraImageName);
                if("USE PICTURE" == name){
                    //use it > place in to-convert directory
                    cameraImageFile.moveTo(ofToDataPath("to-convert/" + cameraImageFile.getFileName()));
                    numFilesToConvert++;
                    checkConvertDirectory = true;
                }else{
                    //delete the file
                    cameraImageFile.remove();
                }
                cameraImageFile.close();
                cameraImages.pop_front();
                cameraImageChanged();
            }
        }
    }
}

void testApp::exit(){
    appSettings.setValue("FACEBOOK:ALBUMID", facebookAlbumId);
    appSettings.setValue("FACEBOOK:ACCESSTOKEN", facebookAccessToken);
    appSettings.saveFile("fbsettings.xml");
}

void testApp::onFileAdded(string &fileName){
    cameraImages.push_back(fileName);
    if(cameraImages.size() == 1) {
        cameraImageChanged();
    }
}

void testApp::onConversionComplete(string &fileName){
    cout << "onConversionComplete: " << fileName << endl;
    checkConvertDirectory = true;
    resultWindow.showImage(fileName);
}

void testApp::cameraImageChanged(){
    if(cameraImages.size() > 0) {
        cameraImage.loadImage(cameraImages.front());
        removePictureButton->setVisible(true);
        removePictureButton->setLabelVisible(true);
        usePictureButton->setVisible(true);
        usePictureButton->setLabelVisible(true);
    } else {
        cameraImage.clear();
        removePictureButton->setVisible(false);
        removePictureButton->setLabelVisible(false);
        usePictureButton->setVisible(false);
        usePictureButton->setLabelVisible(false);
    }
}

//--------------------------------------------------------------
void testApp::update(){
    webView.update();
    if(checkConvertDirectory && !conversionCommand.isThreadRunning()) {
        checkConvertDirectory = false;
        //check to-convert directory
        toConvertDirectory.close();
        toConvertDirectory.open(ofToDataPath("to-convert/"));
        toConvertDirectory.setShowHidden(false);
        toConvertDirectory.allowExt("jpg");
        toConvertDirectory.listDir();
        numFilesToConvert = toConvertDirectory.numFiles();
        if(toConvertDirectory.numFiles() > 0) {
            conversionCommand.execute(appDir, ofToDataPath("to-convert/" + toConvertDirectory.getFile(0).getFileName()), facebookAccessToken, facebookAlbumId);
        }
        toConvertDirectory.close();
    }
}

//--------------------------------------------------------------
void testApp::draw(){
    ofBackground(0);
    gui->draw();
    webView.draw();
    if(cameraImage.isAllocated()){
        ofPushMatrix();
        ofScale(0.1, 0.1);
        cameraImage.draw(0 / .1, 60 / .1);
        ofPopMatrix();
    }
}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){
    webView.x = (ofGetWidth() - webView.getWidth()) / 2;
    webView.y = (ofGetHeight() - webView.getHeight()) / 2;
}