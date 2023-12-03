#include <iostream>
#include <vector>
#include <boost/asio.hpp>
#include <nlohmann/json.hpp>

#define ADDRESS "192.168.1.5"

using namespace boost::asio;

class Server {
public:
    Server(io_service& io_service) : acceptor_(io_service, ip::tcp::endpoint(ip::tcp::v4(), 2056)) { //После многоточий идет инициализация сложных членов класса, к которым будет применен свой конструктор
        StartAccept(io_service);
    }

private:
    void StartAccept(io_service& io_service) {
        while (true) {
            ip::tcp::socket socket(io_service);
            acceptor_.accept(socket);
            HandleClient(socket);
        }
    }
    void HandleClient(ip::tcp::socket& socket) {
        std::cout << "Client connect" << std::endl;
        boost::system::error_code error;
        streambuf request_buffer; // временное хранилище данных
        std::cout << "Read data start" << std::endl;
        read_until(socket, request_buffer, "\n\n", error); // чтение данных из сокета до "\n"
        std::cout << "Read data successfuly" << std::endl;
        if (error) {
            std::cerr << error << std::endl;
            socket.close();
        }
        std::istream request_stream(&request_buffer);
        std::string request_line;
        getline(request_stream, request_line);
        if (request_line.find("POST /STATISTIC") != std::string::npos) { // если пришел POST запрос просто перенаправляем данные в БД
            std::cout << "POST" << std::endl;
            std::string url;
            std::string ip;
            std::string time;
            int complete = 0; //счетчик считанных аргументов
            while (true) {
                getline(request_stream, request_line); // извлекаем каждую строку
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
                if (complete == 3) break; //все нужные аргументы получены
            }
            if (complete != 3) {
                std::cerr << "Mistake arg";
                socket.close();
                return;
            }
            try { //отправка в БД
                io_service io_service_client;
                ip::tcp::resolver resolver(io_service_client);
                ip::tcp::socket socket_client(io_service_client);
                connect(socket_client, resolver.resolve(ip::tcp::resolver::query (ADDRESS, "6379")));
                streambuf request;
                std::ostream request_stream(&request);
                request_stream << "POST /DATABASE/STATISTIC QTP/1.0\n";
                request_stream << "url=" << url << "\n";
                request_stream << "ip=" << ip << "\n";
                request_stream << "time=" << time << "\n";
                request_stream << "\n";
                write(socket_client, request);
                socket_client.close();
            }
            catch (std::exception& e) {
                std::cerr << e.what() << std::endl;
            } 
            socket.close();
        }
        else if (request_line.find("POST /REPORT") != std::string::npos) { 
            //TODO: возврат на запрос, десериализация, детализация, обработка сигнала остановки и сериализация в файл
            std::cout << "POST /REPORT" << std::endl;
            nlohmann::json data;
            std::string priority_1;
            std::string priority_2;
            std::string priority_3;
            while (true) {
                getline(request_stream, request_line); // извлекаем каждую строку
                if (request_line.empty()) {
                    std::cerr << "Request empty";
                    break;
                }
                if (request_line.find("parametrs=") != std::string::npos) {
                    int position_ip = request_line.find("ip");
                    int position_time = request_line.find("time");
                    int position_url = request_line.find("url");
                    if (position_ip == std::string::npos || position_time == std::string::npos || position_url == std::string::npos) {
                        std::cerr << "Invalid request"; //TODO: Возврат клиенту отчета об ошибке
                        priority_1 = "ip";
                        priority_2 = "time"; //Пока что, чтобы не вводить каждый раз
                        priority_3 = "url";
                        break;
                    }
                    if (position_ip < position_time && position_ip < position_url) {
                        priority_1 = "ip";
                        if (position_time < position_url) priority_2 = "time", priority_3 = "url";
                        else priority_2 = "url", priority_3 = "time";
                    }
                    else if (position_time < position_ip && position_time < position_url) {
                        priority_1 = "time";
                        if (position_ip < position_url) priority_2 = "ip", priority_3 = "url";
                        else priority_2 = "url", priority_3 = "ip";
                    }
                    else if (position_url < position_ip && position_url < position_time) {
                        priority_1 = "url";
                        if (position_time < position_ip) priority_2 = "time", priority_3 = "ip";
                        else priority_2 = "ip", priority_3 = "time";
                    }
                    break;
                }
            }
            try { //отправка в БД
                io_service io_service_client;
                ip::tcp::resolver resolver(io_service_client);
                ip::tcp::socket socket_client(io_service_client);
                connect(socket_client, resolver.resolve(ip::tcp::resolver::query (ADDRESS, "6379")));
                streambuf request;
                std::ostream request_stream(&request);
                request_stream << "GET / QTP/1.0\n";
                request_stream << "\n";
                write(socket_client, request);
                streambuf response_buffer;
                read_until(socket_client, response_buffer, "\n\n", error);
                if (error) {
                    std::cerr << error << std::endl;
                    socket_client.close();
                }
                std::istream response_stream(&response_buffer);
                std::string response_line;
                getline(response_stream, response_line);
                if (response_line.find("QTP /") != std::string::npos) {
                    std::cout << "QTP" << std::endl;
                    getline(response_stream, response_line);
                    try {
                        data = nlohmann::json::parse(response_line); //Для дальнейшей обработки JSON данных
                    } catch (const std::exception& e) {
                        std::cerr << "Error parsing JSON: " << e.what() << std::endl;
                    }
                }
                socket_client.close();
            }
            catch (std::exception& e) {
                std::cerr << e.what() << std::endl;
            } 
            nlohmann::json reports = nlohmann::json::array(); //Главный массив объектов
            int id = 1;
            for (const auto& visit : data["visits"]) {
                // Вывод информации о каждом посещении
                std::cout << "IP: " << visit["ip"] << ", Time: " << visit["time"] << ", URL: " << visit["url"] << std::endl;
                nlohmann::json report_priority_1, report_priority_2, report_priority_3; // Создание нового объекта "report" для каждого посещения
                if (priority_1 == "ip") {

                    if (reports.empty()) {//Первая добавление
                        report_priority_1["id"] = id++;
                        report_priority_1["pid"] = 0;
                        report_priority_1["ip"] = visit["ip"];
                        report_priority_1["time"] = NULL;
                        report_priority_1["url"] = NULL;
                        report_priority_1["count"] = 1;

                        if (priority_2 == "time") {
                            report_priority_2["id"] = id++;
                            report_priority_2["pid"] = report_priority_1["id"];
                            report_priority_2["ip"] = NULL;
                            report_priority_2["time"] = visit["time"];
                            report_priority_2["url"] = NULL;
                            report_priority_2["count"] = 1;

                            report_priority_3["id"] = id++;
                            report_priority_3["pid"] = report_priority_2["id"];
                            report_priority_3["ip"] = NULL;
                            report_priority_3["time"] = NULL;
                            report_priority_3["url"] = visit["url"];
                            report_priority_3["count"] = 1;
                        }
                        else {
                            report_priority_2["id"] = id++;
                            report_priority_2["pid"] = report_priority_1["id"];
                            report_priority_2["ip"] = NULL;
                            report_priority_2["time"] = NULL;
                            report_priority_2["url"] = visit["url"];
                            report_priority_2["count"] = 1;

                            report_priority_3["id"] = id++;
                            report_priority_3["pid"] = report_priority_2["id"];
                            report_priority_3["ip"] = NULL;
                            report_priority_3["time"] = visit["time"];
                            report_priority_3["url"] = NULL;
                            report_priority_3["count"] = 1;
                        }
                        reports.push_back(report_priority_1);
                        reports.push_back(report_priority_2);
                        reports.push_back(report_priority_3);
                    }
                    else {
                        int idParrentIp = 0;
                        for (auto& old_report : reports) {
                            if (old_report["ip"] == visit["ip"]) {
                                old_report["count"] = old_report.value("count", 1) + 1;
                                idParrentIp = old_report["id"].get<int>();
                                break;
                            }
                        }
                        if (idParrentIp == 0) {
                            report_priority_1["id"] = id++;
                            report_priority_1["pid"] = 0;
                            report_priority_1["ip"] = visit["ip"];
                            report_priority_1["time"] = NULL;
                            report_priority_1["url"] = NULL;
                            report_priority_1["count"] = 1;
                            if (priority_2 == "time") {
                                report_priority_2["id"] = id++;
                                report_priority_2["pid"] = report_priority_1["id"];
                                report_priority_2["ip"] = NULL;
                                report_priority_2["time"] = visit["time"];
                                report_priority_2["url"] = NULL;
                                report_priority_2["count"] = 1;

                                report_priority_3["id"] = id++;
                                report_priority_3["pid"] = report_priority_2["id"];
                                report_priority_3["ip"] = NULL;
                                report_priority_3["time"] = NULL;
                                report_priority_3["url"] = visit["url"];
                                report_priority_3["count"] = 1;
                            }
                            else {
                                report_priority_2["id"] = id++;
                                report_priority_2["pid"] = report_priority_1["id"];
                                report_priority_2["ip"] = NULL;
                                report_priority_2["time"] = NULL;
                                report_priority_2["url"] = visit["url"];
                                report_priority_2["count"] = 1;

                                report_priority_3["id"] = id++;
                                report_priority_3["pid"] = report_priority_2["id"];
                                report_priority_3["ip"] = NULL;
                                report_priority_3["time"] = visit["time"];
                                report_priority_3["url"] = NULL;
                                report_priority_3["count"] = 1;
                            }
                            reports.push_back(report_priority_1);
                            reports.push_back(report_priority_2);
                            reports.push_back(report_priority_3);
                        }
                        else { //Если ip существовал
                            if (priority_2 == "time") {

                                int idParrentTime = 0;
                                for (auto& old_report : reports) {
                                    int pid = old_report["pid"].get<int>();
                                    std::cout << old_report["time"] << " " << visit["time"] << std::endl;
                                    if ((old_report["time"] == visit["time"]) && (pid == idParrentIp)) {
                                        old_report["count"] = old_report.value("count", 1) + 1;
                                        idParrentTime = old_report["id"].get<int>();
                                        break;
                                    }
                                }
                                if (idParrentTime == 0) {
                                        report_priority_2["id"] = id++;
                                        report_priority_2["pid"] = idParrentIp;
                                        report_priority_2["ip"] = NULL;
                                        report_priority_2["time"] = visit["time"];
                                        report_priority_2["url"] = NULL;
                                        report_priority_2["count"] = 1;

                                        report_priority_3["id"] = id++;
                                        report_priority_3["pid"] = report_priority_2["id"];
                                        report_priority_3["ip"] = NULL;
                                        report_priority_3["time"] = NULL;
                                        report_priority_3["url"] = visit["url"];
                                        report_priority_3["count"] = 1;

                                        reports.push_back(report_priority_2);
                                        reports.push_back(report_priority_3);
                                    }
                                else {
                                    int idParrentUrl = 0;
                                    for (auto& old_report : reports) {
                                        if ((old_report["url"] == visit["url"]) && (old_report["pid"].get<int>() == idParrentTime)) { //Надо сделать не прямой равенство по времени, а интервальное в минуту, я хочу обрезать строку с секнудами и таким образом сравнивать минуты
                                            old_report["count"] = old_report.value("count", 1) + 1;
                                            idParrentUrl = old_report["id"];
                                            break;
                                        }
                                    }
                                    if (idParrentUrl == 0) {
                                        report_priority_3["id"] = id++;
                                        report_priority_3["pid"] = idParrentTime;
                                        report_priority_3["ip"] = NULL;
                                        report_priority_3["time"] = NULL;
                                        report_priority_3["url"] = visit["url"];
                                        report_priority_3["count"] = 1;

                                        reports.push_back(report_priority_3);
                                    }
                                }
                            }
                            else { //Если сначала юрл, а не время
                                int idParrentUrl = 0;
                                for (auto& old_report : reports) {
                                    if ((old_report["url"] == visit["url"]) && (old_report["pid"].get<int>() == idParrentIp)) { //Надо сделать не прямой равенство по времени, а интервальное в минуту, я хочу обрезать строку с секнудами и таким образом сравнивать минуты
                                        old_report["count"] = old_report.value("count", 1) + 1;
                                        idParrentUrl = old_report["id"].get<int>();
                                        break;
                                    }
                                }
                                if (idParrentUrl == 0) {
                                    report_priority_2["id"] = id++;
                                    report_priority_2["pid"] = report_priority_1["id"];
                                    report_priority_2["ip"] = NULL;
                                    report_priority_2["time"] = NULL;
                                    report_priority_2["url"] = visit["url"];
                                    report_priority_2["count"] = 1;

                                    report_priority_3["id"] = id++;
                                    report_priority_3["pid"] = report_priority_2["id"];
                                    report_priority_3["ip"] = NULL;
                                    report_priority_3["time"] = visit["time"];
                                    report_priority_3["url"] = NULL;
                                    report_priority_3["count"] = 1;

                                    reports.push_back(report_priority_2);
                                    reports.push_back(report_priority_3);
                                }
                                else {
                                    int idParrentTime = 0;
                                    for (auto& old_report : reports) {
                                        if ((old_report["url"] == visit["url"]) && (old_report["pid"].get<int>() == idParrentUrl)) { //Надо сделать не прямой равенство по времени, а интервальное в минуту, я хочу обрезать строку с секнудами и таким образом сравнивать минуты
                                            old_report["count"] = old_report.value("count", 1) + 1;
                                            idParrentTime = old_report["id"];
                                            break;
                                        }
                                    }
                                    if (idParrentTime == 0) {
                                        report_priority_3["id"] = id++;
                                        report_priority_3["pid"] = report_priority_2["id"];
                                        report_priority_3["ip"] = NULL;
                                        report_priority_3["time"] = visit["time"];
                                        report_priority_3["url"] = NULL;
                                        report_priority_3["count"] = 1;

                                        reports.push_back(report_priority_3);
                                    }
                                }
                            }
                        }
                    }
                }
                else if (priority_1 == "time") {
                    report_priority_1["pid"] = NULL;
                    report_priority_1["ip"] = NULL;
                    report_priority_1["time"] = visit["time"];
                    report_priority_1["url"] = NULL;
                    if (priority_2 == "ip") {
                        report_priority_2["id"] = id++;
                        report_priority_2["pid"] = report_priority_1["id"];
                        report_priority_2["ip"] = visit["ip"];
                        report_priority_2["time"] = NULL;
                        report_priority_2["url"] = NULL;

                        report_priority_3["id"] = id++;
                        report_priority_3["pid"] = report_priority_2["id"];
                        report_priority_3["ip"] = NULL;
                        report_priority_3["time"] = NULL;
                        report_priority_3["url"] = visit["url"];
                    }
                    else {
                        report_priority_2["id"] = id++;
                        report_priority_2["pid"] = report_priority_1["id"];
                        report_priority_2["ip"] = NULL;
                        report_priority_2["time"] = NULL;
                        report_priority_2["url"] = visit["url"];

                        report_priority_3["id"] = id++;
                        report_priority_3["pid"] = report_priority_2["id"];
                        report_priority_3["ip"] = visit["ip"];
                        report_priority_3["time"] = NULL;
                        report_priority_3["url"] = NULL;
                    }
                }
                else if (priority_1 == "url") {
                    report_priority_1["pid"] = NULL;
                    report_priority_1["ip"] = NULL;
                    report_priority_1["time"] = NULL;
                    report_priority_1["url"] = visit["url"];
                    if (priority_2 == "ip") {
                        report_priority_2["id"] = id++;
                        report_priority_2["pid"] = report_priority_1["id"];
                        report_priority_2["ip"] = visit["ip"];
                        report_priority_2["time"] = NULL;
                        report_priority_2["url"] = NULL;

                        report_priority_3["id"] = id++;
                        report_priority_3["pid"] = report_priority_2["id"];
                        report_priority_3["ip"] = NULL;
                        report_priority_3["time"] = visit["time"];
                        report_priority_3["url"] = NULL;
                    }
                    else {
                        report_priority_2["id"] = id++;
                        report_priority_2["pid"] = report_priority_1["id"];
                        report_priority_2["ip"] = NULL;
                        report_priority_2["time"] = visit["time"];
                        report_priority_2["url"] = NULL;

                        report_priority_3["id"] = id++;
                        report_priority_3["pid"] = report_priority_2["id"];
                        report_priority_3["ip"] = visit["ip"];
                        report_priority_3["time"] = NULL;
                        report_priority_3["url"] = NULL;
                    }
                }
                std::cout << "\n\n json::reports\n" << reports.dump() << std::endl;
            }
            std::cout << "\n\n json::reports\n" << reports.dump() << std::endl;
        }
        else {
            std::cout << "SOMETHING = \"" << request_line << "\"" << std::endl;
            socket.close();
        }
    }
    ip::tcp::acceptor acceptor_;
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
