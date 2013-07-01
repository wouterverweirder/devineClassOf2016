#pragma once

#include "ofMain.h"
#include "ofxFensterManager.h"

class ResultWindow: public ofxFensterListener {
public:
	~ResultWindow() {
		cout << "ResultWindow destroyed" << endl;
	}
	void setup() {
	}
    
    void update(ofxFenster* fenster) {
        ofPoint pt = fenster->getWindowSize();
        windowWidth = pt.x;
        windowHeight = pt.y;
        if(_fileNameChanged) {
            _fileNameChanged = false;
            img.loadImage(_fileName);
        }
    }
    
	void draw() {
        if(img.isAllocated()) {
            float scale = windowHeight / img.height;
            ofPushMatrix();
            ofScale(scale, scale);
            ofTranslate(((windowWidth - img.width*scale) * .5) / scale, 0);
            img.draw(0, 0);
            ofPopMatrix();
        }
	}
    
    void showImage(string fileName) {
        _fileName = fileName;
        _fileNameChanged = true;
    }
    
	ofImage img;
    string _fileName;
    bool _fileNameChanged;
    float windowWidth;
    float windowHeight;

};