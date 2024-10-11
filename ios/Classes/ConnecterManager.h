//
//  ConnecterManager.h
//  GSDK
//

#import "BLEConnecter.h"
#import "Connecter.h"
#import "EthernetConnecter.h"
#import <Foundation/Foundation.h>

/**
 *  @enum ConnectMethod
 *  @discussion 连接方式
 *  @constant BLUETOOTH 蓝牙连接
 *  @constant ETHERNET 网口连接（wifi连接）
 */
typedef NS_ENUM(NSUInteger, ConnectMethod) { BLUETOOTH, ETHERNET };

#define Manager [ConnecterManager sharedInstance]

@interface ConnecterManager : NSObject

@property(nonatomic, strong, nullable) BLEConnecter *bleConnecter;
@property(nonatomic, strong, nullable) Connecter *connecter;

+ (instancetype)sharedInstance;

/**
 *  方法说明：关闭连接 // Method description: close connection
 */
- (void)close;

/**
 *  方法说明: 向输出流中写入数据（只适用于蓝牙） // Method description: write
 * data to the output stream (only for Bluetooth)
 *  @param data 需要写入的数据 // Data to be written
 *  @param progress 写入数据进度 // Progress of writing data
 *  @param callBack 读取输入流中的数据 // Read data from the input stream
 */
- (void)write:(NSData *_Nullable)data
        progress:(void (^_Nullable)(NSUInteger total,
                                    NSUInteger progress))progress
    receCallBack:(void (^_Nullable)(NSData *_Nullable data))callBack;

/**
 *  方法说明：向输出流中写入数据 // Method description: writing data to the
 * output stream
 *  @param callBack 读取数据接口 // Data read callback
 */
- (void)write:(NSData *_Nonnull)data
    receCallBack:(void (^_Nullable)(NSData *_Nonnull data))callBack;

/**
 *  方法说明：向输出流中写入数据 // Method description: writing data to the
 * output stream
 *  @param data 需要写入的数据 // Data to be written
 */
- (void)write:(NSData *_Nonnull)data;

/**
 *  方法说明：停止扫描 // Method description: stop scanning
 */
- (void)stopScan;

/**
 *  方法说明：更新蓝牙状态 // Method description: update Bluetooth status
 *  @param state 蓝牙状态 // Bluetooth state
 */
- (void)didUpdateState:(void (^_Nullable)(NSInteger state))state;

/**
 *  方法说明：连接外设 // Method description: connect peripheral
 *  @param peripheral 需连接的外设 // Peripheral to connect
 *  @param options 其它可选操作 // Other optional operations
 *  @param timeout 连接时间 // Connection timeout
 *  @param connectState 连接状态 // Connection state
 */
- (void)connectPeripheral:(CBPeripheral *_Nullable)peripheral
                  options:(nullable NSDictionary<NSString *, id> *)options
                  timeout:(NSUInteger)timeout
             connectBlack:(void (^_Nullable)(ConnectState state))connectState;

/**
 *  方法说明：连接外设 // Method description: connect peripheral
 *  @param peripheral 需连接的外设 // Peripheral to connect
 *  @param options 其它可选操作 // Other optional operations
 */
- (void)connectPeripheral:(CBPeripheral *_Nullable)peripheral
                  options:(nullable NSDictionary<NSString *, id> *)options;

/**
 *  方法说明：扫描外设 // Method description: scan for peripherals
 *  @param serviceUUIDs 需要发现外设的UUID，设置为nil则发现周围所有外设 // UUIDs
 * of peripherals to discover, or `nil` to discover all nearby peripherals
 *  @param options 其它可选操作 // Other optional operations
 *  @param discover 发现的设备 // Discovered devices callback
 */
- (void)
    scanForPeripheralsWithServices:(nullable NSArray<CBUUID *> *)serviceUUIDs
                           options:
                               (nullable NSDictionary<NSString *, id> *)options
                          discover:(void (^_Nullable)(
                                       CBPeripheral *_Nullable peripheral,
                                       NSDictionary<NSString *, id>
                                           *_Nullable advertisementData,
                                       NSNumber *_Nullable RSSI))discover;

@end
