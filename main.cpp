#include <boost/asio.hpp>
#include "server.hpp"
#include "db.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
   const char* port_env = getenv("PORT");
   int port = port_env ? std::stoi(port_env) : 5050;
   
    //const std::string pgconn = "host=127.0.0.1 port=5432 user=postgres password=postgres dbname=evdata";
	
	std::string pgconn = "host=" + std::string(getenv("PGHOST")? getenv("PGHOST"):"127.0.0.1")
		+ " user=" + std::string(getenv("PGUSER")? getenv("PGUSER"):"postgres")
		+ " password=" + std::string(getenv("PGPASSWORD")? getenv("PGPASSWORD"):"postgres")
		+ " dbname=" + std::string(getenv("PGDATABASE")? getenv("PGDATABASE"):"evdata");

    try {
        DB db(pgconn);
        boost::asio::io_context io_ctx{1};
        Server srv(io_ctx, port, db);

        // run thread pool
        size_t threads = std::max<size_t>(2, std::thread::hardware_concurrency());
        std::vector<std::thread> pool;
        for (size_t i = 0; i < threads; ++i) {
            pool.emplace_back([&]{ io_ctx.run(); });
        }

        for (auto &t : pool) t.join();
    } catch (std::exception &e) {
        std::cerr << "Fatal: " << e.what() << "\n";
    }
}


