#include <iostream>
#include <thread>
#include <ctime>
#include <boost/asio.hpp>
#include <boost/random.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#define ADDRESS "192.168.1.4" //поменять в случае смены локального адреса

using namespace boost::asio;

class Server {
public:
    Server(io_service& io_service) : acceptor_(io_service, ip::tcp::endpoint(ip::tcp::v4(), 8080)) { //После многоточий идет инициализация членов класса, при вызове функции, в данном случае конструктора
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
        streambuf request_buffer; // временное хранилище данных
        std::cout << "Read data start" << std::endl;
        read_until(socket, request_buffer, "\r\n\r\n", error); // чтение данных из сокета до "\r\n\r\n"
        std::cout << "Read data successfuly" << std::endl;
        if (error) {
            std::cerr << error << std::endl;
            socket.close();
        }
        std::istream request_stream(&request_buffer);
        std::string request_line;
        getline(request_stream, request_line);
        if (request_line.find("POST /SHORTEN") != std::string::npos) { // если пришел POST запрос
            std::cout << "POST" << std::endl;
            std::string encrypted_long_url;
            while (true) {
                getline(request_stream, request_line); // извлекаем каждую строку
                if (request_line.empty()) {
                    std::cerr << "Request empty";
                    break;
                }
                if (request_line.find("url=") != std::string::npos) { //Если что то нашел(вернул позицию)
                    encrypted_long_url = request_line.substr(request_line.find("url=") + 4); // +4, чтобы пропустить "URL:"
                    break;
                }
            }
            std::string short_url;
            try { //отправка в БД
                io_service io_service_client;
                ip::tcp::resolver resolver(io_service_client);
                ip::tcp::socket socket_client(io_service_client);
                connect(socket_client, resolver.resolve(ip::tcp::resolver::query (ADDRESS, "6379")));
                streambuf request;
                std::ostream request_stream(&request);
                request_stream << "POST /DATABASE/REF QTP/1.0\n";
                request_stream << "command=GetShortURL\n";
                request_stream << "arg=" << encrypted_long_url;
                request_stream << "\n\n";
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
                    while (true) {
                        getline(response_stream, response_line);
                        if (response_line.find("url=") != std::string::npos) {
                            short_url = response_line.substr(response_line.find("url=") + 4);
                            break;
                        }
                    }
                }
                socket_client.close();
            }
            catch (std::exception& e) {
                std::cerr << e.what() << std::endl;
            }
            std::cout << short_url << std::endl;
            WriteResponse(short_url, socket, 200);
            std::cout << "Response send" << std::endl;
            socket.close();
        }
        else if (request_line.find("GET /") != std::string::npos) {
            std::cout << "GET" << std::endl;
            int start = request_line.find("GET /") + 5;
            int end = request_line.find(' ', start);
            std::string encrypted_short_url = "http://localhost:8080/" + request_line.substr(start, end - start);
            std::string long_url;
            try { //отправка в БД
                io_service io_service_client;
                ip::tcp::resolver resolver(io_service_client);
                ip::tcp::socket socket_client(io_service_client);
                connect(socket_client, resolver.resolve(ip::tcp::resolver::query (ADDRESS, "6379")));
                streambuf request;
                std::ostream request_stream(&request);
                request_stream << "POST /DATABASE/REF QTP/1.0\n";
                request_stream << "command=GetLongURL\n";
                request_stream << "arg=" << encrypted_short_url;
                request_stream << "\n\n";
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
                    while (true) {
                        getline(response_stream, response_line);
                        if (response_line.find("url=") != std::string::npos) {
                            long_url = response_line.substr(response_line.find("url=") + 4);
                            break;
                        }
                    }
                }
                socket_client.close();
            }
            catch (std::exception& e) {
                std::cerr << e.what() << std::endl;
            }
            if (long_url != "No Found") {
                try { //отправка статистики
                    io_service io_service_client;
                    ip::tcp::resolver resolver(io_service_client);
                    ip::tcp::socket socket_client(io_service_client);
                    connect(socket_client, resolver.resolve(ip::tcp::resolver::query (ADDRESS, "2056")));
                    streambuf request;
                    boost::posix_time::time_facet* facet = new boost::posix_time::time_facet("%Y-%m-%d %H:%M");
                    std::stringstream ss;
                    ss.imbue(std::locale(std::locale(), facet));
                    ss << boost::posix_time::second_clock::local_time();
                    std::string time_string = ss.str();
                    std::ostream request_stream(&request);
                    request_stream << "POST /STATISTIC QTP/1.0\n";
                    request_stream << "url=" << long_url <<" (" << encrypted_short_url << ")\n";
                    request_stream << "ip=" << socket.remote_endpoint().address().to_string() << "\n";
                    request_stream << "time=" << time_string << "\n";
                    request_stream << "\n";
                    write(socket_client, request);
                    socket_client.close();
                }
                catch (std::exception& e) {
                    std::cerr << e.what() << std::endl;
                }
                WriteResponse(long_url, socket, 302);
                socket.close();
            }
            else {
                WriteResponse(long_url, socket, 404);
                socket.close();
            }
        }
        else {
            std::cout << "SOMETHING = \"" << request_line << "\"" << std::endl;
            socket.close();
        }
    }
    void WriteResponse(std::string& url, ip::tcp::socket& socket, int code) {
        streambuf response;
        std::ostream response_stream (&response);
        if (code == 200) {
            response_stream << "HTTP/1.1 200 OK\r\n";
            response_stream << "Access-Control-Allow-Origin: *\r\n";
            response_stream << "Access-Control-Allow-Methods: GET, POST, PUT, DELETE\r\n";
            response_stream << "Access-Control-Allow-Headers: Content-Type, Authorization\r\n";
            response_stream << "Content-Type: text/html\r\n";
            response_stream << "Content-Length: " << url.size() << "\r\n";
            response_stream << "\r\n";
            response_stream << url;
        }
        else if (code == 302) {
            response_stream << "HTTP/1.1 302 Found\r\n";
            response_stream << "Access-Control-Allow-Origin: *\r\n";
            response_stream << "Access-Control-Allow-Methods: GET, POST, PUT, DELETE\r\n";
            response_stream << "Access-Control-Allow-Headers: Content-Type, Authorization\r\n";
            response_stream << "Content-Type: text/html\r\n";
            response_stream << "Location: " << url << "\r\n";
            response_stream << "\r\n";
        }
        else if (code == 404) {
            response_stream << "HTTP/1.1 404 Not Found\r\n";
            response_stream << "Access-Control-Allow-Origin: *\r\n";
            response_stream << "Access-Control-Allow-Methods: GET, POST, PUT, DELETE\r\n";
            response_stream << "Access-Control-Allow-Headers: Content-Type, Authorization\r\n";
            response_stream << "Content-Type: text/html\r\n";
            response_stream << "Content-Length: 9\r\n";
            response_stream << "\r\n";
            response_stream << "Not Found";
        }
        write(socket, response);
    }

    ip::tcp::acceptor acceptor_;
};


int main() {
    setlocale(LC_ALL, "Ru");
    try {
        io_service io_service;
        Server server(io_service);
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}