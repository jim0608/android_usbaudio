package au.id.jms.usbaudiodemo;

public interface AudioCallback {
    public void pcmData(byte[] data);
}
