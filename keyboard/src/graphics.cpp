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

int window::runWindow(std::vector<key*> keys){
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
	
	int lastCaps = false;//pressedKeys[3];
	startSDLTime = SDL_GetTicks() / 1000.0f;
	
	while (running){
		shiftHeld = false;
		ctrlHeld = false;
		altHeld = false;
		metaHeld = false;
		fnHeld = false;
		capsHeld = false;
		while (SDL_PollEvent(&event)){
			//printf("EVENT: %d\n", event.type);
			switch (event.type){
				case SDL_KEYDOWN:
					if (event.key.keysym.sym != SDLK_ESCAPE)
					break;
				case SDL_QUIT:
					running = false;
					break;
				case SDL_FINGERDOWN:
					while (accessInputMUX.load(memory_order_acquire) > 0)
						__asm__("nop");		//idle until access is available
					accessInputMUX.fetch_add(1, memory_order_acq_rel);		//obtain access
					eventsToProcess.fetch_add(1, memory_order_acq_rel);		//increase events
					touchIOqueue.push_back({ event.tfinger.x, event.tfinger.y, true });
					accessInputMUX.fetch_sub(1, memory_order_acq_rel);		//release access
					break;
				case SDL_FINGERUP:				
					while (accessInputMUX.load(memory_order_acquire) > 0)
						__asm__("nop");		//idle until access is available
					accessInputMUX.fetch_add(1, memory_order_acq_rel);		//obtain access
					eventsToProcess.fetch_add(1, memory_order_acq_rel);		//increase events
					touchIOqueue.push_back({ event.tfinger.x, event.tfinger.y, false });
					accessInputMUX.fetch_sub(1, memory_order_acq_rel);		//release access
					break;
				case SDL_MOUSEBUTTONDOWN:
					while (accessInputMUX.load(memory_order_acquire) > 0)
						__asm__("nop");		//idle until access is available
					accessInputMUX.fetch_add(1, memory_order_acq_rel);		//obtain access
					eventsToProcess.fetch_add(1, memory_order_acq_rel);		//increase events
					touchIOqueue.push_back({ static_cast<float>(event.button.x) / 3840.0f, 
						static_cast<float>(event.button.y) / 1100.0f, true });
					accessInputMUX.fetch_sub(1, memory_order_acq_rel);		//release access
					break;
				case SDL_MOUSEBUTTONUP:				
					while (accessInputMUX.load(memory_order_acquire) > 0)
						__asm__("nop");		//idle until access is available
					accessInputMUX.fetch_add(1, memory_order_acq_rel);		//obtain access
					eventsToProcess.fetch_add(1, memory_order_acq_rel);		//increase events
					touchIOqueue.push_back({ static_cast<float>(event.button.x) / 3840.0f, 
						static_cast<float>(event.button.y) / 1100.0f, false });
					accessInputMUX.fetch_sub(1, memory_order_acq_rel);		//release access
					break;
					
				default:
					break;
			}		
		}

		//check if time to next frame has passed
		currentTime = SDL_GetTicks();
		elapsed = currentTime - lastTime;
		if (elapsed >= 16){
			//write elapsed time:
			float frameRate = 1.0f / (static_cast<float>(elapsed) / 1000.0f);
			listOfText[0]->updateText(to_string(frameRate).c_str());
			lastTime = currentTime;
		}
		else{
			continue;	//loop back to begining
		}

		//do not run if clearing VRAM:
		if (graphicsLock){	//monitor for mulithreaded requests:
			if (needToClear){
				clearGraphicsMemory();			
				asyncToDo = false;
			}
			if (needCustomShader){
				cout << vertexShaderFileNameREQ << "," << fragmentShaderFilenameREQ << endl;
				asyncOutputValue = loadCustomShaderProgram(vertexShaderFileNameREQ, fragmentShaderFilenameREQ);
				needCustomShader = false; 			
				asyncToDo = false;
			}
			else if (needDrawRect){
				asyncOutputValue = drawRect(zREQ, xREQ, yREQ, widthREQ, heightREQ, rREQ, gREQ, bREQ, aREQ, keyIDREQ, customShaderREQ);
				needDrawRect = false;			
				asyncToDo = false;
			}
			else if (needSetBackground){
				asyncOutputValue = setbackgroundImage(srcREQ);
				needSetBackground = false;			
				asyncToDo = false;
			}
			else if (needDrawCircle){
				asyncOutputValue = drawCircle(zREQ, centerXREQ, centerYREQ, radiusREQ, segmentsREQ, rREQ, gREQ, bREQ, aREQ, keyIDREQ, customShaderREQ);
				needDrawCircle = false;			
				asyncToDo = false;
			}
			else if (needDrawText){
				asyncOutputValue = drawText(textREQ, zREQ, xREQ, yREQ, widthREQ, heightREQ, rREQ, gREQ, bREQ, aREQ, keyIDREQ, modifierREQ, customShaderREQ);
				needDrawText = false;			
				asyncToDo = false;
			}
			else if (needDrawImage){
				cout << srcREQ << endl;	//I have no idea why, but if I leave this out, some textures aren't rendered properly, may be a timing issue
				asyncOutputValue = drawImage(srcREQ, zREQ, xREQ, yREQ, widthREQ, heightREQ, aREQ, keyIDREQ, modifierREQ, customShaderREQ);
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
		if (modShiftKeys.size() > 0){
			for (int i = 0; i < modShiftKeys.size(); i++){
				shiftHeld |= keys[modShiftKeys[i]]->pressed;
			}
		}
		//else { shiftHeld = false; }
		if (modCtrlKeys.size() > 0){
			for (int i = 0; i < modCtrlKeys.size(); i++){
				ctrlHeld |= keys[modCtrlKeys[i]]->pressed;
			}
		}
		//else { ctrlHeld = false; }
		if (modAltKeys.size() > 0){
			for (int i = 0; i < modAltKeys.size(); i++){
				altHeld |= keys[modAltKeys[i]]->pressed;
			}
		}
		//else { altHeld = false; }
		if (modUIKeys.size() > 0){
			for (int i = 0; i < modUIKeys.size(); i++){
				metaHeld |= keys[modUIKeys[i]]->pressed;
			}
		}
		//else { metaHeld = false; }
		if (modFnKeys.size() > 0){
			for (int i = 0; i < modFnKeys.size(); i++){
				fnHeld |= keys[modFnKeys[i]]->pressed;
			}
		}
		//else { fnHeld = false; }
		if (modCapKeys.size() > 0){
			for (int i = 0; i < modCapKeys.size(); i++){
				capsHeld |= keys[modCapKeys[i]]->pressed;
			}
		}
		//else { capsHeld = false; }
		
		
		if (lastCaps != capsHeld && !lastCaps && capsHeld){
			capsOn = !capsOn;
		}

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

		//update time in custom shader programs
		for (int k = 0; k < customShaderPrograms.size(); k++){
			glUseProgram(customShaderPrograms[k]);
			float currentTime = (SDL_GetTicks() / 1000.0f) - startSDLTime;
			glUniform1f(glGetUniformLocation(customShaderPrograms[k], "time"), currentTime);	
		}
		
		glUseProgram(textureShaderProgram);
		int lastCalledShaderProgram = DEFAULTTEXTIMAGESHADER;
		bool draw = false;
		bool advancedShift;
		int blockList = 0;
		bool blocked = false;
		int modifierLevel = -1;
		
		for (int Z = 0; Z < MAXZLEVEL; Z++){
			for (int i = 0; i < graphicsList.size(); i++){
				if (graphicsList[i].z != Z) continue;
				
				draw = false;
				blockList = 0;
				blocked = false;
				modifierLevel = -1;
				//check if the object is tied to a key & if it meets the key status of being rendered
				if (graphicsList[i].keyID >= 0){	//if -1, draw for all
					//heirarchy of display: fn -> alt -> meta -> ctrl -> shift
					advancedShift = (keys[graphicsList[i].keyID]->affectedByCapsLock & capsOn) | shiftHeld;
					if (keys[graphicsList[i].keyID]->affectedByCapsLock && capsOn && shiftHeld)
						advancedShift = false; 
					
					blockList = graphicsList[i].heldTypeModifier >> 16;	//list of blocked keys
					
					if (fnHeld && (graphicsList[i].heldTypeModifier & FORFNTEXTURE)){
						if (graphicsList[i].heldTypeModifier >= 0) draw = true;
						modifierLevel = 4;
					} else if (fnHeld && (blockList & FORFNTEXTURE)){ blocked = true; }
					
					if (altHeld && (graphicsList[i].heldTypeModifier & FORALTTEXTURE)){
						if (graphicsList[i].heldTypeModifier >= 0) draw = true;
						modifierLevel = 3;
					} else if (!draw && altHeld && (blockList & FORALTTEXTURE)){ blocked = true; }
					
					if (metaHeld && (graphicsList[i].heldTypeModifier & FORMETATEXTURE)){
						if (graphicsList[i].heldTypeModifier >= 0) draw = true;
						modifierLevel = 2;
					} else if (!draw && metaHeld && (blockList & FORMETATEXTURE)){ blocked = true; }
					
					if (ctrlHeld && (graphicsList[i].heldTypeModifier & FORCTRLTEXTURE)){
						if (graphicsList[i].heldTypeModifier >= 0) draw = true;
						modifierLevel = 1;
					} else if (!draw && ctrlHeld && (blockList & FORCTRLTEXTURE)){ blocked = true; }
					
					if (advancedShift && (graphicsList[i].heldTypeModifier & FORSHIFTTEXTURE)){
						if (graphicsList[i].heldTypeModifier >= 0) draw = true;
						modifierLevel = 0;
					} else if (!draw && advancedShift && (blockList & FORSHIFTTEXTURE)){ blocked = true; }
					
					if (!blocked && (!fnHeld && !altHeld && !ctrlHeld && !advancedShift && !metaHeld) && (graphicsList[i].heldTypeModifier & FORNORMALTEXTURE)){
						if (graphicsList[i].heldTypeModifier >= 0) draw = true;
					}
					//however, if normal & something in the blocking list is on, then don't turn on
					if (draw && (graphicsList[i].heldTypeModifier & FORNORMALTEXTURE)){
						blockList = graphicsList[i].heldTypeModifier ^ 0x1FF;
						if ((fnHeld && (blockList & FORFNTEXTURE)) || 
						 (metaHeld && (blockList & FORMETATEXTURE)) || 
						 (altHeld && (blockList & FORALTTEXTURE)) || 
						 (ctrlHeld && (blockList & FORCTRLTEXTURE)) || 
						 (advancedShift && (blockList & FORSHIFTTEXTURE)) 
						){
							if (graphicsList[i].heldTypeModifier >= 0) draw = false;
						}
					}
					if (graphicsList[i].heldTypeModifier == -1) draw = true;
					if (!draw || blocked)
						continue;
				}
				
				switch(graphicsList[i].graphicType){
					case DEFAULTSHAPESHADER:
						if (graphicsList[i].customShader >= 0 && lastCalledShaderProgram != graphicsList[i].customShader){
							glUseProgram(customShaderPrograms[graphicsList[i].customShader]);
							lastCalledShaderProgram = graphicsList[i].customShader;
						}
						else if (graphicsList[i].keyID >= 0 && modifierLevel > 0 &&
								keys[graphicsList[i].keyID]->modShader[modifierLevel] >= 0 && 
								lastCalledShaderProgram != keys[graphicsList[i].keyID]->modShader[modifierLevel]){
							glUseProgram(customShaderPrograms[keys[graphicsList[i].keyID]->modShader[modifierLevel]]);
							lastCalledShaderProgram = keys[graphicsList[i].keyID]->modShader[modifierLevel];
						}
						else if (lastCalledShaderProgram != DEFAULTSHAPESHADER && lastCalledShaderProgram != DEFAULTCIRCLESHADER){
							glUseProgram(colorShaderProgram);
							lastCalledShaderProgram = DEFAULTSHAPESHADER;
						}						
						glBindVertexArray(vectorShapeGLuint[graphicsList[i].relativeIndex]);
						glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
						break;

					case DEFAULTCIRCLESHADER:
						if (graphicsList[i].customShader >= 0 && lastCalledShaderProgram != graphicsList[i].customShader){
							glUseProgram(customShaderPrograms[graphicsList[i].customShader]);
							lastCalledShaderProgram = graphicsList[i].customShader;
						}
						else if (graphicsList[i].keyID >= 0 && modifierLevel > 0 &&
								keys[graphicsList[i].keyID]->modShader[modifierLevel] >= 0 && 
								lastCalledShaderProgram != keys[graphicsList[i].keyID]->modShader[modifierLevel]){
							glUseProgram(customShaderPrograms[keys[graphicsList[i].keyID]->modShader[modifierLevel]]);
							lastCalledShaderProgram = keys[graphicsList[i].keyID]->modShader[modifierLevel];
						}
						else if (lastCalledShaderProgram != DEFAULTSHAPESHADER && lastCalledShaderProgram != DEFAULTCIRCLESHADER){
							glUseProgram(colorShaderProgram);	
							lastCalledShaderProgram = DEFAULTCIRCLESHADER;
						}
						glBindVertexArray(vectorCircleuint[graphicsList[i].relativeIndex]);
						glDrawArrays(GL_TRIANGLE_FAN, 0, vectorCircleSegments[graphicsList[i].relativeIndex]);
						break;

					case DEFAULTTEXTIMAGESHADER:
						//pressShaders take precedene over modShaders
						if (graphicsList[i].keyID >= 0 && keys[graphicsList[i].keyID]->pressed &&
							keys[graphicsList[i].keyID]->pressShader >= 0 &&
							lastCalledShaderProgram != keys[graphicsList[i].keyID]->pressShader){	//check keyPressed
							glUseProgram(customShaderPrograms[keys[graphicsList[i].keyID]->pressShader]);
							lastCalledShaderProgram = keys[graphicsList[i].keyID]->pressShader;						
						}
						else if (graphicsList[i].keyID >= 0 && modifierLevel > 0 &&
								keys[graphicsList[i].keyID]->modShader[modifierLevel] >= 0 && 
								lastCalledShaderProgram != keys[graphicsList[i].keyID]->modShader[modifierLevel]){
							glUseProgram(customShaderPrograms[keys[graphicsList[i].keyID]->modShader[modifierLevel]]);
							lastCalledShaderProgram = keys[graphicsList[i].keyID]->modShader[modifierLevel];
						}
						else if (graphicsList[i].keyID >= 0 && keys[graphicsList[i].keyID]->shader >= 0 &&
								lastCalledShaderProgram != keys[graphicsList[i].keyID]->shader){
							glUseProgram(customShaderPrograms[keys[graphicsList[i].keyID]->shader]);
							lastCalledShaderProgram = keys[graphicsList[i].keyID]->shader;
						}
						else if (graphicsList[i].customShader >= 0 && 
								lastCalledShaderProgram != graphicsList[i].customShader){
							glUseProgram(customShaderPrograms[graphicsList[i].customShader]);
							lastCalledShaderProgram = graphicsList[i].customShader;						
						}
						//check for global custom shaders
						else {
							if (graphicsList[i].keyID >= 0 && keys[graphicsList[i].keyID]->pressed && texturePressShaderDefault >= 0){
								if (lastCalledShaderProgram != texturePressShaderDefault){
									glUseProgram(customShaderPrograms[texturePressShaderDefault]);
									lastCalledShaderProgram = texturePressShaderDefault;	
								}
							}
							else if (textureShaderDefault >= 0 && lastCalledShaderProgram != textureShaderDefault){
								glUseProgram(customShaderPrograms[textureShaderDefault]);
								lastCalledShaderProgram = textureShaderDefault;	
							}
							else if (textureShaderDefault == -1 && lastCalledShaderProgram != DEFAULTTEXTIMAGESHADER) //load default shader
							{	
								glUseProgram(textureShaderProgram);
								lastCalledShaderProgram = DEFAULTTEXTIMAGESHADER;	
							}
						}
						
						if (backgroundImageOn && graphicsList[i].relativeIndex == backgroundImgIndex)
							break;
						glBindTexture(GL_TEXTURE_2D, vectorTextures[graphicsList[i].relativeIndex]);	//set this texture to be GL_TEXTURE0
						glBindVertexArray(vectorTextGLuint[graphicsList[i].relativeIndex]);
						glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
						break;

					default:
						SDL_Log("Unknown data type found in graphicsList: %d", graphicsList[i].graphicType);
						break;
				}
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

int window::drawRect(int z, int x, int y, int width, int height, int r, int g, int b, int a, int keyID, int customShader){
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
	order zorder = { z, DEFAULTSHAPESHADER, static_cast<unsigned int>(vectorShapeGLuint.size()), keyID, -1, customShader };
	graphicsList.push_back(zorder);
	//add to shape list
	vectorShapeGLuint.push_back(squareVAO);
	return 0;
}

int window::drawCircle(int z, int centerX, int centerY, int radius, int segments, int r, int g, int b, int a, int keyID, int customShader){
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
	order zorder = { z, DEFAULTCIRCLESHADER, static_cast<unsigned int>(vectorCircleuint.size()), keyID, -1, customShader };
	graphicsList.push_back(zorder);
	//add to circle list
	vectorCircleuint.push_back(circleVAO);
	vectorCircleSegments.push_back(segments+2);
	return 0;
}

int window::drawText(const char* text, int z, int x, int y, int width, int height, int r, int g, int b, int a, int keyID, int modifier, int customShader){
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
	order zorder = { z, DEFAULTTEXTIMAGESHADER, static_cast<unsigned int>(vectorTextGLuint.size()), keyID, modifier, customShader };
	graphicsList.push_back(zorder);
	//add to text list
	vectorTextGLuint.push_back(squareVAO);
	return 0;
}

int window::drawImage(const char* src, int z, int x, int y, int width, int height, int a, int keyID, int modifier, int customShader){
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
	order zorder = { z, DEFAULTTEXTIMAGESHADER, static_cast<unsigned int>(vectorTextGLuint.size()), keyID, modifier, customShader };
	graphicsList.push_back(zorder);
	//add to image list
	vectorTextGLuint.push_back(squareVAO);
	return 0;
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

int window::loadCustomShaderProgram(const char* vertexShaderFilename, const char* fragmentShaderFilename){
	GLuint newShader = loadShaderProgram(vertexShaderFilename, fragmentShaderFilename);
	if (newShader == 0){	//did not load correctly
		cout << "Shader program failed to load" << endl;
		return -1;
	}
	customShaderPrograms.push_back(newShader);	
	return 0;
}

int window::setCustomGlobalTextureShader(int index){
	if (index >= 0 && index < customShaderPrograms.size())
		textureShaderDefault = index;
	else
		return -1;
	return 0;
}
int window::setCustomGlobalPressedTextureShader(int index){
	if (index >= 0 && index < customShaderPrograms.size())
		texturePressShaderDefault = index;
	else
		return -1;
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
	//cout << "Video Ready" << endl;
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
#ifdef AV_CODEC_FLAG_RECON_FRAME
	codecCtx->flags |= AV_CODEC_FLAG_RECON_FRAME;
#endif
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

void window::requestCustomShader(const char* vertexShaderFilename, const char* fragmentShaderFilename){
	asyncToDo = true;
	needCustomShader = true; 
	vertexShaderFileNameREQ = strdup(vertexShaderFilename); 
	fragmentShaderFilenameREQ = strdup(fragmentShaderFilename); 
	return;
}

void window::requestDrawRect(int z, int x, int y, int width, int height, int r, int g, int b, int a, int keyID, int customShader){
	asyncToDo = true;
	needDrawRect = true; 
	zREQ = z;
	xREQ = x;
	yREQ = y;
	widthREQ = width;
	heightREQ = height;
	rREQ = r;
	gREQ = g;
	bREQ = b;
	aREQ = a;
	keyIDREQ = keyID;
	customShaderREQ = customShader;
	return;
}

void window::requestSetbackgroundImage(const char* src){
	asyncToDo = true;
	needSetBackground = true; 
	srcREQ = strdup(src);
	return;
}

void window::requestDrawCircle(int z, int centerX, int centerY, int radius, int segments, int r, int g, int b, int a, int keyID, int customShader){
	asyncToDo = true;
	needDrawCircle = true;
	zREQ = z;
	centerXREQ = centerX;
	centerYREQ = centerY;
	radiusREQ = radius;
	segmentsREQ = segments;
	rREQ = r;
	gREQ = g;
	bREQ = b;
	aREQ = a;
	keyIDREQ = keyID;
	customShaderREQ = customShader;
	return;
}

void window::requestDrawText(const char* text, int z, int x, int y, int width, int height, int r, int g, int b, int a, int keyID, int modifier, int customShader){
	asyncToDo = true;
	needDrawText = true;
	textREQ = strdup(text);
	zREQ = z;
	xREQ = x;
	yREQ = y;
	widthREQ = width;
	heightREQ = height;
	rREQ = r;
	gREQ = g;
	bREQ = b;
	aREQ = a;
	keyIDREQ = keyID;
	modifierREQ = modifier;
	customShaderREQ = customShader;
	return;
}

void window::requestDrawImage(const char* src, int z, int x, int y, int width, int height, int a, int keyID, int modifier, int customShader){
	asyncToDo = true;
	needDrawImage = true;
	srcREQ = strdup(src);
	zREQ = z;
	xREQ = x;
	yREQ = y;
	widthREQ = width;
	heightREQ = height;
	aREQ = a;
	keyIDREQ = keyID;
	modifierREQ = modifier;
	customShaderREQ = customShader;
	return;
}

int window::setModifierKeys(std::vector<int> &leftShift, std::vector<int> &leftCtrl, std::vector<int> &leftAlt, std::vector<int> &leftUI, 
						std::vector<int> &rightShift, std::vector<int> &rightCtrl, std::vector<int> &rightAlt, std::vector<int> &rightUI,
						std::vector<int> &fn, std::vector<int> &caps){
	modShiftKeys.clear();
	modCtrlKeys.clear();
	modAltKeys.clear();
	modUIKeys.clear();
	modFnKeys.clear();
	modCapKeys.clear();
	for (int i = 0; i < leftShift.size(); i++)
		modShiftKeys.push_back(leftShift[i]);
	for (int i = 0; i < rightShift.size(); i++)
		modShiftKeys.push_back(rightShift[i]);
		
	for (int i = 0; i < leftCtrl.size(); i++)
		modCtrlKeys.push_back(leftCtrl[i]);
	for (int i = 0; i < rightCtrl.size(); i++)
		modCtrlKeys.push_back(rightCtrl[i]);
		
	for (int i = 0; i < leftAlt.size(); i++)
		modAltKeys.push_back(leftAlt[i]);
	for (int i = 0; i < rightAlt.size(); i++)
		modAltKeys.push_back(rightAlt[i]);
		
	for (int i = 0; i < leftUI.size(); i++)
		modUIKeys.push_back(leftUI[i]);
	for (int i = 0; i < rightUI.size(); i++)
		modUIKeys.push_back(rightUI[i]);
		
	for (int i = 0; i < fn.size(); i++)
		modFnKeys.push_back(fn[i]);
		
	for (int i = 0; i < caps.size(); i++)
		modCapKeys.push_back(caps[i]);
	
	return 0;
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

	graphicsList.clear();
	graphicsList.shrink_to_fit();
	
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
	
	glDeleteTextures(1, &videoTexture);
	videoTexture = -1;
	
	videoSurfaceBuffer.clear();
	videoSurfaceBuffer.shrink_to_fit();
	
	glDeleteVertexArrays(1, &videoVerticies);
	videoStarted = false;
	videoReady = false;
	fCount = 0;
	
	
	textureShaderDefault = -1;
	texturePressShaderDefault = -1;
	
	if (listOfText.size() > 1){	//keep the first, it's the frame counter
		listOfText.erase(listOfText.begin() + 1, listOfText.end());
	}
	listOfText.shrink_to_fit();
	
	
	modShiftKeys.clear();
	modCtrlKeys.clear();
	modAltKeys.clear();
	modUIKeys.clear();
	modFnKeys.clear();
	modCapKeys.clear();
	
	needToClear = false;
	
	return;
}