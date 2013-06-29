#pragma once

#include <curl/curl.h>

struct BufferStruct
{
    char * buffer;
    size_t size;
};

static size_t WriteMemoryCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
    size_t realsize = size * nmemb;
    
    struct BufferStruct * mem = (struct BufferStruct *) data;
    
    mem->buffer = (char *) realloc(mem->buffer, mem->size + realsize + 1);
    
    if ( mem->buffer )
    {
        memcpy( &( mem->buffer[ mem->size ] ), ptr, realsize );
        mem->size += realsize;
        mem->buffer[ mem->size ] = 0;
    }
    return realsize;
}

class CurlWrapper {
public:
    CurlWrapper(){}
    ~CurlWrapper(){}
    
    string loadURL(string& url){
        curl_global_init( CURL_GLOBAL_ALL );
        
        struct BufferStruct output;
        output.buffer = NULL;
        output.size = 0;
        
        CURL * myHandle;
        CURLcode result;
        myHandle = curl_easy_init ( ) ;
        curl_easy_setopt(myHandle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(myHandle, CURLOPT_WRITEDATA, (void *)&output);
        curl_easy_setopt( myHandle, CURLOPT_URL, url.c_str());
        result = curl_easy_perform( myHandle );
        curl_easy_cleanup( myHandle );
        
        string accessToken = "";
        accessToken.assign(output.buffer, output.size);
        
        if(output.buffer) {
            free(output.buffer);
            output.buffer = 0;
            output.size = 0;
        }
        
        return accessToken;
    }
};