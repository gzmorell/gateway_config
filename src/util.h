//
// Created by gzmorell on 23/03/25.
//

#ifndef UTIL_H
#define UTIL_H

#include "subprocess.hpp"
#include <nlohmann/json.hpp>
#include "ftxui/component/screen_interactive.hpp"  // for ScreenInteractive

using json = nlohmann::json;
namespace fs = std::filesystem;

struct NetWorkConfig
{
    std::string interface = "eth0";
    std::string address = "192.168.1.158";
    std::string netmask = "255.255.255.0";
    std::string gateway = "192.168.1.1";
};

struct GatewayConfig
{
    std::string version;
    std::string ils_address = "192.168.1.156";
    int ils_port = 3001;
    std::string master_password = "MASTER1 MASTER2 MASTER3";
    std::string user_name = "GATEWAY";
    std::string user_password = "GATEWAY1 GATEWAY2 GATEWAY3";
    int server_port = 3001;
    std::string server_password = "nolosabeNadie!";
    std::string priority_address = "0.0.0.0";
    std::vector<std::string> trap_ips = {"10.0.0.2"};
};

inline void ltrim(std::string &s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
                return !std::isspace(ch);
            }));
}

// trim from end (in place)
inline void rtrim(std::string &s)
{
    s.erase(
        std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(),
        s.end());
}

inline void trim(std::string &s)
{
    rtrim(s);
    ltrim(s);
}

inline std::string getPassword()
{
    fs::path Path(R"(/etc/pasarela/settings.json)");
    std::fstream configFile(Path);
    std::string password_plain;
    auto data = json::parse(configFile);

    try {
        std::string password_encrypted = data["Server"]["Password"];
        std::string decrypt = "setup-gw -d ";
        std::string caca;
        auto cmd = subprocess::command{decrypt + password_encrypted};
        cmd > password_plain;
        cmd >= caca;

        try {
            cmd.run();
            trim(password_plain);
        } catch (...) {
            password_plain = "nolosabeNadie!";
        }
    } catch (...) {
        password_plain = "nolosabeNadie!";
    }
    return password_plain;
}

inline void load_gateway_config(GatewayConfig &gw_config)
{
    fs::path Path(R"(/etc/pasarela/settings.json)");
    std::fstream configFile(Path);
    // std::cout << "Trying to open the file" << std::endl;
    try {
        auto data = json::parse(configFile);
        try {
            std::string ils_address;
            ils_address = data["Ils"]["IpAddress"];
            gw_config.ils_address = ils_address;
        } catch (...) {
        }
        try {
            int ils_port;
            ils_port = data["Ils"]["Port"];
            gw_config.ils_port = ils_port;
        } catch (...) {
        }
        try {
            int server_port;
            server_port = data["Server"]["Port"];
            gw_config.server_port = server_port;
        } catch (...) {
        }
        try {
            std::string user_name_encrypted = data["Ils"]["User"]["Name"];
            std::string user_name;
            std::string decrypt = "de -d ";
            std::string caca;
            auto cmd = subprocess::command{decrypt + user_name_encrypted};
            cmd > user_name;
            cmd >= caca;
            try {
                cmd.run();
                trim(user_name);
                gw_config.user_name = user_name;
            } catch (...) {
            }
        } catch (...) {
        }
        try {
            std::vector<std::string> trap_ips;
            trap_ips = data["TrapIps"];
            gw_config.trap_ips = trap_ips;
        } catch (...) {
        }
        try {
            std::string version;
            version = data["Version"];
            gw_config.version = version;
        } catch (...) {
        }
        try {
            std::string priority_address;
            priority_address = data["Server"]["PriorityAddress"];
            gw_config.priority_address = priority_address;
        } catch (...) {
        }
        try {
            std::string user_password_encrypted = data["Ils"]["User"]["Passwords"];
            std::string user_password;
            std::string decrypt = "de -d ";
            std::string caca;
            auto cmd = subprocess::command{decrypt + user_password_encrypted};
            cmd > user_password;
            cmd >= caca;
            try {
                cmd.run();
                trim(user_password);
                gw_config.user_password = user_password;
            } catch (...) {
            }
        } catch (...) {
        }
        try {
            std::string master_password_encrypted = data["Ils"]["Master"];
            std::string master_password;
            std::string decrypt = "de -d ";
            std::string caca;
            auto cmd = subprocess::command{decrypt + master_password_encrypted};
            cmd > master_password;
            cmd >= caca;
            try {
                cmd.run();
                trim(master_password);
                gw_config.master_password = master_password;
            } catch (...) {
            }
        } catch (...) {
        }
        try {
            std::string server_password_encrypted = data["Server"]["Password"];
            std::string server_password;
            std::string decrypt = "de -d ";
            std::string caca;
            auto cmd = subprocess::command{decrypt + server_password_encrypted};
            cmd > server_password;
            cmd >= caca;
            try {
                cmd.run();
                trim(server_password);
                gw_config.server_password = server_password;
            } catch (...) {
            }
        } catch (...) {
        }
    } catch (...) {
        return;
    }
}

inline void nested(const std::string &message, bool ok = true)
{
    auto screen = ScreenInteractive::Fullscreen();
    Color status_color;
    if (ok) {
        status_color = Color::DarkGreen;
    } else {
        status_color = Color::DarkRed;
    }
    auto back_button = Button("Back", screen.ExitLoopClosure(), ButtonOption::Animated());
    const auto layout = Container::Vertical({
        back_button,
    });
    const auto renderer = Renderer(layout, [&] {
        return vbox(
                   {separatorEmpty() | bgcolor(status_color) | flex,
                    paragraphAlignCenter(message) | bold | bgcolor(status_color) | color(Color::White),
                    separatorEmpty() | bgcolor(status_color) | flex,
                    separator(),
                    separatorEmpty() | flex,
                    hbox({
                        separatorEmpty() | flex,
                        back_button->Render(),
                        separatorEmpty() | flex,
                    }),

                    separatorEmpty() | flex,
                   })
               | border;
    });
    screen.Loop(renderer);
}

#endif //UTIL_H
