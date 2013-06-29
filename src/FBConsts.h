#pragma once

using namespace std;

static const string FACEBOOK_APP_ID = "";
static const string FACEBOOK_APP_SECRET = "";
static const string FACEBOOK_ALBUM_ID = ""; //default album id when settings file is empty

static const string FACEBOOK_AUTH_CODE_TARGET_URL = "https://m.facebook.com/dialog/oauth?client_id=";
static const string FACEBOOK_AUTH_CODE_REDIRECT_URL = "&redirect_uri=https://m.facebook.com/connect/login_success.html";
static const string FACEBOOK_AUTH_CODE_PERMISSION_SCOPE = "&scope=publish_stream";

static const string FACEBOOK_ACCESS_TOKEN_TARGET_URL = "https://graph.facebook.com/oauth/access_token?client_id=";
static const string FACEBOOK_ACCESS_TOKEN_REDIRECT_URL = "&redirect_uri=https://m.facebook.com/connect/login_success.html&client_secret=";
static const string FACEBOOK_AUTH_CODE_PREFIX = "&code=";
static const string FACEBOOK_AUTH_CODE_RETURNING_GARBAGE = "https://m.facebook.com/connect/login_success.html?code=";
static const string FACEBOOK_ACCESS_TOKEN_RETURNING_GARBAGE = "https://graph.facebook.com/oauth/access_token?";