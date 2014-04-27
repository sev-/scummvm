package org.iphsoft.simon1.util;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import com.mojotouch.simon.R;

import android.app.Activity;
import android.content.Intent;
import android.graphics.PixelFormat;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;

public class TestGLActivity extends Activity
{
    public static final String EXTRA_GL_RENDERER = "EXTRA_GL_RENDERER";
    
	GLSurfaceView mGLSurfaceView;
	

   	final Handler h = new Handler()
   	{
   		@Override
   		public void handleMessage(Message msg)
   		{
   			if (msg.what == 0) { close( (String)msg.obj ); }
   		}
   	};
   	
    Runnable mSafetyRunnable = new Runnable() {
        
        @Override
        public void run() {
            setResult(Activity.RESULT_CANCELED, null);;
            finish();
        }
    };


	GLSurfaceView.Renderer renderer = new GLSurfaceView.Renderer()
	{
		@Override
		public void onSurfaceCreated(GL10 gl, EGLConfig config) {
			String   glRenderer = gl.glGetString(GL10.GL_RENDERER);
			Message.obtain(h, 0, glRenderer).sendToTarget();
		}

		@Override
		public void onSurfaceChanged(GL10 gl, int width, int height) { }

		@Override
		public void onDrawFrame(GL10 gl) { }
	};

	@Override
	public void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		
		setContentView(R.layout.gl_test);
		
		mGLSurfaceView = (GLSurfaceView)findViewById(R.id.glsurface);
		mGLSurfaceView.setEGLConfigChooser(8, 8, 8, 8, 16, 0);
		mGLSurfaceView.getHolder().setFormat(PixelFormat.TRANSLUCENT);
		mGLSurfaceView.setRenderer(renderer);
		
		h.postDelayed(mSafetyRunnable, 4000);
	}
	
	@Override
	protected void onDestroy() {
	    // TODO Auto-generated method stub
	    super.onDestroy();
	    
	    h.removeCallbacks(mSafetyRunnable);
	}

	public void close(String glInfo)
	{
		Intent intent= new Intent();
		intent.putExtra(EXTRA_GL_RENDERER, glInfo);
		setResult(Activity.RESULT_OK, intent);
		finish();
	}
}