package com.sersoluciones.flutter_pos_printer_platform.usb

import android.annotation.SuppressLint
import android.annotation.TargetApi
import android.app.PendingIntent
import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.hardware.usb.*
import android.os.Build
import android.os.Handler
import android.util.Base64
import android.util.Log
import android.widget.Toast
import com.sersoluciones.flutter_pos_printer_platform.R
import java.nio.charset.Charset
import java.util.*
import java.util.concurrent.CompletableFuture

class USBPrinterService private constructor(private var mHandler: Handler?) {
    private var mContext: Context? = null
    private var mUSBManager: UsbManager? = null
    private var mPermissionIndent: PendingIntent? = null
    private var mUsbDevice: UsbDevice? = null
    private var mUsbDeviceConnection: UsbDeviceConnection? = null
    private var mUsbInterface: UsbInterface? = null
    private var mEndPoint: UsbEndpoint? = null
    var state: Int = STATE_USB_NONE
    private var permissionFuture: CompletableFuture<Boolean>? = null

    fun setHandler(handler: Handler?) {
        mHandler = handler
    }

    private val mUsbDeviceReceiver: BroadcastReceiver = object : BroadcastReceiver() {
        @TargetApi(Build.VERSION_CODES.N)
        override fun onReceive(context: Context, intent: Intent) {
            val action = intent.action
            if (ACTION_USB_PERMISSION == action) {
                synchronized(this) {
                    val usbDevice: UsbDevice? = intent.getParcelableExtra(UsbManager.EXTRA_DEVICE)
                    if (intent.getBooleanExtra(UsbManager.EXTRA_PERMISSION_GRANTED, false)) {
                        Log.i(
                            LOG_TAG,
                            "Success get permission for device ${usbDevice?.deviceId}, vendor_id: ${usbDevice?.vendorId}, product_id: ${usbDevice?.productId}"
                        )
                        mUsbDevice = usbDevice
                        state = STATE_USB_CONNECTED
                        mHandler?.obtainMessage(STATE_USB_CONNECTED)?.sendToTarget()
                        permissionFuture?.complete(true)  // Permission granted
                    } else {
                        Log.i(LOG_TAG, "Permission denied for device")
                        state = STATE_USB_NONE
                        mHandler?.obtainMessage(STATE_USB_NONE)?.sendToTarget()
                        permissionFuture?.complete(false)  // Permission denied
                    }
                }
            } else if (UsbManager.ACTION_USB_DEVICE_DETACHED == action) {
                if (mUsbDevice != null) {
                    Toast.makeText(context, mContext?.getString(R.string.device_off), Toast.LENGTH_LONG).show()
                    closeConnectionIfExists()
                    state = STATE_USB_NONE
                    mHandler?.obtainMessage(STATE_USB_NONE)?.sendToTarget()
                }
            }
        }
    }

    fun init(reactContext: Context?) {
        mContext = reactContext
        mUSBManager = mContext!!.getSystemService(Context.USB_SERVICE) as UsbManager
        mPermissionIndent = if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.S) {
            PendingIntent.getBroadcast(mContext, 0, Intent(ACTION_USB_PERMISSION), PendingIntent.FLAG_MUTABLE)
        } else {
            PendingIntent.getBroadcast(mContext, 0, Intent(ACTION_USB_PERMISSION), 0)
        }
        val filter = IntentFilter(ACTION_USB_PERMISSION)
        filter.addAction(UsbManager.ACTION_USB_DEVICE_DETACHED)
        mContext!!.registerReceiver(mUsbDeviceReceiver, filter)
        Log.v(LOG_TAG, "ESC/POS Printer initialized")
    }
 fun closeConnectionIfExists(): Boolean {
    synchronized(printLock) {
        if (mUsbDeviceConnection != null && mUsbInterface != null) {
            mUsbDeviceConnection!!.releaseInterface(mUsbInterface)
            mUsbDeviceConnection!!.close()
            mUsbInterface = null
            mEndPoint = null
            mUsbDevice = null
            mUsbDeviceConnection = null
            return true  // Connection was closed
        }
        return false  // No connection to close
    }
}

    val deviceList: List<UsbDevice>
        get() {
            if (mUSBManager == null) {
                Toast.makeText(mContext, mContext?.getString(R.string.not_usb_manager), Toast.LENGTH_LONG).show()
                return emptyList()
            }
            return ArrayList(mUSBManager!!.deviceList.values)
        }

    @TargetApi(Build.VERSION_CODES.N)
    fun selectDevice(vendorId: Int, productId: Int): CompletableFuture<Boolean> {

            permissionFuture = CompletableFuture()

        if ((mUsbDevice == null) || (mUsbDevice!!.vendorId != vendorId) || (mUsbDevice!!.productId != productId)) {
            synchronized(printLock) {
                closeConnectionIfExists()
                val usbDevices: List<UsbDevice> = deviceList
                for (usbDevice: UsbDevice in usbDevices) {
                    if ((usbDevice.vendorId == vendorId) && (usbDevice.productId == productId)) {
                        Log.v(LOG_TAG, "Request for device: vendor_id: " + usbDevice.vendorId + ", product_id: " + usbDevice.productId)
                        closeConnectionIfExists()
                        mUSBManager!!.requestPermission(usbDevice, mPermissionIndent)
                        state = STATE_USB_CONNECTING
                        mHandler?.obtainMessage(STATE_USB_CONNECTING)?.sendToTarget()
                        return permissionFuture!!  // Return the future, which will be completed later
                    }
                }
                permissionFuture?.complete(false)  // No device found
            }
        } else {
            mHandler?.obtainMessage(state)?.sendToTarget()
            permissionFuture?.complete(true)  // Device already connected
        }

        return permissionFuture!!  // Return the future immediately, it will complete later
    }


    private fun openConnection(): Boolean {
        if (mUsbDevice == null) {
            Log.e(LOG_TAG, "USB Device is not initialized")
            return false
        }
        if (mUSBManager == null) {
            Log.e(LOG_TAG, "USB Manager is not initialized")
            return false
        }
        if (mUsbDeviceConnection != null) {
            Log.i(LOG_TAG, "USB Connection already connected")
            return true
        }
        val usbInterface = mUsbDevice!!.getInterface(0)
        for (i in 0 until usbInterface.endpointCount) {
            val ep = usbInterface.getEndpoint(i)
            if (ep.type == UsbConstants.USB_ENDPOINT_XFER_BULK) {
                if (ep.direction == UsbConstants.USB_DIR_OUT) {
                    val usbDeviceConnection = mUSBManager!!.openDevice(mUsbDevice)
                    if (usbDeviceConnection == null) {
                        Log.e(LOG_TAG, "Failed to open USB Connection")
                        return false
                    }
                    Toast.makeText(mContext, mContext?.getString(R.string.connected_device), Toast.LENGTH_SHORT).show()
                    return if (usbDeviceConnection.claimInterface(usbInterface, true)) {
                        mEndPoint = ep
                        mUsbInterface = usbInterface
                        mUsbDeviceConnection = usbDeviceConnection
                        true
                    } else {
                        usbDeviceConnection.close()
                        Log.e(LOG_TAG, "Failed to retrieve usb connection")
                        false
                    }
                }
            }
        }
        return true
    }

    fun printText(text: String): Boolean {
        Log.v(LOG_TAG, "Printing text")
        val isConnected = openConnection()
        return if (isConnected) {
            Log.v(LOG_TAG, "Connected to device")
            Thread {
                synchronized(printLock) {
                    val bytes: ByteArray = text.toByteArray(Charset.forName("UTF-8"))
                    val b: Int = mUsbDeviceConnection!!.bulkTransfer(mEndPoint, bytes, bytes.size, 100000)
                    Log.i(LOG_TAG, "Return code: $b")
                }
            }.start()
            true
        } else {
            Log.v(LOG_TAG, "Failed to connect to device")
            false
        }
    }

    fun printRawData(data: String): Boolean {
        Log.v(LOG_TAG, "Printing raw data: $data")
        val isConnected = openConnection()
        return if (isConnected) {
            Log.v(LOG_TAG, "Connected to device")
            Thread {
                synchronized(printLock) {
                    val bytes: ByteArray = Base64.decode(data, Base64.DEFAULT)
                    val b: Int = mUsbDeviceConnection!!.bulkTransfer(mEndPoint, bytes, bytes.size, 100000)
                    Log.i(LOG_TAG, "Return code: $b")
                }
            }.start()
            true
        } else {
            Log.v(LOG_TAG, "Failed to connected to device")
            false
        }
    }

fun printBytes(bytes: ArrayList<Int>): Boolean {
    Log.v(LOG_TAG, "Printing bytes")
    val isConnected = openConnection()
    if (isConnected) {
        Log.v(LOG_TAG, "Connected to device")
        Thread {
            synchronized(printLock) {
                // Convert ArrayList<Int> to ByteArray
                val vectorData: Vector<Byte> = Vector()
                for (i in bytes.indices) {
                    val `val`: Int = bytes[i]
                    vectorData.add(`val`.toByte())
                }
                val byteData = ByteArray(vectorData.size)
                for (i in vectorData.indices) {
                    byteData[i] = vectorData[i]
                }

                if (mUsbDeviceConnection != null && mEndPoint != null) {
                    val b: Int = mUsbDeviceConnection!!.bulkTransfer(mEndPoint, byteData, byteData.size, 100000)

                    if (b < 0) {
                        Log.e(LOG_TAG, "Failed to send data with error code: $b")
                    } else {
                        Log.i(LOG_TAG, "Data transfer completed with return code: $b")
                    }
                       Log.i(LOG_TAG, "Sleeping after printing")
                    Thread.sleep(1000)  // Adjust the sleep time as needed
                } else {
                    Log.e(LOG_TAG, "USB connection or endpoint is null")
                }
                
            }
        }.start()
        return true
    } else {
        Log.v(LOG_TAG, "Failed to connect to device")
        return false
    }
}


    companion object {
        @SuppressLint("StaticFieldLeak")
        private var mInstance: USBPrinterService? = null
        private const val LOG_TAG = "ESC POS Printer"
        private const val ACTION_USB_PERMISSION = "com.flutter_pos_printer.USB_PERMISSION"

        // Constants that indicate the current connection state
        const val STATE_USB_NONE = 0 // we're doing nothing
        const val STATE_USB_CONNECTING = 2 // now initiating an outgoing connection
        const val STATE_USB_CONNECTED = 3 // now connected to a remote device

        private val printLock = Any()

        fun getInstance(handler: Handler): USBPrinterService {
            if (mInstance == null) {
                mInstance = USBPrinterService(handler)
            }
            return mInstance!!
        }
    }
}