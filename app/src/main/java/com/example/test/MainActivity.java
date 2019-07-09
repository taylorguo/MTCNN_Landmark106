package com.example.test;

import android.app.Activity;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.media.Image;
import android.net.Uri;
import android.os.Environment;
import android.support.v4.app.ActivityCompat;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;

import static android.content.ContentValues.TAG;

public class MainActivity extends AppCompatActivity {

    private static final int SELECT_IMAGE = 1;

    private MTCNNBox mtcnnBox = new MTCNNBox();
    private MTCNN mtcnn = new MTCNN();
    private ImageView imageView;
    private Bitmap yourSelectedImage = null;

    /////////////////////////////
    private static final int REQUEST_EXTERNAL_STORAGE = 1;
    private static String[] PERMISSIONS_STORAGE = {
            "android.permission.READ_EXTERNAL_STORAGE",
            "android.permission.WRITE_EXTERNAL_STORAGE" };

    public static void verifyStoragePermissions(Activity activity) {

        try {
            //检测是否有写的权限
            int permission = ActivityCompat.checkSelfPermission(activity,
                    "android.permission.WRITE_EXTERNAL_STORAGE");
            if (permission != PackageManager.PERMISSION_GRANTED) {
                // 没有写的权限，去申请写的权限，会弹出对话框
                ActivityCompat.requestPermissions(activity, PERMISSIONS_STORAGE,REQUEST_EXTERNAL_STORAGE);
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
    ////////////////////////////

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("landmark106");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);


        verifyStoragePermissions(this);
        try {
            copyBigDataToSD("det1.bin");
            copyBigDataToSD("det1.param");
            copyBigDataToSD("det2.bin");
            copyBigDataToSD("det2.param");
            copyBigDataToSD("det3.bin");
            copyBigDataToSD("det3.param");
        } catch (IOException e) {
            e.printStackTrace();
        }

        File sdDir1 = Environment.getExternalStorageDirectory();//获取跟目录
        String sdPath1 = sdDir1.toString() + "/mtcnn/";
        mtcnnBox.FaceDetectionModelInit(sdPath1);
        Log.i(TAG, " ********************************************* MTCNN Box 模型初始化已经完成");

        //////////////
        //////////////

        //verifyStoragePermissions(this);
        try {
            copyBigDataToSD("landmark.bin");
            copyBigDataToSD("landmark.param");
        } catch (IOException e) {
            e.printStackTrace();
        }

        File sdDir = Environment.getExternalStorageDirectory();//获取跟目录
        String sdPath = sdDir.toString() + "/mtcnn/";
        mtcnn.ModelInit(sdPath);
        Log.i(TAG, " ********************************************* MTCNN 模型初始化已经完成");
        mtcnn.SetThreadsNumber(2);

        imageView = (ImageView) findViewById(R.id.imageView);
        Button buttonImage = (Button) findViewById(R.id.buttonImage);
        buttonImage.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View arg0) {
                Intent i = new Intent(Intent.ACTION_PICK);
                i.setType("image/*");
                startActivityForResult(i, SELECT_IMAGE);
            }
        });

        Button buttonDetect = (Button) findViewById(R.id.buttonDetect);
        buttonDetect.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View arg0) {

                mtcnnBox.SetMinFaceSize(50);
                mtcnnBox.SetThreadsNumber(2);
                mtcnnBox.SetTimeCount(5);
                Log.i(TAG, " ********************************************* MTCNN Box 模型参数设置完成");

                if (yourSelectedImage == null)
                    return;

                int width = yourSelectedImage.getWidth();
                int height = yourSelectedImage.getHeight();
                //////
                int bytes = yourSelectedImage.getByteCount();
                ByteBuffer buffer = ByteBuffer.allocate(bytes);
                yourSelectedImage.copyPixelsToBuffer(buffer);
                byte[] imageData= buffer.array();

                long timeDetect = System.currentTimeMillis();
                int[] faceInfo = mtcnnBox.MaxFaceDetect(imageData, width, height,4);
                int left=0, top=0, right=0, bottom=0;
                if (faceInfo.length > 1){
                    for (int i=0; i<faceInfo[0]; i++){
                        left = faceInfo[1+14*i];
                        top = faceInfo[2+14*i];
                        right = faceInfo[3+14*i];
                        bottom = faceInfo[4+14*i];
                    }
                }
                Bitmap cropedImage = Bitmap.createBitmap(yourSelectedImage, left, top, right-left, bottom-top, null, false);
                //////

                Log.i(TAG, " ********* 开始检测");
                //long timeDetect = System.currentTimeMillis();
                float landmark[] = null;
                landmark = mtcnn.Detect(cropedImage, width, height, 4);
                timeDetect = System.currentTimeMillis() - timeDetect;
                Log.i(TAG, " ********* 完成检测, 用时: "+timeDetect);

                if (landmark.length>1){
                    Bitmap drawBitmap = yourSelectedImage.copy(Bitmap.Config.ARGB_8888, true);

                    Canvas canvas = new Canvas(drawBitmap);
                    Paint paint = new Paint();
                    paint.setColor(Color.RED);
                    paint.setStyle(Paint.Style.STROKE);
                    paint.setStrokeWidth(5);

                    for (int i=0; i<106; i++){
                        canvas.drawPoints(new float[]{landmark[i] + left, landmark[i + 1]+top}, paint);
                    }
                    imageView.setImageBitmap(drawBitmap);
                }



            }
        });

    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    //public native String stringFromJNI();

    /////////////////////////////
    private void copyBigDataToSD(String strOutFileName) throws IOException {
        Log.i(TAG, " ***************************** start copy file:    " + strOutFileName);
        File sdDir = Environment.getExternalStorageDirectory();//获取跟目录
        File file = new File(sdDir.toString()+"/mtcnn/");
        if (!file.exists()) {
            file.mkdir();
        }

        String tmpFile = sdDir.toString()+"/mtcnn/" + strOutFileName;
        File f = new File(tmpFile);
        if (f.exists()) {
            Log.i(TAG, "file exists " + strOutFileName);
            return;
        }
        InputStream myInput;
        java.io.OutputStream myOutput = new FileOutputStream(sdDir.toString()+"/mtcnn/"+ strOutFileName);
        myInput = this.getAssets().open(strOutFileName);
        byte[] buffer = new byte[1024];
        int length = myInput.read(buffer);
        while (length > 0) {
            myOutput.write(buffer, 0, length);
            length = myInput.read(buffer);
        }
        myOutput.flush();
        myInput.close();
        myOutput.close();
        Log.i(TAG, "  ***************************** end copy file:  " + strOutFileName);

    }
    ////////////////////////////

    private Bitmap decodeUri(Uri selectedImage) throws FileNotFoundException {
        // Decode image size
        BitmapFactory.Options o = new BitmapFactory.Options();
        o.inJustDecodeBounds = true;
        BitmapFactory.decodeStream(getContentResolver().openInputStream(selectedImage), null, o);

        // The new size we want to scale to
        final int REQUIRED_SIZE = 400;

        // Find the correct scale value. It should be the power of 2.
        int width_tmp = o.outWidth, height_tmp = o.outHeight;
        int scale = 1;
        while (true) {
            if (width_tmp / 2 < REQUIRED_SIZE
                    || height_tmp / 2 < REQUIRED_SIZE) {
                break;
            }
            width_tmp /= 2;
            height_tmp /= 2;
            scale *= 2;
        }

        // Decode with inSampleSize
        BitmapFactory.Options o2 = new BitmapFactory.Options();
        o2.inSampleSize = scale;
        return BitmapFactory.decodeStream(getContentResolver().openInputStream(selectedImage), null, o2);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);

        if (resultCode == RESULT_OK && null != data) {
            Uri selectedImage = data.getData();

            try {
                if (requestCode == 1) {
                    Bitmap bitmap = decodeUri(selectedImage);

                    Bitmap rgba = bitmap.copy(Bitmap.Config.ARGB_8888, true);

                    // resize to 227x227
                    //yourSelectedImage = Bitmap.createScaledBitmap(rgba, 227, 227, false);
                    yourSelectedImage = rgba;

                    imageView.setImageBitmap(yourSelectedImage);
                }
            } catch (FileNotFoundException e) {
                Log.e("MainActivity", "FileNotFoundException");
                return;
            }
        }
    }


}
