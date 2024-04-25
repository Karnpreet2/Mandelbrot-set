#include "ComplexPlane.h"
#include <complex>
#include <cmath>
#include <thread>

ComplexPlane::ComplexPlane(int pixelWidth, int pixelHeight){
	m_pixel_size = {pixelWidth, pixelHeight};
	m_aspectRatio = (float)pixelHeight / (float)pixelWidth;
	m_plane_center = {0, 0};
	m_plane_size = {BASE_WIDTH, BASE_HEIGHT * m_aspectRatio};
	m_ZoomCount = 0;
	m_state = State::CALCULATING;
	m_vArray.setPrimitiveType(Points);
	m_vArray.resize(pixelWidth*pixelHeight);
}

void ComplexPlane::draw(RenderTarget& target, RenderStates states) const{
    target.draw(m_vArray);
}

void ComplexPlane::updateRender(){
    if(m_state == State::CALCULATING){
	    std::vector<std::thread> thread;
	    thread.reserve(m_pixelHeight);
        for(int i = 0; i < m_pixel_size.y; i++){
		thread.emplace_back([this,i](){
            for(int j = 0; j < m_pixel_size.x; j++){
                Vector2f position = mapPixelToCoords(Vector2i({j,i}));
                int iterations = countIterations(position);
                
		Uint8 r, g, b;
                iterationsToRGB(iterations, r, g, b);
		    
		m_vArray[j+i* m_pixel_size.x].position = {(float)j, (float)i};
                m_vArray[j + i * m_pixel_size.x].color = {r, g, b};
            }
	});
        }
	for (auto& thread : thread){thread.join();}
        m_state = State::DISPLAYING;
    }
}
void ComplexPlane::zoomIn(){
    m_ZoomCount++;
    float x_size = BASE_WIDTH * (pow(BASE_ZOOM, m_ZoomCount));
    float y_size = BASE_HEIGHT * m_aspectRatio * (pow(BASE_ZOOM, m_ZoomCount));
    m_plane_size = {x_size, y_size};
    m_state = State::CALCULATING;
}

void ComplexPlane::zoomOut(){
  m_ZoomCount--;
  float x_size = BASE_WIDTH * pow(BASE_ZOOM, m_ZoomCount);
  float y_size = BASE_HEIGHT * m_aspectRatio * pow(BASE_ZOOM, m_ZoomCount);
  m_plane_size = {x_size, y_size};
  m_state = State::CALCULATING;
}

void ComplexPlane::setCenter(Vector2i mousePixel){
	m_plane_center= mapPixelToCoords(mousePixel);
	m_state = State::CALCULATING;
//	Vector2f position = mapPixelToCoords(mousePixel);
//	m_plane_center = position;
//	m_state = State::CALCULATING;

}

void ComplexPlane::setMouseLocation(Vector2i mousePixel){
/*	Vector2f position = mapPixelToCoords(mousePixel);
	m_mouseLocation.x = position.x;
	m_mouseLocation.y = position.y;*/
	m_mouseLocation= mapPixelToCoords(mousePixel);
}

void ComplexPlane::loadText(Text& text){
    stringstream ss;
    ss<< "Mandelbrot set\n";
    ss<< "Center: " << m_plane_center.x << ", " << m_plane_center.y << "\n";
    ss<< "Cursor {" << m_mouseLocation.x << ", "<< m_mouseLocation.y << "}\n";
    ss<< "Left click to zoom in\n";
    ss<< "Right click to zoom out\n";
    text.setString(ss.str());
    text.setCharacterSize(20);
    text.setFillColor(Color::White);
    text.setPosition(10,10);
}

int ComplexPlane::countIterations(Vector2f coord){
    int iterations = 0;
    complex<double> x (0,0);
    complex<double> c (coord.x, coord.y);
    while(iterations < MAX_ITER && abs(x)<2){
        x=pow(x,2)+c;
        iterations++;
    }
    return iterations;
}

void ComplexPlane::iterationsToRGB(size_t count, Uint8& r, Uint8& g, Uint8& b){
	if(count == MAX_ITER){
        	r = 0;
        	g = 0;
        	b = 0;
    	}
	else {
		size_t region = count/(MAX_ITER/5);
		switch (region){
			case 0:
			  r=255;
			  g=0;
			  b=0;
			  break;
			case 1:
			  r=255;
			  g=180;
			  b=0;
			  break;
			case 2:
			  r=0;
			  g=255;
			  b=75;
			  break;
			case 3:
			  r=0;
			  g=150;
			  b=255;
			  break;
			case 4:
			  r=130;
			  g=0;
			  b=255;
			  break;
		}
	}
}

Vector2f ComplexPlane::mapPixelToCoords(Vector2i mousePixel){
	float Xmin = m_plane_center.x - m_plane_size.x / 2.0f;
	float Xmax = m_plane_center.x + m_plane_size.x / 2.0f;
	float Ymin = m_plane_center.y - m_plane_size.y / 2.0f;
	float Ymax = m_plane_center.y + m_plane_size.y / 2.0f;
	float x = ((float)mousePixel.x / m_pixel_size.x) * (Xmax - Xmin) + Xmin;
	float y = ((float)m_pixel_size.y - mousePixel.y)/ m_pixel_size.y * (Ymax - Ymin) + Ymin;
	return Vector2f(x,y);
}
