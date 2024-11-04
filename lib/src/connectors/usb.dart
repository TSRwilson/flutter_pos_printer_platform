import 'dart:async';
import 'dart:io';
import 'dart:typed_data';

import 'package:flutter_pos_printer_platform/discovery.dart';
import 'package:flutter_pos_printer_platform/flutter_pos_printer_platform_image_3.dart';

class UsbPrinterInput extends BasePrinterInput {
  final String? name;
  final String? vendorId;
  final String? productId;
  UsbPrinterInput({
    this.name,
    this.vendorId,
    this.productId,
  });
}

class UsbPrinterInfo {
  String vendorId;
  String productId;
  String manufacturer;
  String product;
  String name;
  String? model;
  bool isDefault = false;
  String deviceId;
  UsbPrinterInfo.Android({
    required this.vendorId,
    required this.productId,
    required this.manufacturer,
    required this.product,
    required this.name,
    required this.deviceId,
  });
  UsbPrinterInfo.Windows({
    required this.name,
    required this.model,
    required this.isDefault,
    this.vendorId = '',
    this.productId = '',
    this.manufacturer = '',
    this.product = '',
    this.deviceId = '',
  });
}

class UsbPrinterConnector implements PrinterConnector<UsbPrinterInput> {
  UsbPrinterConnector._()
      : vendorId = '',
        productId = '',
        name = '' {
    if (Platform.isAndroid) {
      flutterPrinterEventChannelUSB.receiveBroadcastStream().listen((data) {
        if (data is int) {
          _status = USBStatus.values[data];
          _statusStreamController.add(_status);
        }
      });
    }
  }

  static UsbPrinterConnector _instance = UsbPrinterConnector._();

  static UsbPrinterConnector get instance => _instance;

  Stream<USBStatus> get _statusStream => _statusStreamController.stream;
  final StreamController<USBStatus> _statusStreamController =
      StreamController.broadcast();

  UsbPrinterConnector.Android({required this.vendorId, required this.productId})
      : name = '';
  UsbPrinterConnector.Windows({required this.name})
      : vendorId = '',
        productId = '';

  String vendorId;
  String productId;
  String name;
  USBStatus _status = USBStatus.none;
  USBStatus get status => _status;

  setVendor(String vendorId) => this.vendorId = vendorId;
  setProduct(String productId) => this.productId = productId;
  setName(String name) => this.name = name;

  Stream<USBStatus> get currentStatus async* {
    if (Platform.isAndroid) {
      yield* _statusStream.cast<USBStatus>();
    }
  }

  static Future<List<PrinterDiscovered<UsbPrinterInfo>>>
      discoverPrinters() async {
    if (Platform.isAndroid || Platform.isWindows) {
      final List<dynamic>? results =
          await flutterPrinterChannel.invokeMethod<List<dynamic>?>('getList');
      if (results != null) {
        return results.map((dynamic r) {
          if (Platform.isAndroid) {
            return PrinterDiscovered<UsbPrinterInfo>(
              name: r['product'] ?? 'Unknown Product',
              detail: UsbPrinterInfo.Android(
                vendorId: r['vendorId'] ?? '',
                productId: r['productId'] ?? '',
                manufacturer: r['manufacturer'] ?? '',
                product: r['product'] ?? '',
                name: r['name'] ?? '',
                deviceId: r['deviceId'] ?? '',
              ),
            );
          } else {
            return PrinterDiscovered<UsbPrinterInfo>(
              name: r['name'] ?? 'Unknown Name',
              detail: UsbPrinterInfo.Windows(
                isDefault: r['default'] ?? false,
                name: r['name'] ?? '',
                model: r['model'] ?? '',
              ),
            );
          }
        }).toList();
      }
    }
    return [];
  }

  Stream<PrinterDevice> discovery() async* {
    if (Platform.isAndroid || Platform.isWindows) {
      final List<dynamic>? results =
          await flutterPrinterChannel.invokeMethod<List<dynamic>?>('getList');
      if (results != null) {
        for (final device in results) {
          yield PrinterDevice(
            name: device['product'] ?? device['name'] ?? 'Unknown Product',
            vendorId: device['vendorId'] ?? '',
            productId: device['productId'] ?? '',
          );
        }
      }
    }
  }

  Future<bool> _connect({UsbPrinterInput? model}) async {
    try {
      if (Platform.isAndroid) {
        Map<String, dynamic> params = {
          "vendor": int.tryParse(model?.vendorId ?? vendorId) ?? 0,
          "product": int.tryParse(model?.productId ?? productId) ?? 0
        };
        return await flutterPrinterChannel.invokeMethod<bool>(
                'connectPrinter', params) ??
            false;
      } else if (Platform.isWindows) {
        Map<String, dynamic> params = {"name": model?.name ?? name};
        return await flutterPrinterChannel.invokeMethod<bool>(
                'connectPrinter', params) ??
            false;
      }
    } catch (e) {
      // Handle error logging or rethrow if necessary
    }
    return false;
  }

  Future<bool> _close() async {
    try {
      if (Platform.isWindows || Platform.isAndroid) {
        return await flutterPrinterChannel.invokeMethod<bool>('close') ?? false;
      }
    } catch (e) {
      // Handle error logging or rethrow if necessary
    }
    return false;
  }

  @override
  Future<bool> connect(UsbPrinterInput model) async {
    try {
      return await _connect(model: model);
    } catch (e) {
      // Handle error logging if necessary
    }
    return false;
  }

  @override
  Future<bool> disconnect({int? delayMs}) async {
    try {
      return await _close();
    } catch (e) {
      // Handle error logging if necessary
    }
    return false;
  }

  @override
  Future<bool> send(List<int> bytes) async {
    try {
      if (Platform.isAndroid) {
        Map<String, dynamic> params = {"bytes": bytes};
        return await flutterPrinterChannel.invokeMethod<bool>(
                'printBytes', params) ??
            false;
      } else if (Platform.isWindows) {
        Map<String, dynamic> params = {"bytes": Uint8List.fromList(bytes)};
        return (await flutterPrinterChannel.invokeMethod<int>(
                'printBytes', params)) ==
            1;
      }
    } catch (e) {
      await _close();
    }
    return false;
  }
}
