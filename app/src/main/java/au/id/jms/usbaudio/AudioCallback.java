package au.id.jms.usbaudio;

public interface AudioCallback {
    public void pcmData(byte[] data);
}
