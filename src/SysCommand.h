//
//  SysCommand.h
//
//  Created by Brett Renfer on 2/22/12.
//  Copyright (c) 2012 Rockwell Group. All rights reserved.
//

#pragma once
#include "ofThread.h"

class SysCommand : public ofThread
{
    
public:
    
    ofEvent<string> commandComplete;
    
    void callCommand( string command ){
        cmd = command;
        stopThread();
        startThread( false, false );
    }
    
    // CALL THIS DIRECTLY FOR BLOCKING COMMAND
    // thanks to: http://stackoverflow.com/questions/478898/how-to-execute-a-command-and-get-output-of-command-within-c
    std::string exec(char* cmd) {
        FILE* pipe = popen(cmd, "r");
        if (!pipe) return "ERROR";
        char buffer[128];
        std::string result = "";
        while(!feof(pipe)) {
            if(fgets(buffer, 128, pipe) != NULL)
                result += buffer;
        }
        pclose(pipe);
        return result;
    }
    
private:
    void threadedFunction(){
        ofLog( OF_LOG_VERBOSE, "call "+cmd );
        string result = exec( (char *) cmd.c_str() );
        ofLog( OF_LOG_VERBOSE, "RESULT "+result );
        stopThread();
        ofNotifyEvent( commandComplete, result, this );
    }
    
    string cmd;
};