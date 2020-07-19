
#define FFT_BUFFERSIZE 512/1
//#define NPEAKS 22
#define NPEAKS 18


ofSoundStream soundStream;
ofSoundStreamSettings settings;

//ofxDmtrUI3 * uiAudio = &u.uis["uiAudio"];
//ofxDmtrUI3 * uiAudio = NULL;


ofxMicroUI * uiFFT = NULL;
//ofxMicroUI * ui = NULL;
//ofxMicroUI * uiMaster = NULL;
ofFbo * fboFFT = NULL;

struct eachFft {
public:
	FFTOctaveAnalyzer FFTanalyzer;
	fft myfft;
	
	// NOVIDADE
	// mudar pra audioGain, audioFloor, audioClamp, audioMaxClamp
	float audioGain = 0.1;
	float audioFloor = 0;
	bool audioClamp = false;
	float audioMaxClamp = 4.0;
	
	float peaks[NPEAKS] = { 0 };
	float peaksEasy[NPEAKS] = { 0 };
	float easing = 10.0;

	float buffer [FFT_BUFFERSIZE] = {0};
	
	float magnitude	[FFT_BUFFERSIZE] = {0};
	float phase		[FFT_BUFFERSIZE] = {0};
	float power		[FFT_BUFFERSIZE] = {0};
	float avg_power = 0.0f;
	
	void setup() {
		FFTanalyzer.setup(44100, FFT_BUFFERSIZE, 2); //nOctaves
		FFTanalyzer.linearEQIntercept = 0.6f; // reduced gain at lowest frequency
		FFTanalyzer.linearEQSlope = 0.09f; // increasing gain at higher frequencies
	}
	
	void update() {
		myfft.powerSpectrum(0, FFT_BUFFERSIZE/2, buffer, FFT_BUFFERSIZE, &magnitude[0], &phase[0], &power[0], &avg_power);
		FFTanalyzer.calculate(magnitude);
		
		for (int a=0; a<NPEAKS; a++) {
			peaks[a] = FFTanalyzer.peaks[a] * audioGain + audioFloor;
			if (audioClamp) {
				peaks[a] = ofClamp(peaks[a], 0, audioMaxClamp);
			} else {
				peaks[a] = ofClamp(peaks[a], 0, 1000);
			}
			
			if (easing > 1.0) {
				peaksEasy[a] += (peaks[a] - peaksEasy[a]) / easing;
			} else {
				peaksEasy[a] = peaks[a];
			}
		}
	}
} fft1, fft2;

//--------------------------------------------------------------
void audioIn(ofSoundBuffer & input) {
	if (settings.numInputChannels == 2) {
		for (size_t i = 0; i < input.getNumFrames(); i++){
			fft1.buffer[i] = input[i*2];
			fft2.buffer[i] = input[i*2+1];
		}
	} else {
		for (size_t i = 0; i < input.getNumFrames(); i++){
			fft1.buffer[i] = input[i];
		}
	}
}

//--------------------------------------------------------------
float getFreq(int a) {
	a = a % NPEAKS;
	return fft1.peaks[a];
}

//--------------------------------------------------------------
void fftUIEvent(ofxMicroUI::element & e) {
	if (e.name == "audioGain") {
		fft1.audioGain = fft2.audioGain = *e.f;
	}
	else if (e.name == "audioFloor") {
		fft1.audioFloor = fft2.audioFloor = *e.f;
	}
	else if (e.name == "audioMaxClamp") {
		fft1.audioMaxClamp = fft2.audioMaxClamp = *e.f;
	}
	else if (e.name == "audioClamp") {
		fft1.audioClamp = fft2.audioClamp = *e.b;
	}
	
	else if (e.name == "linearEQIntercept") {
		fft1.FFTanalyzer.linearEQIntercept = *e.f;
		fft2.FFTanalyzer.linearEQIntercept = *e.f;
	}
	else if (e.name == "linearEQSlope") {
		fft1.FFTanalyzer.linearEQSlope = *e.f;
		fft2.FFTanalyzer.linearEQSlope = *e.f;
	}
	
	else if (e.name == "decay") {
		fft1.FFTanalyzer.peakDecayRate = *e.f;
		fft2.FFTanalyzer.peakDecayRate = *e.f;
	}
	
	else if (e.name == "peakhold") {
		fft1.FFTanalyzer.peakHoldTime  = *e.f;
		fft2.FFTanalyzer.peakHoldTime  = *e.f;
	}
	
	else if (e.name == "audioEasing") {
		fft1.easing = fft2.easing = *e.f;
	}
	
	
	
	// REVER
	if (e.name == "invertAudio") {
		invertAudio = uiFFT->pBool["invertAudio"];
	}

	else if (e.name == "freq") {
		updateFreqs();
	}
	
	else if (e.name == "balance") {
		balance = *e.f;
	}
}
