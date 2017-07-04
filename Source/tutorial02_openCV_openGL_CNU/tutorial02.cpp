// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>

// Include GLEW
#include <glew.h>

// Include GLFW
#include <glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

//using namespace glm;

#include "../common/shader.hpp"
#include "../common/texture.hpp"
#include "../common/controls.hpp"
#include "../common/objloader.hpp"
#include "../common/vboindexer.hpp"
//#include "highgui.hpp"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include "Utilities.h"
#include "ARDrawingContext.hpp"
#include "MarkerDetector.hpp"


GLFWwindow* window;
const char* FILENAME = "CNU_1.obj";
int width,height;		//opencv video frame width and height


int main( void )
{
	cv::Mat frame;
	cv::VideoCapture cap(0);

	//these are the camera params for my dell laptop
	CameraCalibration	calibration(1078.3862345945590 , 1097.5330948245071, 265.36843235396532, 181.152828521706);
	MarkerDetector myDetector(calibration);

	ARDrawingContext myContext(FILENAME);
	do{
		if (!cap.read(frame))
			return FAIL;

		//marker detectors, make sure these go together
		//I had them reversed and setTransform... was displaying last frames
		//markers
		//makes the markers look like they are slightly behind in displaying
		myDetector.processFrame(frame);
		myContext.setTransformationList(myDetector.getTransformations());

		myContext.draw(frame);

		// Swap buffers
		glfwSwapBuffers(myContext.m_window);
		glfwPollEvents();
		
	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(myContext.m_window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		glfwWindowShouldClose(myContext.m_window) == 0 );

}


//int setupWindow(cv::Mat &frame, cv::VideoCapture &cap, int &width, int &height)
	//{
	//	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//	// Initialise GLFW
	//	if( !glfwInit() )
	//	{
	//		fprintf( stderr, "Failed to initialize GLFW\n" );
	//		return -1;
	//	}
	//
	//	glfwWindowHint(GLFW_SAMPLES, 4);
	//	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	//	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	//	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//
	//	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//	//open CV
	//	; // open the video file for reading
	//	if ( !cap.isOpened() )  // if not success, exit program
	//		return FAIL;
	//	if (!cap.read(frame))  // read a new frame from video
	//		return FAIL;
	//
	//	width = frame.size().width;
	//	height = frame.size().height;
	//	//////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//	// Open a window and create its OpenGL context
	//	window = glfwCreateWindow( width, height, "Tutorial 02 - OpenCV and OpenGL and CNU", NULL, NULL);
	//	if( window == NULL ){
	//		fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
	//		glfwTerminate();
	//		return FAIL;
	//	}
	//	glfwMakeContextCurrent(window);
	//
	//	// Initialize GLEW
	//	glewExperimental = true; // Needed for core profile
	//	if (glewInit() != GLEW_OK) {
	//		fprintf(stderr, "Failed to initialize GLEW\n");
	//		return FAIL;
	//	}
	//
	//	// Ensure we can capture the escape key being pressed below
	//	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	//
	//	// Dark blue background
	//	glClearColor(0.0f, 0.0f, 0.8f, 0.0f);
	//	return SUCCESS;
	//}

//*****************************************************************
//WWORKING CODE THAT DISPLAYS BACKGROUND AND CNU
//int main( void )
//{
//	cv::Mat frame;
//	cv::VideoCapture cap(0);
//	if (!setupWindow(frame,cap, width, height))
//		return FAIL;
//
//	//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	//create VBO, it will hold position and UV data plus CNU data (3)
//	GLuint vboHandles[6];
//	glGenBuffers(6, vboHandles);
//
//	GLuint vertexArrayID	= vboHandles[0];
//	GLuint uvbufferID		= vboHandles[1];
//	GLuint vertexbuffer		= vboHandles[2];	//CNU
//	GLuint uvbuffer			= vboHandles[3];	//CNU
//	GLuint normalbuffer		= vboHandles[4];	//CNU
//	GLuint elementbuffer	= vboHandles[5];	//CNU;
//
//
//	//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	// Create and compile our GLSL program from the shaders
//	GLuint programID = LoadShaders( "SimpleVertexShader.vertexshader", "SimpleFragmentShader.fragmentshader" );
//	print_all (programID);
//
//	static const GLfloat g_vertex_buffer_data[] = { 
//		-1.0f, -1.0f, 0.0f,
//		1.0f, -1.0f, 0.0f,
//		1.0f,  1.0f, 0.0f,
//		-1.0f, -1.0f, 0.0f,
//		1.0f,  1.0f, 0.0f,
//		-1.0f,  1.0f, 0.0f,
//	};
//	static const GLfloat texCords[] = { 
//		0.0f, 0.0f,
//		1.0f, 0.0f,
//		1.0f,  1.0f,
//		0.0f, 0.0f,
//		1.0f, 1.0f,
//		0.0f,  1.0f,
//	};
//
//	// Convert openCV video frame to texture
////	GLuint textureID = matToTexture(frame, GL_NEAREST, GL_NEAREST, GL_CLAMP);
//	GLuint textureID = -1;
//
//	// Get a handle for our "myTextureSampler" uniform
//	//returns an integer that represents the location of a specific uniform variable within a program object.
//	GLuint TextureID  = glGetUniformLocation(programID, "myTextureSampler");
//	glUniform1i(TextureID, 0);	// Set our "myTextureSampler" sampler to user Texture Unit 0
//
//	glBindBuffer(GL_ARRAY_BUFFER, vertexArrayID);
//	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data)*sizeof(float), g_vertex_buffer_data, GL_STATIC_DRAW);
//
//	glBindBuffer(GL_ARRAY_BUFFER, uvbufferID);
//	glBufferData(GL_ARRAY_BUFFER, sizeof(texCords)*sizeof(float), texCords, GL_STATIC_DRAW);
//
//	// Create and set-up the vertex array object
//	GLuint vaoHandle;
//	glGenVertexArrays( 1, &vaoHandle );
//	glBindVertexArray(vaoHandle);
//
//	glEnableVertexAttribArray(0);  // Vertex position
//	glEnableVertexAttribArray(1);  // uv 
//
//
//	//bind vertexes to position 0
//	glBindBuffer(GL_ARRAY_BUFFER, vertexArrayID);
//	glVertexAttribPointer(
//		0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
//		3,                  // size
//		GL_FLOAT,           // type
//		GL_FALSE,           // normalized?
//		0,                  // stride
//		(void*)0            // array buffer offset
//		);
//
//	//bind UVs to position 1
//	glBindBuffer(GL_ARRAY_BUFFER, uvbufferID);
//	glVertexAttribPointer(
//		1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
//		2,                                // size : U+V => 2
//		GL_FLOAT,                         // type
//		GL_FALSE,                         // normalized?
//		0,                                // stride
//		(void*)0                          // array buffer offset
//		);
//
//	//binding to the shader
//	glBindVertexArray(vaoHandle);
//
//
//	//************************************************************************************************************
//	//get the CNU object
//	// Create and compile our CNU GLSL program from the NEW shaders
//	GLuint programIDCNU = LoadShaders( "StandardShading.vertexshader", "StandardShading.fragmentshader" );
//
//	GLuint texID = glGetUniformLocation(programIDCNU, "renderedTexture");
//	GLuint timeID = glGetUniformLocation(programIDCNU, "time");
//
//	// Get a handle for our "MVP" uniform
//	GLuint MatrixID = glGetUniformLocation(programIDCNU, "MVP");
//	GLuint ViewMatrixID = glGetUniformLocation(programIDCNU, "V");
//	GLuint ModelMatrixID = glGetUniformLocation(programIDCNU, "M");
//
//	// Read our .obj file
//	std::vector<unsigned short> indices;
//	std::vector<glm::vec3> indexed_vertices;
//	std::vector<glm::vec2> indexed_uvs;
//	std::vector<glm::vec3> indexed_normals;
//	bool res = loadAssImp(FILENAME, indices, indexed_vertices, indexed_uvs, indexed_normals);
//
//	// Load it into a VBO
//	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
//	glBufferData(GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof(glm::vec3), &indexed_vertices[0], GL_STATIC_DRAW);
//
//	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
//	glBufferData(GL_ARRAY_BUFFER, indexed_uvs.size() * sizeof(glm::vec2), &indexed_uvs[0], GL_STATIC_DRAW);
//
//	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
//	glBufferData(GL_ARRAY_BUFFER, indexed_normals.size() * sizeof(glm::vec3), &indexed_normals[0], GL_STATIC_DRAW);
//
//	// Generate a buffer for the indices as well
//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
//	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0] , GL_STATIC_DRAW);
//
//	// Get a handle for our "LightPosition" uniform
//	//	glUseProgram(programID);
//	GLuint LightID = glGetUniformLocation(programIDCNU, "LightPosition_worldspace");
//
//	////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	do{
//
//		// Clear the screen
//		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		//glClear( GL_COLOR_BUFFER_BIT ); //causes nothing to be displayed
//		glLoadIdentity();
//
//		//*********************************************************************
//		//DRAW BACKGROUND 
//		//Use our shader
//		glUseProgram(programID);
//
//		////get rid of old texture otherwise leak horrendous amount of memory
//		if (textureID !=-1)
//			glDeleteTextures(1,&textureID);
//
//		//// read a new frame from video
//		if (cap.read(frame)) 
//			textureID = matToTexture(frame, GL_NEAREST, GL_NEAREST, GL_CLAMP);
//
//		//// Draw the triangle !
//		glDrawArrays(GL_TRIANGLES, 0, 6); // 3 indices starting at 0 -> 1 triangle
//
//		//*********************************************************************
//		//DRAW CNU
//		// Use our shader
//
//		glEnable(GL_DEPTH_TEST);		
//		glDepthFunc(GL_LESS);		// Accept fragment if it closer to the camera than the former one		
//		glUseProgram(programIDCNU);
//
////do the first then CNU appears accross middle 
//		//glm::mat4 ProjectionMatrix	= getProjectionMatrix();
//		//glm::mat4 ViewMatrix;
//
////do these CNU appears behind
//		glm::mat4 ProjectionMatrix = glm::perspective(35.0f, (float)width / (float)height, 0.1f, 100.0f);
//		glm::mat4 ViewMatrix       = glm::lookAt(
//			glm::vec3(5,5,5), // Camera is at (4,3,3), in World Space
//			glm::vec3(0,0,0), // and looks at the origin
//			glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
//			);
//
//
//		glm::mat4 ModelMatrix		= glm::mat4(1.0);
//		glm::mat4 MVP				= ProjectionMatrix * ViewMatrix * ModelMatrix;
//
//		// Send our transformation to the currently bound shader, 
//		// in the "MVP" uniform
//		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
//		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
//		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);
//
//		// Render to the screen======================================
//		//glBindFramebuffer(GL_FRAMEBUFFER, 2);
//		glViewport(0,0,width,height); // Render on the whole framebuffer, complete from the lower left corner to the upper right
//		//===============================================
//		glm::vec3 lightPos = glm::vec3(4,4,4);
//		glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
//		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);
//
//
//		glEnableVertexAttribArray(2);	//vertexbuffer CNU
//		glEnableVertexAttribArray(3);	//uvbuffer CNU
//		glEnableVertexAttribArray(4);	//normalbuffer CNU
//		glEnableVertexAttribArray(5);	//elementbuffer CNU
//
//		// 1rst attribute buffer : vertices
//		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
//		glVertexAttribPointer(
//			2,                  // attribute
//			3,                  // size
//			GL_FLOAT,           // type
//			GL_FALSE,           // normalized?
//			0,                  // stride
//			(void*)0            // array buffer offset
//			);
//
//		// 2nd attribute buffer : UVs
//		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
//		glVertexAttribPointer(
//			3,                                // attribute
//			2,                                // size
//			GL_FLOAT,                         // type
//			GL_FALSE,                         // normalized?
//			0,                                // stride
//			(void*)0                          // array buffer offset
//			);
//
//		// 3rd attribute buffer : normals
//		glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
//		glVertexAttribPointer(
//			4,                                // attribute
//			3,                                // size
//			GL_FLOAT,                         // type
//			GL_FALSE,                         // normalized?
//			0,                                // stride
//			(void*)0                          // array buffer offset
//			);
//
//		// Index buffer
//		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
//		glVertexAttribPointer(
//			5,                                // attribute
//			3,                                // size
//			GL_FLOAT,                         // type
//			GL_FALSE,                         // normalized?
//			0,                                // stride
//			(void*)0                          // array buffer offset
//			);
//
//		// Draw the triangles !
//		glDrawElements(
//			GL_TRIANGLES,      // mode
//			indices.size(),    // count
//			GL_UNSIGNED_SHORT,   // type
//			(void*)0           // element array buffer offset
//			);
//
//		//turn off depth test so open GL leaves background in the back next iteration
//		glDisable(GL_DEPTH_TEST);
//
//		glDisableVertexAttribArray(2);
//		glDisableVertexAttribArray(3);
//		glDisableVertexAttribArray(4);
//		glDisableVertexAttribArray(5);
//
//		//***************************************************************
//		// Swap buffers
//		glfwSwapBuffers(window);
//		glfwPollEvents();
//
//	} // Check if the ESC key was pressed or the window was closed
//	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
//		glfwWindowShouldClose(window) == 0 );
//
//	// Cleanup VBO
//	glDeleteProgram(programID);
//	glDeleteProgram(programIDCNU);
//
//	// Close OpenGL window and terminate GLFW
//	glfwTerminate();
//
//	return 0;
//}
//
//
//
