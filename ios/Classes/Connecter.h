//
//  Connecter.h
//  GSDK
//

#import <Foundation/Foundation.h>
#import "ConnecterBlock.h"

@interface Connecter : NSObject

// 读取数据 Read data
@property (nonatomic, copy, nullable) ReadData readData;

// 连接状态 Connection status
@property (nonatomic, copy, nullable) ConnectDeviceState state;

/**
 * 方法说明: 连接 // Method description: connect
 */
- (void)connect;

/**
 * 方法说明: 连接到指定设备 // Method description: connect to the specified device
 * @param connectState 连接状态
 */
- (void)connect:(void(^_Nullable)(ConnectState state))connectState;

/**
 * 方法说明: 关闭连接 // Method description: close connection
 */
- (void)close;

/**
 * 发送数据 // send data
 * 向输出流中写入数据 // Write data to the output stream
 */
- (void)write:(NSData *_Nonnull)data receCallBack:(void(^_Nullable)(NSData *_Nonnull data))callBack;
- (void)write:(NSData *_Nonnull)data;

/**
 * 读取数据 // Read data
 * @param data 读取到的数据 // Data that is read
 */
- (void)read:(void(^_Nullable)(NSData *_Nonnull data))data;

@end
