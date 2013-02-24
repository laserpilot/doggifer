#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxGifEncoder.h"
#include "time.h"
#include "ofxXmlSettings.h"
#include "ofxUI.h"


class testApp : public ofBaseApp{
	public:
		void setup();
		void update();
		void draw();
		
		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y);
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
        void updateMotion( unsigned char *pixels );
        void onGifSaved(string & fileName);
        void captureGif();
        void infoOver();
        void drawMotionGraph();
        void UISetup();
        void exit();
    
    
        int camWidth, camHeight;
        ofVideoGrabber  vidGrabber;
    
        //TWITPIC AUTHENTICATION
        ofxXmlSettings keys;
        string consumerToken, consumerSecret, oAuthToken, oAuthSecret, twitpicKey;
    
        ofxCvColorImage			cameraColorImage;
        ofxCvGrayscaleImage		cameraGrayImage;
        ofxCvGrayscaleImage		cameraGrayPrevImage;
        ofxCvGrayscaleImage		cameraGrayDiffImage;
        ofxCvFloatImage			cameraDiffFloatImage;
        ofxCvFloatImage			cameraMotionFloatImage;
        float cameraMotionFadeAmount;
        int trailBlur;
    
        int TL, TR, BL, BR;
        int motionThresh;
    
        unsigned char *motionPixels;
        bool trailsOn;
        bool drawTime;
        bool valOverlay;
        bool motionGraph;
    
    
        //AVG VALUE
        int avgTL[400],avgTR[400],avgBL[400],avgBR[400];
        int avgPos;
        int curAvgTL, curAvgTR, curAvgBL, curAvgBR;
        vector <float> motionGraphTL, motionGraphTR, motionGraphBL, motionGraphBR;
        int graphYScale;
        
        //GIF MAKING
        ofImage testCapture;
        ofImage motionFrames[20];
        int frameLoop; //counting captured frames
        int gifLoop;    //incrementing/saving to circular frame buffer every .25seconds
        int capturePos; //when in gifloop did it capture?
        int gifSize; //how many frames in the gif?
        int futureCount; //count 5 from the time motion triggered
        bool saveFrame; //savesingleframe
        bool motionTrigger; //actual trigger for motion
        bool allowSave; //has there been enough time between saves?
        float lastCaptureTime; //when did it last trigger?
        float timeBetweenSaves; //how much time between saving images?
        int timeBetweenFrames; //how many frames between each gif frame?
        int savedGifCount; //how many gifs have been saved since program open?
        
           
        bool saveToServer;
        bool saveToDisk;
        bool saveGif;
        
        int frameW, frameH;
        int nFrames;
    
        string petTweet;
        string petName;
        string TLPhrase;
        string TRPhrase;
        string BLPhrase;
        string BRPhrase;
    
        bool loadBang;

        ofxGifEncoder gifEncoder;
        ofTrueTypeFont timeFont;
        ofTrueTypeFont guiFont;
        ofTrueTypeFont guiFontSmall;
    
        //Time string
        char buffer [100];
    
        ofxUICanvas *ui, *lowUI; 
        bool guiSnap;
        void guiEvent(ofxUIEventArgs &e);
        ofxUIMovingGraph *TLGraph, *TRGraph, *BLGraph, *BRGraph; 
        float *TLbuffer, *TRbuffer ,*BLbuffer ,*BRbuffer; 
        bool flipDown;
        bool flipOut;
        ofVec2f rollPos;
    
};
