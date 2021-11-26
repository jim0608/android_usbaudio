package au.id.jms.usbaudiodemo;


import static android.hardware.usb.UsbConstants.USB_CLASS_AUDIO;

import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.hardware.usb.UsbConstants;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbDeviceConnection;
import android.hardware.usb.UsbEndpoint;
import android.hardware.usb.UsbInterface;
import android.hardware.usb.UsbManager;
import android.os.Bundle;
import android.support.v4.app.FragmentActivity;
import android.util.Log;
import android.view.Menu;
import android.view.View;
import android.widget.Button;

import org.libusb.UsbHelper;

import java.util.HashMap;
import java.util.Iterator;

import au.id.jms.usbaudio.AudioPlayback;

public class MainActivity extends FragmentActivity {

    private static final String TAG = "UsbAudio";
    private BroadcastReceiver mUsbPermissionActionReceiver;
    private boolean isOpen = true;
    private static final String ACTION_USB_PERMISSION = "com.minelab.droidspleen.USB_PERMISSION";
    PendingIntent mPermissionIntent = null;
    UsbManager mUsbManager = null;
    UsbDevice mAudioDevice = null;
    USBMonitor.UsbControlBlock mCtrlBlock;
    USBAudio mUsbAudio = null;
//    UsbAudio mUsbAudio = null;


    Thread mUsbThread = null;

    private USBMonitor mUSBMonitor;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        Log.d(TAG, "Hello, World!");

        // Grab the USB Device so we can get permission
        mUsbManager = (UsbManager) getSystemService(Context.USB_SERVICE);
        HashMap<String, UsbDevice> deviceList = mUsbManager.getDeviceList();
        Iterator<UsbDevice> deviceIterator = deviceList.values().iterator();


        // Load native lib
        System.loadLibrary("usb100");
        UsbHelper.useContext(getApplicationContext());


        AudioPlayback.setup();

        // Buttons
        final Button openButton = (Button) findViewById(R.id.button0);
        final Button startButton = (Button) findViewById(R.id.button1);
        final Button stopButton = (Button) findViewById(R.id.button2);
        mUsbAudio = new USBAudio();
//        mUsbAudio = new UsbAudio();

        openButton.setEnabled(true);
        startButton.setEnabled(true);
        stopButton.setEnabled(false);
        openButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                openDevice();
                mUsbAudio.initAudio(mCtrlBlock);
//                mUsbAudio.open(mCtrlBlock);
            }
        });

        startButton.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                Log.d(TAG, "Start pressed");

                startButton.setEnabled(false);
                stopButton.setEnabled(true);
                mUsbAudio.startCapture();
//                new Thread(new Runnable() {
//                    public void run() {
//                        while (isOpen) {
//                            mUsbAudio.loop();
//                        }
//                    }
//                }).start();
            }
        });

        stopButton.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                Log.d(TAG, "Stop pressed");
                isOpen = false;
                mUsbAudio.stopCapture();

                startButton.setEnabled(true);
                stopButton.setEnabled(false);
            }
        });

        // Register for permission
        // ACTION_USB_DEVICE_ATTACHED never comes on some devices so it should not be added here

        mUSBMonitor = new USBMonitor(this, mOnDeviceConnectListener);
        mUSBMonitor.setDeviceFilter(DeviceFilter.getDeviceFilters(this, R.xml.device_filter));

        mUSBMonitor.register();


    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        isOpen = false;
        if (mUsbAudio != null) {
            mUsbAudio.closeAudio();
        }
        mUSBMonitor.unregister();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.activity_main, menu);
        return true;
    }

    private void setDevice(UsbDevice device) {
        // Set button to enabled when permission is obtained
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                ((Button) findViewById(R.id.button1)).setEnabled(device != null);
            }
        });

    }

    private UsbManager manager;
    UsbDeviceConnection connection;
    UsbEndpoint mUsbEndpointIn, mUsbEndpointOut;
    UsbInterface mUsbInterfaceInt, musbInterfaceOut;

    public void openDevice() {
        if (mAudioDevice == null) {
            return;
        }
        int interfaceCount = mAudioDevice.getInterfaceCount();
        for (int interfaceIndex = 0; interfaceIndex < interfaceCount; interfaceIndex++) {
            UsbInterface usbInterface = mAudioDevice.getInterface(interfaceIndex);
            if ((USB_CLASS_AUDIO != usbInterface.getInterfaceClass())
                    && (UsbConstants.USB_CLASS_HID != usbInterface.getInterfaceClass())) {
                continue;
            }
            for (int i = 0; i < usbInterface.getEndpointCount(); i++) {
                UsbEndpoint ep = usbInterface.getEndpoint(i);
                if (ep.getType() == UsbConstants.USB_ENDPOINT_XFER_ISOC) {
                    if (ep.getDirection() == UsbConstants.USB_DIR_IN) {
                        mUsbEndpointIn = ep;
                        mUsbInterfaceInt = usbInterface;
                    } else {
                        mUsbEndpointOut = ep;
                        musbInterfaceOut = usbInterface;
                    }

                }
                if (ep.getType() == UsbConstants.USB_ENDPOINT_XFER_INT) {
                    mUsbEndpointOut = ep;
                    musbInterfaceOut = usbInterface;
                }
            }
        }
//        manager = (UsbManager) getSystemService(Context.USB_SERVICE);
//        connection = manager.openDevice(mAudioDevice);
//		boolean result = connection.claimInterface(mUsbInterfaceInt, true);
//		boolean result2 = connection.claimInterface(musbInterfaceOut, true);
    }


    private final USBMonitor.OnDeviceConnectListener mOnDeviceConnectListener = new USBMonitor.OnDeviceConnectListener() {

        @Override
        public void onAttach(UsbDevice device) {
//            Log.i(TAG, "onAttach: " + device);
            setDevice(device);
            mUSBMonitor.requestPermission(device);
//            mCtrlBlock = mUSBMonitor.getDevice(device);
        }

        @Override
        public void onDettach(UsbDevice device) {
//            Log.i(TAG, "onDettach: " + device);
        }

        @Override
        public void onConnect(UsbDevice device, USBMonitor.UsbControlBlock ctrlBlock, boolean createNew) {
//            Log.i(TAG, "onConnect: " + device);
//            if (ctrlBlock.mInfo)
            for (int interfaceIndex = 0; interfaceIndex < device.getInterfaceCount(); interfaceIndex++) {
                if (device.getInterface(interfaceIndex).getInterfaceClass() == USB_CLASS_AUDIO){
                    mCtrlBlock = ctrlBlock;
                    break;
                }
            }


//			isOpen = mUsbAudio.open(ctrlBlock);
            mAudioDevice = device;
        }

        @Override
        public void onDisconnect(UsbDevice device, USBMonitor.UsbControlBlock ctrlBlock) {
//			if (mUsbAudio!=null) {
//				mUsbAudio.close();
//			}
        }

        @Override
        public void onCancel(UsbDevice device) {

        }
    };
}
