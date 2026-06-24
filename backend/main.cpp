/**
 * EmergiCare Backend – main.cpp
 * Entry point for the Drogon C++ web server.
 *
 * Responsibilities:
 *  1. Validate required environment variables (fail-fast)
 *  2. Configure the PostgreSQL database client from env vars
 *  3. Register global CORS headers (pre-routing + post-handling)
 *  4. Load server settings from config.json
 *  5. Start the Drogon event loop
 *
 * Controllers and filters are auto-discovered by Drogon via the
 * METHOD_LIST_BEGIN / ADD_METHOD_TO macros in each controller header.
 */

#include <drogon/drogon.h>
#include <json/json.h>
#include <iostream>
#include <cstdlib>
#include <string>

// ---------------------------------------------------------------------------
// Helper: read an env var or fall back to a default value
// ---------------------------------------------------------------------------
static std::string getEnv(const char* key, const char* defaultVal) {
    const char* val = std::getenv(key);
    return val ? std::string(val) : std::string(defaultVal);
}

int main() {
    // --------------------------------------------------------
    // 1. Fail fast if JWT_SECRET is missing
    // --------------------------------------------------------
    if (!std::getenv("JWT_SECRET")) {
        std::cerr << "[EmergiCare] FATAL: JWT_SECRET environment variable is not set.\n"
                  << "             Set it in your .env file or docker-compose.yml.\n";
        return EXIT_FAILURE;
    }

    // --------------------------------------------------------
    // 2. Configure PostgreSQL from environment variables
    // --------------------------------------------------------
    Json::Value dbConf;
    dbConf["name"]              = "default";
    dbConf["rdbms"]             = "postgresql";
    dbConf["host"]              = getEnv("DB_HOST",     "postgres");
    dbConf["port"]              = std::stoi(getEnv("DB_PORT", "5432"));
    dbConf["dbname"]            = getEnv("DB_NAME",     "emergicare_db");
    dbConf["user"]              = getEnv("DB_USER",     "emergicare");
    dbConf["passwd"]            = getEnv("DB_PASSWORD", "emergicare_secret");
    dbConf["connection_number"] = 5;
    dbConf["is_fast"]           = false;

    drogon::app().addDbClient(dbConf);

    // --------------------------------------------------------
    // 3. Global CORS support
    //    - Pre-routing: short-circuit OPTIONS preflight requests
    //    - Post-handling: add CORS headers to every response
    // --------------------------------------------------------
    drogon::app().registerPreRoutingAdvice(
        [](const drogon::HttpRequestPtr& req,
           drogon::FilterCallback&&      stop,
           drogon::FilterChainCallback&& next) {
            if (req->getMethod() == drogon::Options) {
                auto resp = drogon::HttpResponse::newHttpResponse();
                resp->addHeader("Access-Control-Allow-Origin",  "*");
                resp->addHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
                resp->addHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
                resp->setStatusCode(drogon::k200OK);
                stop(resp);
            } else {
                next();
            }
        });

    drogon::app().registerPostHandlingAdvice(
        [](const drogon::HttpRequestPtr&,
           const drogon::HttpResponsePtr& resp) {
            resp->addHeader("Access-Control-Allow-Origin",  "*");
            resp->addHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
            resp->addHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
        });

    // --------------------------------------------------------
    // 4. Load server configuration (port, threads, log level)
    // --------------------------------------------------------
    drogon::app().loadConfigFile("config.json");

    // --------------------------------------------------------
    // 5. Start the event loop (blocking)
    // --------------------------------------------------------
    std::cout << "[EmergiCare] Server starting on port "
              << getEnv("PORT", "3001") << " ...\n"
              << "[EmergiCare] DB Host: " << getEnv("DB_HOST", "postgres") << "\n";

    drogon::app().run();
    return EXIT_SUCCESS;
}
