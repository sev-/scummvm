
package org.iphsoft.simon1;

import java.io.ByteArrayOutputStream;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.LinkedHashMap;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.egl.EGLSurface;

import org.iphsoft.simon1.util.MyLog;
import org.iphsoft.simon1.util.UiThreadUtils;

import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.Bitmap.Config;
import android.graphics.BitmapFactory;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.os.Environment;
import android.util.Log;
import android.view.SurfaceHolder;
import android.widget.Toast;

import com.mojotouch.simon.R;

public abstract class ScummVM implements SurfaceHolder.Callback, Runnable {

    public interface OnCompletionListener
    {
        public void onComplete();
    }

    final protected static String LOG_TAG = "ScummVM";
    final private AssetManager _asset_manager;
    final private Object _sem_surface;

    private static final int SHADER_TYPE_VERTEX = 0;
    private static final int SHADER_TYPE_FRAGMENT = 1;
    private static final int SHADER_TYPE_LQ_VERTEX = 2;
    private static final int SHADER_TYPE_LQ_FRAGMENT = 3;

    private EGL10 _egl;
    private EGLDisplay _egl_display = EGL10.EGL_NO_DISPLAY;
    private EGLConfig _egl_config;
    private EGLContext _egl_context = EGL10.EGL_NO_CONTEXT;
    private EGLSurface _egl_surface = EGL10.EGL_NO_SURFACE;

    private SurfaceHolder _surface_holder;
    private AudioTrack _audio_track;
    private int _sample_rate = 0;
    private int _buffer_size = 0;

    private boolean _restarting = false;

    private String[] _args;

    private OnCompletionListener mListener;

    final private native void create(AssetManager asset_manager,
            EGL10 egl, EGLDisplay egl_display,
            AudioTrack audio_track,
            int sample_rate, int buffer_size, int scaling_option, int gameType);

    final private native void destroy();

    final private native void setSurface(int width, int height);

    final private native int main(String[] args);

    // pause the engine and all native threads
    final public native void setPause(boolean pause);

    final public native void enableZoning(boolean enable);

    // Feed an event to ScummVM. Safe to call from other threads.
    final public native void pushEvent(int type, int arg1, int arg2, int arg3,
            int arg4, int arg5);

    final public native void addBitmapResource(String key, Object bitmap);

    final public native void addShaderSource(byte[] source, int length, int type);

    final public native void saveGame(int slot, boolean force);

    final public native void loadGame(int slot);

    final public native boolean checkLoadConditions();

    final public native void setTouchpadMode(boolean touchpadMode);

    final public native void setAutoLoadSlot(int slot);

    final public native void gameEventJavaToJNI(int type);

    // Callbacks from C++ peer instance
    abstract protected void getDPI(float[] values);

    abstract protected void displayMessageOnOSD(String msg);

    abstract protected void setWindowCaption(String caption);

    abstract protected String[] getPluginDirectories();

    abstract protected void showVirtualKeyboard(boolean enable);

    abstract protected void onGameOption(int option);

    abstract protected void onGameDisplayStarted();

    abstract protected void gameEventJNIToJava(int type);

    abstract protected String[] getSysArchives();

    public ScummVM(AssetManager asset_manager, SurfaceHolder holder) {
        _asset_manager = asset_manager;
        _sem_surface = new Object();

        holder.addCallback(this);
    }

    public void setOnCompletionListener(OnCompletionListener listener)
    {
        mListener = listener;
    }

    // SurfaceHolder callback
    final public void surfaceCreated(SurfaceHolder holder) {
        Log.d(LOG_TAG, "surfaceCreated");

        // no need to do anything, surfaceChanged() will be called in any case
    }

    // SurfaceHolder callback
    final public void surfaceChanged(SurfaceHolder holder, int format,
            int width, int height) {

        // the orientation may reset on standby mode and the theme manager
        // could assert when using a portrait resolution. so lets not do that.
        if (height > width) {
            Log.d(LOG_TAG, String.format("Ignoring surfaceChanged: %dx%d (%d)",
                    width, height, format));
            return;
        }

        Log.d(LOG_TAG, String.format("surfaceChanged: %dx%d (%d)",
                width, height, format));

        synchronized (_sem_surface) {
            _surface_holder = holder;
            _sem_surface.notifyAll();
        }

        // Don't do anything if restarting
        if (_restarting)
            return;

        // store values for the native code
        setSurface(width, height);
    }

    // SurfaceHolder callback
    final public void surfaceDestroyed(SurfaceHolder holder) {
        Log.d(LOG_TAG, "surfaceDestroyed");

        synchronized (_sem_surface) {
            _surface_holder = null;
            _sem_surface.notifyAll();
        }

        // Don't do anything if restarting
        if (_restarting)
            return;

        // clear values for the native code
        setSurface(0, 0);
    }

    public void setRestarting()
    {
        _restarting = true;
    }

    final public void setArgs(String[] args) {
        _args = args;
    }

    final public void run() {
        try {
            initAudio();
            initEGL();

            // wait for the surfaceChanged callback
            synchronized (_sem_surface) {
                while (_surface_holder == null)
                    _sem_surface.wait();
            }
        } catch (Exception e) {
            deinitEGL();
            deinitAudio();

            throw new RuntimeException("Error preparing the ScummVM thread", e);
        }

        MyLog.d("ScummVM: run: creating");

        // Get the scaler option
        int scalerOption = AndroidPortAdditions.instance().getScalingOption();

        create(_asset_manager, _egl, _egl_display,
                _audio_track, _sample_rate, _buffer_size, scalerOption, AndroidPortAdditions.GAME_TYPE.getValue());

        // Ask the game to test shader performance if we use hardware scaling
        // for the first time
        if ((scalerOption == AndroidPortAdditions.SCALING_OPTION_SHADER || scalerOption == AndroidPortAdditions.SCALING_OPTION_LQ_SHADER)
                && AndroidPortAdditions.instance().getShaderTested() == false)
        {
            gameEventJavaToJNI(AndroidPortAdditions.GAME_EVENT_SHOULD_TEST_SHADER);

            // Show short message to the user
            UiThreadUtils.showBackgroundToast(R.string.shader_test_message, Toast.LENGTH_LONG);
        }
        
        // Pass the ultra mode flag if the device supports it
        if (AndroidPortAdditions.instance().checkUltraModeSupport() && scalerOption == AndroidPortAdditions.SCALING_OPTION_SHADER)
        {
            gameEventJavaToJNI(AndroidPortAdditions.GAME_EVENT_USE_ULTRA_MODE);
        }

        try {
            // Add bitmap resources
            MyLog.d("ScummVM: run: adding bitmap resources to native layer");

            addBitmap("skip.png");
            addBitmap("reveal_items.png");
            addBitmap("mouse_mode.png");
            addBitmap("touch_mode.png");
            addBitmap("frame.png");
            addBitmap("walk.png");
            addBitmap("look.png");
            addBitmap("open.png");
            addBitmap("move.png");
            addBitmap("consume.png");
            addBitmap("pick.png");
            addBitmap("close.png");
            addBitmap("use.png");
            addBitmap("talk.png");
            addBitmap("talk_btn.png");
            addBitmap("menu.png");
            addBitmap("remove.png");
            addBitmap("wear.png");
            addBitmap("give.png");
            addBitmap("music_enhanced_by.png");
            addBitmap("cursor.png");
            addBitmap("touch_indicator.png");
            addBitmap("touch_indicator_selected.png");
            addBitmap("arrow_up.png");
            addBitmap("arrow_down.png");

            // Add shader sources
            MyLog.d("ScummVM: run: adding shaders to native layer");

            addShaderSourceFromResource("vertex.glsl", SHADER_TYPE_VERTEX);
            addShaderSourceFromResource("fragment.glsl", SHADER_TYPE_FRAGMENT);
            addShaderSourceFromResource("lq_vertex.glsl", SHADER_TYPE_LQ_VERTEX);
            addShaderSourceFromResource("lq_fragment.glsl", SHADER_TYPE_LQ_FRAGMENT);

        } catch (IOException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();

            destroy();

            deinitEGL();
            deinitAudio();

            throw new RuntimeException("Error preparing the ScummVM thread", e);
        }

        MyLog.d("ScummVM: run: entering main");

        int res = main(_args);

        MyLog.d("ScummVM: run: destroying");
        destroy();

        deinitEGL();
        deinitAudio();

        if (mListener != null)
        {
            mListener.onComplete();
        }
    }

    private void addBitmap(String name) throws IOException
    {
        BitmapFactory.Options options = new BitmapFactory.Options();
        options.inPreferredConfig = Config.ARGB_8888;

        Bitmap bitmap = BitmapFactory.decodeStream(ScummVMApplication.getContext().getAssets()
                .open(name), null, options);
        addBitmapResource(name, bitmap);

        bitmap.recycle();
    }

    private void addShaderSourceFromResource(String asset, int type) throws IOException
    {
        // Obtain the resource input stream
        InputStream is;
        if (AndroidPortAdditions.USE_SHADER_FILES)
        {
            is = new
                    FileInputStream(Environment.getExternalStorageDirectory().getPath()
                            + "/" + asset);
        }
        else
        {
            is = ScummVMApplication.getContext().getAssets().open(asset);
        }

        ByteArrayOutputStream baos = new ByteArrayOutputStream();

        // Read the resource
        byte[] buf = new byte[1024];
        int count;
        while ((count = is.read(buf)) != -1)
        {
            baos.write(buf, 0, count);
        }
        baos.flush();

        byte[] bytes = baos.toByteArray();

        // Add the source
        addShaderSource(bytes, bytes.length, type);

        is.close();
        baos.close();
    }

    private static int EGL_CONTEXT_CLIENT_VERSION = 0x3098;

    final private void initEGL() throws Exception {
        _egl = (EGL10) EGLContext.getEGL();
        _egl_display = _egl.eglGetDisplay(EGL10.EGL_DEFAULT_DISPLAY);

        int[] version = new int[2];
        _egl.eglInitialize(_egl_display, version);

        int[] num_config = new int[1];
        _egl.eglGetConfigs(_egl_display, null, 0, num_config);

        final int numConfigs = num_config[0];

        if (numConfigs <= 0)
            throw new IllegalArgumentException("No EGL configs");

        EGLConfig[] configs = new EGLConfig[numConfigs];
        _egl.eglGetConfigs(_egl_display, configs, numConfigs, num_config);

        // Android's eglChooseConfig is busted in several versions and
        // devices so we have to filter/rank the configs ourselves.
        _egl_config = chooseEglConfig(configs);

        int[] attrib_list = {
                EGL_CONTEXT_CLIENT_VERSION, 2, EGL10.EGL_NONE
        };

        _egl_context = _egl.eglCreateContext(_egl_display, _egl_config,
                EGL10.EGL_NO_CONTEXT, attrib_list);

        if (_egl_context == EGL10.EGL_NO_CONTEXT)
            throw new Exception(String.format("Failed to create context: 0x%x",
                    _egl.eglGetError()));
    }

    // Callback from C++ peer instance
    final protected EGLSurface initSurface() throws Exception {
        _egl_surface = _egl.eglCreateWindowSurface(_egl_display, _egl_config,
                _surface_holder, null);

        if (_egl_surface == EGL10.EGL_NO_SURFACE)
            throw new Exception(String.format(
                    "eglCreateWindowSurface failed: 0x%x", _egl.eglGetError()));

        _egl.eglMakeCurrent(_egl_display, _egl_surface, _egl_surface,
                _egl_context);

        Log.i(LOG_TAG, String.format("Using EGL %s (%s);",
                _egl.eglQueryString(_egl_display, EGL10.EGL_VERSION),
                _egl.eglQueryString(_egl_display, EGL10.EGL_VENDOR)));

        return _egl_surface;
    }

    // Callback from C++ peer instance
    final protected void deinitSurface() {
        if (_egl_display != EGL10.EGL_NO_DISPLAY) {
            _egl.eglMakeCurrent(_egl_display, EGL10.EGL_NO_SURFACE,
                    EGL10.EGL_NO_SURFACE, EGL10.EGL_NO_CONTEXT);

            if (_egl_surface != EGL10.EGL_NO_SURFACE)
                _egl.eglDestroySurface(_egl_display, _egl_surface);
        }

        _egl_surface = EGL10.EGL_NO_SURFACE;
    }

    final private void deinitEGL() {
        if (_egl_display != EGL10.EGL_NO_DISPLAY) {
            _egl.eglMakeCurrent(_egl_display, EGL10.EGL_NO_SURFACE,
                    EGL10.EGL_NO_SURFACE, EGL10.EGL_NO_CONTEXT);

            if (_egl_surface != EGL10.EGL_NO_SURFACE)
                _egl.eglDestroySurface(_egl_display, _egl_surface);

            if (_egl_context != EGL10.EGL_NO_CONTEXT)
                _egl.eglDestroyContext(_egl_display, _egl_context);

            _egl.eglTerminate(_egl_display);
        }

        _egl_surface = EGL10.EGL_NO_SURFACE;
        _egl_context = EGL10.EGL_NO_CONTEXT;
        _egl_config = null;
        _egl_display = EGL10.EGL_NO_DISPLAY;
        _egl = null;
    }

    final private void initAudio() throws Exception {
        _sample_rate = AudioTrack.getNativeOutputSampleRate(
                AudioManager.STREAM_MUSIC);
        _buffer_size = AudioTrack.getMinBufferSize(_sample_rate,
                AudioFormat.CHANNEL_CONFIGURATION_STEREO,
                AudioFormat.ENCODING_PCM_16BIT);

        // ~50ms
        int buffer_size_want = (_sample_rate * 2 * 2 / 20) & ~1023;

        if (_buffer_size < buffer_size_want) {
            Log.w(LOG_TAG, String.format(
                    "adjusting audio buffer size (was: %d)", _buffer_size));

            _buffer_size = buffer_size_want;
        }

        Log.i(LOG_TAG, String.format("Using %d bytes buffer for %dHz audio",
                _buffer_size, _sample_rate));

        _audio_track = new AudioTrack(AudioManager.STREAM_MUSIC,
                _sample_rate,
                AudioFormat.CHANNEL_CONFIGURATION_STEREO,
                AudioFormat.ENCODING_PCM_16BIT,
                _buffer_size,
                AudioTrack.MODE_STREAM);

        if (_audio_track.getState() != AudioTrack.STATE_INITIALIZED)
            throw new Exception(
                    String.format("Error initializing AudioTrack: %d",
                            _audio_track.getState()));
    }

    final private void deinitAudio() {
        if (_audio_track != null)
            _audio_track.stop();

        _audio_track = null;
        _buffer_size = 0;
        _sample_rate = 0;
    }

    private static final int[] s_eglAttribs = {
            EGL10.EGL_CONFIG_ID,
            EGL10.EGL_BUFFER_SIZE,
            EGL10.EGL_RED_SIZE,
            EGL10.EGL_GREEN_SIZE,
            EGL10.EGL_BLUE_SIZE,
            EGL10.EGL_ALPHA_SIZE,
            EGL10.EGL_CONFIG_CAVEAT,
            EGL10.EGL_DEPTH_SIZE,
            EGL10.EGL_LEVEL,
            EGL10.EGL_MAX_PBUFFER_WIDTH,
            EGL10.EGL_MAX_PBUFFER_HEIGHT,
            EGL10.EGL_MAX_PBUFFER_PIXELS,
            EGL10.EGL_NATIVE_RENDERABLE,
            EGL10.EGL_NATIVE_VISUAL_ID,
            EGL10.EGL_NATIVE_VISUAL_TYPE,
            EGL10.EGL_SAMPLE_BUFFERS,
            EGL10.EGL_SAMPLES,
            EGL10.EGL_STENCIL_SIZE,
            EGL10.EGL_SURFACE_TYPE,
            EGL10.EGL_TRANSPARENT_TYPE,
            EGL10.EGL_TRANSPARENT_RED_VALUE,
            EGL10.EGL_TRANSPARENT_GREEN_VALUE,
            EGL10.EGL_TRANSPARENT_BLUE_VALUE,
            EGL10.EGL_RENDERABLE_TYPE
    };

    final private class EglAttribs extends LinkedHashMap<Integer, Integer> {
        public EglAttribs(EGLConfig config) {
            super(s_eglAttribs.length);

            int[] value = new int[1];

            for (int i : s_eglAttribs) {
                _egl.eglGetConfigAttrib(_egl_display, config, i, value);

                put(i, value[0]);
            }
        }

        private int weightBits(int attr, int size) {
            final int value = get(attr);

            int score = 0;

            if (value == size || (size > 0 && value > size))
                score += 10;

            // penalize for wasted bits
            score -= value - size;

            return score;
        }

        public int weight() {
            int score = 10000;

            if (get(EGL10.EGL_CONFIG_CAVEAT) != EGL10.EGL_NONE)
                score -= 1000;

            // less MSAA is better
            score -= get(EGL10.EGL_SAMPLES) * 100;

            // Must be at least 565, but then smaller is better
            score += weightBits(EGL10.EGL_RED_SIZE, 5);
            score += weightBits(EGL10.EGL_GREEN_SIZE, 6);
            score += weightBits(EGL10.EGL_BLUE_SIZE, 5);
            score += weightBits(EGL10.EGL_ALPHA_SIZE, 0);
            score += weightBits(EGL10.EGL_DEPTH_SIZE, 0);
            score += weightBits(EGL10.EGL_STENCIL_SIZE, 0);

            return score;
        }

        public String toString() {
            String s;

            if (get(EGL10.EGL_ALPHA_SIZE) > 0)
                s = String.format("[%d] RGBA%d%d%d%d",
                        get(EGL10.EGL_CONFIG_ID),
                        get(EGL10.EGL_RED_SIZE),
                        get(EGL10.EGL_GREEN_SIZE),
                        get(EGL10.EGL_BLUE_SIZE),
                        get(EGL10.EGL_ALPHA_SIZE));
            else
                s = String.format("[%d] RGB%d%d%d",
                        get(EGL10.EGL_CONFIG_ID),
                        get(EGL10.EGL_RED_SIZE),
                        get(EGL10.EGL_GREEN_SIZE),
                        get(EGL10.EGL_BLUE_SIZE));

            if (get(EGL10.EGL_DEPTH_SIZE) > 0)
                s += String.format(" D%d", get(EGL10.EGL_DEPTH_SIZE));

            if (get(EGL10.EGL_STENCIL_SIZE) > 0)
                s += String.format(" S%d", get(EGL10.EGL_STENCIL_SIZE));

            if (get(EGL10.EGL_SAMPLES) > 0)
                s += String.format(" MSAAx%d", get(EGL10.EGL_SAMPLES));

            if ((get(EGL10.EGL_SURFACE_TYPE) & EGL10.EGL_WINDOW_BIT) > 0)
                s += " W";
            if ((get(EGL10.EGL_SURFACE_TYPE) & EGL10.EGL_PBUFFER_BIT) > 0)
                s += " P";
            if ((get(EGL10.EGL_SURFACE_TYPE) & EGL10.EGL_PIXMAP_BIT) > 0)
                s += " X";

            switch (get(EGL10.EGL_CONFIG_CAVEAT)) {
                case EGL10.EGL_NONE:
                    break;

                case EGL10.EGL_SLOW_CONFIG:
                    s += " SLOW";
                    break;

                case EGL10.EGL_NON_CONFORMANT_CONFIG:
                    s += " NON_CONFORMANT";

                default:
                    s += String.format(" unknown CAVEAT 0x%x",
                            get(EGL10.EGL_CONFIG_CAVEAT));
            }

            s += " EGL_RENDERABLE_TYPE: " + get(EGL10.EGL_RENDERABLE_TYPE);

            return s;
        }
    };

    private static int EGL_OPENGL_ES2_BIT = 4;

    final private EGLConfig chooseEglConfig(EGLConfig[] configs) {
        EGLConfig res = configs[0];
        int bestScore = -1;

        Log.d(LOG_TAG, "EGL configs:");

        for (EGLConfig config : configs) {
            EglAttribs attr = new EglAttribs(config);

            // must have
            if ((attr.get(EGL10.EGL_SURFACE_TYPE) & EGL10.EGL_WINDOW_BIT) == 0
                    || (attr.get(EGL10.EGL_RENDERABLE_TYPE) & EGL_OPENGL_ES2_BIT) == 0)
                continue;

            int score = attr.weight();

            Log.d(LOG_TAG, String.format("%s (%d)", attr.toString(), score));

            if (score > bestScore) {
                res = config;
                bestScore = score;
            }
        }

        if (bestScore < 0)
            Log.e(LOG_TAG,
                    "Unable to find an acceptable EGL config, expect badness.");

        Log.d(LOG_TAG, String.format("Chosen EGL config: %s",
                new EglAttribs(res).toString()));

        return res;
    }

    static {

        System.loadLibrary("engine");
    }
}
