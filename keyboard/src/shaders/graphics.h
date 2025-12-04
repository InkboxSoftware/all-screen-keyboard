//graphics.h

//standard libraries
#include <stdio.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <fstream>
#include <thread>
#include <pthread.h>

//graphics libraries
#include <GLES3/gl3.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengles2.h>

//for video
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

//custom libraries
#include "shader.h"
#include "texture.h"
#include "text.h"

#define FORNORMALTEXTURE 512
#define FORSHIFTTEXTURE 1024
#define FORCTRLTEXTURE 2048
#define FORALTTEXTURE 4096
#define FORFNTEXTURE 8192

class window{
	
public:
	int initialize(int windowWidth, int windowHeight, bool fullscreen = true);
	int runWindow(bool* pressedKeys);

	//drawing functions:
	int drawRect(int x, int y, int width, int height, int r, int g, int b, int a, int keyID=-1);
	int drawCircle(int centerX, int centerY, int radius, int segments, int r, int g, int b, int a, int keyID=-1);

	int setFont(const char* fontName);	
	int drawText(const char* text, int x, int y, int width, int height, int r, int g, int b, int a, int keyID=-1);
		//text will appear in squished into this box

	//drawImage
	int drawImage(const char* src, int x, int y, int width, int height, int a, int keyID=-1);

	//writeText
	text* writeText(const char* inputText, int x, int y, float size, int r, int g, int b, int a);
		/*function that creates a textObj, which is a class that holds a vector of rects, and a sttring, location, color, etc.
			//returns the handle for the textObj so that you can use textObj.updateText(strring input) to update text
			//uses the built in font texture from the src folder, src/defaultText			
			//only support ascii under 128
		*/			
	int deleteText(text* toRemove);
		//removes text from rendering list


	//set background
	int setbackgroundImage(const char* src);
	void clearbackgroundImage();
	void setbackrgoundColor(int r, int g, int b, int a);
		//if called after set background image, will clear the image

	//only supports 1 video right now
	int loadVideo(const char* filename, int x, int y, int w, int h);

	//load shaders:
	int loadCustomShaderProgram(const char* vertexShaderFilename, const char* fragmentShaderFilename, int type);
		#define TYPE_PRESS_TEXTURE_SHADER 0
		#define TYPE_TEXTURE_SHADER 1
		#define TYPE_OTHER_SHADER 2
	//clear:
	void clearGraphicsMemory();
		//reset all vector lists, vram VAO and VBO, texture count var, reset pressShaderProgram
		
	//locking graphics:
	void setGraphicsLock(bool lockState);	//graphics will not render if lock set to true
	
	//multithreaded functions since graphics calls must be made in main thread:
	void requestClear();
	bool asyncFunctionCompleted();
	int asyncOutput();
	void requestCustomShader(const char* vertexShaderFilename, const char* fragmentShaderFilename, int type);
	void requestDrawRect(int x, int y, int width, int height, int r, int g, int b, int a, int keyID=-1);
	void requestSetbackgroundImage(const char* src);
	void requestDrawCircle(int centerX, int centerY, int radius, int segments, int r, int g, int b, int a, int keyID=-1);
	void requestDrawText(const char* text, int x, int y, int width, int height, int r, int g, int b, int a, int keyID=-1);
	void requestDrawImage(const char* src, int x, int y, int width, int height, int a, int keyID=-1);
	bool isRendering();
	
private:
		//multithread variables: (move these down to private later)
		bool asyncToDo = false;
		bool needToClear = false;
		int asyncOutputValue = -1;
		bool needCustomShader = false; const char* vertexShaderFileNameREQ; const char* fragmentShaderFilenameREQ; int typeREQ; 
		bool needDrawRect = false; int xREQ; int yREQ; int widthREQ; int heightREQ; int rREQ; int gREQ; int bREQ; int aREQ; int keyIDREQ=-1;
		bool needSetBackground = false; const char* srcREQ;
		bool needDrawCircle = false; int centerXREQ; int centerYREQ; int radiusREQ; int segmentsREQ;
		bool needDrawText = false; const char* textREQ;
		bool needDrawImage = false;
		
		
	int windowWidth = 640;
	int windowHeight = 480;
	
	bool graphicsLock = false;
	bool currentlyRendering = false;
	
	struct rectangle{
		float x;
		float y;
		float x2;
		float y2;
	};
	struct color{
		float r;
		float g;
		float b;
		float a;
	};
	struct order{
		//int absoluteOrder;	this is just the index of the item in the vector
		int listType;				//0 = vectorShape, 1 = vectorCircle, 2 = vectorText/vectorImage
		unsigned int relativeIndex;	//index in the above vector
		int keyTieID;				//keyID of relevant key, default -1
		int keyHeldType;			//-1 if no keyHeld types for key, else defines the current key down type
	};
	#define VECTORSHAPETYPE 0
	#define VECTORCIRCLETYPE 1
	#define VECTORTEXTIMAGETYPE 2
	#define PRESSEDTEXTURESHADERTYPE 100
	rectangle createRectangle(int x, int y, int width, int height);
	std::vector<GLfloat> generateCircleVerticies(int cx, int cy, int r, int numSegments);
	color createColor(int r, int g, int b, int a);

	//zorder list
	std::vector<order> zorderList;
	
	//for key down texture switching
	bool shiftHeld = false;
	bool ctrlHeld = false;
	bool altHeld = false;
	bool fnHeld = false;
	bool capsOn = false;
	int setHeldType(int keyID);	//returns the heldType for the order struct, checks all keys to determine if key has multiple textures for held type
	bool isALetterKey(int keyID);

	//list of solid color shapes:
	std::vector<GLuint> vectorShapeGLuint;
	std::vector<GLuint> vectorCircleuint;
		std::vector<int> vectorCircleSegments;	//number of triangles in each circle
	//list of image and text shapes:
	std::vector<GLuint> vectorTextGLuint;
	std::vector<GLuint> vectorTextures;

	const char* currentFont;
	color clearColor;
	GLint textureSamplerUniformLocation;
	int backgroundImgIndex = 0;
	bool backgroundImageOn = false;

	//video:
	const int VIDEOFRAMEBUFFERSIZE = 60;
	GLuint videoVerticies;
	GLuint videoTexture = -1;
	std::vector<SDL_Surface*> videoSurfaceBuffer;
	bool videoStarted = false;
	
	const char* videoFilename;
	bool videoReady = false;
	void playVideo();
	int fCount = 0;

	//shader programs:
	float startSDLTime;
	shader_h* shader;
	GLuint getCurrentTextureShaderProgram();
	GLuint loadShaderProgram(const char* vertexShaderFilename, const char* fragmentShaderFilename);
	GLuint textureShaderProgram;
	GLuint colorShaderProgram;
	GLuint textShaderProgram;
	//custom shader list:
	std::vector<GLuint> customShaderPrograms;
	int indexOfPressTextureShader = -1;
	int indexOfTextureShader = -1;
	
	//text
	std::vector<text*> listOfText;
	GLuint fontTexture;
	GLint textTextureSamplerUniformLocation;
	GLint textColorUniformLocation;
	GLint textCharIndexUniformLocation;
	GLint textXYSizeUniformLocation;
	GLint textIndexUniformLocation;
	GLuint basicVAO;
		//then bind that and use it as the basic rendering thing
		//then for the text, pass on the right parameters
		//make x and y in the text a float based on the screen size (how do I pass it in without passing it in? hardcode?
		//then rewrite the shader to render it out the right way

	//system variables
	SDL_GLContext glContext;
	SDL_Window* window;
};