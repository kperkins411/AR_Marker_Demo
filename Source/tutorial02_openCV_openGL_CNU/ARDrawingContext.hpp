#ifndef ARDrawingContext_HPP
#define ARDrawingContext_HPP

// File includes:
#include <string>
#include "GeometryTypes.hpp"
#include "CameraCalibration.hpp"
#include "glew.h"
#include <glfw3.h>
#include <opencv.hpp>

const int MY_VBO_HANDLES = 8;

class ARDrawingContext
{
public:
	//default call with my DELL laptops calibration parameters
	//CameraCalibration calibration(1078.3862345945590 , 1097.5330948245071, 265.36843235396532, 181.152828521706);
	//you really should calibrate your camera though!
	ARDrawingContext::ARDrawingContext(std::string filename);
	ARDrawingContext(std::string filename, const CameraCalibration& c);
	~ARDrawingContext();

	void setTransformationList(const std::vector<Transformation>& transformations);

	//! Set the new frame for the background
	bool draw(cv::Mat& frame);

	//used by calling app...sigh
	GLFWwindow*			m_window;	

private:
	//filename is the .obj file that we will load
	bool setup(cv::Mat& frame);
	bool setupOpenGL();
	bool setupOpenGL_Background();
	bool setupOpenGL_Object_to_Display();	
	bool setupOpenGL_White_Square();
	void initialize();

	//! Draws the background with video
	void drawBackground( cv::Mat& frame);

	//! Draws the AR
	void draw_CNU_Test();
	void drawAugmentedScene();
	void drawAugmentedScene_Object();
	void drawAugmentedScene_Square();

	//! Builds the right projection matrix from the camera calibration for AR
	bool buildProjectionMatrix( Matrix44& projectionMatrix );

private:
	Transformation		m_patternPose;	//?
	CameraCalibration	m_calibration;
	cv::VideoCapture	m_cap;
	int					m_height;
	int					m_width;
	std::string			m_filename;		//defaults to FILENAME
	cv::Mat				m_frame;		//?
	bool				m_bInitialized;
	Matrix44			m_projectionMatrix;	//??

	//shaders
	GLuint				m_programID_Background;
	GLuint				m_programID_Object;
	GLuint				m_programID_Square;

	//background texture
	GLuint				m_textureID;

	GLuint m_vboHandles[MY_VBO_HANDLES];
	GLuint m_vaoHandle;

	GLuint m_vertexArrayID;
	GLuint m_uvbufferID;	
	GLuint m_vertexbuffer;
	GLuint m_uvbuffer;		
	GLuint m_normalbuffer;
	GLuint m_elementbuffer;
	GLuint m_square_vertices;	
	GLuint m_square_colors;		

	GLuint	m_MatrixID;		
	GLuint	m_ViewMatrixID;	
	GLuint	m_ModelMatrixID;
	GLuint	m_LightID;

	GLuint  m_MatrixID_Square;		
	GLuint	m_ViewMatrixID_Square;
	GLuint	m_ModelMatrixID_Square;
	GLuint	m_LightID_Square;

	int		m_sizeIndices;

	std::vector<Transformation> m_transformations; //?
};

#endif