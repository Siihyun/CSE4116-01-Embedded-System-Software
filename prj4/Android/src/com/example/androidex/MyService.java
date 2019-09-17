package com.example.androidex;

import android.app.Service; 
import android.content.Intent; 
import android.os.Binder;
import android.os.Handler; 
import android.os.IBinder; 
import java.util.Random;

public class MyService extends Service {
    IBinder mBinder = new MyBinder();

    class MyBinder extends Binder {
        MyService getService() { // 서비스 객체를 리턴
            return MyService.this;
        }
    }
    
    @Override
    public IBinder onBind(Intent intent) {
        // 액티비티에서 bindService() 를 실행하면 호출됨
        // 리턴한 IBinder 객체는 서비스와 클라이언트 사이의 인터페이스 정의한다
        return mBinder; // 서비스 객체를 리턴
    }
    int min,sec;
    public String getTime() { // 임의 랜덤값을 리턴하는 메서드
    	if(sec==60){
    		sec = 0;
    		min++;
    	}
    	String time = String.format("%02d",min);
    	time+=":";
    	time+= String.format("%02d",sec);
    	sec++;
        return time;
    }
    @Override
    public void onCreate() {
        super.onCreate();
        min = 0;
        sec = 0;
    }
    public int onStartCommand(Intent intent, int flags, int startId) {
        return super.onStartCommand(intent, flags, startId);
    }
    @Override
    public void onDestroy() {
        super.onDestroy();
    }
}





