package au.id.jms.usbaudio;

import android.annotation.SuppressLint;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.os.Build;
import android.support.annotation.RequiresApi;
import android.util.Log;

public class AudioPlayback {
	private static final String TAG = "AudioPlayback";

	private static final int SAMPLE_RATE_HZ = 32000;
	private static final int channel = 1;

	private static AudioTrack track = null;

	@SuppressLint("NewApi")
	public static void setup() {
		Log.i(TAG, "Audio Playback");

		int channelConfig = channel == 1?AudioFormat.CHANNEL_OUT_MONO:AudioFormat.CHANNEL_OUT_STEREO;
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

	public static void write(byte[] decodedAudio) {
		Log.i(TAG, "write: "+decodedAudio.length);
		track.write(decodedAudio, 0, decodedAudio.length);
	}
}
