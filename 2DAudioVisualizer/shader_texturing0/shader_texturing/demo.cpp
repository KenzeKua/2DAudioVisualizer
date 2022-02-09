#define GLFW_INCLUDE_ES2 1
#define GLFW_DLL 1
//#define GLFW_EXPOSE_NATIVE_WIN32 1
//#define GLFW_EXPOSE_NATIVE_EGL 1

#include <windows.h>

#include <GLES2/gl2.h>
#include <EGL/egl.h>

#include <GLFW/glfw3.h>
//#include <GLFW/glfw3native.h>

#include <fmod.hpp>
#include <fmod_errors.h>

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <fstream> 
#include "bitmap.h"

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

#define TEXTURE_COUNT 2

GLint GprogramID = -1;
GLuint GtextureID[TEXTURE_COUNT];

GLFWwindow* window;

// Fmod
// Must be power of 2 number
#define SPECTRUM_SIZE 512 //1024

FMOD::System* m_fmodSystem;
FMOD::Sound* m_music;
FMOD::Channel* m_musicChannel;

// For storing the octaves (average amplitude)
float octavesArray[18];

float m_spectrumLeft[SPECTRUM_SIZE];
float m_spectrumRight[SPECTRUM_SIZE];

static void error_callback(int error, const char* description)
{
  fputs(description, stderr);
}

GLuint LoadShader(GLenum type, const char *shaderSrc )
{
   GLuint shader;
   GLint compiled;
   
   // Create the shader object
   shader = glCreateShader ( type );

   if ( shader == 0 )
   	return 0;

   // Load the shader source
   glShaderSource ( shader, 1, &shaderSrc, NULL );
   
   // Compile the shader
   glCompileShader ( shader );

   // Check the compile status
   glGetShaderiv ( shader, GL_COMPILE_STATUS, &compiled );

   if ( !compiled ) 
   {
      GLint infoLen = 0;

      glGetShaderiv ( shader, GL_INFO_LOG_LENGTH, &infoLen );
      
      if ( infoLen > 1 )
      {
		 char infoLog[4096];
         glGetShaderInfoLog ( shader, infoLen, NULL, infoLog );
         printf ( "Error compiling shader:\n%s\n", infoLog );            
      }

      glDeleteShader ( shader );
      return 0;
   }

   return shader;
}

GLuint LoadShaderFromFile(GLenum shaderType, std::string path)
{
    GLuint shaderID = 0;
    std::string shaderString;
    std::ifstream sourceFile( path.c_str() );

    if( sourceFile )
    {
        shaderString.assign( ( std::istreambuf_iterator< char >( sourceFile ) ), std::istreambuf_iterator< char >() );
        const GLchar* shaderSource = shaderString.c_str();

		return LoadShader(shaderType, shaderSource);
    }
    else
        printf( "Unable to open file %s\n", path.c_str() );

    return shaderID;
}

void loadTexture(const char* path, GLuint textureID)
{
	CBitmap bitmap(path);

	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); 

	// bilinear filtering.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bitmap.GetWidth(), bitmap.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bitmap.GetBits());
}

int Init ( void )
{
   GLuint vertexShader;
   GLuint fragmentShader;
   GLuint programObject;
   GLint linked;

   //load textures
   glGenTextures(TEXTURE_COUNT, GtextureID);
   loadTexture("../media/rocks.bmp", GtextureID[1]);
   loadTexture("../media/rainbow_bg.bmp", GtextureID[0]);
   //====

   fragmentShader = LoadShaderFromFile(GL_VERTEX_SHADER, "../vertexShader1.vert" );
   vertexShader = LoadShaderFromFile(GL_FRAGMENT_SHADER, "../fragmentShader1.frag" );

   // Create the program object
   programObject = glCreateProgram ( );
   
   if ( programObject == 0 )
      return 0;

   glAttachShader ( programObject, vertexShader );
   glAttachShader ( programObject, fragmentShader );

   glBindAttribLocation ( programObject, 0, "vPosition" );
   glBindAttribLocation ( programObject, 1, "vColor" );
   glBindAttribLocation ( programObject, 2, "vTexCoord" );

   // Link the program
   glLinkProgram ( programObject );

   // Check the link status
   glGetProgramiv ( programObject, GL_LINK_STATUS, &linked );

   if ( !linked ) 
   {
      GLint infoLen = 0;

      glGetProgramiv ( programObject, GL_INFO_LOG_LENGTH, &infoLen );
      
      if ( infoLen > 1 )
      {
		 char infoLog[1024];
         glGetProgramInfoLog ( programObject, infoLen, NULL, infoLog );
         printf ( "Error linking program:\n%s\n", infoLog );            
      }

      glDeleteProgram ( programObject );
      return 0;
   }

   // Store the program object
   GprogramID = programObject;

   glClearColor ( 0.0f, 0.0f, 0.0f, 0.0f );
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

   return 1;
}

void ERRCHECK(FMOD_RESULT result)
{
    if (result != FMOD_OK)
    {
        printf("FMOD error! (%d) &s\n", result, FMOD_ErrorString(result));
    }
}

void initFmod()
{
    FMOD_RESULT result;

    unsigned int version;

    result = FMOD::System_Create(&m_fmodSystem);
    ERRCHECK(result);

    result = m_fmodSystem->getVersion(&version);
    ERRCHECK(result);

    if (version < FMOD_VERSION)
        printf("FMOD Error! You are using an old version of FMOD.", version, FMOD_VERSION);

    // Initialize FMOD system
    result = m_fmodSystem->init(32, FMOD_INIT_NORMAL, 0);
    ERRCHECK(result);

    // Load and set up music
    result = m_fmodSystem->createStream("../media/Smith the Mister - Mitsubachi.mp3", FMOD_SOFTWARE, 0, &m_music);
    ERRCHECK(result);

    // Play the loaded mp3 music
    result = m_fmodSystem->playSound(FMOD_CHANNEL_FREE, m_music, false, &m_musicChannel);
    result = m_musicChannel->setVolume(1.0);
    ERRCHECK(result);

    // Set sound channel loop count
    //m_musicChannel->setLoopCount(10);
}

void updateFmod()
{
    m_fmodSystem->update();

    // Get spectrum for left and right stereo channels
    m_musicChannel->getSpectrum(m_spectrumLeft, SPECTRUM_SIZE, 0, FMOD_DSP_FFT_WINDOW_RECT);
    m_musicChannel->getSpectrum(m_spectrumRight, SPECTRUM_SIZE, 1, FMOD_DSP_FFT_WINDOW_RECT);

    //std::cout << m_spectrumLeft[0] << "\t,\t" << m_spectrumRight[0] << std::endl;
    std::cout << m_spectrumLeft[0] + m_spectrumRight[0] * 0.5f << std::endl;
}

void calculateOctaves()
{
    m_fmodSystem->update();
    m_musicChannel->getSpectrum(m_spectrumLeft, SPECTRUM_SIZE, 0, FMOD_DSP_FFT_WINDOW_RECT);
    m_musicChannel->getSpectrum(m_spectrumRight, SPECTRUM_SIZE, 1, FMOD_DSP_FFT_WINDOW_RECT);
    //std::cout << " " << m_spectrumLeft[4] << "\t,\t" << m_spectrumRight[4] << std::endl;
    std::cout << octavesArray[4] << std::endl;

    static float tempSpectrumTotal = 0.0f;

    /*1 array = 43.06640625 Hz (22 050 Hz / 512 sample size)
    Note B in each octave in brackets
    Octave 0 (30.86771) = [0]
    Octave 1 (61.73541) = [1]
    Octave 2 (123.4708) = [2]
    Octave 3 (246.9417) = [3] - [5]
    Octave 4 (493.8833) = [6] - [13]
    Octave 5 (987.7666) = [14] - [24]
    Octave 6 (1975.533) = [25] - [47]
    Octave 7 (3951.066) = [48] - [91]
    Octave 8 (7902.133) = [92] - [184]*/

    for (int i = 0; i < 9; i++) {

        // Octave 0
        if (i == 0) {
        
            octavesArray[0] = (m_spectrumLeft[i] + m_spectrumRight[i]) * 0.5;
        }
        // Octave 1
        if (i == 1) {
        
            octavesArray[1] = (m_spectrumLeft[i] + m_spectrumRight[i]) * 0.5;
        }
        // Octave 2
        if (i == 2) {
        
            octavesArray[2] = (m_spectrumLeft[i] + m_spectrumRight[i]) * 0.5;
        }
        // Octave 3
        if (i == 3) {
        
            tempSpectrumTotal = 0;
            for (int j = 3; j <= 5; j++)
            {
                tempSpectrumTotal += m_spectrumLeft[j] + m_spectrumRight[j];
            }
            octavesArray[3] = tempSpectrumTotal * 0.5;
        }
        // Octave 4
        if (i == 4) {

            tempSpectrumTotal = 0;
            for (int j = 6; j <= 13; j++)
            {
                tempSpectrumTotal += m_spectrumLeft[j] + m_spectrumRight[j];
            }
            octavesArray[4] = tempSpectrumTotal * 0.5;
        }
        // Octave 5
        if (i == 5) {
        
            tempSpectrumTotal = 0;
            for (int j = 14; j <= 24; j++)
            {
                tempSpectrumTotal += m_spectrumLeft[j] + m_spectrumRight[j];
            }
            octavesArray[5] = tempSpectrumTotal * 0.5;
        }
        // Octave 6
        if (i == 6) {
        
            tempSpectrumTotal = 0;
            for (int j = 25; j <= 47; j++)
            {
                tempSpectrumTotal += m_spectrumLeft[j] + m_spectrumRight[j];
            }
            octavesArray[6] = tempSpectrumTotal * 0.5;
        }
        // Octave 7
        if (i == 7) {
        
            tempSpectrumTotal = 0;
            for (int j = 48; j <= 91; j++)
            {
                tempSpectrumTotal += m_spectrumLeft[j] + m_spectrumRight[j];
            }
            octavesArray[7] = tempSpectrumTotal * 0.5;
        }
        // Octave 8
        if (i == 8) {
        
            tempSpectrumTotal = 0;
            for (int j = 92; j <= 184; j++)
            {
                tempSpectrumTotal += m_spectrumLeft[j] + m_spectrumRight[j];
            }
            octavesArray[8] = tempSpectrumTotal * 0.5;
        }
    }
}

void Draw(void)
{
	// Set the Sampler2D varying variable to the first texture unit(index 0)
	glUniform1i(glGetUniformLocation(GprogramID, "sampler2d"), 0);

	// Modify Factor 1 varying variable
	static float factor1 = 0.0f;
	factor1 += 0.01f;
	GLint factor1Loc = glGetUniformLocation(GprogramID, "Factor1");
	if(factor1Loc != -1)
	{
	   glUniform1f(factor1Loc, factor1);
	}

    // Octave variables
    GLint octave0Loc = glGetUniformLocation(GprogramID, "Octave0");
    if (octave0Loc != -1)
    {
        glUniform1f(octave0Loc, octavesArray[0]);
    }
    GLint octave1Loc = glGetUniformLocation(GprogramID, "Octave1");
    if (octave1Loc != -1)
    {
        glUniform1f(octave1Loc, octavesArray[1]);
    }
    GLint octave2Loc = glGetUniformLocation(GprogramID, "Octave2");
    if (octave2Loc != -1)
    {
        glUniform1f(octave2Loc, octavesArray[2]);
    }
    GLint octave3Loc = glGetUniformLocation(GprogramID, "Octave3");
    if (octave3Loc != -1)
    {
        glUniform1f(octave3Loc, octavesArray[3]);
    }
    GLint octave4Loc = glGetUniformLocation(GprogramID, "Octave4");
    if (octave4Loc != -1)
    {
        glUniform1f(octave4Loc, octavesArray[4]);
    }
    GLint octave5Loc = glGetUniformLocation(GprogramID, "Octave5");
    if (octave5Loc != -1)
    {
        glUniform1f(octave5Loc, octavesArray[5]);
    }
    GLint octave6Loc = glGetUniformLocation(GprogramID, "Octave6");
    if (octave6Loc != -1)
    {
        glUniform1f(octave6Loc, octavesArray[6]);
    }
    GLint octave7Loc = glGetUniformLocation(GprogramID, "Octave7");
    if (octave7Loc != -1)
    {
        glUniform1f(octave7Loc, octavesArray[7]);
    }
    GLint octave8Loc = glGetUniformLocation(GprogramID, "Octave8");
    if (octave8Loc != -1)
    {
        glUniform1f(octave8Loc, octavesArray[8]);
    }

	//===================

   static GLfloat vVertices[] = {-1.0f,  1.0f, 0.0f,
                                 -1.0f, -1.0f, 0.0f,
								  1.0f, -1.0f, 0.0f,
								  1.0f, -1.0f, 0.0f,
								  1.0f,  1.0f, 0.0f,
								 -1.0f,  1.0f, 0.0f};
					  
   static GLfloat vColors[] = {1.0f,  0.0f, 0.0f, 1.0f,
								0.0f, 1.0f, 0.0f, 1.0f,
								0.0f, 0.0f,  1.0f, 1.0f,
								0.0f,  0.0f, 1.0f, 1.0f,
								1.0f, 1.0f, 0.0f, 1.0f,
								1.0f, 0.0f,  0.0f, 1.0f};

   static GLfloat vTexCoords[] = {0.0f,  1.0f,
									0.0f, 0.0f,
									1.0f, 0.0f,
									1.0f,  0.0f,
									1.0f, 1.0f,
									0.0f, 1.0f};

   glBindTexture(GL_TEXTURE_2D, GtextureID[0]);

   // Set the viewport
   glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

   // Clear the color buffer
   glClear(GL_COLOR_BUFFER_BIT);

   // Use the program object
   glUseProgram(GprogramID);

   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vVertices);
   glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, vColors);
   glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, vTexCoords);

   glEnableVertexAttribArray(0);
   glEnableVertexAttribArray(1);
   glEnableVertexAttribArray(2);

   glDrawArrays(GL_TRIANGLES, 0, 6);

   glDisableVertexAttribArray(0);
   glDisableVertexAttribArray(1);
   glDisableVertexAttribArray(2);
}

int main(void)
{
  glfwSetErrorCallback(error_callback);

  // Initialize GLFW library
  if (!glfwInit())
    return -1;

  glfwDefaultWindowHints();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

  // Create and open a window
  window = glfwCreateWindow(WINDOW_WIDTH,
                            WINDOW_HEIGHT,
                            "Hello World",
                            NULL,
                            NULL);

  if (!window)
  {
    glfwTerminate();
    printf("glfwCreateWindow Error\n");
    exit(1);
  }

  glfwMakeContextCurrent(window);

  Init();
  initFmod();

  // Repeat
  while (!glfwWindowShouldClose(window)) {
  
    calculateOctaves();
    Draw();
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}
