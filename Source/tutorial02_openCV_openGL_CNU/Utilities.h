#pragma once
// Include GLEW
#include "../external/glew-1.9.0/include/GL/glew.h"


//openCV
#include "highgui.hpp"

const int SUCCESS=1;
const int FAIL=0;

// Function turn a cv::Mat into a texture, and return the texture ID as a GLuint for use
// REMEMBER TO DELETE THIS TEXTUREID USING 	glDeleteTextures(1,&textureID);
// OR ELSE YOU GET A MEMORY LEAK
GLuint matToTexture(cv::Mat &mat, GLenum minFilter, GLenum magFilter, GLenum wrapFilter);

//error display functions
const char* GL_type_to_string (GLenum type); 

//print shader info to std::out
void _print_programme_info_log (GLuint programme);
void print_all (GLuint programme);

