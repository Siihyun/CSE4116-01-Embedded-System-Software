package com.example.androidex;

import java.util.Random;

import com.example.androidex.MyService.MyBinder;

import android.R.bool;
import android.app.Activity;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.graphics.Color;
import android.graphics.drawable.ColorDrawable;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.text.Editable;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.TextView;


public class MainActivity2 extends Activity{
	/*service*/
	MyService ms = new MyService();
	boolean isService = false;
	
    ServiceConnection conn = new ServiceConnection() {
    	public void onServiceConnected(ComponentName name,IBinder service) {
    		MyService.MyBinder mb = (MyService.MyBinder) service;
    		ms = mb.getService(); 
    		isService = true;
    		 Log.e("LOG", "onServiceConnected()");
    	}
    	public void onServiceDisconnected(ComponentName name) {
    		// 서비스와 연결이 끊겼을 때 호출되는 메서드
    		isService = false;
    		Log.e("LOG", "onServiceDisconnected()");
    	}
    };
    /*service end*/
    
    /*thread*/
    
    Handler mHandler=new Handler(){
		public void handleMessage(Message msg){
			if(msg.what==0){
				String time = ms.getTime();      	
				mBackText.setText(time);
			}
		}
	};
	class BackThread extends Thread{
		int mBackValue=0;
		Handler sHandler;
		
		BackThread(Handler handler){
			sHandler=handler;
		}
		public void run(){
			while(true){
				if(count_start == false) continue;
				mHandler.sendEmptyMessage(0);
				try{Thread.sleep(1000);}catch(InterruptedException e){;}
			}
		}
	}
	/*thread end*/
	boolean count_start = false;
	TextView mBackText;
	BackThread mThread;
    LinearLayout linear;
    EditText editText1;
    OnClickListener ltn;
    OnClickListener Button_move_listener;
    Button addbtn;
    Button [] btn = new Button[25]; // button array
    int [] random_num = new int[26]; // generating random number
    int [] random_check = new int[26]; // generating random number
    int black_button_num; // this is for deciding black button number
    int row,col;
    Context context;
    Random r = new Random();
    int count = 0;
    LinearLayout.LayoutParams params =
            new LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.MATCH_PARENT,
            LinearLayout.LayoutParams.MATCH_PARENT,
            1f
            );
   
    public boolean IsFinish(){
    	boolean flag = true;
        for(int i = 0 ; i < row*col-1 ; i++){ // check if puzzle is completed or not
        	String tmp = (String)btn[i].getText();
        	if(Integer.parseInt(tmp) != i+1) {
        		flag= false;
        		break;
        	}
        }
        return flag;
    }
    
    protected void onCreate(Bundle savedInstanceState) {
    	
        // TODO Auto-generated method stub
    	super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main2);
        linear = (LinearLayout)findViewById(R.id.container);
        editText1 = (EditText) findViewById(R.id.editText1);
        addbtn=(Button)findViewById(R.id.increase);
        mBackText=(TextView)findViewById(R.id.backvalue);
     	context = this;

        ltn=new OnClickListener(){
            public void onClick(View v){
            	Intent intent = new Intent(MainActivity2.this,  MyService.class); 
            	bindService(intent, conn, Context.BIND_AUTO_CREATE);
            	count_start = true;
            	boolean flag = true;
                String input = editText1.getText().toString();
                col = Integer.parseInt(input.split("\\s")[0]);
                row = Integer.parseInt(input.split("\\s")[1]);
                for(int i = 0 ; i < row*col ; i++){ // generate random number
                    random_num[i] = r.nextInt(row*col) + 1; // button[i] = random_num[i]
                    if(random_check[random_num[i]]==0){
                        if(random_num[i] == row * col){
                            black_button_num = i;
                        }
                        random_check[random_num[i]] = 1;
                    }
                    else   
                        i--;   
                }
                for(int i = 0 ; i < row ; i++){
                       LinearLayout buttonlayout = new LinearLayout(context); // new layout for button
                       buttonlayout.setLayoutParams(params); // set layout
                       buttonlayout.setOrientation(LinearLayout.HORIZONTAL); // set orientation
                       for(int j = 0 ; j < col ; j++){
                           btn[count] = new Button(context);
                           btn[count].setId(count); // set button id
                           btn[count].setLayoutParams(params); // set layout
                           btn[count].setText(String.valueOf(random_num[count])); // set button num
                           btn[count].setOnClickListener(Button_move_listener);
                           buttonlayout.addView(btn[count]);
                           if(count == black_button_num)
                               btn[count].setBackgroundColor(Color.BLACK);
                           count++;
                       }
                       linear.addView(buttonlayout);
                    }
            	if(IsFinish()){
            		//if(isService){
            			isService = false;
                    	unbindService(conn);
                  //  }
                    count_start = false;
                    Intent intent1=new Intent(MainActivity2.this, MainActivity.class);
        			startActivity(intent1);
            	}	
            }
        };
       
        Button_move_listener =new OnClickListener(){ // button listener for button move
            public void onClick(View v){
                Button clicked_button = (Button)v;
                int idx = clicked_button.getId(); // get clicked button's position
                if(idx == black_button_num) return; // return if black button pushed
                boolean flag = true;
                if(idx % col != 0 && idx - 1 == black_button_num){ // left check
                    String tmp = (String)btn[idx].getText();
                    btn[idx-1].setText(tmp);
                    btn[idx-1].setBackgroundResource(android.R.drawable.btn_default);
                    btn[idx].setBackgroundColor(Color.BLACK);
                    black_button_num = idx;
                }
                if(idx % col != col-1 && idx + 1 == black_button_num){ // right check
                    String tmp = (String)btn[idx].getText();
                    btn[idx+1].setText(tmp);
                    btn[idx+1].setBackgroundResource(android.R.drawable.btn_default);
                    btn[idx].setBackgroundColor(Color.BLACK);
                    black_button_num = idx;
                }
                if(idx >= col && idx - col == black_button_num){ // above check
                    String tmp = (String)btn[idx].getText();
                    btn[idx-col].setText(tmp);
                    btn[idx-col].setBackgroundResource(android.R.drawable.btn_default);
                    btn[idx].setBackgroundColor(Color.BLACK);
                    black_button_num = idx;
                }
                if(idx < row * col - col && idx + col == black_button_num){ // below check
                    String tmp = (String)btn[idx].getText();
                    btn[idx + col].setText(tmp);
                    btn[idx+col].setBackgroundResource(android.R.drawable.btn_default);
                    btn[idx].setBackgroundColor(Color.BLACK);
                    black_button_num = idx;
                }
                for(int i = 0 ; i < count-1 ; i++){ // check if puzzle is completed or not
                	String tmp = (String)btn[i].getText();
                	if(Integer.parseInt(tmp) != i+1) {
                		flag= false;
                		break;
                	}
                }
      
                if(flag){ // if finish
                	//if(isService){
                		unbindService(conn);
                		isService = false;
                	//}
                	count_start = false;
                	Intent intent=new Intent(MainActivity2.this, MainActivity.class);
    				startActivity(intent);
                }
            }
        };
        addbtn.setOnClickListener(ltn);  
        
    	mThread=new BackThread(mHandler);
		mThread.setDaemon(true);
		mThread.start();
		
    }
}
