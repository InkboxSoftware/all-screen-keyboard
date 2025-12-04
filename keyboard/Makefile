programName = keyboard

build:
	g++ src/main.cpp src/keyboard.cpp src/readHallEffect.cpp src/graphics.cpp src/text.cpp src/shader.cpp src/texture.cpp src/serialKeyboard.cpp -o ${programName} -lwiringPi -lSDL2 -lSDL2_image -lSDL2_ttf -lGLESv2 -lavutil -lavformat -lavcodec -lswscale -lswresample
	@echo "Build Finished"

run:
	./${programName}

test: build run

clean:
	-rm ${programName}
	-rm graphicsTest
	-rm keyTest
	@echo "Clean Finished"

serialTest:
	g++ src/serial.cpp -o serialTest -lwiringPi
	./serialTest
	-rm serialTest

keyTest:
	g++ src/keyTest.cpp src/readHallEffect.cpp -o keyTest -lwiringPi -lSDL2
	./keyTest
	-rm keyTest

graphicsTest:
	g++ src/graphics.cpp src/clayman.cpp -o graphicsTest -lSDL2 -lSDL2_image -lSDL2_ttf -lGLESv2
	./graphicsTest
	-rm graphicsTest

plugin:
	g++ -shared -fPIC -o $(name).so src/plugins-dev/$(src).cpp
	@echo "Built plugin: $(name)"

py:
	python src/bluetooth/gattServer.py