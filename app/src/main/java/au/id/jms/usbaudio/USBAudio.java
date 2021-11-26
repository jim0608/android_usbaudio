package au.id.jms.usbaudio;

import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.text.TextUtils;
import android.util.Log;

import au.id.jms.usbaudiodemo.USBMonitor;


public class USBAudio {
    private String TAG = "USBAudio";

    static {
        System.loadLibrary("USBAudio");
    }

    protected long mNativePtr;

    private native long nativeCreate();

    private native void nativeClose(long id_camera);

    private native int nativeInit(long id_camera, int vid, int pid, int busnum, int devaddr, int fd, String usbfs);

    private native int nativeGetSampleRate(long id_camera);

    private native int nativeGetChannelCount(long id_camera);

    private native int nativeGetBitResolution(long id_camera);

    private native boolean nativeIsRunning(long paramLong);

    private native int nativeStartCapture(long id_camera);

    private native int nativeStopCapture(long id_camera);

    private static final String DEFAULT_USBFS = "/dev/bus/usb";

    private USBMonitor.UsbControlBlock mCtrlBlock;

    public USBAudio() {
        mNativePtr = nativeCreate();

        Log.i(TAG, "USBAudio: nativeCreate");
    }


    public void initAudio(USBMonitor.UsbControlBlock ctrlBlock) {
        int result;
        try {
            mCtrlBlock = ctrlBlock.clone();
            result = nativeInit(mNativePtr,
                    mCtrlBlock.getVenderId(), mCtrlBlock.getProductId(),
                    mCtrlBlock.getBusNum(),
                    mCtrlBlock.getDevNum(),
                    mCtrlBlock.getFileDescriptor(),
                    getUSBFSName(mCtrlBlock));
        } catch (final Exception e) {
            Log.w(TAG, e);
            result = -1;
        }
        Log.i(TAG, "initAudio: " + result);
        if (result < 0) {
            return;
        }
        setAudioConfig();
    }

    public void setAudioConfig() {
        this.channel = nativeGetChannelCount(mNativePtr);
        SAMPLE_RATE_HZ = nativeGetSampleRate(mNativePtr);
        int bit = nativeGetBitResolution(mNativePtr);
        Log.i(TAG, "setAudioConfig: Channel " + channel);
        Log.i(TAG, "setAudioConfig: SampleRate " + SAMPLE_RATE_HZ);
        Log.i(TAG, "setAudioConfig: BitResolution " + bit);
        initPlay();
    }

    public void startCapture() {
        new Thread(new Runnable() {
            @Override
            public void run() {
                nativeStartCapture(mNativePtr);
            }
        }).start();
    }

    public void stopCapture() {
        nativeStopCapture(mNativePtr);
    }

    public void destroyAudio() {
        nativeClose(mNativePtr);
    }


    private final String getUSBFSName(final USBMonitor.UsbControlBlock ctrlBlock) {
        String result = null;
        final String name = ctrlBlock.getDeviceName();
        final String[] v = !TextUtils.isEmpty(name) ? name.split("/") : null;
        if ((v != null) && (v.length > 2)) {
            final StringBuilder sb = new StringBuilder(v[0]);
            for (int i = 1; i < v.length - 2; i++)
                sb.append("/").append(v[i]);
            result = sb.toString();
        }
        if (TextUtils.isEmpty(result)) {
            Log.w(TAG, "failed to get USBFS path, try to use default path:" + name);
            result = DEFAULT_USBFS;
        }
        return result;
    }

    private int SAMPLE_RATE_HZ = 48000;
    private int channel = 1;

    private static AudioTrack track = null;

    private void initPlay() {
        int channelConfig = channel == 1 ? AudioFormat.CHANNEL_OUT_MONO : AudioFormat.CHANNEL_OUT_STEREO;
        int bufSize = AudioTrack.getMinBufferSize(SAMPLE_RATE_HZ,
                channelConfig, AudioFormat.ENCODING_PCM_16BIT);
        Log.d(TAG, "Buf size: " + bufSize);

        track = new AudioTrack(AudioManager.STREAM_MUSIC,
                SAMPLE_RATE_HZ,
                channelConfig,
                AudioFormat.ENCODING_PCM_16BIT,
                bufSize,
                AudioTrack.MODE_STREAM);
        track.play();
    }

    public void pcmData(byte[] data) {
        Log.i(TAG, "pcmData: " + data.length);
        track.write(data, 0, data.length);
    }
}
