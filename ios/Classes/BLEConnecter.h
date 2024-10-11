#import "Connecter.h"
#import <CoreBluetooth/CoreBluetooth.h>

@interface BLEConnecter : Connecter

@property(nonatomic, strong, nullable) CBCharacteristic *airPatchChar;
@property(nonatomic, strong, nullable) CBCharacteristic *transparentDataWriteChar;
@property(nonatomic, strong, nullable) CBCharacteristic *transparentDataReadOrNotifyChar;
@property(nonatomic, strong, nullable) CBCharacteristic *connectionParameterChar;

@property(nonatomic, strong, nullable) CBUUID *transServiceUUID;
@property(nonatomic, strong, nullable) CBUUID *transTxUUID;
@property(nonatomic, strong, nullable) CBUUID *transRxUUID;
@property(nonatomic, strong, nullable) CBUUID *disUUID1;
@property(nonatomic, strong, nullable) CBUUID *disUUID2;
@property(nonatomic, strong, nullable) NSArray *serviceUUID;

@property(nonatomic, copy, nullable) DiscoverDevice discover;
@property(nonatomic, copy, nullable) UpdateState updateState;
@property(nonatomic, copy, nullable) WriteProgress writeProgress;

@property(nonatomic, assign) NSUInteger datagramSize;
@property(nonatomic, strong, nullable) CBPeripheral *connPeripheral;

- (void)configureTransparentServiceUUID:(nonnull NSString *)serviceUUID 
                                 txUUID:(nonnull NSString *)txUUID 
                                 rxUUID:(nonnull NSString *)rxUUID;

- (void)scanForPeripheralsWithServices:(nullable NSArray<CBUUID *> *)serviceUUIDs 
                               options:(nullable NSDictionary<NSString *, id> *)options 
                              discover:(void(^_Nullable)(CBPeripheral *_Nullable peripheral, 
                                                        NSDictionary<NSString *, id> *_Nullable advertisementData, 
                                                        NSNumber *_Nullable RSSI))discover;

- (void)stopScan;

- (void)didUpdateState:(void(^_Nullable)(NSInteger state))state;

- (void)connectPeripheral:(CBPeripheral *_Nullable)peripheral 
                  options:(nullable NSDictionary<NSString *, id> *)options 
                  timeout:(NSUInteger)timeout 
             connectBlack:(void(^_Nullable)(ConnectState state))connectState;

- (void)connectPeripheral:(CBPeripheral *_Nullable)peripheral 
                  options:(nullable NSDictionary<NSString *, id> *)options;

- (void)closePeripheral:(nonnull CBPeripheral *)peripheral;

- (void)write:(NSData *_Nullable)data 
     progress:(void(^_Nullable)(NSUInteger total, NSUInteger progress))progress 
 receCallBack:(void(^_Nullable)(NSData *_Nullable))callBack;

- (void)writeValue:(nonnull NSData *)data 
  forCharacteristic:(nonnull CBCharacteristic *)characteristic 
              type:(CBCharacteristicWriteType)type;

@end
