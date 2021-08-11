#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <complex.h>
#include <valarray>

using namespace std;
typedef std::complex<double> Complex;
typedef std::valarray<Complex> CArray;

void EventHandler(sf::RenderWindow& window){
	sf::Event event;
	while(window.pollEvent(event)) {
		if(event.type == sf::Event::Closed) 
			window.close();
		else if(event.type == sf::Event::KeyPressed) {
			if(sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
				//
			} else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
				window.close();
			} else if(sf::Keyboard::isKeyPressed(sf::Keyboard::W) && event.key.control){
				window.close();
			}
		}
	}
}
class FFT {
private:
	sf::SoundBuffer m_buf;
	sf::Sound m_sound;
	int m_sampleRate;
	int m_sampleCount;
	int m_bufSz;
	int m_offset;
	vector<Complex> samples;
	CArray bin;
	sf::VertexArray SoundWave;
	sf::VertexArray LinesSpectrum;
	sf::VertexArray BarSpectrum;
	vector<float> window;
	void getSamples() {
		sf::Vector2f sigpos(10.f, 400.f);
		m_offset = (m_sound.getPlayingOffset().asSeconds())*m_sampleRate;
		if(m_offset < m_sampleCount) {
			for(int i(m_offset); i< m_bufSz+m_offset; i++) {
				samples[i-m_offset] = Complex(m_buf.getSamples()[i]*window[i-m_offset],0);
				//SoundWave[i-m_offset] = sf::Vertex(sf::Vector2f((i-m_offset)*10.f, 20.f));
				SoundWave[i-m_offset] = sf::Vertex(sigpos+sf::Vector2f((i-m_offset)/(float)(m_bufSz)*900.f, samples[i-m_offset].real()/200.f),sf::Color::Red);
			}
		}
	}
	void fft(CArray& x)
	{
		const size_t N = x.size();
		if (N <= 1) return;
	
		// divide
		CArray even = x[std::slice(0, N/2, 2)];
		CArray  odd = x[std::slice(1, N/2, 2)];
	
		// conquer
		fft(even);
		fft(odd);
	
		// combine
		for (size_t k = 0; k < N/2; ++k)
		{
			Complex t = polar(1.0, -2 * M_PI * k / N) * odd[k];
			x[k    ] = even[k] + t;
			x[k+N/2] = even[k] - t;
		}
	}
	void lines() {
		LinesSpectrum.clear();
		sf::Vector2f pos = sf::Vector2f(10.f, 50.f);
		LinesSpectrum.append(sf::Vertex(pos,sf::Color::Green));
		for(int i(1); i< m_bufSz/4; i++) {
			sf::Vector2f specY = scale((float)i);
			
			LinesSpectrum.append(sf::Vertex(pos+specY,sf::Color::Green));
		}
		LinesSpectrum.append(sf::Vertex(pos));
	}
	float scaleY(float y, float factor) {
		//return ((y*y)*(factor*factor));
		return y*factor;
	}
	float scaleX(float x, float factor) {
		return log(x+1.f)/log(m_bufSz/24.f)*factor;
	}
	sf::Vector2f scale(float i) {
		return {scaleX(i,900.f*0.75f),
				scaleY((float)abs(bin[i]), 0.00001f)};
	}
	void bars1() {
		BarSpectrum.clear();
		sf::Vector2f pos(10.f,50.f);
		for(int i(0); i<m_bufSz/4; i++) {
			sf::Vector2f specY = scale((float)i);

			BarSpectrum.append(sf::Vertex(pos+specY,sf::Color::Green)); 
			BarSpectrum.append(pos+sf::Vector2f(specY.x,0));
			//BarSpectrum.append(sf::Vertex(pos+specY+sf::Vector2f(0.2f,0),sf::Color::Green)); 
			//BarSpectrum.append(pos+sf::Vector2f(specY.x+0.2f,0));
		}
	}

public:
	FFT (const string& path, int const bufSize) {
		if(!m_buf.loadFromFile(path)) throw "File is not exist";
		m_sound.setBuffer(m_buf);
		m_sound.play();

		m_sampleRate = m_buf.getSampleRate()*m_buf.getChannelCount();
		m_sampleCount = m_buf.getSampleCount();

		m_bufSz = bufSize;
		samples.resize(bufSize);
		SoundWave.setPrimitiveType(sf::LineStrip);
		SoundWave.resize(bufSize);
		LinesSpectrum.setPrimitiveType(sf::LineStrip);
		LinesSpectrum.resize(bufSize/2);
		BarSpectrum.setPrimitiveType(sf::Lines);
		BarSpectrum.resize(bufSize);
		m_offset = 0;
		for(int i(0) ; i < m_bufSz ; i++) window.push_back(0.54-0.46*cos(2*M_PI*i/(float)m_bufSz)) ;
	}
	void draw(sf::RenderWindow& window) {
		getSamples();
		//SoundWave.clear();
		//SpectrumWave.clear();
		bin = CArray(samples.data(), m_bufSz);
		fft(bin);
		lines();
		bars1();
		window.draw(SoundWave);
		//window.draw(LinesSpectrum);
		window.draw(BarSpectrum);
	}
};

int main(int ac, char **av) {
	sf::ContextSettings settings;
	//settings.antialiasingLevel = 8;

	sf::RenderWindow window(sf::VideoMode(930,500),"FFT audio spectrum", sf::Style::Default, settings);
	window.setFramerateLimit(30);
	window.setPosition(sf::Vector2i(0,270));
	//window.setVerticalSyncEnabled(true);

	// Revert the default coordinate
	// https://stackoverflow.com/questions/34310771/is-sfmlview-inverted-y-axis-standard-how-to-workaround-it
	sf::View view = window.getDefaultView();
	view.setSize(930.f, -500.f);
	window.setView(view); 
	FFT *fft;
	try {
		if(ac > 1) {
			fft = new FFT(av[1],16384/2);
		} else {
			//FFT fft("resources/FurElise.flac",16384/2);
			//FFT fft("resources/moonlightSonata.flac",16384/2);
			//fft = new FFT("resources/saveyourtears.flac",16384/4);
			//FFT fft("resources/creep.wav",16384/2);
			//FFT fft("resources/MelodiaDlaZuzi.wav",16384/2);
			fft = new FFT("resources/PopcornJohnnyBrave.wav",16384/2);
		}
	} catch (const char* msg){
		cerr << msg << endl;
		exit(-1);
	}
	while(window.isOpen()) {
		EventHandler(window);
		window.clear();
		// window.draw();
		fft->draw(window);
		window.display();
	}

}