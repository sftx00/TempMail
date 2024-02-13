#include <iostream>
#include <string>
#include <curl/curl.h>
#include <json/json.h>
#include <conio.h>

class TempMailService {
public:
    TempMailService() {
        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl = curl_easy_init();
        generateTempEmail(); 
    }

    ~TempMailService() {
        if (curl) {
            curl_easy_cleanup(curl);
        }
        curl_global_cleanup();
    }

    void generateTempEmail() {
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

        // Извлекаем email из массива
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

    void showMenu(const bool refreshed) {
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
                system("cls"); // clear of console
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

    void checkNewEmails() {
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
            Json::CharReaderBuilder readerBuilder;
            std::istringstream jsonStream(response);

            if (Json::parseFromStream(readerBuilder, jsonStream, &root, nullptr)) {
                std::cout << "Received JSON response: " << response << std::endl;

                if (root.isArray()) {
                    for (const auto& entry : root) {
                        if (entry.isObject() && entry.isMember("id")) {
                            std::cout << "ID: " << entry["id"].asString() << std::endl;
                            std::cout << "From: " << entry["from"].asString() << std::endl;
                            std::cout << "Subject: " << entry["subject"].asString() << std::endl;
                            std::cout << "Date: " << entry["date"].asString() << std::endl;
                            std::cout << "----------------------" << std::endl;
                        }
                        else {
                            std::cerr << "Invalid entry in JSON array, expected 'id' field." << std::endl;
                        }
                    }
                }
                else {
                    std::cerr << "Invalid JSON structure: Expected array as root." << std::endl;
                }
            }
            else {
                std::cerr << "Failed to parse JSON response." << std::endl;
            }

            std::cout << "Press '0' to return to the menu, any other key to refresh..." << std::endl;
            char key = _getch();
            if (key == '0') {
                system("cls");
                showMenu(false);
                return;
            }
            else {
                system("cls"); // Очищаем консоль
            }
        }
    }

private:
    static size_t writeCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
        size_t totalSize = size * nmemb;
        output->append(static_cast<char*>(contents), totalSize);
        return totalSize;
    }

    std::pair<std::string, std::string> currentEmail;
    CURL* curl;
};

int main() {
    TempMailService tempMailService;
    tempMailService.showMenu(false);

    return 0;
}