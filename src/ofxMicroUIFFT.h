/*
ofxMicroUIFFT
*/

#pragma once
#include "ofMain.h"
#include "ofEvents.h"
#include "ofxMicroUI.h"

#include "fft.h"
#include "FFTOctaveAnalyzer.h"

class ofxMicroUIFFT : public ofBaseApp {
public:

	#include "DmtrFFT0.h"
	
	bool cadaFrequencia[NPEAKS];
	float equalizador[NPEAKS];

	int nBands = NPEAKS;
	int larguraEq = 13;
	
	float updownTemp;
	float numeroFrequencias;
	//ofImage eq;
	float updown = 0;
	float updownEasy = 0;

	bool invertAudio = false;

	int sliderWidth, sliderHeight;
	bool debug = false;

	// mudar um pouco isso aqui
	bool output = false;
	bool useSoundPlayer = false;

	bool calculate = true;
	bool useLeft = true;
	float balance = 0.0;
	
	//--------------------------------------------------------------
	void setupFFT() {
		cout << "setup FFT " << endl;
		ofxMicroUI::slider2d * slider = ((ofxMicroUI::slider2d*)uiFFT->getElement("freq"));
		slider->alwaysRedraw = true;
		fboFFT = &slider->fbo;

		auto devices = soundStream.getDeviceList();
		for (auto & d : devices) {
			if (d.isDefaultInput) {
				ofxMicroUI::messageBox(d.name);
				settings.setInDevice(d);
			}
		}
		
		//soundStream.printDeviceList();
		settings.setInListener(this);
		settings.sampleRate = 44100;
		settings.numOutputChannels = 0;
		settings.numInputChannels = 1;
		settings.bufferSize = FFT_BUFFERSIZE;
		soundStream.setup(settings);
		
		fft1.setup();
		fft2.setup();
	}


	//--------------------------------------------------------------
	void setup() {
		cout << "ofxDmtrFft		::: Setup" << endl;
		ofAddListener(ofEvents().draw, this, &ofxMicroUIFFT::onDraw);
		if (calculate) {
			ofAddListener(ofEvents().update, this, &ofxMicroUIFFT::onUpdate);
		}
		ofAddListener(ofEvents().exit, this, &ofxMicroUIFFT::onExit);
		updateFreqs();
		setupFFT();
	}

	
	void onDraw(ofEventArgs &data) {
//		draw();
		
	}
	
	
	void drawDebug() {
		ofPushMatrix();
//		ofTranslate(200,200);
		int width = 500;
		for (int i = 0; i< NPEAKS; i++) {
			float altura = fft1.peaks[i] * 3.0;
			float x = ofMap(i,0,NPEAKS,0,width);
			ofSetColor(255,0,0);
			ofDrawRectangle(x,600-altura,width/NPEAKS-1,altura);
		}
		ofPopMatrix();
	}

	void onUpdate(ofEventArgs &data) {
		//update();
		updateFFT();
		fft1.update();
		fft2.update();
	}
	
//	void update() { }
	
	void onExit(ofEventArgs &data) {  ofSoundStreamClose();  }

	void setUI(ofxMicroUI & uiFft) {
		cout << "FFT setUI" << endl;
//		ui = &uiFft;
		uiFFT = &uiFft;
		fboFFT = &((ofxMicroUI::slider2d*)uiFFT->getElement("freq"))->fbo;
		ofxMicroUI::element * e = uiFFT->getElement("freq");
		sliderWidth = uiFFT->getElement("freq")->rect.width;
		sliderHeight = uiFFT->getElement("freq")->rect.height;
		
	//	cout << ui->UINAME << endl;
	//	cout << "ofxDmtrFft setUI " << sliderWidth << ":" << sliderHeight << endl;

		larguraEq = int(sliderWidth / (nBands+2));
		
		debug = true;
		if (debug) {
			cout << "------- ofxDmtrFft setUI" << endl;
			cout << uiFFT->uiName << endl;
			cout << sliderWidth << endl;
			cout << larguraEq << endl;
			cout << "-----" << endl;
		}

//		fboFFT.allocate(sliderWidth, sliderHeight, GL_RGBA);
//		fboFFT.begin();
//		ofClear(0,255);
//		fboFFT.end();
		
		ofAddListener(uiFft.uiEvent,this, &ofxMicroUIFFT::fftUIEvent);
		updateFreqs();
	}

//	void setUIMaster(ofxMicroUI & u) {
//		uiMaster = &u;
//	}

	void updateFFT() {
//		cout << "updateFFT" << endl;
		fboFFT->begin();
		ofClear(0,255);
		
		ofFill();
		updownTemp = 0;

		for (int i = 0; i < NPEAKS; i++) {
			equalizador[i] = fft1.peaks[i];
			if (cadaFrequencia[i]) {
				updownTemp += equalizador[i];
				// magenta
//				ofSetColor(255,0,80);
				ofSetColor(uiFFT->_settings->alertColor);
			} else {
				ofSetColor(255);
			}
			int h = ofMap(equalizador[i],0,1,0,sliderHeight,true);
			ofDrawRectangle(i*larguraEq, sliderHeight - h, larguraEq-1, h);
		}
		updownTemp /= (float)numeroFrequencias;
		updownTemp = ofClamp(updownTemp, 0, 1);
		updownTemp = invertAudio ? 1.0 - updownTemp : updownTemp;
		updown = updownTemp;

		if (uiFFT->pFloat["audioEasing"]) {
			updownEasy += (updownTemp - updownEasy) / uiFFT->pFloat["audioEasing"];
			updown = updownEasy;
		}

		// RESULTANTE, FREQUENCIA GERAL
//		ofSetColor(80,0,255);
		ofSetColor(uiFFT->_settings->alertColor2);
		int h = ofMap(updown,0,1,0,sliderHeight,true);
		ofDrawRectangle(NPEAKS*larguraEq, sliderHeight - h, larguraEq*4, h);
		fboFFT->end();
	}
	
	void drawFFT() {
		ofSetColor (255);
		float w = 20.0;
		float h = 30.0;
		for (int a=0; a<NPEAKS; a++) {
	//		float freq = fft1.FFTanalyzer.peaks[a];
			float freq = fft1.peaks[a];
			ofColor cor = ofColor(0,255,0).getLerped(ofColor(255,0,50), ofClamp(freq/10.0, 0, 1));
			ofSetColor(cor);
			ofDrawRectangle(a*w, 300, w-1, -freq*h);
			float freq2 = fft2.peaks[a];
			ofDrawRectangle(a*w + w*19, 300, w-1, -freq2*h);
		}
	}

	//void updateFFT() {
	//	fboFFT->begin();
	//	ofClear(0,0);
	//	drawFFT();
	//	fboFFT->end();
	//}


	void updateFreqs() {
		ofPoint value;
		if (uiFFT != NULL) {
			value = uiFFT->pVec2["freq"];
		} else {
			value = ofPoint(.5, .5);
		}
		int larguraUtil = sliderWidth;
		int graveAgudo = value.x * (NPEAKS+2);

		int frequenciasAdjacentes = (1.0-value.y) * 10.0;
		int cadaFrequenciaContagem = 0;
		for (int a=0; a<NPEAKS; a++) {
			bool dentroFora = ofInRange(a, graveAgudo - frequenciasAdjacentes, graveAgudo + frequenciasAdjacentes);
			cadaFrequencia[a] = dentroFora;
			if (dentroFora) {
				cadaFrequenciaContagem++;
			}
			numeroFrequencias = cadaFrequenciaContagem;
		}
	}
};
