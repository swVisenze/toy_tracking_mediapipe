package com.visenze;

import android.content.Context;
import android.util.Log;

import com.google.mediapipe.framework.AndroidAssetUtil;
import com.google.mediapipe.framework.AndroidPacketCreator;
import com.google.mediapipe.framework.Graph;
import com.google.mediapipe.framework.MediaPipeException;
import com.google.mediapipe.framework.Packet;
import com.google.mediapipe.framework.PacketCallback;
import com.google.mediapipe.framework.PacketGetter;
import com.google.mediapipe.formats.proto.DetectionProto.Detection;
import com.google.mediapipe.formats.proto.LocationDataProto;

import java.io.File;
import java.nio.ByteBuffer;
import java.util.List;
import java.util.concurrent.atomic.AtomicBoolean;

public class ToyTracking {

    private static final String TAG = "ToyTracking";
    private static final String BINARY_GRAPH_NAME = "toy_mobile_cpu.binarypb";
    private static final String INPUT_VIDEO_STREAM_NAME = "input_video_cpu";
    private static final String OUTPUT_TRACKED_DETECTION_STREAM_NAME = "tracked_detections";

    private Graph mediapipeGraph;
    private AndroidPacketCreator packetCreator;
    private final AtomicBoolean started = new AtomicBoolean(false);
    private long previousTimestamp = 0;
    private String videoInputStream;
    private String detectionOutputStream;

    static {
        // Load all native libraries needed by the app.
        System.loadLibrary("mediapipe_jni");
        System.loadLibrary("opencv_java3");
    }

    public static void init(Context context, int width, int height) {
        AndroidAssetUtil.initializeNativeAssetManager(context);
        nativeInit(width, height);
    }

    public static void destroy() {
        nativeDestroy();
    }

    private static native void nativeInit(int width, int heigth);

    private static native void nativeDestroy();


    public ToyTracking(Context context) {
        // Initialize asset manager so that MediaPipe native libraries can access the app assets, e.g.,
        // binary graphs.
        AndroidAssetUtil.initializeNativeAssetManager(context);

        this.mediapipeGraph = new Graph();
        if ((new File(BINARY_GRAPH_NAME)).isAbsolute()) {
            this.mediapipeGraph.loadBinaryGraph(BINARY_GRAPH_NAME);
        } else {
            this.mediapipeGraph.loadBinaryGraph(AndroidAssetUtil.getAssetBytes(context.getAssets(), BINARY_GRAPH_NAME));
        }

        this.packetCreator = new AndroidPacketCreator(this.mediapipeGraph);
        addVideoStreams(INPUT_VIDEO_STREAM_NAME, OUTPUT_TRACKED_DETECTION_STREAM_NAME);
    }

    private void addVideoStreams(String inputStream, String detectionOutputStream) {
        this.videoInputStream = inputStream;
//        this.videoOutputStream = videoOuputStream;
        this.detectionOutputStream = detectionOutputStream;
//        this.mediapipeGraph.setParentGlContext(parentNativeContext);
        //adb shell setprop log.tag.ToyTracking VERBOSE
//        if (this.videoOutputStream != null) {
//            this.mediapipeGraph.addPacketCallback(this.videoOutputStream, new PacketCallback() {
//                public void process(Packet packet) {
//                    TextureFrame frame = PacketGetter.getTextureFrame(packet);
//                }
//            });
//            this.videoSurfaceOutput = this.mediapipeGraph.addSurfaceOutput(this.videoOutputStream);
//            this.videoSurfaceOutput.setFlipY(FLIP_FRAMES_VERTICALLY);
//        }
        if (this.detectionOutputStream != null) {
            this.mediapipeGraph.addPacketCallback(this.detectionOutputStream, new PacketCallback() {
                @Override
                public void process(Packet packet) {
//                    Log.v(TAG, "received tracked detection result");
                    List<Detection> detections = PacketGetter.getProtoVector(packet, Detection.parser());
                    String debugOut = getDetectionsDebugString(detections);
                    Log.v(TAG, debugOut);
                }
            });
        }
    }

    public void init() {}

    public void process(byte[] bufferInput, int imageWidth, int imageHeight) {
        long timeMicro = System.nanoTime() / 1000;
        ByteBuffer buffer = ByteBuffer.wrap(bufferInput);
        tracking(buffer, imageWidth, imageHeight, timeMicro);
    }

    public void tracking(ByteBuffer buffer, int imageWidth, int imageHeight, long timestamp) {
        Packet imagePacket = null;
        try {
            Log.v(TAG, String.format("Input buffer at: %d width: %d height: %d", timestamp, imageWidth, imageHeight));

            if (this.maybeAcceptNewFrame(timestamp)) {

                imagePacket = this.packetCreator.createRgbaImageFrame(buffer, imageWidth, imageHeight);

                try {
                    this.mediapipeGraph.addConsumablePacketToInputStream(this.videoInputStream, imagePacket, timestamp);
                    imagePacket = null;
                } catch (MediaPipeException exception) {
                    Log.v(TAG, "exception: ",  exception);
                }
            }
        } catch (RuntimeException exception) {
            Log.v(TAG, "exception: ",  exception);
        } finally {
            if (imagePacket != null) {
                imagePacket.release();
            }
        }
    }


    public void reset(String code) {
        if (!this.started.getAndSet(true)) {
            this.mediapipeGraph.startRunningGraph();
        }
    }


    public void close() {
        if (this.started.get()) {
            try {
                this.mediapipeGraph.closeAllPacketSources();
                this.mediapipeGraph.waitUntilGraphDone();
            } catch (MediaPipeException exception) {
                Log.e(TAG, "Mediapipe error: ", exception);
            }

            try {
                this.mediapipeGraph.tearDown();
            } catch (MediaPipeException exception) {
                Log.e(TAG, "Mediapipe error: ", exception);
            }
            this.started.getAndSet(false);
        }
    }

    private String getDetectionsDebugString(List<Detection> detections) {
        if(detections.size() == 0) {
            return "no detections";
        }
        String outputStr = "Number of detections: " + detections.size() +"\n";
        int index = 0;
        for(Detection detection: detections) {
            outputStr +=
                    "\t#detection [" + index +"]\n";
            LocationDataProto.LocationData.RelativeBoundingBox box = detection.getLocationData().getRelativeBoundingBox();

            outputStr +=
                    "\t(detectionId"+detection.getDetectionId() +" x: "+box.getXmin() + " y: "+box.getYmin() + ")\n";
        }
        return outputStr;
    }


    private boolean maybeAcceptNewFrame(long timestamp) {
        if (!this.started.getAndSet(true)) {
            this.mediapipeGraph.startRunningGraph();
        }
        if(timestamp > previousTimestamp) {
            previousTimestamp = timestamp;
            return true;
        } else {
            return false;
        }
    }

}
