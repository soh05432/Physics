#include <Renderer/Renderer.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

int main( int argc, char* argv[] )
{
	GLFWwindow* window;

	if ( !glfwInit() )
	{
		return -1;
	}

	int width = 1024;
	int height = 768;

	window = glfwCreateWindow( width, height, "FreetypeTest", nullptr, nullptr );

	Assert( window, "glfwCreateWindow failed" );

	glfwMakeContextCurrent( window );

	Renderer renderer;
	initRenderer( width, height, renderCtx );

	while ( !glfwWindowShouldClose( window ) )
	{
		glfwPollEvents();

		glClearColor( 1.0f, 1.0f, 1.0f, 1.0f );
		glClear( GL_COLOR_BUFFER_BIT );
		glColor3f( 0.0f, 0.0f, 0.0f );

		drawBox( Vector4( 100.f, 100.f ), Vector4( 300.f, 300.f ) );

		drawText( "bbbbbbb", Vector4( 25.f, 25.f ), BLUE );

		stepRenderer();

		glfwSwapBuffers( window );
	}

	glfwDestroyWindow( window );

	glfwTerminate();

	exit( EXIT_SUCCESS );

	return 0;
}