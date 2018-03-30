#include <Common/Base.h>

#include <Renderer/Renderer.h>
#include <Renderer/Shader.h>
#include <Common/FileIO.h>

#define _USE_MATH_DEFINES
#include <math.h>
#include <vector>
#include <sstream>
#include <map>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

typedef Renderer::Line Line;
typedef Renderer::Text Text;

inline unsigned int SetRGBA( unsigned char r, unsigned char g, unsigned char b, unsigned char a )
{
	return ( r ) | ( g << 8 ) | ( b << 16 ) | ( a << 24 );
}

GLuint LoadShaders( const char* vertexShaderPath, const char* fragmentShaderPath )
{
	std::string vertShaderSrcStr;
	Assert( FileIO::loadFileStream( vertexShaderPath, vertShaderSrcStr ) == 0, "Failed to read shader" );

	std::string fragShaderSrcStr;
	Assert( FileIO::loadFileStream( fragmentShaderPath, fragShaderSrcStr ) == 0, "Failed to read shader" );

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile vertex shader
	printf( "Compiling vertex shader: %s\n", vertexShaderPath );
	const char* vertShaderSrcPtr = vertShaderSrcStr.c_str();
	GLuint vertShaderId = glCreateShader( GL_VERTEX_SHADER );
	glShaderSource( vertShaderId, 1, &vertShaderSrcPtr, nullptr );
	glCompileShader( vertShaderId );

	glGetShaderiv( vertShaderId, GL_COMPILE_STATUS, &Result );
	glGetShaderiv( vertShaderId, GL_INFO_LOG_LENGTH, &InfoLogLength );
	if ( InfoLogLength > 0 )
	{
		std::vector<char> VertexShaderErrorMessage( InfoLogLength + 1 );
		glGetShaderInfoLog( vertShaderId, InfoLogLength, nullptr, &VertexShaderErrorMessage[0] );
		printf( "%s\n", &VertexShaderErrorMessage[0] );
	}

	// Compile fragment shader
	printf( "Compiling fragment shader: %s\n", fragmentShaderPath );
	const char* fragShaderSrcPtr = fragShaderSrcStr.c_str();
	GLuint fragShaderId = glCreateShader( GL_FRAGMENT_SHADER );
	glShaderSource( fragShaderId, 1, &fragShaderSrcPtr, nullptr );
	glCompileShader( fragShaderId );

	glGetShaderiv( fragShaderId, GL_COMPILE_STATUS, &Result );
	glGetShaderiv( fragShaderId, GL_INFO_LOG_LENGTH, &InfoLogLength );
	if ( InfoLogLength > 0 )
	{
		std::vector<char> FragmentShaderErrorMessage( InfoLogLength + 1 );
		glGetShaderInfoLog( fragShaderId, InfoLogLength, nullptr, &FragmentShaderErrorMessage[0] );
		printf( "%s\n", &FragmentShaderErrorMessage[0] );
	}

	// Link vertex and fragment shaders
	printf( "Linking program\n" );
	GLuint programId = glCreateProgram();
	glAttachShader( programId, vertShaderId );
	glAttachShader( programId, fragShaderId );
	glLinkProgram( programId );

	glGetProgramiv( programId, GL_LINK_STATUS, &Result );
	glGetProgramiv( programId, GL_INFO_LOG_LENGTH, &InfoLogLength );
	if ( InfoLogLength > 0 )
	{
		std::vector<char> ProgramErrorMessage( InfoLogLength + 1 );
		glGetProgramInfoLog( programId, InfoLogLength, nullptr, &ProgramErrorMessage[0] );
		printf( "%s\n", &ProgramErrorMessage[0] );
	}

	glDetachShader( programId, vertShaderId );
	glDetachShader( programId, fragShaderId );

	glDeleteShader( vertShaderId );
	glDeleteShader( fragShaderId );

	return programId;
}

struct Character
{
	GLuint m_textureId;
	Vector4 m_size;
	Vector4 m_bearing;
	FT_Pos m_advance;
};
std::map<GLchar, Character> Characters;

int initializeFreeType()
{
	FT_Library library = nullptr;
	Assert( !FT_Init_FreeType( &library ), "failed to initialize freetype" );

	FT_Face face = nullptr;
	Assert( !FT_New_Face( library, "C:\\Windows\\Fonts\\consola.ttf", 0, &face ),
			"failed to load font face" );

	Assert( !FT_Set_Pixel_Sizes( face, 0, 16 ), "failed to define font size" );

	// Pre-load characters
	// Rendering technique taken from
	// https://learnopengl.com/#!In-Practice/Text-Rendering

	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

	for ( GLubyte c = 0; c < 128; c++ )
	{
		Assert( !FT_Load_Char( face, c, FT_LOAD_RENDER ),
				"failed to load character from font face" );

		GLuint texture;
		glGenTextures( 1, &texture );
		glBindTexture( GL_TEXTURE_2D, texture );
		glTexImage2D( GL_TEXTURE_2D,
					  0,
					  GL_RED,
					  face->glyph->bitmap.width,
					  face->glyph->bitmap.rows,
					  0,
					  GL_RED,
					  GL_UNSIGNED_BYTE,
					  face->glyph->bitmap.buffer);

		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

		Character character =
		{
			texture, 
			Vector4( (Real)face->glyph->bitmap.width, (Real)face->glyph->bitmap.rows ),
			Vector4( (Real)face->glyph->bitmap_left, (Real)face->glyph->bitmap_top ),
			face->glyph->advance.x
		};

		Characters.insert( std::pair<GLchar, Character>( c, character ) );
	}

	FT_Done_Face( face );
	FT_Done_FreeType( library );

	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	glGenVertexArrays( 1, &m_textVAO );
	glGenBuffers( 1, &m_textVBO );
	glBindVertexArray( m_textVAO );
	glBindBuffer( GL_ARRAY_BUFFER, m_textVBO );
	glBufferData( GL_ARRAY_BUFFER, sizeof( GLfloat ) * 6 * 4, nullptr, GL_DYNAMIC_DRAW );
	glEnableVertexAttribArray(0);
	glVertexAttribPointer( 0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof( GL_FLOAT ), 0 );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindVertexArray( 0 );

	return 0;
}

Renderer::Renderer()
{

}

int Renderer::initRenderer( int width, int height )
{
	GLenum err = glewInit();
	Assert( err == GLEW_OK, "failed to initialize glew" );

	m_left = -width * 0.5;
	m_right = width * 0.8;
	m_bottom = -height * 0.5;
	m_top = height * 0.8;
	m_projection = glm::ortho( ( float )m_left, ( float )m_right, ( float )m_bottom, ( float )m_top );

	m_lineShader = Shader( "../Renderer/VertexShader.shader", "../Renderer/FragmentShader.shader" );

	glGenVertexArrays( 1, &m_lineVAO );
	glGenBuffers( 2, m_lineVBO );
	glBindVertexArray( m_lineVAO );

	glBindBuffer( GL_ARRAY_BUFFER, m_lineVBO[0] );
	glBufferData( GL_ARRAY_BUFFER, sizeof( GL_FLOAT ) * 4, nullptr, GL_DYNAMIC_DRAW );
	glEnableVertexAttribArray( 0 );
	glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 0, nullptr );
	
	glBindBuffer( GL_ARRAY_BUFFER, m_lineVBO[1] );
	glBufferData( GL_ARRAY_BUFFER, sizeof( GL_FLOAT ) * 8, nullptr, GL_DYNAMIC_DRAW );
	glEnableVertexAttribArray( 1 );
	glVertexAttribPointer( 1, 4, GL_FLOAT, GL_FALSE, 0, nullptr );

	glBindVertexArray( 0 );

	m_textShader = Shader( "../Renderer/TextVertexShader.shader", "../Renderer/TextFragmentShader.shader" );

	//initializeFreeType();

	return 0;
}

int Renderer::stepRenderer()
{
	for ( auto i = 0; i < m_renderLines.size(); i++ )
	{
		renderLine( m_renderLines[i] );
	}
	m_renderLines.clear();

	for ( auto i = 0; i < m_renderTexts.size(); i++ )
	{
		renderText( m_renderTexts[i] );
	}
	m_renderTexts.clear();

	return 0;
}

Line::Line( const Vector4& pointA, const Vector4& pointB, unsigned int color )
	: m_pointA(pointA), m_pointB(pointB), m_color(color)
{

}

void Renderer::renderLine( const Line& line ) const
{
	GLfloat vertices[] =
	{
		line.m_pointA( 0 ), line.m_pointA( 1 ), line.m_pointB( 0 ), line.m_pointB( 1 )
	};

	GLfloat color[] =
	{
		( GLfloat ) ( line.m_color & 0xff ) / 255.f,
		( GLfloat ) ( ( line.m_color >> 8 ) & 0xff ) / 255.f,
		( GLfloat ) ( ( line.m_color >> 16 ) & 0xff ) / 255.f,
		( GLfloat ) ( ( line.m_color >> 24 ) & 0xff ) / 255.f,
		( GLfloat ) ( line.m_color & 0xff ) / 255.f,
		( GLfloat ) ( ( line.m_color >> 8 ) & 0xff ) / 255.f,
		( GLfloat ) ( ( line.m_color >> 16 ) & 0xff ) / 255.f,
		( GLfloat ) ( ( line.m_color >> 24 ) & 0xff ) / 255.f
	};

	m_lineShader.use();

	m_lineShader.setMat4( "projection", m_projection );

	glBindVertexArray( m_lineVAO );
	glBindBuffer( GL_ARRAY_BUFFER, m_lineVBO[0] );
	glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof( vertices ), vertices );
	glBindBuffer( GL_ARRAY_BUFFER, m_lineVBO[1] );
	glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof( color ), color );
	glDrawArrays( GL_LINE_STRIP, 0, 2 );
	glBindVertexArray( 0 );
}

void Renderer::getDimensions( float & left, float & right, float & bottom, float & top )
{
	left = m_left;
	right = m_right;
	bottom = m_bottom;
	top = m_top;
}

void Renderer::drawLine( const Vector4& pa, const Vector4& pb, unsigned int color )
{
	m_renderLines.push_back( Line( pa, pb, color ) );
}

void Renderer::drawCross( const Vector4& pos, const Real rot, const Real len, unsigned int color )
{
	Vector4 needle( len / 2.f, 0.f );

	Vector4 a, b, c, d;

	a = pos + needle.getRotatedDir( rot );
	b = pos + needle.getRotatedDir( rot + 90.f * g_degToRad );
	c = pos + needle.getRotatedDir( rot + 180.f * g_degToRad );
	d = pos + needle.getRotatedDir( rot + 270.f * g_degToRad );

	drawLine( a, c, color );
	drawLine( b, d, color );
}

void Renderer::drawArrow( const Vector4& pos, const Vector4& dir, unsigned int color )
{
	Vector4 dst; dst.setAdd( pos, dir );
	drawLine( pos, dst, color );

	Vector4 dirFrac = dir * .5f;
	Vector4 headLeft = dirFrac.getRotatedDir( 135.f * g_degToRad );
	Vector4 headRight = dirFrac.getRotatedDir( -135.f * g_degToRad );
	Vector4 dstToHeadLeft = dst + headLeft;
	Vector4 dstToHeadRight = dst + headRight;

	drawLine( dst, dstToHeadLeft, color );
	drawLine( dst, dstToHeadRight, color );
}

void Renderer::drawBox( const Vector4& max, const Vector4& min, unsigned int color )
{
	Vector4 upperLeft( min( 0 ), max( 1 ) );
	Vector4 bottomRight( max( 0 ), min( 1 ) );

	drawLine( max, upperLeft, color );
	drawLine( max, bottomRight, color );
	drawLine( min, upperLeft, color );
	drawLine( min, bottomRight, color );
}

void Renderer::drawCircle( const Vector4& pos, const Real radius, unsigned int color )
{
	Real step = 2 * (Real)M_PI * STEP_RENDER_CIRCLE;
	Real full = ( 2.f + STEP_RENDER_CIRCLE ) * M_PI;

	for ( Real i = step; i < full; i += step )
	{
		Vector4 na, nb;
		na.set( radius * cos( i ), radius * sin( i ) );
		nb.set( radius * cos( i + step ), radius * sin( i + step ) );
		drawLine( pos + na, pos + nb );
	}
}

Text::Text( const std::string& str, const Vector4& pos, const Real scale, unsigned int color )
	: m_str( str ), m_pos( pos ), m_scale( scale ), m_color( color )
{

}

void Renderer::renderText( const Text& text ) const
{
	m_textShader.use();

	m_textShader.setMat4( "projection", m_projection );

	m_textShader.setVec3(
		"textColor",
		( GLfloat ) ( text.m_color & 0xff ) / 255.f,
		( GLfloat ) ( text.m_color >> 8 & 0xff ) / 255.f,
		( GLfloat ) ( text.m_color >> 16 & 0xff ) / 255.f );

    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(m_textVAO);

	Real x = text.m_pos( 0 );
	Real y = text.m_pos( 1 );

    // Iterate through all characters
    std::string::const_iterator c;
    for (c = text.m_str.begin(); c != text.m_str.end(); c++)
	{
		Character ch = Characters[*c];

		GLfloat xpos = x + ch.m_bearing( 0 ) * text.m_scale;
		GLfloat ypos = y - ( ch.m_size( 1 ) - ch.m_bearing( 1 ) ) * text.m_scale;

		GLfloat w = ch.m_size( 0 ) * text.m_scale;
		GLfloat h = ch.m_size( 1 ) * text.m_scale;
        // Update VBO for each character
        GLfloat vertices[6][4] = {
            { xpos,     ypos + h,   0.0, 0.0 },
            { xpos,     ypos,       0.0, 1.0 },
            { xpos + w, ypos,       1.0, 1.0 },

            { xpos,     ypos + h,   0.0, 0.0 },
            { xpos + w, ypos,       1.0, 1.0 },
            { xpos + w, ypos + h,   1.0, 0.0 }
        };
        // Render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.m_textureId);

        // Update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, m_textVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); 
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // Render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.m_advance >> 6) * text.m_scale; // Bitshift by 6 to get value in pixels (2^6 = 64)
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderer::drawText( const std::string& str, const Vector4& pos, const Real scale, unsigned int color )
{
	m_renderTexts.push_back( Text( str, pos, scale, color ) );
}
