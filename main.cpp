#include "locationTracker.h"
#include <iostream>

int main() {
    try {
        net::io_context ioc;
        tcp::acceptor acceptor(ioc, { tcp::v4(), 9090 });
        auto store = std::make_shared<LocationStore>();

        std::cout << "Server running on http://0.0.0.0:9090\n";

        std::function<void()> do_accept;
        do_accept = [&]() {
            acceptor.async_accept([&](boost::system::error_code ec, tcp::socket socket) {
                if (!ec) {
                    socket.set_option(net::ip::tcp::no_delay(true));
                    std::make_shared<Session>(std::move(socket), store)->start();
                }
                do_accept();
            });
        };

        do_accept();

        // Run on 4 threads for concurrent connections
        std::vector<std::thread> threads;
        for (int i = 0; i < 4; ++i) {
            threads.emplace_back([&ioc]() { ioc.run(); });
        }

        for (auto& t : threads) {
            t.join();
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
}
