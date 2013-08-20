#include <iostream>

#include "Systems/OpenGLSystem.h"
#ifdef WIN32
#include "win32.h"
#endif

int main(int argCount, char **argValues) {
	OpenGLSystem glsys;
	IOpSys* os = nullptr;
#ifdef WIN32
	os = new win32();
#endif
	os->CreateGraphicsWindow();
	const int* version = glsys.Start();
	if (version[0] == -1) {
		std::cout<< "Error starting OpenGL!"<<std::endl;
	} else {
		std::cout<<"OpenGL version: " << version[0] << "." << version[1] << std::endl;
	}
	std::vector<Property> props;
	glsys.Factory("GLSprite", 1, props);
	glsys.Factory("GLSprite", 2, props);
	{
		Property prop1("scale");
		prop1.Set<float>(100.0f);
		props.push_back(prop1);
		Property prop2("x");
		prop2.Set(0.0f);
		props.push_back(prop2);
		Property prop3("y");
		prop3.Set<float>(0.0f);
		props.push_back(prop3);
		Property prop4("z");
		prop4.Set<float>(300.0f);
		props.push_back(prop4);
		glsys.Factory("GLIcoSphere", 3, props);
	}
	props.clear();
	{
		Property prop1("scale");
		prop1.Set<float>(200.0f);
		props.push_back(prop1);
		Property prop2("x");
		prop2.Set<float>(500.0f);
		props.push_back(prop2);
		Property prop3("y");
		prop3.Set<float>(0.0f);
		props.push_back(prop3);
		Property prop4("z");
		prop4.Set<float>(500.0f);
		props.push_back(prop4);
		glsys.Factory("GLIcoSphere", 4, props);
	}
	props.clear();
	{
		Property prop1("scale");
		prop1.Set<float>(1000.0f);
		props.push_back(prop1);
		Property prop2("x");
		prop2.Set<float>(500.0f);
		props.push_back(prop2);
		Property prop3("y");
		prop3.Set<float>(500.0f);
		props.push_back(prop3);
		Property prop4("z");
		prop4.Set<float>(2000.0f);
		props.push_back(prop4);
		glsys.Factory("GLIcoSphere", 5, props);
	}

	os->SetupTimer();
	double delta;
	while (os->MessageLoop()) {
		delta = os->GetDeltaTime();
		if (glsys.Update(delta)) {
			os->Present();
		}

		// Translation keys
		if (os->KeyDown('W')) { // Move forward
			if (os->KeyDown('B')) {
				glsys.Move(0.0f,0.0f,100.0f * (float)delta / 1000.0f);
			} else {
				glsys.Move(0.0f,0.0f,10.0f * (float)delta / 1000.0f);
			}
		} else if (os->KeyDown('S')) { // Move backward
			glsys.Move(0.0f,0.0f,-10.0f * (float)delta / 1000.0f);
		}
		if (os->KeyDown('A')) { 
			glsys.Rotate(0.0f,-90.0f * (float)delta / 1000.0f,0.0f); // Yaw left.
		} else if (os->KeyDown('D')) {
			glsys.Rotate(0.0f,90.0f * (float)delta / 1000.0f,0.0f); // Yaw right.
		}
		if (os->KeyDown('F')) { 
			glsys.Move(-10.0f * (float)delta / 1000.0f,0.0f,0.0f); // Strafe Left
		} else if (os->KeyDown('G')) {
			glsys.Move(10.0f * (float)delta / 1000.0f,0.0f,0.0f); // Strafe Right
		}
		if (os->KeyDown('E')) { // Move up
			glsys.Move(0.0f,10.0f * (float)delta / 1000.0f,0.0f);
		} else if (os->KeyDown('C')) { // Move down
			glsys.Move(0.0f,-10.0f * (float)delta / 1000.0f,0.0f);
		}
		if (os->KeyDown('Q')) { // Pitch Up
			glsys.Rotate(-90.0f * (float)delta / 1000.0f,0.0f,0.0f);
		} else if (os->KeyDown('Z')) { // Pitch Down
			glsys.Rotate(90.0f * (float)delta / 1000.0f,0.0f,0.0f);
		}

		if (os->KeyDown('R')) { // Roll left
			glsys.Rotate(0.0f,0.0f,-90.0f * (float)delta / 1000.0f);
		} else if (os->KeyDown('T')) { // Roll right
			glsys.Rotate(0.0f,0.0f,90.0f * (float)delta / 1000.0f);
		}
	}
	delete os;

	return 0;
}
