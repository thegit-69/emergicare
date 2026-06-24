#include <drogon/drogon.h>
#include <json/json.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <string>

static std::string getEnv(const char* key, const char* defaultVal) {
    const char* val = std::getenv(key);
    return (val && strlen(val) > 0) ? std::string(val) : std::string(defaultVal);
}

int main() {
    if (!std::getenv("JWT_SECRET") || strlen(std::getenv("JWT_SECRET")) == 0) {
        std::cerr << "[EmergiCare] FATAL: JWT_SECRET is not set!\n";
        return EXIT_FAILURE;
    }

    // 1. Read existing config.json
    std::ifstream file("config.json");
    Json::Value config;
    if (file.is_open()) {
        file >> config;
        file.close();
    } else {
        std::cerr << "[EmergiCare] FATAL: Failed to read config.json\n";
        return EXIT_FAILURE;
    }

    // 2. Inject DB configuration dynamically
    Json::Value dbConf;
    dbConf["name"] = "default";
    dbConf["rdbms"] = "postgresql";
    dbConf["connection_number"] = 5;

    const char* dbUrl = std::getenv("DATABASE_URL");
    if (dbUrl && strlen(dbUrl) > 0) {
        std::cout << "[EmergiCare] DB Mode: Connection String (managed DB)\n";
        std::string url(dbUrl);
        std::string prefix = "postgresql://";
        if (url.find(prefix) == 0) url = url.substr(prefix.length());
        
        auto atPos = url.find('@');
        if (atPos != std::string::npos) {
            std::string creds = url.substr(0, atPos);
            std::string rest = url.substr(atPos + 1);
            
            auto colonPos = creds.find(':');
            if (colonPos != std::string::npos) {
                dbConf["user"] = creds.substr(0, colonPos);
                dbConf["passwd"] = creds.substr(colonPos + 1);
            } else {
                dbConf["user"] = creds;
                dbConf["passwd"] = "";
            }
            
            auto slashPos = rest.find('/');
            std::string hostPort = rest.substr(0, slashPos);
            std::string dbNameQuery = (slashPos != std::string::npos) ? rest.substr(slashPos + 1) : "";
            
            auto hpColon = hostPort.find(':');
            if (hpColon != std::string::npos) {
                dbConf["host"] = hostPort.substr(0, hpColon);
                dbConf["port"] = std::stoi(hostPort.substr(hpColon + 1));
            } else {
                dbConf["host"] = hostPort;
                dbConf["port"] = 5432;
            }
            
            auto qPos = dbNameQuery.find('?');
            if (qPos != std::string::npos) {
                std::string dbName = dbNameQuery.substr(0, qPos);
                std::string query = dbNameQuery.substr(qPos + 1);
                dbConf["dbname"] = dbName;
                if (query.find("sslmode=require") != std::string::npos) {
#ifdef _WIN32
                    _putenv_s("PGSSLMODE", "require");
#else
                    setenv("PGSSLMODE", "require", 1);
#endif
                }
            } else {
                dbConf["dbname"] = dbNameQuery;
            }
        }
    } else {
        std::cout << "[EmergiCare] DB Mode: Individual params (local dev)\n";
        dbConf["host"] = getEnv("DB_HOST", "postgres");
        dbConf["port"] = std::stoi(getEnv("DB_PORT", "5432"));
        dbConf["dbname"] = getEnv("DB_NAME", "emergicare_db");
        dbConf["user"] = getEnv("DB_USER", "emergicare");
        dbConf["passwd"] = getEnv("DB_PASSWORD", "emergicare_secret");
        
        std::cout << "[EmergiCare] DB Host: " << dbConf["host"].asString() << ":" << dbConf["port"].asInt() << "\n";
    }

    config["db_clients"].append(dbConf);

    // 3. Load configuration via Json::Value
    drogon::app().loadConfigJson(config);

    // 4. CORS Setup
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

    std::cout << "[EmergiCare] Starting server on port " << getEnv("PORT", "3001") << " ...\n";

    drogon::app().run();
    return EXIT_SUCCESS;
}
