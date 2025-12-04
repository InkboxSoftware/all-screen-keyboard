//custom libraries
#include "graphics.h"

using namespace std;

const int textureWidth = 256;
const int textureHeight = 256;

//Vertex type to define 2D position
typedef struct vertex_s {
	float position[2];
	float texCoord[3];
} Vertex;

typedef struct vertexColor {
	float position[2];
	float color[4];
} VertexColorSolid;

typedef struct vertexBasic {
	float position[2];
	float texCoord[2];
} VertexBasic;

GLuint vaoTextureCreate(const Vertex* verticies, int size){
	//create vertex buffer object
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	//copy the vertex data to VRAM
	glBufferData(GL_ARRAY_BUFFER, size, verticies, GL_STATIC_DRAW);	//store in VRAM
	
	//define position attribute location
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
	glEnableVertexAttribArray(0);
	//define texCoord attribute location
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
	glEnableVertexAttribArray(1);

	//unbind
	glBindVertexArray(0);	//unbind VAO

	//check for problems:
	GLenum err = glGetError();
	if (err != GL_NO_ERROR){
		glDeleteBuffers(1, &vbo);
		SDL_Log("Creating VBO failed: %u\n", err);
		vbo = 0;
	}
	return vao;
}

GLuint vaoColorCreate(const VertexColorSolid* verticies, int size){
	//create vertex buffer object
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	//copy the vertex data to VRAM
	glBufferData(GL_ARRAY_BUFFER, size, verticies, GL_STATIC_DRAW);	//store in VRAM
	
	//define position attribute location
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(VertexColorSolid), (void*)offsetof(VertexColorSolid, position));
	glEnableVertexAttribArray(0);
	//define texCoord attribute location
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(VertexColorSolid), (void*)offsetof(VertexColorSolid, color));
	glEnableVertexAttribArray(1);

	//unbind
	glBindVertexArray(0);	//unbind VAO

	//check for problems:
	GLenum err = glGetError();
	if (err != GL_NO_ERROR){
		glDeleteBuffers(1, &vbo);
		SDL_Log("Creating VBO failed: %u\n", err);
		vbo = 0;
	}
	return vao;
}

GLuint vaoTextCreate(const VertexBasic* verticies, int size){
	//create vertex buffer object
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	//copy the vertex data to VRAM
	glBufferData(GL_ARRAY_BUFFER, size, verticies, GL_STATIC_DRAW);	//store in VRAM
	
	//define position attribute location
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(VertexBasic), (void*)offsetof(VertexBasic, position));
	glEnableVertexAttribArray(0);
	//define texCoord attribute location
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexBasic), (void*)offsetof(VertexBasic, texCoord));
	glEnableVertexAttribArray(1);

	//unbind
	glBindVertexArray(0);	//unbind VAO

	//check for problems:
	GLenum err = glGetError();
	if (err != GL_NO_ERROR){
		glDeleteBuffers(1, &vbo);
		SDL_Log("Creating VBO failed: %u\n", err);
		vbo = 0;
	}
	return vao;
}

void vboFree(GLuint vbo){
	glDeleteBuffers(1, &vbo);
}

int window::initialize(int windowWidth, int windowHeight, bool fullScreen){
	this->windowWidth = windowWidth;
	this->windowHeight = windowHeight;
	//init SDL
	SDL_Init(SDL_INIT_VIDEO);
	//init window var
	window = NULL;
	//create OpenGL context
	glContext = NULL;

	shader = new shader_h();

	//request OpenGL ES 3.0
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

	//force usage of GLES backend
	SDL_SetHint(SDL_HINT_OPENGL_ES_DRIVER, "1");

	//double buffering
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	//create window
	int fullscreenVal = SDL_WINDOW_FULLSCREEN;
	if (!fullScreen){
		fullscreenVal = 0;
	}
	
	window = SDL_CreateWindow("Keyboard", SDL_WINDOWPOS_CENTERED, 
			SDL_WINDOWPOS_CENTERED, windowWidth, windowHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | fullscreenVal);
	//disable mouse
	SDL_ShowCursor(SDL_DISABLE);
	
	//create context
	glContext = SDL_GL_CreateContext(window);


	//load the texture shader program
	textureShaderProgram = loadShaderProgram("src/shaders/texture.vert", "src/shaders/texture.frag");
	if (!textureShaderProgram){
		return -1;
	}
	//load the color shader program
	colorShaderProgram = loadShaderProgram("src/shaders/colorShape.vert", "src/shaders/colorShape.frag");
	if (!colorShaderProgram){
		return -1;
	}
	//load the text shader program
	textShaderProgram = loadShaderProgram("src/shaders/text.vert", "src/shaders/text.frag");
	if (!textShaderProgram){
		return -1;
	}

	//GLint textureSamplerUniformLocation to set which texture unit to use later
	textureSamplerUniformLocation = glGetUniformLocation(textureShaderProgram, "textureSampler");
	if (textureSamplerUniformLocation < 0){
		SDL_Log("Could not get textureSampler's location");
		return -1;
	}
	
	//get the location of all the text shader parameters:
	textTextureSamplerUniformLocation = glGetUniformLocation(textShaderProgram, "textureSampler");
	textColorUniformLocation = glGetUniformLocation(textShaderProgram, "color");
	textCharIndexUniformLocation = glGetUniformLocation(textShaderProgram, "charIndex");
	textXYSizeUniformLocation = glGetUniformLocation(textShaderProgram, "XYSize");
	textIndexUniformLocation = glGetUniformLocation(textShaderProgram, "index");
	if (textTextureSamplerUniformLocation < 0 || textColorUniformLocation < 0 || textCharIndexUniformLocation < 0 || 
		textXYSizeUniformLocation < 0 || textIndexUniformLocation < 0){
		SDL_Log("Could not get (a) TEXT shader uniform's location");
		return -1;
	}
	//create font texture:
	fontTexture = textureLoad("img/font.png");
	if (!fontTexture){
		SDL_Log("Could not find font texture");
		return -1;
	}
	
	//basic vertex:
	float width = ((static_cast<float>(windowHeight) * 0.1) / (static_cast<float>(windowWidth))) / 4;
	//ok, this value here isn't half of the rect height
	VertexBasic vb[] = {
		{{-width, 0.05f},	{0.0f, 0.0f}},	
		{{width, 0.05f}, 	{1.0f, 0.0f}},	
		{{-width, -0.05f}, 	{0.0f, 1.0f}},	
		{{width, -0.05f}, 	{1.0f, 1.0f}}
	};
	basicVAO = vaoTextCreate(vb, sizeof(vb));
	if(!basicVAO){
		return -1;
	}	

	//enable transparency:
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(GL_FALSE);

	return 0;
}

GLuint window::getCurrentTextureShaderProgram(){
	switch (indexOfTextureShader){
		case -1:
			return textureShaderProgram;
		default:
			return customShaderPrograms[indexOfTextureShader];
	}
	//should never get here, but just in case:
	return textureShaderProgram;	
}

int window::runWindow(bool* pressedKeys){
	//running
	bool running = true;
	SDL_Event event;
	
	//if video is found, start playing the video in new thread:
	thread videoThread;
	if (videoReady){
		videoThread = thread(&window::playVideo, this);
	}
	
	Uint32 lastTime = SDL_GetTicks();
	Uint32 currentTime = SDL_GetTicks();
	Uint32 elapsed = SDL_GetTicks();
	Uint32 elapsedVideo = SDL_GetTicks();
	Uint32 lastTimeVideo = SDL_GetTicks();
	
	int lastCaps = pressedKeys[3];
	startSDLTime = SDL_GetTicks() / 1000.0f;
	
	while (running){
		while (SDL_PollEvent(&event)){
			//printf("EVENT: %d\n", event.type);
			if (event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE))
				running = false;
		}

		//do not run if clearing VRAM:
		if (graphicsLock){	//monitor for mulithreaded requests:
			if (needToClear){
				clearGraphicsMemory();			
				asyncToDo = false;
			}
			if (needCustomShader){
				cout << vertexShaderFileNameREQ << "," << fragmentShaderFilenameREQ << endl;
				asyncOutputValue = loadCustomShaderProgram(vertexShaderFileNameREQ, fragmentShaderFilenameREQ, typeREQ);
				needCustomShader = false; 			
				asyncToDo = false;
			}
			else if (needDrawRect){
				asyncOutputValue = drawRect(xREQ, yREQ, widthREQ, heightREQ, rREQ, gREQ, bREQ, aREQ, keyIDREQ);
				needDrawRect = false;			
				asyncToDo = false;
			}
			else if (needSetBackground){
				asyncOutputValue = setbackgroundImage(srcREQ);
				needSetBackground = false;			
				asyncToDo = false;
			}
			else if (needDrawCircle){
				asyncOutputValue = drawCircle(centerXREQ, centerYREQ, radiusREQ, segmentsREQ, rREQ, gREQ, bREQ, aREQ, keyIDREQ);
				needDrawCircle = false;			
				asyncToDo = false;
			}
			else if (needDrawText){
				asyncOutputValue = drawText(textREQ, xREQ, yREQ, widthREQ, heightREQ, rREQ, gREQ, bREQ, aREQ, keyIDREQ);
				needDrawText = false;			
				asyncToDo = false;
			}
			else if (needDrawImage){
				cout << srcREQ << endl;	//I have no idea why, but if I leave this out, some textures aren't rendered properly, may be a timing issue
				asyncOutputValue = drawImage(srcREQ, xREQ, yREQ, widthREQ, heightREQ, aREQ, keyIDREQ);
				needDrawImage = false;			
				asyncToDo = false;
			}
			//all async tasks are accomplished by this point
			startSDLTime = SDL_GetTicks() / 1000.0f;	//set time for initializtion of custom shaders when unfrozen
			continue;
		}
		//set currently rendering:
		currentlyRendering = true;

		//check key pressed values		
		if (lastCaps != pressedKeys[3] && !lastCaps && pressedKeys[3]){
			capsOn = !capsOn;
		}
		shiftHeld = pressedKeys[4] | pressedKeys[75];
		ctrlHeld = pressedKeys[5] | pressedKeys[79];
		altHeld = pressedKeys[7] | pressedKeys[76];
		fnHeld = pressedKeys[78];
		lastCaps = pressedKeys[3];

		//clear window with color
		glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
		glClear(GL_COLOR_BUFFER_BIT);

		//background image:
		glUseProgram(textureShaderProgram);
		glActiveTexture(GL_TEXTURE0);
		glUniform1i(textureSamplerUniformLocation, 0);	
		if (backgroundImageOn){
			glBindTexture(GL_TEXTURE_2D, vectorTextures[backgroundImgIndex]);	//set this texture to be GL_TEXTURE0
			glBindVertexArray(vectorTextGLuint[backgroundImgIndex]);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		}
		
		//background video:
		if (videoStarted && videoTexture != -1){	//means there is some video buffer ready to render:
			glBindTexture(GL_TEXTURE_2D, videoTexture);	//set this texture to be GL_TEXTURE0
			glBindVertexArray(videoVerticies);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		}


		glUseProgram(getCurrentTextureShaderProgram());
		if (indexOfTextureShader >= 0){
			float currentTime = (SDL_GetTicks() / 1000.0f) - startSDLTime;
			glUniform1f(glGetUniformLocation(getCurrentTextureShaderProgram(), "time"), currentTime);	
		}
		int lastCalledShaderProgram = VECTORTEXTIMAGETYPE;
		bool draw = false;
		bool advancedShift;
		bool isLetter;
		for (int i = 0; i < zorderList.size(); i++){
			draw = false;
			//check if the object is tied to a key & if it meets the key status of being rendered
			if (zorderList[i].keyHeldType >= 0){
				//heirarchy of display: fn -> alt -> ctrl -> shift -> normal
				isLetter = isALetterKey(zorderList[i].keyTieID);
				advancedShift = (isLetter & capsOn) | shiftHeld;
				if (isLetter && capsOn && shiftHeld)
					advancedShift = false; 
				if (fnHeld && (zorderList[i].keyHeldType & FORFNTEXTURE)){
					draw = true;
				}
				if (altHeld && (zorderList[i].keyHeldType & FORALTTEXTURE)){
					draw = true;
				}
				if (ctrlHeld && (zorderList[i].keyHeldType & FORCTRLTEXTURE)){
					draw = true;
				}
				if (advancedShift && (zorderList[i].keyHeldType & FORSHIFTTEXTURE)){
					draw = true;
				}
				if ((!fnHeld && !altHeld && !ctrlHeld && !advancedShift) && (zorderList[i].keyHeldType & FORNORMALTEXTURE)){
					draw = true;
				}
				if (!draw)
					continue;
			}
					
			switch(zorderList[i].listType){
				case VECTORSHAPETYPE:
					if (lastCalledShaderProgram != VECTORSHAPETYPE && lastCalledShaderProgram != VECTORCIRCLETYPE)
						glUseProgram(colorShaderProgram);
					lastCalledShaderProgram = VECTORSHAPETYPE;
					glBindVertexArray(vectorShapeGLuint[zorderList[i].relativeIndex]);
					glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
					break;

				case VECTORCIRCLETYPE:
					if (lastCalledShaderProgram != VECTORSHAPETYPE && lastCalledShaderProgram != VECTORCIRCLETYPE)
						glUseProgram(colorShaderProgram);
					lastCalledShaderProgram = VECTORCIRCLETYPE;
					glBindVertexArray(vectorCircleuint[zorderList[i].relativeIndex]);
					glDrawArrays(GL_TRIANGLE_FAN, 0, vectorCircleSegments[zorderList[i].relativeIndex]);
					break;

				case VECTORTEXTIMAGETYPE:
					if (pressedKeys[zorderList[i].keyTieID]){	//check keyPressed
						glUseProgram(customShaderPrograms[indexOfPressTextureShader]);	
						lastCalledShaderProgram = PRESSEDTEXTURESHADERTYPE;						
					}
					else if (lastCalledShaderProgram != VECTORTEXTIMAGETYPE){
						glUseProgram(getCurrentTextureShaderProgram());	
						lastCalledShaderProgram = VECTORTEXTIMAGETYPE;
					}
					if (backgroundImageOn && zorderList[i].relativeIndex == backgroundImgIndex)
						break;
					glBindTexture(GL_TEXTURE_2D, vectorTextures[zorderList[i].relativeIndex]);	//set this texture to be GL_TEXTURE0
					glBindVertexArray(vectorTextGLuint[zorderList[i].relativeIndex]);
					glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
					break;

				default:
					SDL_Log("Unknown data type found in zorderlist: %d", zorderList[i].listType);
					break;
			}
		}
		//render debugging text on top:
		glUseProgram(textShaderProgram);	//use text shader
		glBindTexture(GL_TEXTURE_2D, fontTexture);	//set font texture to be GL_Texture
		glUniform1i(textTextureSamplerUniformLocation, 0);	//set to use texture 0
		glBindVertexArray(basicVAO);
		const char* textOut;
		for (int i = 0; i < listOfText.size(); i++){
			continue;
			glUniform4f(textColorUniformLocation, listOfText[i]->c.r, listOfText[i]->c.g, listOfText[i]->c.b, listOfText[i]->c.a);	//set text color
			glUniform3f(textXYSizeUniformLocation, listOfText[i]->locationX, listOfText[i]->locationY, listOfText[i]->size);	//set size and origin location
			textOut = listOfText[i]->getText();
			for (int j = 0; j < strlen(textOut); j++){
				glUniform1i(textIndexUniformLocation, j);	//set index in string
				glUniform1i(textCharIndexUniformLocation, textOut[j]);	//set character to draw
				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);	//render text
			}
		}

		//update window
		SDL_GL_SwapWindow(window);
		
		
		elapsedVideo = SDL_GetTicks() - lastTimeVideo;
		if (videoStarted && videoSurfaceBuffer.size() > 0 && elapsedVideo > 30){	//means there is some video buffer ready to render:
			lastTimeVideo = SDL_GetTicks();
			if (videoSurfaceBuffer.size() <= 1)
				cout << "buffer size: " << videoSurfaceBuffer.size() << endl;
			if (videoTexture == -1){
				//cout << "init frame" << endl;
				videoTexture = textureFromSDLSurface(videoSurfaceBuffer[0]);
			}
			else{
				//cout << "replace frame: " << videoTexture << endl;
				replaceTextureFromSDLSurface(videoTexture, videoSurfaceBuffer[0]);
			}
			//remove start of buffer:
			videoSurfaceBuffer.erase(videoSurfaceBuffer.begin());
			//cout << "buffer size: " << videoSurfaceBuffer.size() << endl;
		}
		//rendering has finished:
		currentlyRendering = false;
		
		bool pass = false;
		while(!pass){
			currentTime = SDL_GetTicks();
			elapsed = currentTime - lastTime;
			if (elapsed >= 16){
				//write elapsed time:
				float frameRate = 1.0f / (static_cast<float>(elapsed) / 1000.0f);
				listOfText[0]->updateText(to_string(frameRate).c_str());
				lastTime = currentTime;
				pass = true;
			}
		}
			
	}
	printf("QUIT\n");
	SDL_GL_DeleteContext(glContext);
	SDL_DestroyWindow(window);
	SDL_Quit();
	
	if (videoReady)
		pthread_cancel(videoThread.native_handle());	//wait to join

	return 0;
}

text* window::writeText(const char* inputText, int x, int y, float size, int r, int g, int b, int a){
	text* newText = new text(inputText, x, y, size, r, g, b, a);
	listOfText.push_back(newText);
	return newText;
}

int window::deleteText(text* toRemove){
	auto it = find(listOfText.begin(), listOfText.end(), toRemove);
	if (it != listOfText.end()){
		//remove
		listOfText.erase(it);
		delete toRemove;	//free memory
		toRemove = nullptr;
	}
	else{
		return -1;	//not able to locate in list
	}
	return 0;
}

int window::drawRect(int x, int y, int width, int height, int r, int g, int b, int a, int keyID){
	rectangle rect = createRectangle(x, y, width, height);
	color c = createColor(r, g, b, a);

	//create verticies
		VertexColorSolid verticiesSquare[] = {
		{{rect.x, rect.y},	{c.r, c.g, c.b, c.a}},	
		{{rect.x2, rect.y}, 	{c.r, c.g, c.b, c.a}},	
		{{rect.x, rect.y2}, 	{c.r, c.g, c.b, c.a}},	
		{{rect.x2, rect.y2}, 	{c.r, c.g, c.b, c.a}}
	};
	GLuint squareVAO = vaoColorCreate(verticiesSquare, sizeof(verticiesSquare));
	if(!squareVAO){
		return -1;
	}	
	//set order
	int keyIDRaw = keyID & 0xFF;
	order zorder = { VECTORSHAPETYPE, static_cast<unsigned int>(vectorShapeGLuint.size()), keyIDRaw, setHeldType(keyID) };
	zorderList.push_back(zorder);
	//add to shape list
	vectorShapeGLuint.push_back(squareVAO);
	return 0;
}

int window::drawCircle(int centerX, int centerY, int radius, int segments, int r, int g, int b, int a, int keyID){
	vector<GLfloat> circlePoints = generateCircleVerticies(centerX, centerY, radius, segments);
	color c = createColor(r, g, b, a);

	//create verticies
	int numTriangles = circlePoints.size() / 2;
	VertexColorSolid verticiesCircle[numTriangles]; 
	for (int i = 0; i < numTriangles; i++){
		verticiesCircle[i] = {{circlePoints[i * 2], circlePoints[(i * 2) + 1]},	{c.r, c.g, c.b, c.a}};
	}
	GLuint circleVAO = vaoColorCreate(verticiesCircle, sizeof(verticiesCircle));
	if(!circleVAO){
		return -1;
	}	
	//set order
	int keyIDRaw = keyID & 0xFF;
	order zorder = { VECTORCIRCLETYPE, static_cast<unsigned int>(vectorCircleuint.size()), keyIDRaw, setHeldType(keyID) };
	zorderList.push_back(zorder);
	//add to circle list
	vectorCircleuint.push_back(circleVAO);
	vectorCircleSegments.push_back(segments+2);
	return 0;
}

int window::drawText(const char* text, int x, int y, int width, int height, int r, int g, int b, int a, int keyID){
	rectangle rect = createRectangle(x, y, width, height);
	color col = createColor(r, g, b, a);

	//create font texture:
	SDL_Color c = {static_cast<unsigned char>(r), static_cast<unsigned char>(g), static_cast<unsigned char>(b)};
	GLuint textureTextGL = textureLoadFromFont(currentFont, text, 200, c);
	vectorTextures.push_back(textureTextGL);

	//create verticies
		Vertex verticiesSquare[] = {
		{{rect.x, rect.y},	{0.0f, 0.0f, col.a}},	
		{{rect.x2, rect.y}, 	{1.0f, 0.0f, col.a}},	
		{{rect.x, rect.y2}, 	{0.0f, 1.0f, col.a}},	
		{{rect.x2, rect.y2}, 	{1.0f, 1.0f, col.a}}
	};

	GLuint squareVAO = vaoTextureCreate(verticiesSquare, sizeof(verticiesSquare));
	if(!squareVAO){
		return -1;
	}	
	//set order
	int keyIDRaw = keyID & 0xFF;
	order zorder = { VECTORTEXTIMAGETYPE, static_cast<unsigned int>(vectorTextGLuint.size()), keyIDRaw, setHeldType(keyID) };
	zorderList.push_back(zorder);
	//add to text list
	vectorTextGLuint.push_back(squareVAO);
	return 0;
}

int window::drawImage(const char* src, int x, int y, int width, int height, int a, int keyID){
	rectangle rect = createRectangle(x, y, width, height);
	float alpha = static_cast<float>(a) / 255.0f;

	//create image texture:
	GLuint textureImg = textureLoad(src);
	if (!textureImg){
		return -1;
	}
	vectorTextures.push_back(textureImg);

	//create verticies
		Vertex verticiesSquare[] = {
		{{rect.x, rect.y},		{0.0f, 0.0f, alpha}},	
		{{rect.x2, rect.y}, 	{1.0f, 0.0f, alpha}},	
		{{rect.x, rect.y2}, 	{0.0f, 1.0f, alpha}},	
		{{rect.x2, rect.y2}, 	{1.0f, 1.0f, alpha}}
	};

	GLuint squareVAO = vaoTextureCreate(verticiesSquare, sizeof(verticiesSquare));
	if(!squareVAO){
		return -1;
	}	
	//set order
	int keyIDRaw = keyID & 0xFF;
	order zorder = { VECTORTEXTIMAGETYPE, static_cast<unsigned int>(vectorTextGLuint.size()), keyIDRaw, setHeldType(keyID) };
	zorderList.push_back(zorder);
	//add to image list
	vectorTextGLuint.push_back(squareVAO);
	return 0;
}

int window::setHeldType(int keyID){
	//the purpose of this function is to check if the key has multiple textures for different key held conditions
	//i.e.a texture for no keys held, a texture for when shift is held, and so on
	//if no conditions exist, then the special return code here is -1
	//otherwise it will set the return number to be the conditions under which it is able to be shown	
	int multipleHeldTypes = (keyID & 0xFF00);	//if this is over normal key value, it includes a held key condition
	int keyIDRaw = keyID & 0xFF;
	for (int i = 0; i < zorderList.size(); i++){
		if (zorderList[i].keyTieID == keyIDRaw && zorderList[i].keyHeldType > 0  && !(zorderList[i].keyHeldType & FORNORMALTEXTURE)){
			multipleHeldTypes |= zorderList[i].keyHeldType;
		}
	}
	
	//check if held key conditions are met
	if (multipleHeldTypes > 0){
		//conditions are true, all relevant key held conditions stored in multipleHeldTypes.
		//each texture tied to the key should have it's keyHeldType adjusted to reflect these changes
		//keys with held conditions are only shown during when those keys are active
		//keys without conditions (normal conditions or unassigned) are active during all other condtions
		int remainingConditions = (FORSHIFTTEXTURE | FORCTRLTEXTURE | FORALTTEXTURE | FORFNTEXTURE);	//all possible conditions
		remainingConditions ^= multipleHeldTypes;	//get the opposite of whatever conditions are relevant
		//cout << "remaining Conditions: " << remainingConditions << endl << "all held: " << multipleHeldTypes << endl;
		remainingConditions |= FORNORMALTEXTURE;	//allow for normal operating conditions
		for (int i = 0; i < zorderList.size(); i++){
			if (zorderList[i].keyTieID == keyIDRaw && (zorderList[i].keyHeldType == -1 || zorderList[i].keyHeldType & FORNORMALTEXTURE)){
				zorderList[i].keyHeldType = remainingConditions;
			}
		}
		//return held condition:
		if (keyID > 0xFF){
			return (keyID - keyIDRaw);
		}
		return remainingConditions;
	}
	
	return -1;
}

int window::setbackgroundImage(const char* src){
	rectangle rect = createRectangle(0, 0, windowWidth, windowHeight);
	float alpha = 1.0f;

	//create image texture:
	GLuint textureImg = textureLoad(src);
	if (!textureImg){
		return -1;
	}
	vectorTextures.push_back(textureImg);

	//create verticies
		Vertex verticiesSquare[] = {
		{{rect.x, rect.y},		{0.0f, 0.0f, alpha}},	
		{{rect.x2, rect.y}, 	{1.0f, 0.0f, alpha}},	
		{{rect.x, rect.y2}, 	{0.0f, 1.0f, alpha}},	
		{{rect.x2, rect.y2}, 	{1.0f, 1.0f, alpha}}
	};

	GLuint squareVAO = vaoTextureCreate(verticiesSquare, sizeof(verticiesSquare));
	if(!squareVAO){
		return -1;
	}	
	vectorTextGLuint.push_back(squareVAO);

	backgroundImgIndex = vectorTextures.size() - 1;
	backgroundImageOn = true;

	return 0;
}
void window::clearbackgroundImage(){
	backgroundImageOn = false;	//turn off background image
	return;
}

window::rectangle window::createRectangle(int x, int y, int width, int height){
	rectangle rect;
	//turn coordinates and color into openGL format
	float windowWidthHalf = (static_cast<float>(windowWidth)) / 2.0f;
	float windowHeightHalf = (static_cast<float>(windowHeight)) / 2.0f;

	rect.x = (static_cast<float>(x) - windowWidthHalf) / windowWidthHalf;
	rect.y = -((static_cast<float>(y) - windowHeightHalf) / windowHeightHalf);
	rect.x2 = (static_cast<float>(x + width) - windowWidthHalf) / windowWidthHalf;
	rect.y2 = -((static_cast<float>(y + height) - windowHeightHalf) / windowHeightHalf);
	return rect;
}

vector<GLfloat> window::generateCircleVerticies(int cx, int cy, int r, int numSegments){
	vector<GLfloat> verticies;
	verticies.push_back(static_cast<float>(cx));
	verticies.push_back(static_cast<float>(cy));
	float radius = static_cast<float>(r);
	float windowWidthHalf = (static_cast<float>(windowWidth)) / 2.0f;
	float windowHeightHalf = (static_cast<float>(windowHeight)) / 2.0f;
	
	for (int i = 0; i <= numSegments; i++){
		float theta = 2.0f * M_PI * float(i) / float(numSegments);
		float x = cx + radius * cos(theta);
		float y = cy + radius * sin(theta);
		verticies.push_back(x);
		verticies.push_back(y);
	}
	//convert to openGL screen size values:
	for (int i = 0; i < verticies.size(); i+=2){
		verticies[i] 	= (verticies[i] - windowWidthHalf) / windowWidthHalf;
		verticies[i+1] 	= -((verticies[i+1] - windowHeightHalf) / windowHeightHalf);
	}

	return verticies;
}

window::color window::createColor(int r, int g, int b, int a){
	color c;
	c.r = static_cast<float>(r) / 255.0f;
	c.g = static_cast<float>(g) / 255.0f;
	c.b = static_cast<float>(b) / 255.0f;
	c.a = static_cast<float>(a) / 255.0f;
	return c;
}

int window::setFont(const char* fontName){
	//check if file exists and is ttf
	ifstream file(fontName);
	if (strstr(fontName, ".ttf") == nullptr){
		cout << fontName << " is not a ttf file" << endl;
		return -1;
	}
	if (!file.good()){
		cout << fontName << " file not found" << endl;
		return -1;
	}
	currentFont = strdup(fontName);
	return 0;
}

void window::setbackrgoundColor(int r, int g, int b, int a){
	clearColor = createColor(r, g, b, a);
	backgroundImageOn = false;	//turn off background image
	return;
}

GLuint window::loadShaderProgram(const char* vertexShaderFilename, const char* fragmentShaderFilename){
	//expose this function to the public to load in shaders from outside the graphics library
	return shader->shaderProgamLoad(vertexShaderFilename, fragmentShaderFilename);
}

int window::loadCustomShaderProgram(const char* vertexShaderFilename, const char* fragmentShaderFilename, int type){
	if (type >= TYPE_OTHER_SHADER){
		SDL_Log("Custom shader load error, Shader type not supported: %d", type);
		return -1;
	}
	
	

	customShaderPrograms.push_back(loadShaderProgram(vertexShaderFilename, fragmentShaderFilename));
	switch (type){
		case TYPE_TEXTURE_SHADER:
			indexOfTextureShader = customShaderPrograms.size() - 1;
			break;
		case TYPE_PRESS_TEXTURE_SHADER:
			indexOfPressTextureShader = customShaderPrograms.size() - 1;
			break;
		default:
			//should never get here due to check on top, but just in case:
			SDL_Log("Whoa, how'd you get past the check on top? Custom shader load error, Shader type not supported: %d", type);
			break;
	}
	
	return 0;
}

int window::loadVideo(const char* filename, int x, int y, int w, int h){
	//determine if file exists
	ifstream file(filename);
	if (!file.good()){
		cout << filename << " video file not found" << endl;
		return -1;
	}
	rectangle rect = createRectangle(x, y, w, h);

	//create verticies
	float alpha = 1.0f;
	Vertex verticiesSquare[] = {
		{{rect.x, rect.y},		{0.0f, 0.0f, alpha}},	
		{{rect.x2, rect.y}, 	{1.0f, 0.0f, alpha}},	
		{{rect.x, rect.y2}, 	{0.0f, 1.0f, alpha}},	
		{{rect.x2, rect.y2}, 	{1.0f, 1.0f, alpha}}
	};
	
	videoVerticies = vaoTextureCreate(verticiesSquare, sizeof(verticiesSquare));
	if(!videoVerticies){
		return -1;
	}	
	
	//glGenTextures(1, &videoTexture);
	videoFilename = strdup(filename);
	videoReady = true;
	fCount = 0;
	cout << "Video Ready" << endl;
	return 0;
}

void window::playVideo(){
	beginVideoRender:
	while (graphicsLock || !videoReady)
		__asm__("nop");
	cout << "new video to render" << endl;
	//av_log_set_level(AV_LOG_DEBUG);
	//init FFmpeg
	avformat_network_deinit();
	avformat_network_init();
	AVFormatContext* formatCtx = nullptr;
	if (avformat_open_input(&formatCtx, videoFilename, nullptr, nullptr) < 0){
		cerr << "Failed to open video file" << endl;
		return;
	}
	if (avformat_find_stream_info(formatCtx, nullptr) < 0){
		cerr << "Failed to find stream info" << endl;
		return;
	}
	
	//Find video stream
	int videoStreamIndex = -1;
	for (int i = 0; i < formatCtx->nb_streams; i++){
		if (formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO){
			videoStreamIndex = i;
			break;
		}
	}
	if (videoStreamIndex == -1){
		SDL_Log("No Video Stream Found");
		avformat_close_input(&formatCtx);
		return;
	}
	
	//open codec
	const AVCodec* codec = avcodec_find_decoder(formatCtx->streams[videoStreamIndex]->codecpar->codec_id);
	AVCodecContext* codecCtx = avcodec_alloc_context3(codec);
	avcodec_parameters_to_context(codecCtx, formatCtx->streams[videoStreamIndex]->codecpar);
	
	//enable flags
	codecCtx->flags |= AV_CODEC_FLAG_RECON_FRAME;
	codecCtx->pix_fmt = AV_PIX_FMT_NV12;
	if (avcodec_open2(codecCtx, codec, nullptr) < 0){
		cerr << "failed to open codec" << endl;
		return;
	}
	
	cout << "Codec: " << codecCtx->codec->name << " pixel format: " << codecCtx->pix_fmt << " size: " << codecCtx->width << endl;
	
	//read frames
	AVPacket* packet = av_packet_alloc();
	AVFrame* frame = av_frame_alloc();
	playVideoStream:
	while (av_read_frame(formatCtx, packet) >= 0){
		if (graphicsLock)
			goto RESETVIDEO;
		//if (!videoOn) return;
		if (packet->stream_index == videoStreamIndex){
			//cout << "Send packet " << packet->size << " bytes" <<  endl;
			sendpacket:
			int ret = avcodec_send_packet(codecCtx, packet);
			if (ret < 0){
				char errbuf[AV_ERROR_MAX_STRING_SIZE];
				av_strerror(ret, errbuf, AV_ERROR_MAX_STRING_SIZE);
				cerr << "Error sending packets: " << errbuf << endl;
			}
			getframe:
			auto res = avcodec_receive_frame(codecCtx, frame);
			if (res == AVERROR(EAGAIN)){
				//cout << "EAGAIN " << endl;
				av_frame_unref(frame);
				goto sendpacket;
			}
			
			if (res == AVERROR_EOF){
				cout << "EOF" << endl;
				break;
			}
			/*else if (res != 0){
				cout << "Other code: " << res << endl;
			}*/
			
			if (res == 0){
				//cout << "FRAME: " << fCount++ << endl;
				//cout << frame->width << ", " << frame->height << ", " << frame->best_effort_timestamp << " - " << packet->pts << ", " << packet->dts << endl;
					struct SwsContext* swsCtx = sws_getContext(
						frame->width, frame->height, (AVPixelFormat)frame->format, frame->width, frame->height, AV_PIX_FMT_RGBA, SWS_BILINEAR, NULL, NULL, NULL);
						uint8_t* rgbaData[1];
						int rgbaLinesize[1];
						rgbaData[0] = (uint8_t*)malloc(frame->width * frame->height * 4);
						rgbaLinesize[0] = frame->width * 4;
						sws_scale(swsCtx, frame->data, frame->linesize, 0, frame->height, rgbaData, rgbaLinesize);
					

				videoSurfaceBuffer.push_back(SDL_CreateRGBSurfaceFrom(rgbaData[0], frame->width, frame->height, 32, frame->width * 4, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000));
				sws_freeContext(swsCtx);
				free(rgbaData[0]);
				av_frame_unref(frame);
				
				while (videoSurfaceBuffer.size() >= VIDEOFRAMEBUFFERSIZE){	//buffer of frames
					__asm__("nop");
					videoStarted = true;
					if (graphicsLock){
						av_packet_unref(packet);
						goto RESETVIDEO;
					}
				}
			}
		}
		av_packet_unref(packet);
	}
	
	//get any final frames to be generated
	while (avcodec_receive_frame(codecCtx, frame)){
		//cout << "FRAME: " << fCount++ << endl;=
		//cout << frame->width << ", " << frame->height << ", " << frame->best_effort_timestamp << " - " << packet->pts << ", " << packet->dts << endl;
			struct SwsContext* swsCtx = sws_getContext(
				frame->width, frame->height, (AVPixelFormat)frame->format, frame->width, frame->height, AV_PIX_FMT_RGBA, SWS_BILINEAR, NULL, NULL, NULL);
				uint8_t* rgbaData[1];
				int rgbaLinesize[1];
				rgbaData[0] = (uint8_t*)malloc(frame->width * frame->height * 4);
				rgbaLinesize[0] = frame->width * 4;
				sws_scale(swsCtx, frame->data, frame->linesize, 0, frame->height, rgbaData, rgbaLinesize);
			

		videoSurfaceBuffer.push_back(SDL_CreateRGBSurfaceFrom(rgbaData[0], frame->width, frame->height, 32, frame->width * 4, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000));
		sws_freeContext(swsCtx);
		free(rgbaData[0]);
		av_frame_unref(frame);
	}
	
	//cout << "end decode" << endl;
	
	//start again:
	av_seek_frame(formatCtx, videoStreamIndex, 0, AVSEEK_FLAG_BACKWARD);	//rewind to begining
	avcodec_flush_buffers(codecCtx);
	fCount = 0;
	goto playVideoStream;

	RESETVIDEO:
	videoReady = false;
	//free memory:
	av_frame_free(&frame);
	av_packet_free(&packet);
	avcodec_free_context(&codecCtx);
	avformat_close_input(&formatCtx);
	if (graphicsLock)	//restart
		goto beginVideoRender;
	
	return;
}

bool window::isRendering(){
	cout << "video ready: " << videoReady << " rendering: " << currentlyRendering << endl;
	return videoReady | currentlyRendering; 
}

bool window::isALetterKey(int keyID){
	switch (keyID){
		case (9):
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
		case 19:
		case 20:
		case 21:
		case 22:
		case 23:
		case 34:
		case 35:
		case 36:
		case 37:
		case 38:
		case 39:
		case 42:
		case 43:
		case 44:
		case 45:
		case 46:
		case 47:
		case 58:
		case 59:
		case 60:
			return true;
		default:
			break;
	}
	return false;
}

//mulithreaded functions:
bool window::asyncFunctionCompleted(){
	return !asyncToDo;
}

int window::asyncOutput(){
	return asyncOutputValue;
}

void window::requestClear(){
	asyncToDo = true;
	needToClear = true;
}

void window::requestCustomShader(const char* vertexShaderFilename, const char* fragmentShaderFilename, int type){
	asyncToDo = true;
	needCustomShader = true; 
	vertexShaderFileNameREQ = strdup(vertexShaderFilename); 
	fragmentShaderFilenameREQ = strdup(fragmentShaderFilename); 
	typeREQ = type;
	return;
}

void window::requestDrawRect(int x, int y, int width, int height, int r, int g, int b, int a, int keyID){
	asyncToDo = true;
	needDrawRect = true; 
	xREQ = x;
	yREQ = y;
	widthREQ = width;
	heightREQ = height;
	rREQ = r;
	gREQ = g;
	bREQ = b;
	aREQ = a;
	keyIDREQ = keyID;
	return;
}

void window::requestSetbackgroundImage(const char* src){
	asyncToDo = true;
	needSetBackground = true; 
	srcREQ = strdup(src);
	return;
}

void window::requestDrawCircle(int centerX, int centerY, int radius, int segments, int r, int g, int b, int a, int keyID){
	asyncToDo = true;
	needDrawCircle = true;
	centerXREQ = centerX;
	centerYREQ = centerY;
	radiusREQ = radius;
	segmentsREQ = segments;
	rREQ = r;
	gREQ = g;
	bREQ = b;
	aREQ = a;
	keyIDREQ = keyID;
	return;
}

void window::requestDrawText(const char* text, int x, int y, int width, int height, int r, int g, int b, int a, int keyID){
	asyncToDo = true;
	needDrawText = true;
	textREQ = strdup(text);
	xREQ = x;
	yREQ = y;
	widthREQ = width;
	heightREQ = height;
	rREQ = r;
	gREQ = g;
	bREQ = b;
	aREQ = a;
	keyIDREQ = keyID;
	return;
}

void window::requestDrawImage(const char* src, int x, int y, int width, int height, int a, int keyID){
	asyncToDo = true;
	needDrawImage = true;
	srcREQ = strdup(src);
	xREQ = x;
	yREQ = y;
	widthREQ = width;
	heightREQ = height;
	aREQ = a;
	keyIDREQ = keyID;
	return;
}

//clearing graphics:
void window::setGraphicsLock(bool lockState){
	graphicsLock = lockState;
	if (graphicsLock)
		cout << "Graphics locked" << endl;
	else
		cout << "Graphics unlocked" << endl;
	return;
}

void window::clearGraphicsMemory(){	
	cout << "Clearing graphics memory" << endl;

	while (currentlyRendering){
		__asm__("nop");	//wait for rendering to finish
	}
	//do not allow further rendering:
	setGraphicsLock(true);

	zorderList.clear();
	zorderList.shrink_to_fit();
	
	for (int i = 0; i < vectorShapeGLuint.size(); i++){
		glDeleteVertexArrays(1, &vectorShapeGLuint[i]);
	}
	vectorShapeGLuint.clear();
	vectorShapeGLuint.shrink_to_fit();
	
	for (int i = 0; i < vectorCircleuint.size(); i++){
		glDeleteVertexArrays(1, &vectorCircleuint[i]);
	}
	vectorCircleuint.clear();
	vectorCircleuint.shrink_to_fit();
	vectorCircleSegments.clear();
	vectorCircleSegments.shrink_to_fit();
	
	for (int i = 0; i < vectorTextGLuint.size(); i++){
		glDeleteVertexArrays(1, &vectorTextGLuint[i]);
	}
	vectorTextGLuint.clear();
	vectorTextGLuint.shrink_to_fit();
	
	for (int i = 0; i < vectorTextures.size(); i++){
		glDeleteTextures(1, &vectorTextures[i]);
	}
	vectorTextures.clear();
	vectorTextures.shrink_to_fit();

	backgroundImgIndex = 0;
	backgroundImageOn = false;

	for (int i = 0; i < customShaderPrograms.size(); i++){
		shader->shaderProgramDestroy(customShaderPrograms[i]);
	}
	customShaderPrograms.clear();
	customShaderPrograms.shrink_to_fit();
	indexOfPressTextureShader = -1;
	indexOfTextureShader = -1;
	
	glDeleteTextures(1, &videoTexture);
	videoTexture = -1;
	
	videoSurfaceBuffer.clear();
	videoSurfaceBuffer.shrink_to_fit();
	
	glDeleteVertexArrays(1, &videoVerticies);
	videoStarted = false;
	videoReady = false;
	fCount = 0;
	
	if (listOfText.size() > 1){	//keep the first, it's the frame counter
		listOfText.erase(listOfText.begin() + 1, listOfText.end());
	}
	listOfText.shrink_to_fit();
	
	needToClear = false;
	
	return;
}