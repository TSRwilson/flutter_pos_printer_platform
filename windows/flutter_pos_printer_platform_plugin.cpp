#include "include/flutter_pos_printer_platform/flutter_pos_printer_platform_plugin.h"

// This must be included before many other Windows headers.
#include <windows.h>

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>

#include <map>
#include <memory>
#include <sstream>
#include <iostream>  // Include the iostream header for std::cout

#include "include/printer.h"

namespace
{
  using flutter::EncodableList;
  using flutter::EncodableMap;
  using flutter::EncodableValue;

  class FlutterPosPrinterPlatformPlugin: public flutter::Plugin
  {
  public:
    static void RegisterWithRegistrar(flutter::PluginRegistrarWindows *registrar);

    FlutterPosPrinterPlatformPlugin();

    virtual ~FlutterPosPrinterPlatformPlugin();

  private:
    // Called when a method is called on this plugin's channel from Dart.
    void HandleMethodCall(
        const flutter::MethodCall<flutter::EncodableValue> &method_call,
        std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
  };

  // static
  void FlutterPosPrinterPlatformPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarWindows *registrar)
  {
    auto channel =
        std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
            registrar->messenger(), "com.sersoluciones.flutter_pos_printer_platform",
            &flutter::StandardMethodCodec::GetInstance());

    auto plugin = std::make_unique<FlutterPosPrinterPlatformPlugin>();

    channel->SetMethodCallHandler(
        [plugin_pointer = plugin.get()](const auto &call, auto result)
        {
          plugin_pointer->HandleMethodCall(call, std::move(result));
        });

    registrar->AddPlugin(std::move(plugin));
  }

  FlutterPosPrinterPlatformPlugin::FlutterPosPrinterPlatformPlugin() {}

  FlutterPosPrinterPlatformPlugin::~FlutterPosPrinterPlatformPlugin() {}

  void FlutterPosPrinterPlatformPlugin::HandleMethodCall(
      const flutter::MethodCall<EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<EncodableValue>> result)
  {
    // Get arguments the C++ way
    const auto *args = std::get_if<EncodableMap>(method_call.arguments());

    if (method_call.method_name().compare("getList") == 0)
    {
      auto printers = PrintManager::listPrinters();
      auto list = EncodableList{};
      for (auto printer : printers)
      {
        auto map = EncodableMap{};
        map[EncodableValue("name")] =
            EncodableValue(printer.name);
        map[EncodableValue("model")] =
            EncodableValue(printer.model);
        map[EncodableValue("default")] =
            EncodableValue(printer.default);
        map[EncodableValue("available")] =
            EncodableValue(printer.available);
        list.push_back(map);
      }

      return result->Success(list);
    }
    else if (method_call.method_name().compare("connectPrinter") == 0)
    {
      std::string printerName;

      if (args)
      {
        auto name_it = args->find(EncodableValue("name"));
        if (name_it != args->end())
        {
          printerName = std::get<std::string>(name_it->second);
        }

        auto success = PrintManager::pickPrinter(printerName);

        std::cout << "pickPrinter completed with success: " << success << std::endl;

        return result->Success(EncodableValue(success));
      }

      return result->Success(EncodableValue(false));
    }
    else if (method_call.method_name().compare("close") == 0)
    {
      auto success = PrintManager::close();
      return result->Success(EncodableValue(success));
    }
    else if (method_call.method_name().compare("printBytes") == 0)
    {
      std::vector<uint8_t> bytes;

      if (args)
      {
        auto bytes_it = args->find(EncodableValue("bytes"));
        if (bytes_it != args->end())
        {
          bytes = std::get<std::vector<uint8_t>>(bytes_it->second);
        }

        auto success = PrintManager::printBytes(bytes);
        return result->Success(EncodableValue(success));
      }
    }
    else
    {
      result->NotImplemented();
    }
  }

} // namespace

void FlutterPosPrinterPlatformPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar)
{
  FlutterPosPrinterPlatformPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrarWindows>(registrar));
}
