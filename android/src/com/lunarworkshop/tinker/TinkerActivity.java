package com.lunarworkshop.tinker;

import java.io.IOException;
import java.util.Arrays;

import android.view.*;
import android.app.*;
import android.content.res.*;
import android.graphics.*;
import android.content.*;
import android.util.*;
import android.os.*;

import android.util.Log;

import org.libsdl.app.SDLActivity;

public class TinkerActivity extends SDLActivity {
	void TinkerLog(String s)
	{
		//Log.v("TinkerDebug", s);
	}

	AssetManager am;
	String[]     assetList;
	int          assetList_index;

	public boolean assetOpenDir(String sDirectory)
	{
		if (am == null)
			am = getAssets();

		try
		{
			assetList = am.list(sDirectory);
		}
		catch (IOException e)
		{
			return false;
		}

		if (assetList == null)
			return false;

		assetList_index = 0;
		return true;
	}

	public String assetGetNext()
	{
		if (assetList_index >= assetList.length)
			return null;

		return assetList[assetList_index++];
	}

	public boolean assetIsDirectory(String sDirectory)
	{
		if (!assetExists(sDirectory))
			return false;

		if (am == null)
			am = getAssets();

		try
		{
			return am.open(sDirectory) == null;
		}
		catch (IOException e)
		{
			// Assume the file exists so if it can't be opened as a file it must be a directory.
			return true;
		}
	}

	public boolean assetExists(String sFile)
	{
		if (am == null)
			am = getAssets();

		int slash = sFile.lastIndexOf('/');

		try
		{
			if (slash < 0)
				return Arrays.asList(am.list("")).contains(sFile);

			String sPath = sFile.substring(0, slash);
			String sFileOnly = sFile.substring(slash+1);

			return Arrays.asList(am.list(sPath)).contains(sFileOnly);
		}
		catch (IOException e)
		{
			return false;
		}
	}

	@Override
	public void onActionModeFinished(ActionMode mode)
	{
		TinkerLog("onActionModeFinished()");
		super.onActionModeFinished(mode);
	}

	@Override
	public void onActionModeStarted(ActionMode mode)
	{
		TinkerLog("onActionModeStarted()");
		super.onActionModeStarted(mode);
	}

	@Override
	public void onAttachFragment(Fragment fragment)
	{
		TinkerLog("onAttachFragment()");
		super.onAttachFragment(fragment);
	}

	@Override
	public void onAttachedToWindow()
	{
		TinkerLog("onAttachedToWindow()");
		super.onAttachedToWindow();
	}

	@Override
	public void onBackPressed()
	{
		TinkerLog("onBackPressed()");
		super.onBackPressed();
	}

	@Override
	public void onConfigurationChanged(Configuration newConfig)
	{
		TinkerLog("onConfigurationChanged()");
		super.onConfigurationChanged(newConfig);
	}

	@Override
	public void onContentChanged()
	{
		TinkerLog("onContentChanged()");
		super.onContentChanged();
	}

	@Override
	public boolean onContextItemSelected(MenuItem item)
	{
		TinkerLog("onContextItemSelected()");
		return super.onContextItemSelected(item);
	}

	@Override
	public void onContextMenuClosed(Menu menu)
	{
		TinkerLog("onContextMenuClosed()");
		super.onContextMenuClosed(menu);
	}

	@Override
	public void onCreateContextMenu(ContextMenu menu, View v, ContextMenu.ContextMenuInfo menuInfo)
	{
		TinkerLog("onCreateContextMenu()");
		super.onCreateContextMenu(menu, v, menuInfo);
	}

	@Override
	public CharSequence onCreateDescription()
	{
		TinkerLog("onCreateDescription()");
		return super.onCreateDescription();
	}

	@Override
	public void onCreateNavigateUpTaskStack(TaskStackBuilder builder)
	{
		TinkerLog("onCreateNavigateUpTaskStack()");
		super.onCreateNavigateUpTaskStack(builder);
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu)
	{
		TinkerLog("onCreateOptionsMenu()");
		return super.onCreateOptionsMenu(menu);
	}

	@Override
	public boolean onCreatePanelMenu(int featureId, Menu menu)
	{
		TinkerLog("onCreatePanelMenu()");
		return super.onCreatePanelMenu(featureId, menu);
	}

	@Override
	public View onCreatePanelView(int featureId)
	{
		TinkerLog("onCreatePanelView()");
		return super.onCreatePanelView(featureId);
	}

	@Override
	public boolean onCreateThumbnail(Bitmap outBitmap, Canvas canvas)
	{
		TinkerLog("onCreateThumbnail()");
		return super.onCreateThumbnail(outBitmap, canvas);
	}

	@Override
	public View onCreateView(View parent, String name, Context context, AttributeSet attrs)
	{
		TinkerLog("onCreateView()");
		return super.onCreateView(parent, name, context, attrs);
	}

	@Override
	public View onCreateView(String name, Context context, AttributeSet attrs)
	{
		TinkerLog("onCreateView()");
		return super.onCreateView(name, context, attrs);
	}

	@Override
	public void onDetachedFromWindow()
	{
		TinkerLog("onDetachedFromWindow()");
		super.onDetachedFromWindow();
	}

	@Override
	public boolean onGenericMotionEvent(MotionEvent event)
	{
		TinkerLog("onGenericMotionEvent()");
		return super.onGenericMotionEvent(event);
	}

	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event)
	{
		TinkerLog("onKeyDown()");
		return super.onKeyDown(keyCode, event);
	}

	@Override
	public boolean onKeyLongPress(int keyCode, KeyEvent event)
	{
		TinkerLog("onKeyLongPress()");
		return super.onKeyLongPress(keyCode, event);
	}

	@Override
	public boolean onKeyMultiple(int keyCode, int repeatCount, KeyEvent event)
	{
		TinkerLog("onKeyMultiple()");
		return super.onKeyMultiple(keyCode, repeatCount, event);
	}

	@Override
	public boolean onKeyShortcut(int keyCode, KeyEvent event)
	{
		TinkerLog("onKeyShortcut()");
		return super.onKeyShortcut(keyCode, event);
	}

	@Override
	public boolean onKeyUp(int keyCode, KeyEvent event)
	{
		TinkerLog("onKeyUp()");
		return super.onKeyUp(keyCode, event);
	}

	@Override
	public void onLowMemory()
	{
		TinkerLog("onLowMemory()");
		super.onLowMemory();
	}

	@Override
	public boolean onMenuItemSelected(int featureId, MenuItem item)
	{
		TinkerLog("onMenuItemSelected()");
		return super.onMenuItemSelected(featureId, item);
	}

	@Override
	public boolean onMenuOpened(int featureId, Menu menu)
	{
		TinkerLog("onMenuOpened()");
		return super.onMenuOpened(featureId, menu);
	}

	@Override
	public boolean onNavigateUp()
	{
		TinkerLog("onNavigateUp()");
		return super.onNavigateUp();
	}

	@Override
	public boolean onNavigateUpFromChild(Activity child)
	{
		TinkerLog("onNavigateUpFromChild()");
		return super.onNavigateUpFromChild(child);
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item)
	{
		TinkerLog("onOptionsItemSelected()");
		return super.onOptionsItemSelected(item);
	}

	@Override
	public void onOptionsMenuClosed(Menu menu)
	{
		TinkerLog("onOptionsMenuClosed()");
		super.onOptionsMenuClosed(menu);
	}

	@Override
	public void onPanelClosed(int featureId, Menu menu)
	{
		TinkerLog("onPanelClosed()");
		super.onPanelClosed(featureId, menu);
	}

	@Override
	public void onPrepareNavigateUpTaskStack(TaskStackBuilder builder)
	{
		TinkerLog("onPrepareNavigateUpTaskStack()");
		super.onPrepareNavigateUpTaskStack(builder);
	}

	@Override
	public boolean onPrepareOptionsMenu(Menu menu)
	{
		TinkerLog("onPrepareOptionsMenu()");
		return super.onPrepareOptionsMenu(menu);
	}

	@Override
	public boolean onPreparePanel(int featureId, View view, Menu menu)
	{
		TinkerLog("onPreparePanel()");
		return super.onPreparePanel(featureId, view, menu);
	}

	@Override
	public void onProvideAssistData(Bundle data)
	{
		TinkerLog("onProvideAssistData()");
		super.onProvideAssistData(data);
	}

	@Override
	public boolean onSearchRequested()
	{
		TinkerLog("onSearchRequested()");
		return super.onSearchRequested();
	}

	@Override
	public boolean onTouchEvent(MotionEvent event)
	{
		TinkerLog("onTouchEvent()");
		return super.onTouchEvent(event);
	}

	@Override
	public boolean onTrackballEvent(MotionEvent event)
	{
		TinkerLog("onTrackballEvent()");
		return super.onTrackballEvent(event);
	}

	@Override
	public void onTrimMemory(int level)
	{
		TinkerLog("onTrimMemory()");
		super.onTrimMemory(level);
	}

	@Override
	public void onUserInteraction()
	{
		TinkerLog("onUserInteraction()");
		super.onUserInteraction();
	}

	@Override
	public void onWindowAttributesChanged(WindowManager.LayoutParams params)
	{
		TinkerLog("onWindowAttributesChanged()");
		super.onWindowAttributesChanged(params);
	}

	@Override
	public void onWindowFocusChanged(boolean hasFocus)
	{
		TinkerLog("onWindowFocusChanged()");
		super.onWindowFocusChanged(hasFocus);
	}

	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data)
	{
		TinkerLog("onActivityResult()");
		super.onActivityResult(requestCode, resultCode, data);
	}

	@Override
	protected void onApplyThemeResource(Resources.Theme theme, int resid, boolean first)
	{
		TinkerLog("onApplyThemeResource()");
		super.onApplyThemeResource(theme, resid, first);
	}

	@Override
	protected void onChildTitleChanged(Activity childActivity, CharSequence title)
	{
		TinkerLog("onChildTitleChanged()");
		super.onChildTitleChanged(childActivity, title);
	}

	@Override
	protected void onCreate(Bundle savedInstanceState)
	{
		TinkerLog("onCreate()");
		super.onCreate(savedInstanceState);
	}

	@Override
	protected void onDestroy()
	{
		TinkerLog("onDestroy()");
		super.onDestroy();
	}

	@Override
	protected void onNewIntent(Intent intent)
	{
		TinkerLog("onNewIntent()");
		super.onNewIntent(intent);
	}

	@Override
	protected void onPause()
	{
		TinkerLog("onPause()");
		super.onPause();
	}

	@Override
	protected void onPostCreate(Bundle savedInstanceState)
	{
		TinkerLog("onPostCreate()");
		super.onPostCreate(savedInstanceState);
	}

	@Override
	protected void onPostResume()
	{
		TinkerLog("onPostResume()");
		super.onPostResume();
	}

	@Override
	protected void onRestart()
	{
		TinkerLog("onRestart()");
		super.onRestart();
	}

	@Override
	protected void onRestoreInstanceState(Bundle savedInstanceState)
	{
		TinkerLog("onRestoreInstanceState()");
		super.onRestoreInstanceState(savedInstanceState);
	}

	@Override
	protected void onResume()
	{
		TinkerLog("onResume()");
		super.onResume();
	}

	@Override
	protected void onSaveInstanceState(Bundle outState)
	{
		TinkerLog("onSaveInstanceState()");
		super.onSaveInstanceState(outState);
	}

	@Override
	protected void onStart()
	{
		TinkerLog("onStart()");
		super.onStart();
	}

	@Override
	protected void onStop()
	{
		TinkerLog("onStop()");
		super.onStop();
	}

	@Override
	protected void onTitleChanged(CharSequence title, int color)
	{
		TinkerLog("onTitleChanged()");
		super.onTitleChanged(title, color);
	}

	@Override
	protected void onUserLeaveHint()
	{
		TinkerLog("onUserLeaveHint()");
		super.onUserLeaveHint();
	}
}
