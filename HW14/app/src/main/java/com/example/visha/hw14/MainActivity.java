package com.example.visha.hw14;

import android.Manifest;
import android.app.Activity;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.hardware.Camera;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.SurfaceTexture;
import android.hardware.usb.UsbDeviceConnection;
import android.hardware.usb.UsbManager;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.TextureView;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.ScrollView;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.widget.TextView;

import com.hoho.android.usbserial.driver.CdcAcmSerialDriver;
import com.hoho.android.usbserial.driver.ProbeTable;
import com.hoho.android.usbserial.driver.UsbSerialDriver;
import com.hoho.android.usbserial.driver.UsbSerialPort;
import com.hoho.android.usbserial.driver.UsbSerialProber;
import com.hoho.android.usbserial.util.SerialInputOutputManager;

import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import static android.graphics.Color.blue;
import static android.graphics.Color.green;
import static android.graphics.Color.red;
import static android.graphics.Color.rgb;

public class MainActivity extends Activity implements TextureView.SurfaceTextureListener {
    private Camera mCamera; //object representing the camera
    private TextureView mTextureView;
    private SurfaceView mSurfaceView;
    private SurfaceHolder mSurfaceHolder;
    private Bitmap bmp = Bitmap.createBitmap(640, 480, Bitmap.Config.ARGB_8888);
    private Canvas canvas = new Canvas(bmp);
    private Paint paint1 = new Paint();
    private TextView mTextView;
    SeekBar myControl;
    SeekBar myControl2;
    SeekBar myControl3;

    static long prevtime = 0; // for FPS calculation

    //    SeekBar myControl;
//    TextView myTextView;
//    Button button;
//    TextView myTextView2;
//    ScrollView myScrollView;
//    TextView myTextView3;
//    SeekBar myControlR;
//    SeekBar myControlT;
//    TextView myTextViewR;
//    TextView myTextViewT;
//    int progressChanged = 0;
//    int S = 20;
//    int T = 10;
////    private Camera mCamera;
//    private TextView mTextView;
//    private TextureView mTextureView;
//    private SurfaceView mSurfaceView;
//    private SurfaceHolder mSurfaceHolder;
//    private Bitmap bmp = Bitmap.createBitmap(640, 480, Bitmap.Config.ARGB_8888);
//    private Canvas canvas = new Canvas(bmp);
//    private Paint paint1 = new Paint();
    private UsbManager manager;
    private UsbSerialPort sPort;
    private final ExecutorService mExecutor = Executors.newSingleThreadExecutor();
    private SerialInputOutputManager mSerialIoManager;

//    static long prevtime = 0; // for FPS calculation

    @Override

    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON); // keeps the screen from turning off

        mTextView = (TextView) findViewById(R.id.cameraStatus);
        myControl = (SeekBar) findViewById(R.id.seek1);


        // see if the app has permission to use the camera
        ActivityCompat.requestPermissions(MainActivity.this, new String[]{Manifest.permission.CAMERA}, 1);
        if (ContextCompat.checkSelfPermission(this, Manifest.permission.CAMERA) == PackageManager.PERMISSION_GRANTED) {
            mSurfaceView = (SurfaceView) findViewById(R.id.surfaceview);
            mSurfaceHolder = mSurfaceView.getHolder();

            mTextureView = (TextureView) findViewById(R.id.textureview);
            mTextureView.setSurfaceTextureListener(this);

            myControl = (SeekBar) findViewById(R.id.seek1);
            myControl2 = (SeekBar) findViewById(R.id.seek2);
            myControl3 = (SeekBar) findViewById(R.id.seek2);

            // set the paintbrush for writing text on the image
            paint1.setColor(0xffff0000); // red
            paint1.setTextSize(24);

            mTextView.setText("started camera");
        } else {
            mTextView.setText("no camera permissions"); //this happens whe the app crashes because you have no camera permissions
        }

    }

    public void onSurfaceTextureAvailable(SurfaceTexture surface, int width, int height) {
        mCamera = Camera.open();
        Camera.Parameters parameters = mCamera.getParameters();
        parameters.setPreviewSize(640, 480); //resolution, increasing numbers might decrease noise but it's more data which makes code slower
        parameters.setFocusMode(Camera.Parameters.FOCUS_MODE_INFINITY); // no autofocusing cause might slow down frame per seconds of camera
        parameters.setAutoExposureLock(true); // keep the white balance constant, otherwise white balance set just from first picture
        mCamera.setParameters(parameters); //can set to black and white etc
        mCamera.setDisplayOrientation(90); // rotate to portrait mode

        try {
            mCamera.setPreviewTexture(surface);
            mCamera.startPreview();
        } catch (IOException ioe) {
            // Something bad happened
        }
    }

    //infinite while loop
    public void onSurfaceTextureSizeChanged(SurfaceTexture surface, int width, int height) {
        // Ignored, Camera does all the work for us
    }

    public boolean onSurfaceTextureDestroyed(SurfaceTexture surface) {
        mCamera.stopPreview();
        mCamera.release();
        return true;
    }

    int threshred = 0; // threshold for red
    int threshgreen = 0; // comparison value for green
    int threshblue = 0; // comparison value for blue
    int sum = 0; //sum of pixel colors
    int sum_r = 0; //sum of pixel colors times the radius
    int centre_of_mass = 0; //reddest point in the line

    // the important function
    public void onSurfaceTextureUpdated(SurfaceTexture surface) {
        // every time there is a new Camera preview frame
        mTextureView.getBitmap(bmp);

        myControl.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                threshred = progress;
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) { //on click
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) { //on release

            }
        });

        myControl2.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                threshgreen = progress;
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) { //on click
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) { //on release

            }
        });

        myControl3.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                threshblue = progress;
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) { //on click
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) { //on release

            }
        });

        final Canvas c = mSurfaceHolder.lockCanvas();
        if (c != null) {
            //for (int k=bmp.getHeight()/2; k < bmp.getHeight()/2+1; k++) {//loop over rows of pixels

            int[] pixels = new int[bmp.getWidth()]; // pixels[] is the RGBA data, array whose size is width of image: grab rows of the image and put pixel data into array
            //int startY = k; // which row in the bitmap to analyze to read
            bmp.getPixels(pixels, 0, bmp.getWidth(), 0, 240, bmp.getWidth(), 1);

            sum = 0;
            sum_r = 0;
            // in the row find the centre of mass of red
            for (int i = 0; i < bmp.getWidth(); i++) {//loop over every element of pixels
                if  (red(pixels[i])-green(pixels[i]) > threshgreen && red(pixels[i])-green(pixels[i]) > threshblue && red(pixels[i])>threshred) { //adjust based on how  red is the definition of red
                    pixels[i] = rgb(1, 1, 1); // over write the pixel with pure black
                    sum = sum + (red(pixels[i]) + green(pixels[i]) + blue(pixels[i]));
                    sum_r = sum_r + (red(pixels[i]) + green(pixels[i]) + blue(pixels[i])) * i;
                }

            }

            if (sum>5) {
                centre_of_mass = (sum_r / sum);
            }
            else{
                centre_of_mass = 0;
            }

            // update the row
            bmp.setPixels(pixels, 0, bmp.getWidth(), 0, 240, bmp.getWidth(), 1);
            //}
        }

//        String sendString = String.valueOf(centre_of_mass) + '\n';
//        try {
//            sPort.write(sendString.getBytes(), 10); // 10 is the timeout
//        } catch (IOException e) { }

        // draw a circle at some position
        //int pos = 50;
        canvas.drawCircle(centre_of_mass, 240, 5, paint1); // x position, y position, diameter, color

        // write the pos as text
        canvas.drawText("CM = " + centre_of_mass, 10, 200, paint1);
        c.drawBitmap(bmp, 0, 0, null);
        mSurfaceHolder.unlockCanvasAndPost(c);

        // calculate the FPS to see how fast the code is running
        long nowtime = System.currentTimeMillis();
        long diff = nowtime - prevtime;
        mTextView.setText("FPS " + 1000 / diff);
        prevtime = nowtime;
    }
}