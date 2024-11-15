﻿#pragma once

#include "Client.hpp"
#include "GUI/Engine/Engine.hpp"
#include "../Utils/Versions/VersionUtils.hpp"
#include <filesystem>
#include <thread>
#include <wingdi.h>
#include <wininet.h>

//winrt stuff
#include "winrt/windows.applicationmodel.core.h"
#include "winrt/Windows.UI.ViewManagement.h"
#include "winrt/Windows.UI.Core.h"
#include "winrt/windows.system.h"


std::string Client::settingspath = Utils::getRoamingPath() + R"(\Flarial\Config\main.flarial)";
Settings Client::settings = Settings();
std::vector<std::string> Client::onlinePlayers;

bool notifiedOfConnectionIssue = false;

std::string Client::current_commit = COMMIT_HASH;



void DownloadAndSave(const std::string& url, const std::string& path) {

    if (Client::settings.getSettingByName<bool>("dlassets")->value || !std::filesystem::exists(path)) {
        char test[256];
        strcpy_s(test, "https://flarialbackup.ashank.tech/");
        if (InternetCheckConnectionA(test, FLAG_ICC_FORCE_CONNECTION, 0))
            URLDownloadToFileW(nullptr, FlarialGUI::to_wide(url).c_str(), FlarialGUI::to_wide(path).c_str(), 0, nullptr);
        else {
            if(notifiedOfConnectionIssue) return;
            notifiedOfConnectionIssue = true;
            MessageBox(nullptr, "Flarial: Failed to download assets! Try using vpn.", "", MB_OK);
            ModuleManager::terminate();
            Client::disable = true;
        }

    }

}

std::vector<std::string> Client::getPlayersVector(const nlohmann::json& data) {
    std::vector<std::string> allPlayers;

    // Iterate over each server in the JSON object
    for (const auto & it : data) {
        if (it.contains("players")) {
            // Get the "players" array for the server
            const auto& players = it.at("players");

            // Add each player to the allPlayers vector
            for (const auto& player : players) {
                allPlayers.push_back(player);
            }
        }
    }

    if(SDK::clientInstance && SDK::clientInstance->getLocalPlayer()) {
        std::string name = SDK::clientInstance->getLocalPlayer()->getPlayerName();

        std::string clearedName = Utils::removeNonAlphanumeric(Utils::removeColorCodes(name));
        if (clearedName.empty()) clearedName = Utils::removeColorCodes(name);

        allPlayers.push_back(clearedName);
    }

    return allPlayers;
}

bool Client::disable = false;

void Client::setWindowTitle(std::wstring title) {
    using namespace winrt::Windows::UI::ViewManagement;
    using namespace winrt::Windows::ApplicationModel::Core;

    CoreApplication::MainView().CoreWindow().DispatcherQueue().TryEnqueue([title = std::move(title)]() {
        ApplicationView::GetForCurrentView().Title(title);
        });
}

void Client::initialize() {

    std::string title = WinrtUtils::getFormattedVersion() + " " + current_commit;
    setWindowTitle(L"Flarial " + FlarialGUI::to_wide(title));
    
    VersionUtils::init();
    Client::version = WinrtUtils::getFormattedVersion();

    if (!VersionUtils::isSupported(Client::version)) {
        Logger::fatal("Minecraft version is not supported!");
        MessageBox(NULL, "Flarial: this version is not supported!", "", MB_OK);
        ModuleManager::terminate();
        Client::disable = true;
        return;
    }

    VersionUtils::addData();

    std::filesystem::path folder_path(Utils::getRoamingPath() + "\\Flarial");
    if (!exists(folder_path)) {
        create_directory(folder_path);
    }

    std::filesystem::path folder_path2(Utils::getRoamingPath() + "\\Flarial\\assets");
    if (!exists(folder_path2)) {
        create_directory(folder_path2);
    }

    std::filesystem::path folder_path3(Utils::getRoamingPath() + "\\Flarial\\logs");
    if (!exists(folder_path3)) {
        create_directory(folder_path3);
    }

    std::filesystem::path folder_path4(Utils::getRoamingPath() + "\\Flarial\\Config");
    if (!exists(folder_path4)) {
        create_directory(folder_path4);
    }

    Client::CheckSettingsFile();
    Client::LoadSettings();

    if (Client::settings.getSettingByName<std::string>("fontname") == nullptr)
        Client::settings.addSetting("fontname", (std::string) "Space Grotesk");

    if (Client::settings.getSettingByName<std::string>("mod_fontname") == nullptr)
        Client::settings.addSetting("mod_fontname", (std::string) "Space Grotesk");

    if (Client::settings.getSettingByName<float>("blurintensity") == nullptr)
        Client::settings.addSetting("blurintensity", 2.0f);

    if (Client::settings.getSettingByName<bool>("killdx") == nullptr)
        Client::settings.addSetting("killdx", false);

    if (Client::settings.getSettingByName<bool>("disable_alias") == nullptr)
        Client::settings.addSetting("disable_alias", false);

    if (Client::settings.getSettingByName<bool>("vsync") == nullptr)
        Client::settings.addSetting("vsync", false);

    if (Client::settings.getSettingByName<bool>("promotions") == nullptr)
        Client::settings.addSetting("promotions", true);

    if (Client::settings.getSettingByName<bool>("donotwait") == nullptr)
        Client::settings.addSetting("donotwait", true);

    if (Client::settings.getSettingByName<std::string>("bufferingmode") == nullptr)
        Client::settings.addSetting("bufferingmode", (std::string) "Double Buffering");

    if (Client::settings.getSettingByName<std::string>("swapeffect") == nullptr)
        Client::settings.addSetting("swapeffect", (std::string) "FLIP_SEQUENTIAL");


    if (Client::settings.getSettingByName<bool>("disableanims") == nullptr)
        Client::settings.addSetting("disableanims", false);

    if (Client::settings.getSettingByName<bool>("anonymousApi") == nullptr)
        Client::settings.addSetting("anonymousApi", false);

    if (Client::settings.getSettingByName<bool>("dlassets") == nullptr)
        Client::settings.addSetting("dlassets", true);

    if (Client::settings.getSettingByName<bool>("noicons") == nullptr)
        Client::settings.addSetting("noicons", false);

    if (Client::settings.getSettingByName<bool>("noshadows") == nullptr)
        Client::settings.addSetting("noshadows", false);

    if (Client::settings.getSettingByName<bool>("watermark") == nullptr)
        Client::settings.addSetting("watermark", true);

    if (Client::settings.getSettingByName<bool>("centreCursor") == nullptr)
        Client::settings.addSetting("centreCursor", false);

    if (Client::settings.getSettingByName<std::string>("aliasingMode") == nullptr)
        Client::settings.addSetting("aliasingMode", (std::string) "Default");

    if (Client::settings.getSettingByName<std::string>("ejectKeybind") == nullptr)
        Client::settings.addSetting("ejectKeybind", (std::string) "");

    if (Client::settings.getSettingByName<bool>("enabledModulesOnTop") == nullptr)
        Client::settings.addSetting("enabledModulesOnTop", false);

    if (Client::settings.getSettingByName<float>("rgb_speed") == nullptr)
        Client::settings.addSetting("rgb_speed", 1.0f);

    if (Client::settings.getSettingByName<float>("rgb_saturation") == nullptr)
        Client::settings.addSetting("rgb_saturation", 1.0f);

    if (Client::settings.getSettingByName<float>("rgb_value") == nullptr)
        Client::settings.addSetting("rgb_value", 1.0f);

    if (Client::settings.getSettingByName<float>("modules_font_scale") == nullptr)
        Client::settings.addSetting("modules_font_scale", 1.0f);

    if (Client::settings.getSettingByName<float>("gui_font_scale") == nullptr)
        Client::settings.addSetting("gui_font_scale", 1.0f);

    if (Client::settings.getSettingByName<bool>("overrideFontWeight") == nullptr)
        Client::settings.addSetting("overrideFontWeight", (bool)false);

    if (Client::settings.getSettingByName<std::string>("fontWeight") == nullptr)
        Client::settings.addSetting("fontWeight", (std::string) "Normal");

    FlarialGUI::ExtractImageResource(IDR_RED_LOGO_PNG, "red-logo.png","PNG");

    FlarialGUI::LoadFont(IDR_FONT_TTF);

    FlarialGUI::LoadFont(IDR_FONT_BOLD_TTF);

    FlarialGUI::LoadFont(IDR_MINECRAFTIA_TTF);

    HookManager::initialize();
    ModuleManager::initialize();
}

std::string window = "Minecraft";

HWND hWnd = FindWindow(nullptr, window.c_str());
RECT currentRect = {0};
RECT clientRect = {0};
RECT previousRect = {0};
bool InHudScreen = false;

bool toes = false;

void Client::centerCursor() {
    if(Client::disable) return;
    if (hWnd != nullptr && Client::settings.getSettingByName<bool>("centreCursor")->value) {
        if (!toes) {
            toes = true;
            std::thread wow([&]() {
                while (!Client::disable && Client::settings.getSettingByName<bool>("centreCursor")->value) {
                    GetWindowRect(hWnd, &currentRect);
                    GetClientRect(hWnd, &clientRect);

                    if (memcmp(&currentRect, &previousRect, sizeof(RECT)) != 0) {
                        previousRect = currentRect;
                    }
                    Sleep(1000);
                }
                if (Client::disable && !Client::settings.getSettingByName<bool>("centreCursor")->value) toes = false;
            });

            wow.detach();
        }


        if ((SDK::getCurrentScreen() != "hud_screen" && InHudScreen) ||
            (SDK::getCurrentScreen() == "hud_screen" && !InHudScreen)) {
            GetWindowRect(hWnd, &currentRect);
            GetClientRect(hWnd, &clientRect);

            int windowX = currentRect.left;
            int windowY = currentRect.top;

            int centerX = windowX + (clientRect.right) / 2;
            int centerY = windowY + (clientRect.bottom) / 2;

            SetCursorPos(centerX, centerY);

            InHudScreen = !InHudScreen;
        }
    }
}