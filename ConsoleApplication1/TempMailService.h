#ifndef TEMP_MAIL_SERVICE_H
#define TEMP_MAIL_SERVICE_H

#include <iostream>
#include <string>
#include <vector>
#include <curl/curl.h>
#include <json/json.h>
#include <Windows.h>
#include <conio.h>

class TempMailService {
public:
    TempMailService();
    ~TempMailService();

    void showMenu(const bool refreshed);

private:
    static size_t writeCallback(void* contents, size_t size, size_t nmemb, std::string* output);
    void generateTempEmail();
    void checkNewEmails();
    void printFormattedEmail(const std::string& messageResponse);
    std::string convertHtmlToText(const std::string& html);

    std::pair<std::string, std::string> currentEmail;
    CURL* curl;
};

#endif // TEMP_MAIL_SERVICE_H
