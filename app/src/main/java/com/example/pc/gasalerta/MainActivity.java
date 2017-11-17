package com.example.pc.gasalerta;

import android.provider.Settings;
import android.support.v7.app.AlertDialog;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.text.TextUtils;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import com.google.firebase.database.ChildEventListener;
import com.google.firebase.database.DataSnapshot;
import com.google.firebase.database.DatabaseError;
import com.google.firebase.database.DatabaseReference;
import com.google.firebase.database.FirebaseDatabase;
import com.google.firebase.database.ValueEventListener;
import com.google.firebase.iid.FirebaseInstanceId;

import java.util.Map;

public class MainActivity extends AppCompatActivity {

    private DatabaseReference dRef;
    private ImageView img;
    private TextView statusText, horaText;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        dRef = FirebaseDatabase.getInstance().getReference();
        img = (ImageView) findViewById(R.id.imageView4);
        statusText = (TextView) findViewById(R.id.statusView);
        horaText = (TextView) findViewById(R.id.horaView);


        final DatabaseReference statusDR = dRef.child("MQ-6/status");
        final DatabaseReference hora = dRef.child("MQ-6/data|hora");

        hora.addValueEventListener(new ValueEventListener() {
            @Override
            public void onDataChange(DataSnapshot dataSnapshot) {
                String mens = dataSnapshot.getValue(String.class);
                horaText.setText(mens);
                horaText.setVisibility(View.VISIBLE);
            }

            @Override
            public void onCancelled(DatabaseError databaseError) {

            }
        });
        statusDR.addValueEventListener(new ValueEventListener() {
            @Override
            public void onDataChange(DataSnapshot dataSnapshot) {
                boolean mens = dataSnapshot.getValue(Boolean.class);


                if (mens == false) {
                    img.setImageResource(R.drawable.goodicons);
                    statusText.setText("Normal");


                }
                if (mens == true) {
                    img.setImageResource(R.drawable.badicons);
                    statusText.setText("Vazando Gás");
                }
            }

            @Override
            public void onCancelled(DatabaseError databaseError) {

            }
        });


    }


}