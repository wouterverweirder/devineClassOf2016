#pragma once

#include "FBConsts.h"
#include "CurlWrapper.h"

using namespace std;

class GetAccessTokenCommand {
public:
    GetAccessTokenCommand(){}
    ~GetAccessTokenCommand(){}
    
    string execute(const string& authTokenUrl){
        string authCode = authTokenUrl;
        authCode.erase(0, FACEBOOK_AUTH_CODE_RETURNING_GARBAGE.length());
        string tokenURL = FACEBOOK_ACCESS_TOKEN_TARGET_URL + FACEBOOK_APP_ID + FACEBOOK_ACCESS_TOKEN_REDIRECT_URL + FACEBOOK_APP_SECRET + FACEBOOK_AUTH_CODE_PREFIX + authCode;
        
        CurlWrapper wrapper;
        string result = wrapper.loadURL(tokenURL);
        
        string accessToken = getAccessTokenFromResult(result);
        if("" != accessToken){
            //trade in for longer token
            string exchangeTokenUrl = "https://graph.facebook.com/oauth/access_token?grant_type=fb_exchange_token&client_id=" + FACEBOOK_APP_ID + "&client_secret=" + FACEBOOK_APP_SECRET + "&fb_exchange_token=" + accessToken;
            result = wrapper.loadURL(exchangeTokenUrl);
            accessToken = getAccessTokenFromResult(result);
        }
        return accessToken;
    }
    
    string getAccessTokenFromResult(string result){
        string prefix = "access_token=";
		string delim = "&";
		string expire = "expires";
        string accessToken = "";
		result.erase(0, prefix.length());
		vector<string> split = ofSplitString(result, delim);
		for (int i = 0; i < split.size(); i++){
			if (!ofIsStringInString(split[i], expire)){
				accessToken = split[i];
				break;
			}
		}
        return accessToken;
    }
};