//
//  WebView.h
//  facebook_login_and_renew_token
//
//  Created by Wouter Verweirder on 22/06/13.
//
//

#pragma once

#include "ofMain.h"
#include <Awesomium/WebCore.h>

class WebViewBeginLoadingEvent {
public:
    WebViewBeginLoadingEvent(   const std::string& _url,
                             const std::wstring& _frameName,
                             int _statusCode,
                             const std::wstring& _mimeType):
    url(_url), frameName(_frameName), statusCode(_statusCode), mimeType(_mimeType){
    }
    const std::string& url;
    const std::wstring& frameName;
    int statusCode;
    const std::wstring& mimeType;
};

class WebView : public Awesomium::WebViewListener {
public:
    WebView(){
        Awesomium::WebCoreConfig config;
        webCore = new Awesomium::WebCore(config);
        webView = NULL;
        x = 0.0;
        y = 0.0;
        visible = true;
        //listeners
        ofAddListener(ofEvents().keyPressed, this, &WebView::keyPressed);
        ofAddListener(ofEvents().mouseMoved, this, &WebView::mouseMoved);
        ofAddListener(ofEvents().mouseDragged, this, &WebView::mouseDragged);
        ofAddListener(ofEvents().mousePressed, this, &WebView::mousePressed);
        ofAddListener(ofEvents().mouseReleased, this, &WebView::mouseReleased);
    }
    ~WebView(){
        ofRemoveListener(ofEvents().keyPressed, this, &WebView::keyPressed);
        ofRemoveListener(ofEvents().mouseMoved, this, &WebView::mouseMoved);
        ofRemoveListener(ofEvents().mouseDragged, this, &WebView::mouseDragged);
        ofRemoveListener(ofEvents().mousePressed, this, &WebView::mousePressed);
        ofRemoveListener(ofEvents().mouseReleased, this, &WebView::mouseReleased);
        webView->destroy();
    }
    
    ofEvent<const WebViewBeginLoadingEvent> beginLoading;
    
    void keyPressed(ofKeyEventArgs& args){
        if(visible) {
            switch(args.key) {
                case 8: case 127:
                    injectKey(Awesomium::KeyCodes::AK_BACK);
                    return;
                case 9:
                    injectKey(Awesomium::KeyCodes::AK_TAB);
                    return;
                case OF_KEY_LEFT:
                    injectKey(Awesomium::KeyCodes::AK_LEFT);
                    return;
                case OF_KEY_UP:
                    injectKey(Awesomium::KeyCodes::AK_UP);
                    return;
                case OF_KEY_RIGHT:
                    injectKey(Awesomium::KeyCodes::AK_RIGHT);
                    return;
                case OF_KEY_DOWN:
                    injectKey(Awesomium::KeyCodes::AK_DOWN);
                    return;
                case OF_KEY_PAGE_UP:
                    injectKey(Awesomium::KeyCodes::AK_PRIOR);
                    return;
                case OF_KEY_PAGE_DOWN:
                    injectKey(Awesomium::KeyCodes::AK_NEXT);
                    return;
                case OF_KEY_HOME:
                    injectKey(Awesomium::KeyCodes::AK_HOME);
                    return;
                case OF_KEY_END:
                    injectKey(Awesomium::KeyCodes::AK_END);
                    return;
            }
            
            Awesomium::WebKeyboardEvent keyEvent;
            keyEvent.text[0] = args.key;
            keyEvent.unmodifiedText[0] = args.key;
            keyEvent.type = Awesomium::WebKeyboardEvent::TYPE_CHAR;
            keyEvent.virtualKeyCode = args.key;
            keyEvent.nativeKeyCode = args.key;
            webView->injectKeyboardEvent(keyEvent);
        }
    }
    
    void injectKey(int keyCode) {
        Awesomium::WebKeyboardEvent keyEvent;
        
        char* buf = new char[20];
        keyEvent.virtualKeyCode = keyCode;
        Awesomium::getKeyIdentifierFromVirtualKeyCode(keyEvent.virtualKeyCode, &buf);
        strcpy(keyEvent.keyIdentifier, buf);
        delete[] buf;
        
        keyEvent.modifiers = 0;
        keyEvent.nativeKeyCode = 0;
        keyEvent.type = Awesomium::WebKeyboardEvent::TYPE_KEY_DOWN;
        
        webView->injectKeyboardEvent(keyEvent);
        
        keyEvent.type = Awesomium::WebKeyboardEvent::TYPE_KEY_UP;
        
        webView->injectKeyboardEvent(keyEvent);
    }
    
    void mouseMoved(ofMouseEventArgs& args){
        if(visible) {
            webView->injectMouseMove(args.x - x, args.y - y);
        }
    }
    
    //--------------------------------------------------------------
    void mouseDragged(ofMouseEventArgs& args){
        if(visible) {
            webView->injectMouseMove(args.x - x, args.y - y);
        }
    }
    
    //--------------------------------------------------------------
    void mousePressed(ofMouseEventArgs& args){
        if(visible) {
            webView->injectMouseDown(Awesomium::LEFT_MOUSE_BTN);
        }
    }
    
    //--------------------------------------------------------------
    void mouseReleased(ofMouseEventArgs& args){
        if(visible) {
            webView->injectMouseUp(Awesomium::LEFT_MOUSE_BTN);
        }
    }
    
    void loadURL(string &url){
        webView->loadURL(url);
    }
    
    void allocate(int w, int h, int internalGlDataType){
        if(webView == NULL) {
            webTexture.allocate(w, h, internalGlDataType);
            webView = webCore->createWebView(webTexture.getWidth(), webTexture.getHeight());
            webView->setListener(this);
        }
    }
    
    float getWidth(){
        return webTexture.getWidth();
    }
    
    float getHeight(){
        return webTexture.getHeight();
    }
    
    float x;
    float y;
    bool visible;
    
    void update(){
        webCore->update();
        if(webView->isDirty()) {
            const Awesomium::RenderBuffer *renderBuffer = webView->render();
            if(renderBuffer) {
                webTexture.loadData(renderBuffer->buffer, webTexture.getWidth(), webTexture.getHeight(), GL_BGRA);
            }
        }
    }
    
    void draw(){
        if(visible && webTexture.isAllocated()) {
            webTexture.draw(x, y);
        }
    }
    
    void onBeginNavigation(Awesomium::WebView* caller,
                           const std::string& url,
                           const std::wstring& frameName){}
	
	void onBeginLoading(Awesomium::WebView* caller,
                        const std::string& url,
                        const std::wstring& frameName,
                        int statusCode, const std::wstring& mimeType){
        ofNotifyEvent(beginLoading, WebViewBeginLoadingEvent(url, frameName, statusCode, mimeType));
    }
	
	void onFinishLoading(Awesomium::WebView* caller){}
	
	void onCallback(Awesomium::WebView* caller,
                    const std::wstring& objectName,
                    const std::wstring& callbackName,
                    const Awesomium::JSArguments& args){}
	
	void onReceiveTitle(Awesomium::WebView* caller,
                        const std::wstring& title,
                        const std::wstring& frameName){}
	
	void onChangeTooltip(Awesomium::WebView* caller,
                         const std::wstring& tooltip){}
	
	void onChangeCursor(Awesomium::WebView* caller,
                        Awesomium::CursorType cursor){}
    
	void onChangeKeyboardFocus(Awesomium::WebView* caller,
                               bool isFocused){}
	
	void onChangeTargetURL(Awesomium::WebView* caller,
                           const std::string& url){}
	
	void onOpenExternalLink(Awesomium::WebView* caller,
                            const std::string& url,
                            const std::wstring& source){}
    
	void onRequestDownload(Awesomium::WebView* caller,
                           const std::string& url){}
	
	void onWebViewCrashed(Awesomium::WebView* caller){}
	
	void onPluginCrashed(Awesomium::WebView* caller,
                         const std::wstring& pluginName){}
	
	void onRequestMove(Awesomium::WebView* caller, int x, int y){}
	
	void onGetPageContents(Awesomium::WebView* caller,
                           const std::string& url,
                           const std::wstring& contents){}
	
	void onDOMReady(Awesomium::WebView* caller){}
    
	void onRequestFileChooser(Awesomium::WebView* caller,
                              bool selectMultipleFiles,
                              const std::wstring& title,
                              const std::wstring& defaultPath){}
    
	void onGetScrollData(Awesomium::WebView* caller,
                         int contentWidth,
                         int contentHeight,
                         int preferredWidth,
                         int scrollX,
                         int scrollY){}
    
	void onJavascriptConsoleMessage(Awesomium::WebView* caller,
                                    const std::wstring& message,
                                    int lineNumber,
                                    const std::wstring& source){}
    
	void onGetFindResults(Awesomium::WebView* caller,
                          int requestID,
                          int numMatches,
                          const Awesomium::Rect& selection,
                          int curMatch,
                          bool finalUpdate){}
    
	void onUpdateIME(Awesomium::WebView* caller,
                     Awesomium::IMEState imeState,
                     const Awesomium::Rect& caretRect){}
    
	void onShowContextMenu(Awesomium::WebView* caller,
                           int mouseX,
                           int mouseY,
                           Awesomium::MediaType type,
                           int mediaState,
                           const std::string& linkURL,
                           const std::string& srcURL,
                           const std::string& pageURL,
                           const std::string& frameURL,
                           const std::wstring& selectionText,
                           bool isEditable,
                           int editFlags){}
    
	void onRequestLogin(Awesomium::WebView* caller,
                        int requestID,
                        const std::string& requestURL,
                        bool isProxy,
                        const std::wstring& hostAndPort,
                        const std::wstring& scheme,
                        const std::wstring& realm){}
    
	void onChangeHistory(Awesomium::WebView* caller,
                         int backCount,
                         int forwardCount){}
    
	void onFinishResize(Awesomium::WebView* caller,
                        int width,
                        int height){}
    
	void onShowJavascriptDialog(Awesomium::WebView* caller,
                                int requestID,
                                int dialogFlags,
                                const std::wstring& message,
                                const std::wstring& defaultPrompt,
                                const std::string& frameURL){}
    
protected:
    Awesomium::WebView *webView;
    Awesomium::WebCore *webCore;
    ofTexture webTexture;
};