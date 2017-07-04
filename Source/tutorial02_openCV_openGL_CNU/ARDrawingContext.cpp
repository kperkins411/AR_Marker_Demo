/*****************************************************************************
*   Markerless AR desktop application.
******************************************************************************
*   by Khvedchenia Ievgen, 5th Dec 2012
*   http://computer-vision-talks.com
*
*	modified heavily by keith perkins
******************************************************************************
*   Ch3 of the book "Mastering OpenCV with Practical Computer Vision Projects"
*   Copyright Packt Publishing 2012.
*   http://www.packtpub.com/cool-projects-with-opencv/book
*****************************************************************************/
#include "ARDrawingContext.hpp"
#include <glew.h>
#include <glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../common/shader.hpp"
#include "../common/texture.hpp"
#include "../common/objloader.hpp"
#include "../common/vboindexer.hpp"
#include "Utilities.h"

//TODO make these adjustable
const float NEAR_PLANE	= 0.1f;
const float FAR_PLANE	= 100.0f;

//default call with my DELL laptops calibration parameters
//you really should calibrate your camera
ARDrawingContext::ARDrawingContext(std::string filename):m_filename(filename), m_calibration(1078.3862345945590 , 1097.5330948245071, 265.36843235396532, 181.152828521706)
{
	initialize();
}

ARDrawingContext::ARDrawingContext(std::string filename,const CameraCalibration& c):m_filename(filename),m_calibration(c)
{
	initialize();
}

void ARDrawingContext::initialize(){
		m_cap					= 0;
		m_height				= 0; 
		m_width					= 0; 
		m_window				= NULL;
		m_bInitialized			= false;
		m_programID_Background	= -1;
		m_programID_Object		= -1;
		m_programID_Square		= -1;
		m_textureID				= -1;
		m_vertexArrayID			= -1;
		m_uvbufferID			= -1;	
		m_vertexbuffer			= -1;
		m_uvbuffer				= -1;		
		m_normalbuffer			= -1;
		m_square_vertices		= -1;	 
		m_square_colors			= -1;	
		m_elementbuffer			= -1;
		m_vaoHandle				= -1;
		m_MatrixID				= -1;		
		m_ViewMatrixID			= -1;	
		m_ModelMatrixID			= -1;
		m_LightID				= -1;
		m_MatrixID_Square		= -1;		
		m_ViewMatrixID_Square	= -1;
		m_ModelMatrixID_Square	= -1;
		m_LightID_Square		= -1;
		m_sizeIndices			= 0;
}
ARDrawingContext::~ARDrawingContext()
{
	if (m_vaoHandle != -1)
		glDeleteVertexArrays (1, &m_vaoHandle );

	// Cleanup VBO
	glDeleteProgram(m_programID_Background);
	glDeleteProgram(m_programID_Object);
	glDeleteProgram(m_programID_Square);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();
}
//filename is the .obj file that we will load
bool ARDrawingContext::setup(cv::Mat& frame){

	if (m_bInitialized)
		return true;

	// Initialise GLFW
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		return false;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//setup the defaults
	m_width = frame.size().width;
	m_height = frame.size().height;
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Open a window and create its OpenGL context
	m_window = glfwCreateWindow( m_width, m_height, "OpenCV and OpenGL and CNU", NULL, NULL);
	if( m_window == NULL ){
		fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(m_window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return false;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(m_window, GLFW_STICKY_KEYS, GL_TRUE);

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.8f, 0.0f);

	//set everything else up as well
	m_bInitialized =					buildProjectionMatrix( m_projectionMatrix);
	m_bInitialized = m_bInitialized &&	setupOpenGL();
	m_bInitialized = m_bInitialized &&  setupOpenGL_Background();
	m_bInitialized = m_bInitialized &&  setupOpenGL_Object_to_Display();
	m_bInitialized = m_bInitialized &&  setupOpenGL_White_Square();

	return m_bInitialized;
}

bool ARDrawingContext::setupOpenGL(){
	//create VBO, it will hold position and UV data plus CNU data (3)
	glGenBuffers(MY_VBO_HANDLES, m_vboHandles);

	m_vertexArrayID		= m_vboHandles[0];
	m_uvbufferID		= m_vboHandles[1];
	m_vertexbuffer		= m_vboHandles[2];	//CNU
	m_uvbuffer			= m_vboHandles[3];	//CNU
	m_normalbuffer		= m_vboHandles[4];	//CNU
	m_elementbuffer		= m_vboHandles[5];	//CNU;
	m_square_vertices	= m_vboHandles[6];	//CNU;
	m_square_colors		= m_vboHandles[7];	//CNU;

	return true;
}
bool ARDrawingContext::setupOpenGL_Background(){

	// Create and compile our GLSL program from the shaders
	m_programID_Background = LoadShaders( "SimpleVertexShader.vertexshader", "SimpleFragmentShader.fragmentshader" );
	print_all (m_programID_Background);

	//edge to edge -1 to 1
	static const GLfloat g_vertex_buffer_data[] = { 
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		1.0f,  1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f,
		1.0f,  1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
	};

	//UV edge to edge 0 to 1
	static const GLfloat texCords[] = { 
		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f,  1.0f,
		0.0f, 0.0f,
		1.0f, 1.0f,
		0.0f,  1.0f,
	};

	// Convert openCV video frame to texture
	//	GLuint textureID = matToTexture(frame, GL_NEAREST, GL_NEAREST, GL_CLAMP);
//	GLuint textureID = -1;

	// Get a handle for our "myTextureSampler" uniform
	//returns an integer that represents the location of a specific uniform variable within a program object.
	GLuint TextureID  = glGetUniformLocation(m_programID_Background, "myTextureSampler");
	glUniform1i(TextureID, 0);	// Set our "myTextureSampler" sampler to user Texture Unit 0

	glBindBuffer(GL_ARRAY_BUFFER, m_vertexArrayID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data)*sizeof(float), g_vertex_buffer_data, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, m_uvbufferID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(texCords)*sizeof(float), texCords, GL_STATIC_DRAW);

	// Create and set-up the vertex array object
	glGenVertexArrays( 1, &m_vaoHandle );
	glBindVertexArray(m_vaoHandle);
	//TODO delete or make local m_vaoHandle

	glEnableVertexAttribArray(0);  // Vertex position
	glEnableVertexAttribArray(1);  // uv 

	//bind vertexes to position 0
	glBindBuffer(GL_ARRAY_BUFFER, m_vertexArrayID);
	glVertexAttribPointer(
		0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
		);

	//bind UVs to position 1
	glBindBuffer(GL_ARRAY_BUFFER, m_uvbufferID);
	glVertexAttribPointer(
		1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
		2,                                // size : U+V => 2
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
		);

	//binding to the shader
	glBindVertexArray(m_vaoHandle);

	return true;
}
bool ARDrawingContext::setupOpenGL_Object_to_Display(){
	//get the CNU object
	// Create and compile our CNU GLSL program from the NEW shaders
	m_programID_Object = LoadShaders( "StandardShading.vertexshader", "StandardShading.fragmentshader" );


	GLuint texID = glGetUniformLocation(m_programID_Object, "renderedTexture");
	GLuint timeID = glGetUniformLocation(m_programID_Object, "time");

	// Get a handle for our "MVP" uniform
	m_MatrixID			= glGetUniformLocation(m_programID_Object, "MVP");
	m_ViewMatrixID		= glGetUniformLocation(m_programID_Object, "V");
	m_ModelMatrixID		= glGetUniformLocation(m_programID_Object, "M");

	// Read our .obj file
	std::vector<unsigned short> indices;
	std::vector<glm::vec3> indexed_vertices;
	std::vector<glm::vec2> indexed_uvs;
	std::vector<glm::vec3> indexed_normals;
	bool res = loadAssImp(m_filename.c_str(), indices, indexed_vertices, indexed_uvs, indexed_normals);

	// Load it into a VBO
	glBindBuffer(GL_ARRAY_BUFFER, m_vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof(glm::vec3), &indexed_vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, m_uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_uvs.size() * sizeof(glm::vec2), &indexed_uvs[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, m_normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_normals.size() * sizeof(glm::vec3), &indexed_normals[0], GL_STATIC_DRAW);

	// Generate a buffer for the indices as well
	m_sizeIndices =indices.size();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_elementbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0] , GL_STATIC_DRAW);

	// Get a handle for our "LightPosition" uniform
	//	glUseProgram(programID);
	m_LightID = glGetUniformLocation(m_programID_Object, "LightPosition_worldspace");

	return true;
}
bool ARDrawingContext::setupOpenGL_White_Square(){
	//get the CNU object
	// Create and compile our CNU GLSL program from the NEW shaders

	m_programID_Square = LoadShaders( "VertexShader_Square.fragmentshader", "fragmentShader_Square.fragmentshader" );
	print_all (m_programID_Square);

	const GLfloat squareVertices[] = {
		-0.5f, -0.5f,0.0f,
		0.5f,  -0.5f,0.0f,
		0.5f,   0.5f,0.0f,
		-0.5f, -0.5f,0.0f,
		0.5f,  0.5f,0.0f,
		-0.5f,   0.5f,0.0f
	};

	//GLuint texID = glGetUniformLocation(m_programID_Object, "renderedTexture");
	//GLuint timeID = glGetUniformLocation(m_programID_Object, "time");

	// Get a handle for our "MVP" uniform
	m_MatrixID_Square			= glGetUniformLocation(m_programID_Square, "MVP");

	// Load it into a VBO
	glBindBuffer(GL_ARRAY_BUFFER, m_square_vertices);
	glBufferData(GL_ARRAY_BUFFER, sizeof(squareVertices)*sizeof(float), squareVertices, GL_STATIC_DRAW);

	return true;
}

bool ARDrawingContext::buildProjectionMatrix( Matrix44& projectionMatrix)
{
	if (m_width==0 || m_height == 0)
		return false;

	float nearPlane = NEAR_PLANE;	// Near clipping distance
	float farPlane  = FAR_PLANE;	// Far clipping distance

	// Camera parameters
	Matrix33 calibIntrinsic = m_calibration.getIntrinsic();

	//from camera calibration for Dell Lattitude E7440
	//see camera.yml 
	//camera intrinsic matrix

	//data: [	1.0783862345945590e+003,	0.,							2.6536843235396532e+002, 
	//			0.,							1.0975330948245071e+003,	1.8115282852170600e+002, 
	//			0.,							0.,							1. ]
	//distortion_coefficients: !!opencv-matrix
	//data: [ 2.8872859982866134e-001, -1.2244053537417756e+000,
	//	  -1.6014958480651827e-002, -1.9883313443179864e-002,
	//	  2.4407108524750809e+000 ]

	float f_x = calibIntrinsic.mat[0][0]; // Focal length in x axis
	float f_y = calibIntrinsic.mat[1][1]; // Focal length in y axis (usually the same?)
	float c_x = calibIntrinsic.mat[0][2]; // Camera primary point x
	float c_y = calibIntrinsic.mat[1][2]; // Camera primary point y

	projectionMatrix.data[0] = -2.0f * f_x / m_width;
	projectionMatrix.data[1] = 0.0f;
	projectionMatrix.data[2] = 0.0f;
	projectionMatrix.data[3] = 0.0f;

	projectionMatrix.data[4] = 0.0f;
	projectionMatrix.data[5] = 2.0f * f_y / m_height;
	projectionMatrix.data[6] = 0.0f;
	projectionMatrix.data[7] = 0.0f;

	projectionMatrix.data[8] = -(2.0f * c_x / m_width - 1.0f);			//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!why the hell does this work!!! should remove leading -
	projectionMatrix.data[9] = 2.0f * c_y / m_height - 1.0f;    
	projectionMatrix.data[10] = -( farPlane + nearPlane) / ( farPlane - nearPlane );
	projectionMatrix.data[11] = -1.0f;

	projectionMatrix.data[12] = 0.0f;
	projectionMatrix.data[13] = 0.0f;
	projectionMatrix.data[14] = -2.0f * farPlane * nearPlane / ( farPlane - nearPlane );        
	projectionMatrix.data[15] = 0.0f;
	return true;
}
bool ARDrawingContext::draw(cv::Mat& frame)
{
	//need to setup with default
	if (!m_bInitialized)
		setup(frame);	

	// Clear the screen
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		//glClear( GL_COLOR_BUFFER_BIT ); //causes nothing to be displayed
	glLoadIdentity();

	drawBackground(frame); 
	drawAugmentedScene();
	//draw_CNU_Test();
	return true;
}
void ARDrawingContext::drawBackground(cv::Mat& frame)
{
		//turn off depth test so open GL leaves background in the back next iteration
//		glDisable(GL_DEPTH_TEST);

		//*********************************************************************
		//DRAW BACKGROUND 
		//Use our shader
		glUseProgram(m_programID_Background);

		////get rid of old texture otherwise leak horrendous amount of memory
		if (m_textureID !=-1)
			glDeleteTextures(1,&m_textureID);

		//// read a new frame from video
		m_textureID = matToTexture(frame, GL_NEAREST, GL_NEAREST, GL_CLAMP);

		//// Draw the triangle !
		glDrawArrays(GL_TRIANGLES, 0, 6); // 3 indices starting at 0 -> 1 triangle
}
void ARDrawingContext::setTransformationList(const std::vector<Transformation>& transformations)
{
	m_transformations = transformations;
}


void ARDrawingContext::drawAugmentedScene(){
	drawAugmentedScene_Square();
	drawAugmentedScene_Object();
}	
void ARDrawingContext::drawAugmentedScene_Square(){
	// ensure that open gl rendering is done properly
	glEnable(GL_DEPTH_TEST);		
	glDepthFunc(GL_LESS);		// Accept fragment if it closer to the camera than the former one

	glUseProgram(m_programID_Square);

	glm::mat4	ModelMatrix			= glm::mat4(1.0);
	Matrix44	ViewMatrix;

	for (size_t transformationIndex=0; transformationIndex<m_transformations.size(); transformationIndex++)
	{
		//the modelview matrix, same as the view matrix
		Transformation& transformation			= m_transformations[transformationIndex];
		ViewMatrix								= transformation.getMat44();

		//apparently this works
		glm::mat4 PM				= glm::make_mat4(&m_projectionMatrix.data[0]);
		glm::mat4 VM1				= glm::make_mat4(&ViewMatrix.data[0]);

		//head is up is what I need
		glm::mat4 VM = glm::scale(VM1,glm::vec3(0.5f, -0.5f, 1.0f));

		glm::mat4 MVP				= 	PM * VM * ModelMatrix;

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(m_MatrixID_Square, 1, GL_FALSE, &MVP[0][0]);
	
		//lets put it on a white square
		glEnableVertexAttribArray(6);	//square_vertices	
		glBindBuffer (GL_ARRAY_BUFFER , m_square_vertices );
		glVertexAttribPointer(
			6,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);	

		// Draw the triangles !
		glDrawElements(
			GL_TRIANGLES,		// mode
			6,					// count
			GL_UNSIGNED_SHORT,  // type
			(void*)0			// element array buffer offset
			);
	}

	//turn off depth test so open GL leaves background in the back next iteration
	glDisable(GL_DEPTH_TEST);
	glDisableVertexAttribArray(6);
}
void ARDrawingContext::drawAugmentedScene_Object(){
	// ensure that open gl rendering is done properly
	glEnable(GL_DEPTH_TEST);		
	glDepthFunc(GL_LESS);		// Accept fragment if it closer to the camera than the former one

	glUseProgram(m_programID_Object);

	glm::mat4	ModelMatrix			= glm::mat4(1.0);
	Matrix44	ViewMatrix;

	for (size_t transformationIndex=0; transformationIndex<m_transformations.size(); transformationIndex++)
	{
		//the modelview matrix, same as the view matrix
		Transformation& transformation			= m_transformations[transformationIndex];
		ViewMatrix								= transformation.getMat44();

		//apparently this works
		glm::mat4 PM				= glm::make_mat4(&m_projectionMatrix.data[0]);
		glm::mat4 VM1				= glm::make_mat4(&ViewMatrix.data[0]);

		//head is up is what I need
		glm::mat4 VM = glm::scale(VM1,glm::vec3(0.5f, -0.5f, 1.0f));
//		glm::mat4 VM = glm::scale(VM1,glm::vec3(0.5f, 0.5f, 1.0f));

		glm::mat4 MVP				= 	PM * VM * ModelMatrix;

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(m_MatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(m_ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniformMatrix4fv(m_ViewMatrixID, 1, GL_FALSE, &VM[0][0]);

		glm::vec3 lightPos = glm::vec3(4,4,4);
		glUniform3f(m_LightID, lightPos.x, lightPos.y, lightPos.z);
		glUniformMatrix4fv(m_ViewMatrixID, 1, GL_FALSE, &VM[0][0]);

		glEnableVertexAttribArray(2);	//vertexbuffer CNU
		glEnableVertexAttribArray(3);	//uvbuffer CNU
		glEnableVertexAttribArray(4);	//normalbuffer CNU
		glEnableVertexAttribArray(5);	//elementbuffer CNU

		// 1rst attribute buffer : vertices
		glBindBuffer(GL_ARRAY_BUFFER, m_vertexbuffer);
		glVertexAttribPointer(
			2,                  // attribute
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

		// 2nd attribute buffer : UVs
		glBindBuffer(GL_ARRAY_BUFFER, m_uvbuffer);
		glVertexAttribPointer(
			3,                                // attribute
			2,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
			);

		// 3rd attribute buffer : normals
		glBindBuffer(GL_ARRAY_BUFFER, m_normalbuffer);
		glVertexAttribPointer(
			4,                                // attribute
			3,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
			);

		// Index buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_elementbuffer);
		glVertexAttribPointer(
			5,                                // attribute
			3,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
			);

		// Draw the triangles !
		glDrawElements(
			GL_TRIANGLES,		// mode
			m_sizeIndices,		// count
			GL_UNSIGNED_SHORT,  // type
			(void*)0			// element array buffer offset
			);
	}

	//turn off depth test so open GL leaves background in the back next iteration
	glDisable(GL_DEPTH_TEST);

	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);
	glDisableVertexAttribArray(4);
	glDisableVertexAttribArray(5);


}
//just going to draw CNU in middle of screen
void ARDrawingContext::draw_CNU_Test(){
		// ensure that open gl rendering is done properly
		glEnable(GL_DEPTH_TEST);		
		glDepthFunc(GL_LESS);		// Accept fragment if it closer to the camera than the former one		
		glUseProgram(m_programID_Object);

		glm::mat4 ProjectionMatrix = glm::perspective(35.0f, (float)m_width / (float)m_height, 0.1f, 100.0f);
		glm::mat4 ViewMatrix       = glm::lookAt(
			glm::vec3(5,5,5), // Camera is at (4,3,3), in World Space
			glm::vec3(0,0,0), // and looks at the origin
			glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
			);



		glm::mat4 ModelMatrix		= glm::mat4(1.0);
		glm::mat4 MVP				= ProjectionMatrix * ViewMatrix * ModelMatrix;

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(m_MatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(m_ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniformMatrix4fv(m_ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

		// Render to the screen======================================
		//glBindFramebuffer(GL_FRAMEBUFFER, 2);
		glViewport(0,0,m_width,m_height); // Render on the whole framebuffer, complete from the lower left corner to the upper right
		//===============================================
		glm::vec3 lightPos = glm::vec3(4,4,4);
		glUniform3f(m_LightID, lightPos.x, lightPos.y, lightPos.z);
		glUniformMatrix4fv(m_ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

		glEnableVertexAttribArray(2);	//vertexbuffer CNU
		glEnableVertexAttribArray(3);	//uvbuffer CNU
		glEnableVertexAttribArray(4);	//normalbuffer CNU
		glEnableVertexAttribArray(5);	//elementbuffer CNU

		// 1rst attribute buffer : vertices
		glBindBuffer(GL_ARRAY_BUFFER, m_vertexbuffer);
		glVertexAttribPointer(
			2,                  // attribute
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

		// 2nd attribute buffer : UVs
		glBindBuffer(GL_ARRAY_BUFFER, m_uvbuffer);
		glVertexAttribPointer(
			3,                                // attribute
			2,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
			);

		// 3rd attribute buffer : normals
		glBindBuffer(GL_ARRAY_BUFFER, m_normalbuffer);
		glVertexAttribPointer(
			4,                                // attribute
			3,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
			);

		// Index buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_elementbuffer);
		glVertexAttribPointer(
			5,                                // attribute
			3,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
			);

		// Draw the triangles !
		glDrawElements(
			GL_TRIANGLES,      // mode
			m_sizeIndices,    // count
			GL_UNSIGNED_SHORT,   // type
			(void*)0           // element array buffer offset
			);

		//turn off depth test so open GL leaves background in the back next iteration
		glDisable(GL_DEPTH_TEST);

		glDisableVertexAttribArray(2);
		glDisableVertexAttribArray(3);
		glDisableVertexAttribArray(4);
		glDisableVertexAttribArray(5);
}


