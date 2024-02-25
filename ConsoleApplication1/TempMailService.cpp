#include "TempMailService.h"

TempMailService::TempMailService() {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    generateTempEmail();
}

TempMailService::~TempMailService() {
    if (curl) {
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
}

void TempMailService::showMenu(const bool refreshed) {
    if (refreshed) {
        std::cout << "\033[6;32mSuccessfully refreshed!\033[0m" << std::endl;
        std::cout << "" << std::endl;
    }
    else {
        std::cout << "" << std::endl;
    }

    std::cout << "\033[1;31mTempl\033[0m v0.1" << std::endl;

    std::cout << "" << std::endl;
    std::cout << "----------------------------" << std::endl;
    std::cout << "" << std::endl;

    while (true) {
        std::cout << "Your \033[1;32mtemp\033[0m\033[0m mail: \033[4;32m" << currentEmail.first << "@" << currentEmail.second << "\033[0m" << std::endl;

        std::cout << "" << std::endl;

        std::cout << "Choose an option:" << std::endl;
        std::cout << "1. \033[7;32m>_\033[0m Emails" << std::endl;
        std::cout << "2. \033[7;32m>_\033[0m Change email" << std::endl;
        std::cout << "0. \033[7;32m>_\033[0m Exit" << std::endl;

        int choice;
        char key = _getch();
        std::cout << key << std::endl;

        switch (key) {
        case '0':
            std::cout << "Exiting the program." << std::endl;
            return;
        case '1':
            system("cls");
            checkNewEmails();
            break;
        case '2':
            system("cls");
            generateTempEmail();
            showMenu(true);
            break;
        default:
            std::cout << "Invalid choice. Try again." << std::endl;
            break;
        }
    }
}

void TempMailService::generateTempEmail() {
    std::string apiUrl = "https://www.1secmail.com/api/v1/";
    std::string url = apiUrl + R"(/?action=genRandomMailbox&count=1)";

    CURLcode res;
    std::string email;

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &email);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            std::cerr << "Failed to perform request: " << curl_easy_strerror(res) << std::endl;
            return;
        }
    }

    Json::Value root;
    Json::Reader reader;
    if (reader.parse(email, root) && root.isArray() && root.size() > 0) {
        std::string fullEmail = root[0].asString();
        size_t atIndex = fullEmail.find('@');
        if (atIndex != std::string::npos) {
            currentEmail.first = fullEmail.substr(0, atIndex);
            currentEmail.second = fullEmail.substr(atIndex + 1);
        }
        else {
            std::cerr << "Invalid email format." << std::endl;
        }
    }
    else {
        std::cerr << "Failed to parse generated email." << std::endl;
    }
}

void TempMailService::checkNewEmails() {
    std::vector<std::string> emails; 

    while (true) {
        std::string apiUrl = "https://www.1secmail.com/api/v1/";
        std::string url = apiUrl + R"(/?action=getMessages&login=)" + currentEmail.first + R"(&domain=)" + currentEmail.second;

        CURLcode res;
        std::string response;

        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

            res = curl_easy_perform(curl);

            if (res != CURLE_OK) {
                std::cerr << "Failed to perform request: " << curl_easy_strerror(res) << std::endl;
                return;
            }
        }

        Json::Value root;
        Json::Reader reader;
        if (reader.parse(response, root) && root.isArray()) {
            emails.clear(); 
            for (const auto& entry : root) {
                if (entry.isObject() && entry.isMember("id")) {
                    std::string emailInfo = "ID: " + entry["id"].asString() + "\n";
                    emailInfo += "From: " + entry["from"].asString() + "\n";
                    emailInfo += "Subject: " + entry["subject"].asString();
                    emails.insert(emails.begin(), emailInfo);
                }
                else {
                    std::cerr << "Invalid entry in JSON array, expected 'id' field." << std::endl;
                }
            }
        }
        else {
            std::cerr << "Failed to parse JSON response." << std::endl;
        }

        system("cls");
        std::cout << "Recent emails:" << std::endl;
        int count = 0;
        for (auto it = emails.rbegin(); it != emails.rend() && count < 4; ++it) {
            std::cout << "--------------------------------------" << std::endl;
            std::cout << *it << std::endl;
            count++;
        }
        std::cout << "--------------------------------------" << std::endl;

        std::cout << "Press the number of the email you want to open (1-4), or 'r' to refresh, or '0' to return to the menu: ";
        char key = _getch();
        if (key == 'r') {
            continue; 
        }
        else if (key == '0') {
            system("cls");
            return;
        }
        else if (key >= '1' && key <= '4') {
            int choice = key - '0';
            if (choice <= count) {
                system("cls");
                std::string emailId = root[choice - 1]["id"].asString();

                std::string messageUrl = apiUrl + R"(/?action=readMessage&login=)" +
                    currentEmail.first + R"(&domain=)" + currentEmail.second + "&id=" + emailId;

                std::string messageResponse;

                if (curl) {
                    curl_easy_setopt(curl, CURLOPT_URL, messageUrl.c_str());
                    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
                    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &messageResponse);

                    res = curl_easy_perform(curl);

                    if (res != CURLE_OK) {
                        std::cerr << "Failed to perform request: " << curl_easy_strerror(res) << std::endl;
                        return;
                    }
                }

                std::cout << "--------------------------------------" << std::endl;
                std::cout << "Email content:" << std::endl;
                printFormattedEmail(messageResponse);
                std::cout << "--------------------------------------" << std::endl;

                std::cout << "Press any key to return to the menu..." << std::endl;
                _getch();
                system("cls");
                return;
            }
        }
    }
}

void TempMailService::printFormattedEmail(const std::string& messageResponse) {
    Json::Value root;
    Json::Reader reader;
    if (reader.parse(messageResponse, root) && root.isObject()) {
        std::cout << "From: " << root["from"].asString() << std::endl;
        std::cout << "Subject: " << root["subject"].asString() << std::endl;
        std::cout << "Date: " << root["date"].asString() << std::endl;
        std::cout << "--------------------------------------" << std::endl;

        std::string htmlBody = root["htmlBody"].asString();
        std::string textBody = convertHtmlToText(htmlBody);
        std::cout << "Body:" << std::endl;
        std::cout << textBody << std::endl;
    }
    else {
        std::cerr << "Failed to parse email content." << std::endl;
    }
}

std::string TempMailService::convertHtmlToText(const std::string& html) {
    std::string text = html;
    size_t pos = 0;
    while ((pos = text.find("<")) != std::string::npos) {
        size_t endPos = text.find(">", pos);
        if (endPos != std::string::npos) {
            text.erase(pos, endPos - pos + 1);
        }
    }
    return text;
}

size_t TempMailService::writeCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t totalSize = size * nmemb;
    output->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}
