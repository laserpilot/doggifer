#include "testApp.h"
/*
* Permission is hereby granted, free of charge, to any person
* obtaining a copy of this software and associated documentation
* files (the "Software"), to deal in the Software without
* restriction, including without limitation the rights to use,
* copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following
* conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
* OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
* HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*
* ----------------------
 
 Doggifer takes motion capture animated gif portraits of your pet or home and posts them to a specified twitter account. 
 
 This code is just a silly personal project, so some areas are not commented well or may not make complete sense...proceed with caution!
 
 Thanks so much to Reza Ali for ofxUI and Jesus Gollonet and Nick Hardeman for ofxGifEncoder!

*/

//--------------------------------------------------------------
void testApp::setup(){
    

    
    //Camera settings
    camWidth=480;
    camHeight = 360;
   // testCapture.allocate(camWidth, camHeight, GL_RGB);
    //vidGrabber.setDeviceID(3); //dont set this explicitly
    vidGrabber.initGrabber(camWidth, camHeight);
    
    //CV motion settings
	cameraColorImage.allocate( camWidth, camHeight );
	cameraGrayImage.allocate( camWidth, camHeight );
	cameraGrayPrevImage.allocate( camWidth, camHeight );
	cameraGrayDiffImage.allocate( camWidth, camHeight );
	cameraDiffFloatImage.allocate( camWidth, camHeight );
	cameraMotionFloatImage.allocate( camWidth, camHeight );
	
	cameraMotionFadeAmount = 0.95f;
    avgPos = curAvgTL =curAvgTR = curAvgBL = curAvgBR = TL = BL = TR = BR = 0;
    
    timeFont.loadFont("fonts/BellGothicStd-Light.otf", 13);
    guiFont.loadFont("fonts/MorSCOR_.ttf", 14);
    guiFontSmall.loadFont("fonts/MorSCOR_.ttf", 9);
    
    ofFilePath filepath;
    cout<<"_____FILEPATH: "+ filepath.getCurrentWorkingDirectory()<<endl;
    
    //GIF STUFF
    nFrames = 0;
    gifEncoder.setup(camWidth, camHeight, .25, 256);
    ofAddListener(ofxGifEncoder::OFX_GIF_SAVE_FINISHED, this, &testApp::onGifSaved);

    frameLoop = 0;
    gifSize = 10; //maximum number of frames in gif
    lastCaptureTime = 0;
    gifLoop = 0;
    timeBetweenFrames = 15; //measured in frames
    futureCount = 0;
    savedGifCount = 0;

    timeBetweenSaves = 300; //300 seconds between saves
    motionThresh = 30;
    
    saveToServer = saveGif = allowSave = saveFrame = motionTrigger = false;
    petTweet = "Your pet moved!";

    
    trailBlur = 3;
    flipDown = false;
    rollPos = ofVec2f(1280,360);
    petName = "Your Pet ";
    //TLPhrase = " moved near the back room. ";
    //TRPhrase = " shifted on the couch. ";
    //BLPhrase = " rummaged in her toy bin. ";
    //BRPhrase = " wandered near her food. ";
    
    loadBang = false; //necessary because system dialogs were getting triggered on load
    UISetup();
    loadBang  = true;
    
    cout <<TLPhrase<<TRPhrase<<BLPhrase<<BRPhrase<<endl;
    
    saveToDisk = true;
    trailsOn = true;
    drawTime = true;
    valOverlay = true;
    motionGraph = true;
    
    motionGraphTL.assign(400, 0.0);
    motionGraphTR.assign(400, 0.0);
    motionGraphBL.assign(400, 0.0);
    motionGraphBR.assign(400, 0.0);
    
    graphYScale = 100; //highest point on the scale
    
    
}

//--------------------------------------------------------------
void testApp::update(){
    
    bool bNewFrame = false;
    
    vidGrabber.grabFrame();
    bNewFrame = vidGrabber.isFrameNew();
    
    if( bNewFrame )
	{
		updateMotion( vidGrabber.getPixels() );
        frameLoop++; //increment counter
        if (frameLoop > timeBetweenFrames){ //every 15 frames, save another frame
            frameLoop = 0; //reset
            
            if (gifLoop>=9) {
                gifLoop = -1; //if all frames have been saved, loop back to 0. Set to -1 here since it increments on next step
            }
            gifLoop++;
            futureCount++; 
            //cout << "Future Count: " << futureCount <<endl;
            saveFrame = true; //tells draw to save the current visible frame to the gif buffer
        }
	}
    
    //Time Formatting
    time_t rawtime;
    struct tm * timeinfo;
    
    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    
    //ref: http://www.cplusplus.com/reference/clibrary/ctime/strftime/
    strftime (buffer,100,"%a %b %d %I:%M:%S%p",timeinfo); 
    
    //This makes sure it doesn't save too amny gifs too often
    if (lastCaptureTime < ofGetElapsedTimef()-timeBetweenSaves) { //if the last capture was longer than the current time minus the time between saves
        allowSave = true;
    }
    else {
        allowSave = false;
    }
    
    //Just for GUI revealing/hiding
    if (flipDown) {
        rollPos.y+=25;
        rollPos.y = ofClamp(rollPos.y, 360, 600);
    }
    else{
        rollPos.y-=25;
        rollPos.y = ofClamp(rollPos.y, 360, 600);
    }
    
    if (flipOut){
        rollPos.x-=40;
        rollPos.x = ofClamp(rollPos.x, 480, 1280);
    }
    else{
        rollPos.x+=40;
        rollPos.x = ofClamp(rollPos.x, 480, 1280);
    }
    
    ofSetWindowShape(rollPos.x, rollPos.y);

         
    
}
//--------------------------------------------------------------
void testApp :: updateMotion( unsigned char *pixels )
{
	cameraColorImage.setFromPixels( pixels, camWidth, camHeight );
	
	cameraGrayPrevImage	= cameraGrayImage;
	cameraGrayImage		= cameraColorImage;
	
	cameraGrayDiffImage.absDiff( cameraGrayImage, cameraGrayPrevImage );
	cameraGrayDiffImage.threshold( 30 );
	
    if(trailsOn){ //draw motion trails
        cameraDiffFloatImage	= cameraGrayDiffImage;
        cameraMotionFloatImage	*= cameraMotionFadeAmount;
        cameraMotionFloatImage	+= cameraDiffFloatImage;
        cameraMotionFloatImage.blurGaussian( trailBlur );
    }
    else{
        
    }
    TL = cameraGrayDiffImage.countNonZeroInRegion(0, 0, 240, 180);
    TR = cameraGrayDiffImage.countNonZeroInRegion(240, 0, 240, 180);
    BL = cameraGrayDiffImage.countNonZeroInRegion(0, 180, 240, 180);
    BR = cameraGrayDiffImage.countNonZeroInRegion(240, 180, 240, 180);
    
    
    //I don't think I'm averaging these correctly, or not across enough values, but seems to do the job
    int avgBuffer = 100;
    avgTL[avgPos%avgBuffer] = TL;
    avgTR[avgPos%avgBuffer] = TR;
    avgBL[avgPos%avgBuffer] = BL;
    avgBR[avgPos%avgBuffer] = BR;
    avgPos++;
    if (avgPos>avgBuffer) {
        for (int i =0; i<avgBuffer; i++) {
            curAvgTL = avgTL[i] + curAvgTL;
        }
        for (int i =0; i<avgBuffer; i++) {
            curAvgTR = avgTR[i] + curAvgTR;
        }
        for (int i =0; i<avgBuffer; i++) {
            curAvgBL = avgBL[i] + curAvgBL;
        }
        for (int i =0; i<avgBuffer; i++) {
            curAvgBR = avgBR[i] + curAvgBR;
        }
        curAvgTL = curAvgTL/avgBuffer;
        curAvgTR = curAvgTR/avgBuffer;
        curAvgBL = curAvgBL/avgBuffer;
        curAvgBR = curAvgBR/avgBuffer;
    }
    
    //GUI drawing
    TLGraph->addPoint(TL);
    TRGraph->addPoint(TR);
    BLGraph->addPoint(BL);
    BRGraph->addPoint(BR);
    
    float scaledTL = ofMap(TL, 0, graphYScale, 0.0, 1.0);
    float scaledTR = ofMap(TR, 0, graphYScale, 0.0, 1.0);
    float scaledBL = ofMap(BL, 0, graphYScale, 0.0, 1.0);
    float scaledBR = ofMap(BR, 0, graphYScale, 0.0, 1.0);
    motionGraphTL.push_back(scaledTL);
    motionGraphTR.push_back(scaledTR);
    motionGraphBL.push_back(scaledBL);
    motionGraphBR.push_back(scaledBR);
    //if we are bigger the the size we want to record - lets drop the oldest value -- from ALL of them
	if( motionGraphTL.size() >= 400 ){
		motionGraphTL.erase(motionGraphTL.begin(), motionGraphTL.begin()+1);
        motionGraphTR.erase(motionGraphTR.begin(), motionGraphTR.begin()+1);
        motionGraphBL.erase(motionGraphBL.begin(), motionGraphBL.begin()+1);
        motionGraphBR.erase(motionGraphBR.begin(), motionGraphBR.begin()+1);
	}
    
}

//--------------------------------------------------------------
void testApp::draw(){
    ofBackground(0); //clear the buffer!
    
    
    if(trailsOn){ //draw trails
        ofEnableBlendMode(OF_BLENDMODE_ADD);
        ofSetColor(255, 255, 255, 200);
        cameraMotionFloatImage.draw(0,0, camWidth, camHeight);
    }
    
    ofSetColor(255, 255, 255,255);
    vidGrabber.draw(0, 0, camWidth, camHeight); //Draw camera feed
    ofDisableBlendMode();
    
    //Draw the Time
    if(drawTime){
        ofEnableAlphaBlending();
        ofSetColor(0, 127, 255,127);
        ofRect(235, 310, 245, 30);
        ofSetColor(67);
        timeFont.drawString(ofToString(buffer), 240,330); //draw time
        ofSetColor(255);
        timeFont.drawString(ofToString(buffer), 240,331); //draw time shadow
        ofDisableAlphaBlending();
    }
    
    if(valOverlay){
        guiFontSmall.drawString("TL: " + ofToString(TL), 20, 20);
        guiFontSmall.drawString("TR: " + ofToString(TR), 430, 20);
        guiFontSmall.drawString("BR: " + ofToString(BR), 430, 350);
        guiFontSmall.drawString("BL: " + ofToString(BL), 20, 350);
    }
    
    if(motionGraph){
        ofPushMatrix();
        ofScale(.5, .5);
        ofTranslate(0, 70);
        drawMotionGraph();
        ofPopMatrix();
    }
    
    ///
    ///Everything above this gets saved to the gif----------------------
    ///
    
    if (saveFrame) {
        motionFrames[gifLoop%gifSize].grabScreen(0, 0, camWidth, camHeight); //frames are continuously being saved/overwritten so that a time warp can occur for the triggered gif...so the shown gif is actually a couple seconds before when it was triggered
        saveFrame = false; //gets reset in update
    }
    
    
    if ((TL>curAvgTL+motionThresh || TR>curAvgTR+motionThresh || BR>curAvgBR+motionThresh || BL>curAvgBL+motionThresh) && allowSave) { //if a threshold is crossed and it is time to save a new gif, then create the new pet tweet to be used
        if(TL>curAvgTL+motionThresh){
            petTweet = petName+" " + TLPhrase +" "+"Motion Amount:" + ofToString(TL);
        }
        else if(TR>curAvgTR+motionThresh){
            petTweet = petName+" " + TRPhrase +" "+"Motion Amount:"+ofToString(TR);
        }
        else if(BR>curAvgBR+motionThresh){
            petTweet = petName+" " + BRPhrase +" "+"Motion Amount:"+ofToString(BR);
        }
        else if(BL>curAvgBL+motionThresh){
            petTweet = petName+" " + BLPhrase +" "+"Motion Amount:"+ofToString(BL);
        }
        cout <<"-----------------Motion Trigger!---------------------" <<endl;
        cout <<petTweet<<endl;
        motionTrigger = true;
        lastCaptureTime = ofGetElapsedTimef();
        capturePos = gifLoop;
        allowSave = false;
        futureCount = 0;
    }
    
    //now only save when it has been 5 frames after when it was triggered to allow for time warp
    if (futureCount>5 && motionTrigger) { //future count keeps track
        cout <<"Starting actual gif save" <<endl;
        captureGif();
        allowSave = false;
        motionTrigger = false;
    }
    
    //Overlay
    infoOver();
 
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
    switch (key) {
        case 'g':
            saveGif = !saveGif;
            break;
        case 'r':
            lastCaptureTime = ofGetElapsedTimef(); //reset time so it doesn't capture
            break;
        case ' ':
            allowSave = true;
            break;
        case '[':
            flipDown = !flipDown;
            break;
        case ']':
            flipOut = !flipOut;
            break;
        default:
            break;
    }
}
//--------------------------------------------------------------
void testApp::onGifSaved(string &fileName) {
    cout << "Gif saved as " << fileName << endl;
    
    if(saveToServer){
        char curlCommand[2500];
        ofFilePath filepath;
        //consumer token = consumer key on oauthtool on twitter
        sprintf(curlCommand, "curl -v -F \"consumer_token=%s\" -F \"consumer_secret=%s\" -F \"oauth_token=%s\" -F \"oauth_secret=%s\" -F \"message=%s\" -F \"key=%s\" -F \"media=@%s\" http://api.twitpic.com/1/uploadAndPost.json", consumerToken.c_str(), consumerSecret.c_str(), oAuthToken.c_str(), oAuthSecret.c_str(), petTweet.c_str(), twitpicKey.c_str(), filepath.getAbsolutePath(fileName).c_str()); 
        cout<< "Curl command: " << curlCommand<< endl;
        system(curlCommand); //run the command
    }
    
    //Clear frames after tweeting
    cout << "Clearing frames" << endl;
    gifEncoder.reset();
    savedGifCount++;
}

//--------------------------------------------------------------
void testApp::captureGif() {
    cout << "GifSize: " << gifSize << endl;
    
    if(saveToDisk){
        for (int i = 0; i<gifSize; i++) {
            gifEncoder.addFrame(motionFrames[i].getPixels(), camWidth, camHeight, 24, .15f);
        }
        cout <<"Gif compiled. Now saving\n" << endl;
        gifEncoder.save(ofToString(ofGetSystemTime())+".gif");
    }

    
    
}
//--------------------------------------------------------------
void testApp::infoOver() {
    
    ofEnableAlphaBlending();
    ofSetColor(100, 100, 255, 130);
    //Draw crosshairs
    ofLine(240, 0, 240, 360);
    ofLine(0, 180, 480, 180);

    int radius;
    ofEnableSmoothing();
    if (TL>curAvgTL+motionThresh) {
        ofSetColor(0*255.0, 0.678431*255.0, 0.937255*255.0, 0.696078*255.0);
        radius = ofMap(TL, 0, 3000, 0, 20, true);
        ofCircle(220,160,radius-3);
        ofCircle(220,160,radius);
    }
    if (TR>curAvgTR+motionThresh) {
        ofSetColor(0*255.0, 0.678431*255.0, 0.937255*255.0, 0.696078*255.0);
        radius = ofMap(TR, 0, 3000, 0, 20,true);
        ofCircle(260,160,radius-3);
        ofCircle(260,160,radius);
    }
    if (BL>curAvgBL+motionThresh) {
        ofSetColor(0*255.0, 0.678431*255.0, 0.937255*255.0, 0.696078*255.0);
        radius = ofMap(BL, 0, 3000, 0, 20, true);
        ofCircle(220,200,radius-3);
        ofCircle(220,200,radius);
    }
    if (BR>curAvgBR+motionThresh) {
        ofSetColor(0*255.0, 0.678431*255.0, 0.937255*255.0, 0.696078*255.0);
        radius = ofMap(BR, 0, 3000, 0, 20, true);
        ofCircle(260,200,radius-3);
        ofCircle(260,200,radius);
    }
    ofDisableSmoothing();
    ofPushMatrix();
    ofTranslate(-130, 190);
    if (saveToServer) {
        ofSetColor(255, 255, 255,100);
        ofRect(950, 10, 200, 25);
        ofSetColor(0, 255, 0);
        guiFont.drawString("Post to Twitter: YES", 960,30);
    }
    else {
        ofSetColor(255, 0, 0);
        guiFont.drawString("Post to Twitter: NO", 960,30);
    }
    
    ofSetColor(0, 150, 255);
    
    guiFont.drawString("Time 'til next save: " + ofToString(lastCaptureTime - (ofGetElapsedTimef()-timeBetweenSaves)) , 960,55);
    guiFont.drawString("Time Between Saves: " + ofToString(timeBetweenSaves/60) + "min" , 960,80);
    guiFont.drawString("Number of gifs saved: " + ofToString(savedGifCount) , 960,105);
    guiFont.drawString("Motion Threshold: " + ofToString(motionThresh) , 960,130);
    guiFont.drawString("Last Tweet: " + petTweet , 960,155);
    ofPopMatrix();
    
    //drawMotionGraph();
    
}
//--------------------------------------------------------------
void testApp::drawMotionGraph(){
    
    //Top Left
    ofSetColor(0, 150, 255);
    //lets draw the volume history as a graph
    guiFont.drawString("TL", 20, 40);
    ofPushMatrix();
    //ofTranslate(-30, 30);
    ofScale(0.5,0.2);
    ofBeginShape();
    for (int i = 0; i < motionGraphTL.size(); i++){
        if( i == 0 ) ofVertex(i, 200);
        
        ofVertex(i, 200 - motionGraphTL[i] * 30);
        
        if( i == motionGraphTL.size() -1 ) ofVertex(i, 200);
    }
    ofEndShape(false);
	ofSetColor(255);
    ofLine(0, ofMap(motionThresh, 0, graphYScale, 200, 0,true), 400, ofMap(motionThresh, 0, graphYScale, 200, 0,true));
    ofPopMatrix();
    
    //Top Right
    ofSetColor(0, 150, 255);
    guiFont.drawString("TR", 20, 80);
    ofPushMatrix();
    ofTranslate(0, 60);
    ofScale(0.5,0.2);
    ofBeginShape();
    for (int i = 0; i < motionGraphTR.size(); i++){
        if( i == 0 ) ofVertex(i, 200);
        
        ofVertex(i, 200 - motionGraphTR[i] * 30);
        
        if( i == motionGraphTR.size() -1 ) ofVertex(i, 200);
    }
    ofEndShape(false);
	ofSetColor(255);
    ofLine(0, ofMap(motionThresh, 0, graphYScale, 200, 0,true), 400, ofMap(motionThresh, 0, graphYScale, 200, 0,true));
    ofPopMatrix();
    
    //Bottom Left
    ofSetColor(0, 150, 255);
     guiFont.drawString("BL", 20, 130);
    ofPushMatrix();
    ofTranslate(0, 120);
    ofScale(0.5,0.2);
    ofBeginShape();
    for (int i = 0; i < motionGraphBL.size(); i++){
        if( i == 0 ) ofVertex(i, 200);
        
        ofVertex(i, 200 - motionGraphBL[i] * 30);
        
        if( i == motionGraphBL.size() -1 ) ofVertex(i, 200);
    }
    ofEndShape(false);
	ofSetColor(255);
    ofLine(0, ofMap(motionThresh, 0, graphYScale, 200, 0,true), 400, ofMap(motionThresh, 0, graphYScale, 200, 0,true));
    ofPopMatrix();
    
    //Bottom Right
    ofSetColor(0, 150, 255);
    guiFont.drawString("BR", 20, 200);
    ofPushMatrix();
    ofTranslate(0, 180);
    ofScale(0.5,0.2);
    ofBeginShape();
    for (int i = 0; i < motionGraphBR.size(); i++){
        if( i == 0 ) ofVertex(i, 200);
        
        ofVertex(i, 200 - motionGraphBR[i] * 30);
        
        if( i == motionGraphBR.size() -1 ) ofVertex(i, 200);
    }
    ofEndShape(false);
	ofSetColor(255);
    ofLine(0, ofMap(motionThresh, 0, graphYScale, 200, 0,true ), 400, ofMap(motionThresh, 0, graphYScale, 200, 0,true));
    ofPopMatrix();
    
}
//--------------------------------------------------------------
void testApp::UISetup(){
    
    
    ofSetColor(255, 255, 255);
    ui = new ofxUICanvas(480,0,800,360);
    guiSnap = false;
    
    ui->setTheme(OFX_UI_THEME_MACOSX);
    ui->setPadding(20);
    
    float length = 300;
    float height = 16;
    ui->loadSettings("GUI/guiSettings.xml"); 
    ofPoint xRange = ofVec2f(0,1920) ;
    ofPoint yRange = ofVec2f(0,1080);
    ofPoint xScaleRange = ofVec2f(0,2.0) ;
    ofPoint yScaleRange = ofVec2f(0,2.0);
    ui->addWidgetDown(new ofxUILabel("Doggifer", OFX_UI_FONT_LARGE)); 
    ui->addWidgetDown(new ofxUIToggle(20, 20, false, "Save To Twitter"));
    ui->addWidgetRight(new ofxUIToggle(20, 20, true, "Save To File"));
    ui->addWidgetDown(new ofxUIToggle(20, 20, true, "Trails"));
    ui->addWidgetRight(new ofxUIToggle(20, 20, true, "Show Clock"));
    ui->addWidgetRight(new ofxUIToggle(20, 20, true, "Show Graph"));
    ui->addWidgetRight(new ofxUIToggle(20, 20, true, "Show Values"));
    ui->addWidgetRight(new ofxUILabel( "Press [ and ] to show/hide menus", OFX_UI_FONT_SMALL));
    ui->addSlider("Time Between Saves",0.0,2000.0,300.0,length,height);
    ui->addWidgetRight( new ofxUISlider("Motion Threshold",3.0,2000.0,30.0,length,height));
    ui->addWidgetDown(new ofxUISlider("Trails Amount",0.0,1.0,.95,length,height));
    ui->addWidgetRight(new ofxUISlider("Blur Amount",0.0,10.0,3.0,length,height));
    ui->addTextInput("Pet Name", "Pet", 300);
    
    vector<float> buffer; //This is just a buffer for initialization
    for(int i = 0; i < 256; i++)
    {
        buffer.push_back(0.0);
    }
    
    ui->addWidgetDown(new ofxUILabel("Motion Graph", OFX_UI_FONT_SMALL)); 		
    TLGraph = (ofxUIMovingGraph *) ui->addWidgetDown(new ofxUIMovingGraph(150, 40, buffer, 256, 0, 3000, "Top Left Motion"));
    
    TRGraph = (ofxUIMovingGraph *) ui->addWidgetRight(new ofxUIMovingGraph(150, 40, buffer, 256, 0, 3000, "Top Right Motion"));
    BLGraph = (ofxUIMovingGraph *) ui->addWidgetDown(new ofxUIMovingGraph(150, 40, buffer, 256, 0, 3000, "Bottom Left Motion"));
    BRGraph = (ofxUIMovingGraph *) ui->addWidgetRight(new ofxUIMovingGraph(150, 40, buffer, 256, 0, 3000, "Bottom Right Motion"));
    ofAddListener(ui->newGUIEvent, this, &testApp::guiEvent); 
    ui->loadSettings("GUI/guiSettings.xml"); 
    
    
      //lowUI->loadSettings("GUI/keys_and_phrases.xml"); 
    //Use this GUI for less commonly adjusted settings
    lowUI = new ofxUICanvas(0,360,1280,360);
    lowUI->setTheme(OFX_UI_THEME_RUSTICORANGE);
    lowUI-> setPadding(50);
    lowUI->loadSettings("GUI/keys_and_phrases.xml"); 
    cout<<"loading UI settings" <<endl;
    
    lowUI->addWidgetDown(new ofxUILabel("Consumer Secret", OFX_UI_FONT_SMALL)); 
        lowUI->addWidgetRight(new ofxUITextInput("Consumer Secret", consumerSecret, 500));
        lowUI->addWidgetRight(new ofxUILabel("Top Left Phrase", OFX_UI_FONT_SMALL)); 
        lowUI->addWidgetRight(new ofxUITextInput("TLPhrase", TLPhrase, 500));
    
    lowUI->addWidgetDown(new ofxUILabel("Consumer Token", OFX_UI_FONT_SMALL)); 
        lowUI->addWidgetRight(new ofxUITextInput("Consumer Token", consumerToken, 500));
        lowUI->addWidgetRight(new ofxUILabel("Top Right Phrase", OFX_UI_FONT_SMALL)); 
        lowUI->addWidgetRight(new ofxUITextInput("TRPhrase", TRPhrase, 500));
    
    lowUI->addWidgetDown(new ofxUILabel("oAuth Token", OFX_UI_FONT_SMALL)); 
        lowUI->addWidgetRight(new ofxUITextInput("oAuth Token", oAuthToken, 500));
        lowUI->addWidgetRight(new ofxUILabel("Bottom Left Phrase", OFX_UI_FONT_SMALL)); 
        lowUI->addWidgetRight(new ofxUITextInput("BLPhrase", BLPhrase, 500));

    lowUI->addWidgetDown(new ofxUILabel("oAuth Secret", OFX_UI_FONT_SMALL)); 
        lowUI->addWidgetRight(new ofxUITextInput("oAuth Secret", oAuthSecret, 500));
        lowUI->addWidgetRight(new ofxUILabel("Bottom Right Phrase", OFX_UI_FONT_SMALL)); 
        lowUI->addWidgetRight(new ofxUITextInput("BRPhrase", BRPhrase, 500));
    
    lowUI->addWidgetDown(new ofxUILabel("Twit Pic Key", OFX_UI_FONT_SMALL)); 
        lowUI->addWidgetRight(new ofxUITextInput("Twit Pic", twitpicKey, 500));
    
    lowUI->addWidgetRight(new ofxUILabel("Doggifer by Blair Neal. 2013.", OFX_UI_FONT_SMALL)); 
    lowUI->addWidgetRight(new ofxUILabel("www.blairneal.com", OFX_UI_FONT_SMALL));
    
    ofAddListener(lowUI->newGUIEvent, this, &testApp::guiEvent); 
    lowUI->loadSettings("GUI/keys_and_phrases.xml"); 
}

//--------------------------------------------------------------
void testApp::guiEvent(ofxUIEventArgs &e)
{
    string name = e.widget->getName(); 
	int kind = e.widget->getKind(); 
	
    
    if(e.widget->getName() == "Trails Amount")
    {
        ofxUISlider *slider = (ofxUISlider *) e.widget;    
        //farThreshold = slider->getScaledValue();
        cameraMotionFadeAmount = slider->getScaledValue();
    }
    else if(name == "Blur Amount")
	{
		ofxUISlider *slider = (ofxUISlider *) e.widget; 
		//nearThreshold = slider->getScaledValue(); 
        trailBlur = slider->getScaledValue();
	}
    else if(name == "Time Between Saves")
	{
		ofxUISlider *slider = (ofxUISlider *) e.widget; 
        timeBetweenSaves = slider->getScaledValue();
	}
    else if(name == "Motion Threshold")
	{
		ofxUISlider *slider = (ofxUISlider *) e.widget; 
        motionThresh = slider->getScaledValue();
	}
    else if(name == "Save To Twitter")
	{
        ofxUIToggle *toggle = (ofxUIToggle *) e.widget;
        saveToServer = toggle->getValue();  
	}
    else if(name == "Save To File")
	{
        ofxUIToggle *toggle = (ofxUIToggle *) e.widget;
        saveToDisk = toggle->getValue();  
	}
    else if(name == "Trails")
	{
		ofxUIToggle *toggle = (ofxUIToggle *) e.widget; 
		//nearThreshold = slider->getScaledValue(); 
        trailsOn = toggle->getValue();
	}
    else if(name == "Show Clock")
	{
		ofxUIToggle *toggle = (ofxUIToggle *) e.widget;
        drawTime = toggle->getValue();
	}
    else if(name == "Show Graph")
	{
		ofxUIToggle *toggle = (ofxUIToggle *) e.widget;
        motionGraph = toggle->getValue();
	}
    else if(name == "Show Values")
	{
		ofxUIToggle *toggle = (ofxUIToggle *) e.widget;
        valOverlay = toggle->getValue();
	}
    else if(name == "Pet Name")
	{
        ofxUITextInput *textInput = (ofxUITextInput *) e.widget;
        if(textInput->getTriggerType() == OFX_UI_TEXTINPUT_ON_FOCUS && loadBang){
            cout << "ON FOCUS: "; 
            petName = ofSystemTextBoxDialog("Pet Name", petName);
            textInput->setTextString(petName);
            cout<<"Pet name: " + petName<<endl;
        }
       
	}
    else if(name == "TLPhrase")
	{
        ofxUITextInput *textInput = (ofxUITextInput *) e.widget;
        TLPhrase = textInput->getTextString();
        if(textInput->getTriggerType() == OFX_UI_TEXTINPUT_ON_FOCUS && loadBang){
            TLPhrase = ofSystemTextBoxDialog("Phrase for Top Left", TLPhrase);
            textInput->setTextString(TLPhrase);
            cout<<"TL Phrase: " + TLPhrase<<endl;
        }

	}
    else if(name == "TRPhrase")
	{
        ofxUITextInput *textInput = (ofxUITextInput *) e.widget;
        TRPhrase = textInput->getTextString();
        if(textInput->getTriggerType() == OFX_UI_TEXTINPUT_ON_FOCUS && loadBang){
            TRPhrase = ofSystemTextBoxDialog("Phrase for Top Right", TRPhrase);
            cout<<"loadbang:" <<loadBang<<endl;
            textInput->setTextString(TRPhrase);
            cout<<"TR Phrase: " + TRPhrase<<endl;
        }
	}
    else if(name == "BLPhrase")
	{
        ofxUITextInput *textInput = (ofxUITextInput *) e.widget; 
        BLPhrase = textInput->getTextString();
        if(textInput->getTriggerType() == OFX_UI_TEXTINPUT_ON_FOCUS && loadBang){
            BLPhrase = ofSystemTextBoxDialog("Phrase for Bottom Left", BLPhrase);
            textInput->setTextString(BLPhrase);
            cout<<"BL Phrase: " + BLPhrase<<endl;
        }
	}
    else if(name == "BRPhrase")
	{
        ofxUITextInput *textInput = (ofxUITextInput *) e.widget;
        BRPhrase = textInput->getTextString();
        if(textInput->getTriggerType() == OFX_UI_TEXTINPUT_ON_FOCUS && loadBang){
            BRPhrase = ofSystemTextBoxDialog("Phrase for Bottom Right", BRPhrase);
            textInput->setTextString(BRPhrase);
            cout<<"BR Phrase: " + BRPhrase<<endl;
        }
	}
    else if(name == "Consumer Secret")
	{
        ofxUITextInput *textInput = (ofxUITextInput *) e.widget;
        consumerSecret = textInput->getTextString();
        if(textInput->getTriggerType() == OFX_UI_TEXTINPUT_ON_FOCUS && loadBang){
            consumerSecret = ofSystemTextBoxDialog("Consumer Secret", consumerSecret);
            textInput->setTextString(consumerSecret);
            cout<<"Consumer Secret: " + consumerSecret<<endl;
        }
	}
    else if(name == "Consumer Token")
	{
        ofxUITextInput *textInput = (ofxUITextInput *) e.widget;
        consumerToken = textInput->getTextString();
        if(textInput->getTriggerType() == OFX_UI_TEXTINPUT_ON_FOCUS && loadBang){
            consumerToken = ofSystemTextBoxDialog("Consumer Token", consumerToken);
            textInput->setTextString(consumerToken);
            cout<<"Consumer Token: " + consumerToken<<endl;
        }
	}
    else if(name == "oAuth Token")
	{
        ofxUITextInput *textInput = (ofxUITextInput *) e.widget;
        oAuthToken = textInput->getTextString();
        if(textInput->getTriggerType() == OFX_UI_TEXTINPUT_ON_FOCUS && loadBang){
            oAuthToken = ofSystemTextBoxDialog("oAuth Token", oAuthToken);
            textInput->setTextString(oAuthToken);
            cout<<"oAuth Token: " + oAuthToken<<endl;
        }
	}
    else if(name == "oAuth Secret")
	{
        ofxUITextInput *textInput = (ofxUITextInput *) e.widget;
        oAuthSecret = textInput->getTextString();
        if(textInput->getTriggerType() == OFX_UI_TEXTINPUT_ON_FOCUS && loadBang){
            oAuthSecret = ofSystemTextBoxDialog("oAuth Secret", oAuthSecret);
            textInput->setTextString(oAuthSecret);
            cout<<"oAuth Secret: " + oAuthSecret<<endl;
        }
	}
    else if(name == "Twit Pic")
	{
        ofxUITextInput *textInput = (ofxUITextInput *) e.widget;
        twitpicKey = textInput->getTextString();
        if(textInput->getTriggerType() == OFX_UI_TEXTINPUT_ON_FOCUS && loadBang){
            twitpicKey = ofSystemTextBoxDialog("Twit Pic Key", twitpicKey);
            textInput->setTextString(twitpicKey);
            cout<<"Twit Pic Key: " + twitpicKey<<endl;
        }
	}
    
    
    
    
}
//--------------------------------------------------------------
void testApp::exit() {

    gifEncoder.exit();
    vidGrabber.close();
    cout<< "Saving settings and exiting Doggifer" <<endl;
    ui->saveSettings("GUI/guiSettings.xml"); 
    lowUI->saveSettings("GUI/keys_and_phrases.xml"); 
    delete ui; 
    delete lowUI;
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){
        
}
