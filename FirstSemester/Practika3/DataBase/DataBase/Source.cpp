#include <iostream>
#include <fstream>
#include <ctime>
#include <vector>
#include <thread>
#include <csignal>
#include <boost/asio.hpp>
#include <boost/random.hpp>
#include <nlohmann/json.hpp>

using namespace boost::asio;

class Visit {
public:
    Visit(const std::string& url, const std::string& ip, const std::string& time) { //�����������
        url_ = url;
        ip_ = ip;
        time_ = time;
    }
    std::string GetURL() {
        return url_;
    }
    std::string GetIP() {
        return ip_;
    }
    std::string GetTime() {
        return time_;
    }
private:
    std::string url_;
    std::string ip_;
    std::string time_;
};

class HashTable { //���-������� � ������� �������� ��������� ��� ������� �������� ��� ������
public:
    HashTable() { //�����������
        for (int i = 0; i < 255; ++i) {
            keys[i] = "";
            values[i] = "";
        }
    }
    std::string GetShortURL(const std::string& value) {
        for (int i = 0; i < 255; ++i) {
            if (values[i] == value) return keys[i];
        }
        std::string new_key = RandomURL();
        SetURL(new_key, value);
        return new_key;
    }
    std::string GetLongURL(const std::string& key) {
        if (values[FuncHash(key)] != "") return values[FuncHash(key)];
        return "No Found";
    }
private:
    void SetURL(const std::string& key, const std::string& value) {
        int index = FuncHash(key);
        bool is_coincidence = false;
        while (values[index] != "") {
            if (keys[index] == key) { //���� ���� ��������� � �����
                is_coincidence = true;
                break;
            }
            index = (index + FuncHashSec(key[0])) % 255; //���� ������ �� ������� ��������� � ���������
        }
        keys[index] = key;
        values[index] = value;
    }
    std::string RandomURL() {
        const std::string alphabet = "qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM1234567890";
        boost::minstd_rand generator(static_cast<unsigned int>(std::time(0)));
        boost::random::uniform_int_distribution<int> rand_alph(0, alphabet.length() - 1);
        boost::random::uniform_int_distribution<int> rand_len(3, 5);
        boost::random::variate_generator<boost::minstd_rand&, boost::random::uniform_int_distribution<int>> alph_gen(generator, rand_alph);
        boost::random::variate_generator<boost::minstd_rand&, boost::random::uniform_int_distribution<int>> len_gen(generator, rand_len);
        std::string url = "http://localhost:8080/";
        int len_url = len_gen();
        for (int i = 0; i < len_url; ++i) {
            url += alphabet[alph_gen()];
        }
        return url;
    }
    int FuncHash(std::string key) {
        int sum = 0;
        for (int i = 0; key[i]; ++i)
            sum += key[i];
        return sum % 100;
    }

    int FuncHashSec(char ch) {
        int sum = ch * 37;
        return sum % 100;
    }
    std::string keys[255];
    std::string values[255];
};

class Server {
public:
    Server(io_service& io_service) : acceptor_(io_service, ip::tcp::endpoint(ip::tcp::v4(), 6379)), data_base() { //����� ���������� ���� ������������� ������� ������ ������, � ������� ����� �������� ���� �����������
        StartAccept(io_service);
    }

private:
    void StartAccept(io_service& io_service) {
        while (true) {
            ip::tcp::socket socket(io_service);
            acceptor_.accept(socket);
            std::thread(&Server::HandleClient, this, std::move(socket)).detach();
        }
    }

    void HandleClient(ip::tcp::socket socket) {
        std::cout << "Client connect" << std::endl;
        boost::system::error_code error;
        streambuf request_buffer; // ��������� ��������� ������
        std::cout << "Read data start" << std::endl;
        read_until(socket, request_buffer, "\n\n", error); // ������ ������ �� ������ �� "\n\n"
        std::cout << "Read data successfuly" << std::endl;
        if (error) {
            std::cerr << error << std::endl;
            socket.close();
        }
        std::istream request_stream(&request_buffer);
        std::string request_line;
        getline(request_stream, request_line);
        if (request_line.find("POST /DATABASE/REF") != std::string::npos) { // ���� ������ POST ������
            std::cout << "POST /DATABASE/REF" << std::endl;
            std::string command;
            std::string arg;
            int complete = 0; //������� ��������� ����������
            while (true) {
                getline(request_stream, request_line); // ��������� ������ ������
                if (request_line.empty()) {
                    std::cerr << "Request empty";
                    break;
                }
                if (request_line.find("command=") != std::string::npos) {
                    size_t position = request_line.find("command=");
                    if (position != std::string::npos) {
                        command = request_line.substr(position + 8);
                        complete++;
                    }
                }
                else if (request_line.find("arg=") != std::string::npos) {
                    size_t position = request_line.find("arg=");
                    if (position != std::string::npos) {
                        arg = request_line.substr(position + 4);
                        complete++;
                    }
                }
                if (complete == 2) break; //��� ������ ��������� ��������
            }
            if (complete != 2) {
                std::cerr << "Mistake arg";
                socket.close();
                return;
            }
            std::string response; //����� �������
            if (command == "GetShortURL") {
                response = data_base.GetShortURL(Decrypt(arg));
            }
            else if(command == "GetLongURL") {
                response = data_base.GetLongURL(arg);
            }
            std::cout << "command = " << command << " arg = " << arg << " response = " << response << std::endl; //log
            WriteUrlResponse(response, socket);
            std::cout << "Response send" << std::endl;
            socket.close();
        }
        else if (request_line.find("POST /DATABASE/STATISTIC") != std::string::npos) {
            std::cout << "POST /DATABASE/STATISTIC" << std::endl;
            std::string url;
            std::string ip;
            std::string time;
            int complete = 0; //������� ��������� ����������
            while (true) {
                getline(request_stream, request_line); // ��������� ������ ������
                if (request_line.empty()) {
                    std::cerr << "Request empty";
                    break;
                }
                if (request_line.find("url=") != std::string::npos) {
                    size_t position = request_line.find("url=");
                    if (position != std::string::npos) {
                        url = request_line.substr(position + 4);
                        complete++;
                    }
                }
                else if (request_line.find("ip=") != std::string::npos) {
                    size_t position = request_line.find("ip=");
                    if (position != std::string::npos) {
                        ip = request_line.substr(position + 3);
                        complete++;
                    }
                }
                else if (request_line.find("time=") != std::string::npos) {
                    size_t position = request_line.find("time=");
                    if (position != std::string::npos) {
                        time = request_line.substr(position + 5);
                        complete++;
                    }
                }
                if (complete == 3) break; //��� ������ ��������� ��������
            }
            if (complete != 3) {
                std::cerr << "Mistake arg";
                socket.close();
                return;
            }
            visits.emplace_back(url, ip, time); //���������� � ������ ������ ���� Visit ��������� ����������� ������ ������� emplace_back
            using json = nlohmann::json;
            // 1. �������� ������������ ��������� �� �����
            json jsonOutput;
            std::ifstream inFile("visits.json");
            inFile.seekg(0, std::ios::end);
            if (inFile.tellg() != 0) {
                // ���� �� ������, ��������� ���������� � ���������� jsonOutput
                inFile.seekg(0, std::ios::beg);
                std::string fileContent((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
                jsonOutput = json::parse(fileContent);
            } else {
                // ���� ����, �������������� ������ ��������
                jsonOutput["visits"] = json::array();
            }
            inFile.close();
            // 2. ������� ����� ���������
            json jsonVisit; // ������� JSON ������ ��� ������� ���������.
            jsonVisit["url"] = url; // ��������� ���� "url".
            jsonVisit["time"] = time; // ��������� ���� "time".
            jsonVisit["ip"] = ip; // ��������� ���� "ip".
            jsonOutput["visits"].push_back(jsonVisit); // ��������� JSON ������ ��� ��������� � ������.
            // 3. ������� ����������� ������ ������� � ����
            std::ofstream outFile("visits.json");
            outFile << jsonOutput.dump(4) << std::endl;
            outFile.close();
            socket.close();
        }
        else if (request_line.find("GET /") != std::string::npos) {
            std::cout << "GET /" << std::endl;
            nlohmann::json data;
            std::ifstream input_file("visits.json");
            // ��������, ������� �� ������� ����
            if (input_file.is_open()) {
                // ������ ������ �� ����� � ������
                std::string file_contents((std::istreambuf_iterator<char>(input_file)), std::istreambuf_iterator<char>());

                // �������������� JSON
                try {
                    data = nlohmann::json::parse(file_contents);
                } catch (const std::exception& e) {
                    std::cerr << "������ ��� �������������� JSON: " << e.what() << std::endl;
                }

                // �������� �����
                input_file.close();
            } else {
                std::cerr << "�� ������� ������� ����" << std::endl;
            }
            
            WriteJsonResponse(data, socket);
            std::cout << "Response send" << std::endl;
            socket.close();
        }
        else {
            std::cout << "SOMETHING = \"" << request_line << "\"" << std::endl;
            socket.close();
        }
    }

    std::string Decrypt(std::string& encrypted_long_url) {
        std::string long_url;
        for (int i = 0; i < encrypted_long_url.size(); ++i) {
            if (encrypted_long_url[i] == '%' && i + 2 < encrypted_long_url.size()) {
                int first_symbol = encrypted_long_url[i + 1] < 58 ? encrypted_long_url[i + 1] - '0' : encrypted_long_url[i + 1] - 'A' + 10;
                int second_symbol = encrypted_long_url[i + 2] < 58 ? encrypted_long_url[i + 2] - '0' : encrypted_long_url[i + 2] - 'A' + 10;
                char symbol = (first_symbol << 4) | second_symbol;
                long_url.push_back(symbol);
                i += 2;
            }
            else if (encrypted_long_url[i] == '+') {
                long_url.push_back(' ');
            }
            else {
                long_url.push_back(encrypted_long_url[i]);
            }
        }
        return long_url;
    }
    void WriteUrlResponse(std::string& url, ip::tcp::socket& socket) {
        streambuf response;
        std::ostream response_stream (&response);
        response_stream << "QTP /1.0\n";
        response_stream << "url=" << url;
        response_stream << "\n\n";
        write(socket, response);
    }

    void WriteJsonResponse(nlohmann::json& json, ip::tcp::socket& socket) {
        streambuf response;
        std::ostream response_stream (&response);
        response_stream << "QTP /1.0\n";
        response_stream << json.dump();
        response_stream << "\n\n\n";
        write(socket, response);
    }

    ip::tcp::acceptor acceptor_;
    HashTable data_base;
    std::vector<Visit> visits;
};


int main() {
    try {
        io_service io_service;
        Server server(io_service);
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}