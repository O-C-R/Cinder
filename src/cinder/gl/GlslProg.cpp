/*
 Copyright (c) 2010, The Barbarian Group
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without modification, are permitted provided that
 the following conditions are met:
 
 * Redistributions of source code must retain the above copyright notice, this list of conditions and
 the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
 the following disclaimer in the documentation and/or other materials provided with the distribution.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 */

#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Context.h"
#include "cinder/gl/ConstantConversions.h"
#include "cinder/gl/Environment.h"
#include "cinder/Log.h"

#include "glm/gtc/type_ptr.hpp"

using namespace std;

namespace cinder { namespace gl {
    
static GlslProg::UniformSemanticMap	sDefaultUniformNameToSemanticMap;
static GlslProg::AttribSemanticMap	sDefaultAttribNameToSemanticMap;
	
class UniformBuffer {
public:
	UniformBuffer( uint32_t bufferSize )
	: mBuffer( new uint8_t[bufferSize] ),
	mBufferSize( bufferSize )
	{
		memset( mBuffer, std::numeric_limits<int>::max(), mBufferSize );
	}
	
	UniformBuffer( const UniformBuffer &other ) = delete;
	UniformBuffer& operator=( const UniformBuffer &rhs ) = delete;
	UniformBuffer( UniformBuffer &&other ) = delete;
	UniformBuffer& operator=( UniformBuffer &&rhs ) = delete;
	
	~UniformBuffer()
	{
		auto ptr = (uint8_t*)mBuffer;
		delete [] ptr;
	}
	
	bool shouldBuffer( uint32_t beginningByte, uint32_t size, const void * valuePointer )
	{
		auto ptr = ((uint8_t*)mBuffer + beginningByte);
		if( memcmp( ptr, valuePointer, size ) == 0 ) {
			return false;
		}
		else {
			memcpy( ptr, valuePointer, size );
			return true;
		}
	}
	
private:
	void*		mBuffer;
	uint32_t	mBufferSize;
};

//////////////////////////////////////////////////////////////////////////
// GlslProg::Format
GlslProg::Format::Format()
#if ! defined( CINDER_GL_ES_2 )
	: mTransformFormat( -1 )
#endif
{
}

GlslProg::Format& GlslProg::Format::vertex( const DataSourceRef &dataSource )
{
	if( dataSource ) {
		Buffer buffer( dataSource );
		mVertexShader.resize( buffer.getDataSize() + 1 );
		memcpy( (void*)mVertexShader.data(), buffer.getData(), buffer.getDataSize() );
		mVertexShader[buffer.getDataSize()] = 0;
	}
	else
		mVertexShader.clear();

	return *this;
}

GlslProg::Format& GlslProg::Format::vertex( const string &vertexShader )
{
	if( ! vertexShader.empty() )
		mVertexShader = vertexShader;
	else
		mVertexShader.clear();

	return *this;
}

GlslProg::Format& GlslProg::Format::fragment( const DataSourceRef &dataSource )
{
	if( dataSource ) {
		Buffer buffer( dataSource );
		mFragmentShader.resize( buffer.getDataSize() + 1 );
		memcpy( (void*)mFragmentShader.data(), buffer.getData(), buffer.getDataSize() );
		mFragmentShader[buffer.getDataSize()] = 0;
	}
	else
		mFragmentShader.clear();
		
	return *this;
}

GlslProg::Format& GlslProg::Format::fragment( const string &fragmentShader )
{
	if( ! fragmentShader.empty() )
		mFragmentShader = fragmentShader;
	else
		mFragmentShader.clear();

	return *this;
}

#if ! defined( CINDER_GL_ES )
GlslProg::Format& GlslProg::Format::geometry( const DataSourceRef &dataSource )
{
	if( dataSource ) {
		Buffer buffer( dataSource );
		mGeometryShader.resize( buffer.getDataSize() + 1 );
		memcpy( (void*)mGeometryShader.data(), buffer.getData(), buffer.getDataSize() );
		mGeometryShader[buffer.getDataSize()] = 0;
	}
	else
		mGeometryShader.clear();
		
	return *this;
}

GlslProg::Format& GlslProg::Format::geometry( const string &geometryShader )
{
	if( ! geometryShader.empty() )
		mGeometryShader = geometryShader;
	else
		mGeometryShader.clear();

	return *this;
}

GlslProg::Format& GlslProg::Format::tessellationCtrl( const DataSourceRef &dataSource )
{
	if( dataSource ) {
		Buffer buffer( dataSource );
		mTessellationCtrlShader.resize( buffer.getDataSize() + 1 );
		memcpy( (void*)mTessellationCtrlShader.data(), buffer.getData(), buffer.getDataSize() );
		mTessellationCtrlShader[buffer.getDataSize()] = 0;
	}
	else
		mTessellationCtrlShader.clear();
	
	return *this;
}

GlslProg::Format& GlslProg::Format::tessellationCtrl( const string &tessellationCtrlShader )
{
	if( ! tessellationCtrlShader.empty() )
		mTessellationCtrlShader = tessellationCtrlShader;
	else
		mTessellationCtrlShader.clear();
	
	return *this;
}

GlslProg::Format& GlslProg::Format::tessellationEval( const DataSourceRef &dataSource )
{
	if( dataSource ) {
		Buffer buffer( dataSource );
		mTessellationEvalShader.resize( buffer.getDataSize() + 1 );
		memcpy( (void*)mTessellationEvalShader.data(), buffer.getData(), buffer.getDataSize() );
		mTessellationEvalShader[buffer.getDataSize()] = 0;
	}
	else
		mTessellationEvalShader.clear();
	
	return *this;
}

GlslProg::Format& GlslProg::Format::tessellationEval( const string &tessellationEvalShader )
{
	if( ! tessellationEvalShader.empty() )
		mTessellationEvalShader = tessellationEvalShader;
	else
		mTessellationEvalShader.clear();
	
	return *this;
}
#endif // ! defined( CINDER_GL_ES )

GlslProg::Format& GlslProg::Format::attrib( geom::Attrib semantic, const std::string &attribName )
{
    bool exists = false;
    for( auto & attrib : mAttributes ) {
        if( attrib.mName == attribName ) {
            attrib.mSemantic = semantic;
            exists = true;
            break;
        }
        else if( attrib.mSemantic == semantic ) {
            attrib.mName = attribName;
            exists = true;
            break;
        }
    }
    if( ! exists ) {
        Attribute attrib;
        attrib.mName = attribName;
        attrib.mSemantic = semantic;
        mAttributes.push_back( attrib );
    }
	return *this;
}

GlslProg::Format& GlslProg::Format::uniform( UniformSemantic semantic, const std::string &uniformName )
{
    bool exists = false;
    for( auto & uniform : mUniforms ) {
        if( uniform.mName == uniformName ) {
            uniform.mSemantic = semantic;
            exists = true;
            break;
        }
        else if( uniform.mSemantic == semantic ) {
            uniform.mName = uniformName;
            exists = true;
            break;
        }
    }
    if( ! exists ) {
        Uniform uniform;
        uniform.mName = uniformName;
        uniform.mSemantic = semantic;
        mUniforms.push_back( uniform );
    }
	return *this;
}

GlslProg::Format& GlslProg::Format::attribLocation( const std::string &attribName, GLint location )
{
    bool exists = false;
    for( auto & attrib : mAttributes ) {
        if( attrib.mName == attribName ) {
            attrib.mLoc = location;
            exists = true;
            break;
        }
        else if( attrib.mLoc == location ) {
            attrib.mName = attribName;
            exists = true;
            break;
        }
    }
    if( ! exists ) {
        Attribute attrib;
        attrib.mName = attribName;
        attrib.mLoc = location;
        mAttributes.push_back( attrib );
    }
	return *this;
}

GlslProg::Format& GlslProg::Format::attribLocation( geom::Attrib attribSemantic, GLint location )
{
    Attribute attrib;
    attrib.mSemantic = attribSemantic;
    attrib.mLoc = location;
    mAttributes.push_back( attrib );
	return *this;
}

#if ! defined( CINDER_GL_ES )
GlslProg::Format& GlslProg::Format::fragDataLocation( GLuint colorNumber, const std::string &name )
{
	mFragDataLocations[name] = colorNumber;
	return *this;
}
#endif

//////////////////////////////////////////////////////////////////////////
// GlslProg statics

GlslProgRef GlslProg::create( const Format &format )
{
	return GlslProgRef( new GlslProg( format ) );
}

#if ! defined( CINDER_GL_ES )
GlslProgRef GlslProg::create( DataSourceRef vertexShader, DataSourceRef fragmentShader, DataSourceRef geometryShader, DataSourceRef tessEvalShader, DataSourceRef tessCtrlShader )
{
	return GlslProgRef( new GlslProg( GlslProg::Format().vertex( vertexShader ).fragment( fragmentShader ).geometry( geometryShader ).tessellationEval( tessEvalShader ).tessellationCtrl( tessCtrlShader ) ) );
}

GlslProgRef GlslProg::create( const string &vertexShader, const string &fragmentShader, const string &geometryShader, const string &tessEvalShader, const string &tessCtrlShader )
{
	return GlslProgRef( new GlslProg( GlslProg::Format().vertex( vertexShader ).fragment( fragmentShader ).geometry( geometryShader ).tessellationEval( tessEvalShader ).tessellationCtrl( tessCtrlShader ) ) );
}
	
#else
GlslProgRef GlslProg::create( DataSourceRef vertexShader, DataSourceRef fragmentShader )
{
	return GlslProgRef( new GlslProg( GlslProg::Format().vertex( vertexShader ).fragment( fragmentShader ) ) );
}

GlslProgRef GlslProg::create( const string &vertexShader, const string &fragmentShader )
{
	return GlslProgRef( new GlslProg( GlslProg::Format().vertex( vertexShader ).fragment( fragmentShader ) ) );
}
	
#endif
	
GlslProg::~GlslProg()
{
	auto ctx = gl::context();
	if( ctx )
		ctx->glslProgDeleted( this );

	if( mHandle ) {
		glDeleteProgram( (GLuint)mHandle );
	}
}

//////////////////////////////////////////////////////////////////////////
// GlslProg

GlslProg::GlslProg( const Format &format )
#if ! defined( CINDER_GL_ES_2 )
: mTransformFeedbackFormat( -1 )
#endif
{
	mHandle = glCreateProgram();
	
	if( ! format.getVertex().empty() )
		loadShader( format.getVertex(), GL_VERTEX_SHADER );
	if( ! format.getFragment().empty() )
		loadShader( format.getFragment(), GL_FRAGMENT_SHADER );
#if ! defined( CINDER_GL_ES )
	if( ! format.getGeometry().empty() )
		loadShader( format.getGeometry(), GL_GEOMETRY_SHADER );
	if( ! format.getTessellationCtrl().empty() )
		loadShader( format.getTessellationCtrl(), GL_TESS_CONTROL_SHADER );
	if( ! format.getTessellationEval().empty() )
		loadShader( format.getTessellationEval(), GL_TESS_EVALUATION_SHADER );
#endif
    
    auto & userDefinedAttribs = format.getAttributes();
	
	// if the user has provided a location make sure to bind that location before
	// we go further, still don't know that this is good.
	for( auto &attrib : userDefinedAttribs )
		if( attrib.mLoc > -1 )
			glBindAttribLocation( mHandle, attrib.mLoc, attrib.mName.c_str() );
	
#if ! defined( CINDER_GL_ES_2 )
	if( ! format.getVaryings().empty() && format.getTransformFormat() > 0 ) {
		auto & userVaryings = format.getVaryings();
		mTransformFeedbackVaryings.resize( format.getVaryings().size() );
		std::vector<const GLchar*> transformFeedbackStarts;
		int i = 0;
		// This is a mess due to an NVidia driver bug on MSW which expects the memory passed to glTransformFeedbackVaryings
		// to still be around after the call. We allocate the storage and put it on the GlslProg itself to be freed at destruction
		for( auto & transformFeedbackVaryings : mTransformFeedbackVaryings ) {
			transformFeedbackVaryings.mName = userVaryings[i++];
			transformFeedbackStarts.push_back( transformFeedbackVaryings.mName.c_str() );
		}
		mTransformFeedbackFormat = format.getTransformFormat();
		glTransformFeedbackVaryings( mHandle, (GLsizei)mTransformFeedbackVaryings.size(), transformFeedbackStarts.data(), mTransformFeedbackFormat );
	}
#endif

#if ! defined( CINDER_GL_ES )
	// setup fragment data locations
	for( const auto &fragDataLocation : format.getFragDataLocations() )
		glBindFragDataLocation( mHandle, fragDataLocation.second, fragDataLocation.first.c_str() );
#endif

	link();
	
	cacheActiveAttribs();
#if ! defined( CINDER_GL_ES_2 )
	if( ! format.getVaryings().empty() ) {
		cacheActiveTransformFeedbackVaryings();
	}
	cacheActiveUniformBlocks();
#endif
	cacheActiveUniforms();
	
	auto & userDefinedUniforms = format.getUniforms();
	// check if the user thinks there's a uniform that isn't active
	for( auto &userUniform : userDefinedUniforms ) {
		bool foundUserDefined = false;
		for( auto & activeUniform : mUniforms ) {
			// if the user defined name and the active name are the same
			if( userUniform.mName == activeUniform.mName ) {
				foundUserDefined = true;
				// If we've found the uniform, change the semantic to the
				// user defined semantic.
				activeUniform.mSemantic = userUniform.mSemantic;
				break;
			}
		}
		if( ! foundUserDefined ) {
			CI_LOG_E( "Unknown uniform: \"" << userUniform.mName << "\"" );
			mLoggedUniforms.insert( userUniform.mName );
		}
	}
	// make sure we get all of the semantic info correct from the user
	for( auto &userAttrib : userDefinedAttribs ) {
		bool active = false;
		for( auto &activeAttrib : mAttributes ) {
			// check if either the user defined name or user defined loc for
			// this attrib are the same as the active attribute
			if( userAttrib.mName == activeAttrib.mName ||
			    userAttrib.mLoc == activeAttrib.mLoc ) {
				// we've found the user defined attribute
				active = true;
				// check if the user defined attribute has cached what type
				// of semantic this is
				if( userAttrib.mSemantic != geom::Attrib::NUM_ATTRIBS ) {
					activeAttrib.mSemantic = userAttrib.mSemantic;
				}
				break;
			}
		}
		if( !active ) {
			CI_LOG_E( "Unknown attribute: \"" << userAttrib.mName << "\"" );
		}
	}
    cout << *this << endl;
    
	setLabel( format.getLabel() );
	gl::context()->glslProgCreated( this );
}

GlslProg::UniformSemanticMap& GlslProg::getDefaultUniformNameToSemanticMap()
{
	static bool initialized = false;
	if( ! initialized ) {
		sDefaultUniformNameToSemanticMap["ciModelMatrix"] = UNIFORM_MODEL_MATRIX;
		sDefaultUniformNameToSemanticMap["ciModelMatrixInverse"] = UNIFORM_MODEL_MATRIX_INVERSE;
		sDefaultUniformNameToSemanticMap["ciModelMatrixInverseTranspose"] = UNIFORM_MODEL_MATRIX_INVERSE_TRANSPOSE;
		sDefaultUniformNameToSemanticMap["ciViewMatrix"] = UNIFORM_VIEW_MATRIX;
		sDefaultUniformNameToSemanticMap["ciViewMatrixInverse"] = UNIFORM_VIEW_MATRIX_INVERSE;
		sDefaultUniformNameToSemanticMap["ciModelView"] = UNIFORM_MODEL_VIEW;
		sDefaultUniformNameToSemanticMap["ciModelViewInverse"] = UNIFORM_MODEL_VIEW_INVERSE;
		sDefaultUniformNameToSemanticMap["ciModelViewInverseTranspose"] = UNIFORM_MODEL_VIEW_INVERSE_TRANSPOSE;
		sDefaultUniformNameToSemanticMap["ciModelViewProjection"] = UNIFORM_MODEL_VIEW_PROJECTION;
		sDefaultUniformNameToSemanticMap["ciModelViewProjectionInverse"] = UNIFORM_MODEL_VIEW_PROJECTION_INVERSE;
		sDefaultUniformNameToSemanticMap["ciProjectionMatrix"] = UNIFORM_PROJECTION_MATRIX;
		sDefaultUniformNameToSemanticMap["ciProjectionMatrixInverse"] = UNIFORM_PROJECTION_MATRIX_INVERSE;
		sDefaultUniformNameToSemanticMap["ciNormalMatrix"] = UNIFORM_NORMAL_MATRIX;
		sDefaultUniformNameToSemanticMap["ciViewportMatrix"] = UNIFORM_VIEWPORT_MATRIX;
		sDefaultUniformNameToSemanticMap["ciWindowSize"] = UNIFORM_WINDOW_SIZE;
		sDefaultUniformNameToSemanticMap["ciElapsedSeconds"] = UNIFORM_ELAPSED_SECONDS;
		initialized = true;
	}
	
	return sDefaultUniformNameToSemanticMap;
}

GlslProg::AttribSemanticMap& GlslProg::getDefaultAttribNameToSemanticMap()
{
	static bool initialized = false;
	if( ! initialized ) {
		sDefaultAttribNameToSemanticMap["ciPosition"] = geom::Attrib::POSITION;
		sDefaultAttribNameToSemanticMap["ciNormal"] = geom::Attrib::NORMAL;
		sDefaultAttribNameToSemanticMap["ciTangent"] = geom::Attrib::TANGENT;
		sDefaultAttribNameToSemanticMap["ciBitangent"] = geom::Attrib::BITANGENT;
		sDefaultAttribNameToSemanticMap["ciTexCoord0"] = geom::Attrib::TEX_COORD_0;
		sDefaultAttribNameToSemanticMap["ciTexCoord1"] = geom::Attrib::TEX_COORD_1;
		sDefaultAttribNameToSemanticMap["ciTexCoord2"] = geom::Attrib::TEX_COORD_2;
		sDefaultAttribNameToSemanticMap["ciTexCoord3"] = geom::Attrib::TEX_COORD_3;
		sDefaultAttribNameToSemanticMap["ciColor"] = geom::Attrib::COLOR;
		sDefaultAttribNameToSemanticMap["ciBoneIndex"] = geom::Attrib::BONE_INDEX;
		sDefaultAttribNameToSemanticMap["ciBoneWeight"] = geom::Attrib::BONE_WEIGHT;
		initialized = true;
	}
	
	return sDefaultAttribNameToSemanticMap;
}

void GlslProg::loadShader( const std::string &shaderSource, GLint shaderType )
{
	GLuint handle = glCreateShader( shaderType );
	const char *cStr = shaderSource.c_str();
	glShaderSource( handle, 1, reinterpret_cast<const GLchar**>( &cStr ), NULL );
	glCompileShader( handle );
	
	GLint status;
	glGetShaderiv( (GLuint) handle, GL_COMPILE_STATUS, &status );
	if( status != GL_TRUE ) {
		std::string log = getShaderLog( (GLuint)handle );
		throw GlslProgCompileExc( log, shaderType );
	}
	glAttachShader( mHandle, handle );
}

void GlslProg::link()
{
	glLinkProgram( mHandle );
	
	// test for a GLSL link error and throw it if we have one
	GLint status;
	glGetProgramiv( mHandle, GL_LINK_STATUS, &status );
	if( status != GL_TRUE ) {
		string log;
		GLint logLength = 0;
		GLint charsWritten = 0;
		glGetProgramiv( mHandle, GL_INFO_LOG_LENGTH, &logLength );
		
		if( logLength > 0 ) {
			unique_ptr<GLchar[]> debugLog( new GLchar[logLength+1] );
			glGetProgramInfoLog( mHandle, logLength, &charsWritten, debugLog.get() );
			log.append( debugLog.get(), 0, logLength );
		}
		
		throw GlslProgLinkExc( log );
	}
}
	
void GlslProg::cacheActiveAttribs()
{
	GLint numActiveAttrs = 0;
	glGetProgramiv( mHandle, GL_ACTIVE_ATTRIBUTES, &numActiveAttrs );
	
	auto & semanticNameMap = getDefaultAttribNameToSemanticMap();
	for( GLint i = 0; i < numActiveAttrs; ++i ) {
		char name[512];
		GLsizei nameLength;
		GLint count;
		GLenum type;
		
		glGetActiveAttrib( mHandle, (GLuint)i, 511, &nameLength, &count, &type, name );
		auto loc = glGetAttribLocation( mHandle, name );
		
		geom::Attrib attributeSemantic = geom::Attrib::NUM_ATTRIBS;
		auto foundSemantic = semanticNameMap.find( name );
		if( foundSemantic != semanticNameMap.end() ) {
			attributeSemantic = foundSemantic->second;
		}
		
		Attribute attrib;
		attrib.mName		= name;
		attrib.mLoc			= loc;
		attrib.mCount		= count;
		attrib.mType		= type;
		attrib.mSemantic	= attributeSemantic;
		mAttributes.push_back( attrib );
	}
}
	
void GlslProg::cacheActiveUniforms()
{
	GLint numActiveUniforms = 0;
	glGetProgramiv( mHandle, GL_ACTIVE_UNIFORMS, &numActiveUniforms );
	
	auto & semanticNameMap = getDefaultUniformNameToSemanticMap();
	
	uint32_t uniformBufferSize = 0;
	
	for( GLint i = 0; i < numActiveUniforms; ++i ) {
		char name[512];
		GLsizei nameLength;
		GLint count;
		GLenum type;
		
		glGetActiveUniform( mHandle, (GLuint)i, 511, &nameLength, &count, &type, name );
		auto loc = glGetUniformLocation( mHandle, name );
		if( loc != -1 ) {
			UniformSemantic uniformSemantic = UniformSemantic::USER_DEFINED_UNIFORM;
			
			auto foundSemantic = semanticNameMap.find( name );
			if( foundSemantic != semanticNameMap.end() ) {
				uniformSemantic = foundSemantic->second;
			}
			
			Uniform uniform;
			uniform.mName		= name;
			uniform.mLoc		= loc;
			uniform.mIndex      = i;
			uniform.mCount		= count;
			uniform.mType		= type;
			uniform.mDataSize	= count * glTypeToBytes( type );
			uniform.mSemantic	= uniformSemantic;
			uniform.mBytePointer = uniformBufferSize;
			uniformBufferSize	+= uniform.mDataSize;
			mUniforms.push_back( uniform );
		}
	}
	
	if ( numActiveUniforms ) {
		mUniformBuffer = new UniformBuffer( uniformBufferSize );
	}
}

#if ! defined( CINDER_GL_ES_2 )
void GlslProg::cacheActiveUniformBlocks()
{
	GLint numActiveUniformBlocks = 0;
	glGetProgramiv( mHandle, GL_ACTIVE_UNIFORM_BLOCKS, &numActiveUniformBlocks );
	
	auto & semanticNameMap = getDefaultUniformNameToSemanticMap();
	for( GLint i = 0; i < numActiveUniformBlocks; i++ ) {
		GLchar name[500];
		GLsizei nameLength;
		GLint blockBinding;
		GLint dataSize;
		GLint numActiveUniforms;
		
		glGetActiveUniformBlockName( mHandle, i, 500, &nameLength, name );
		name[nameLength] = 0;
		
		glGetActiveUniformBlockiv( mHandle, i, GL_UNIFORM_BLOCK_BINDING, &blockBinding );
		glGetActiveUniformBlockiv( mHandle, i, GL_UNIFORM_BLOCK_DATA_SIZE, &dataSize );
		glGetActiveUniformBlockiv( mHandle, i, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &numActiveUniforms );
		
		const GLuint loc = glGetUniformBlockIndex( mHandle, name );
		
		UniformBlock uniformBlock;
		uniformBlock.mName = name;
		uniformBlock.mLoc = loc;
		uniformBlock.mSize = dataSize;
		uniformBlock.mBlockBinding = blockBinding;
		
		std::vector<GLint> uniformIndices( numActiveUniforms );
		glGetActiveUniformBlockiv( mHandle, i, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, uniformIndices.data() );
		std::vector<GLuint> unSignedIndices( numActiveUniforms );
		int j = 0;
		for( auto & index : uniformIndices ) {
			char name[512];
			GLsizei nameLength;
			GLint count;
			GLenum type;
			
			glGetActiveUniform( mHandle, (GLuint)index, 511, &nameLength, &count, &type, name );
			
			UniformSemantic uniformSemantic = UniformSemantic::USER_DEFINED_UNIFORM;
			
			auto foundSemantic = semanticNameMap.find( name );
			if( foundSemantic != semanticNameMap.end() ) {
				uniformSemantic = foundSemantic->second;
			}
			
			Uniform uniform;
			uniform.mName		= name;
			uniform.mLoc		= -1;
			uniform.mIndex      = index;
			uniform.mCount		= count;
			uniform.mType		= type;
			uniform.mSemantic	= uniformSemantic;
			unSignedIndices[j++]= index;
			
			uniformBlock.mActiveUniforms.push_back( uniform );
		}

		std::vector<GLint> uniformOffset( numActiveUniforms );
		glGetActiveUniformsiv( mHandle,
							  numActiveUniforms,
							  unSignedIndices.data(),
							  GL_UNIFORM_OFFSET,
							  uniformOffset.data() );
		std::vector<GLint> uniformArrayStride( numActiveUniforms );
		glGetActiveUniformsiv( mHandle,
							  numActiveUniforms,
							  unSignedIndices.data(),
							  GL_UNIFORM_ARRAY_STRIDE,
							  uniformArrayStride.data() );
		std::vector<GLint> uniformMatrixStride( numActiveUniforms );
		glGetActiveUniformsiv( mHandle,
							  numActiveUniforms,
							  unSignedIndices.data(),
							  GL_UNIFORM_MATRIX_STRIDE,
							  uniformMatrixStride.data() );
		
		uniformBlock.mActiveUniformInfo.insert( make_pair( GL_UNIFORM_OFFSET, uniformOffset ) );
		uniformBlock.mActiveUniformInfo.insert( make_pair( GL_UNIFORM_ARRAY_STRIDE, uniformArrayStride ) );
		uniformBlock.mActiveUniformInfo.insert( make_pair( GL_UNIFORM_MATRIX_STRIDE, uniformMatrixStride ) );
		
		mUniformBlocks.push_back( uniformBlock );
	}
}

void GlslProg::cacheActiveTransformFeedbackVaryings()
{
	GLint numActiveTransformFeedbackVaryings;
	glGetProgramiv( mHandle, GL_TRANSFORM_FEEDBACK_VARYINGS, &numActiveTransformFeedbackVaryings );
	
	if( numActiveTransformFeedbackVaryings > 0 ) {
		for( int i = 0; i < numActiveTransformFeedbackVaryings; i++ ) {
			GLchar name[500];
			GLint length;
			GLsizei count;
			GLenum type;
			
			glGetTransformFeedbackVarying( mHandle, i, 500, &length, &count, &type, name );
			name[length] = 0;
			
			auto varying = findTransformFeedbackVaryings( name );
			if( varying ) {
				varying->mCount = count;
				varying->mType = type;
			}
		}
		
		// TODO: This needs to be created correctly and checked, this is where the magic will happen.
//		static const std::string nextComponent = "glNextComponent";
//		static const std::string nextBuffer = "glNextBuffer";
//		
//		// This isn't pretty but it will work to tell us about what the attributes are
//		if( mTransformFeedbackFormat == GL_INTERLEAVED_ATTRIBS ) {
//			for( auto & varying : mTransformFeedbackVaryings ) {
//				// if the count is zero we must have reached a special variable, i.e. gl_Ne
//				if( varying.mCount == 0 ) {
//					if ( ! varying.mName.compare( 0, nextComponent.size(), nextComponent ) ) {
//						auto dimStr = varying.mName.substr( nextComponent.size(), 1 );
//						auto dimInt = std::stoi( dimStr );
//						
//					}
//					else if( varying.mName.compare( 0, nextBuffer.size(), nextBuffer ) ) {
//						
//					}
//				}
//			}
//		}
	}
}
#endif // ! defined( CINDER_GL_ES_2 )

void GlslProg::bind() const
{
	// this will in turn call bindImpl; this is so that the context can update its reference to the active shader
	gl::context()->bindGlslProg( std::const_pointer_cast<GlslProg>( shared_from_this() ) );
}

// This is called by the Context whenever a GlslProg is bound. The indirection is so that the Context can update its reference to the active shader
void GlslProg::bindImpl()
{
	glUseProgram( mHandle );
}

std::string GlslProg::getShaderLog( GLuint handle ) const
{
	string log;
	
	GLchar* debugLog;
	GLint charsWritten	= 0;
	GLint debugLength	= 0;
	glGetShaderiv( handle, GL_INFO_LOG_LENGTH, &debugLength );
	
	if ( debugLength > 0 ) {
		debugLog = new GLchar[ debugLength ];
		glGetShaderInfoLog( handle, debugLength, &charsWritten, debugLog );
		log.append( debugLog, 0, debugLength );
		delete [] debugLog;
	}
	
	return log;
}
	
void GlslProg::logMissingUniform( const std::string &name ) const
{
	if( mLoggedUniforms.count( name ) == 0 ) {
		CI_LOG_E( "Unknown uniform: \"" << name << "\"" );
		mLoggedUniforms.insert( name );
	}
}
	
void GlslProg::logUniformWrongType( const std::string &name, GLenum uniformType, const std::string &userType ) const
{
	if( mLoggedUniforms.count( name ) == 0 ) {
		CI_LOG_W("Uniform type mismatch for \"" << name << "\", expected "
				 << gl::constantToString(uniformType) << " and received a " << userType << ".");
		mLoggedUniforms.insert( name );
	}
}
	
void GlslProg::setLabel( const std::string &label )
{
	mLabel = label;
#if defined( CINDER_GL_ES )
#if ! defined( CINDER_GL_ANGLE )
	env()->objectLabel( GL_PROGRAM_OBJECT_EXT, mHandle, (GLsizei)label.size(), label.c_str() );
#endif
#else
	env()->objectLabel( GL_PROGRAM, mHandle, (GLsizei)label.size(), label.c_str() );
#endif
}

GLint GlslProg::getUniformLocation( const std::string &name ) const
{
	auto found = findUniform( name );
	if( found )
		return found->mLoc;
	else
		return  -1;
}
	
GlslProg::Attribute* GlslProg::findAttrib( const std::string &name )
{
	Attribute* ret = nullptr;
	for( auto & attrib : mAttributes ) {
		if( attrib.mName == name ) {
			ret = &attrib;
			break;
		}
	}
	return ret;
}

const GlslProg::Attribute* GlslProg::findAttrib( const std::string &name ) const
{
	const Attribute* ret = nullptr;
	for( auto & attrib : mAttributes ) {
		if( attrib.mName == name ) {
			ret = &attrib;
			break;
		}
	}
	return ret;
}
	
GlslProg::Uniform* GlslProg::findUniform( const std::string &name )
{
	Uniform* ret = nullptr;
	for( auto & uniform : mUniforms ) {
		if( uniform.mName == name ) {
			ret = &uniform;
			break;
		}
	}
	return ret;
}
	
const GlslProg::Uniform* GlslProg::findUniform( const std::string &name ) const
{
	const Uniform* ret = nullptr;
	for( const auto & uniform : mUniforms ) {
		if( uniform.mName == name ) {
			ret = &uniform;
			break;
		}
	}
	return ret;
}
    
bool GlslProg::hasAttribSemantic( geom::Attrib semantic ) const
{
    return find_if( mAttributes.begin(),
                   mAttributes.end(),
                   [ semantic ]( const Attribute & attrib ){
                       return attrib.mSemantic == semantic;
                   }) != mAttributes.end();
}
    
GLint GlslProg::getAttribSemanticLocation( geom::Attrib semantic ) const
{
    auto found = find_if( mAttributes.begin(),
                         mAttributes.end(),
                         [ semantic ]( const Attribute &attrib ){
                             return attrib.mSemantic == semantic;
                         });
    if( found != mAttributes.end() )
        return found->mLoc;
    else
        return -1;
}
    
GLint GlslProg::getAttribLocation( const std::string &name ) const
{
	auto found = findAttrib( name );
    if( found )
        return found->mLoc;
    else
        return -1;
}
    
#if ! defined( CINDER_GL_ES_2 )

GlslProg::UniformBlock* GlslProg::findUniformBlock( const std::string &name )
{
	UniformBlock* ret = nullptr;
	for( auto & uniformBlock : mUniformBlocks ) {
		if( uniformBlock.mName == name ) {
			ret = &uniformBlock;
			break;
		}
	}
	return ret;
}
	
const GlslProg::UniformBlock* GlslProg::findUniformBlock( const std::string &name ) const
{
	const UniformBlock* ret = nullptr;
	for( auto & uniformBlock : mUniformBlocks ) {
		if( uniformBlock.mName == name ) {
			ret = &uniformBlock;
			break;
		}
	}
	return ret;
}

void GlslProg::uniformBlock( int loc, int binding )
{
	auto found = find_if( mUniformBlocks.begin(),
						 mUniformBlocks.end(),
						 [=]( const UniformBlock &block ){
							 return block.mLoc == loc;
						 });
	if( found != mUniformBlocks.end() ) {
		if( found->mBlockBinding != binding ) {
			found->mBlockBinding = binding;
			glUniformBlockBinding( mHandle, found->mLoc, binding );
		}
	}
	else {
		CI_LOG_E("Uniform block at " << loc << " location not found");
	}
}

void GlslProg::uniformBlock( const std::string &name, GLint binding )
{
	auto found = findUniformBlock( name );
	if( found ) {
		if( found->mBlockBinding != binding ) {
			found->mBlockBinding = binding;
			glUniformBlockBinding( mHandle, found->mLoc, binding );
		}
	}
	else {
		CI_LOG_E("Uniform block \"" << name << "\" not found");
	}
}

GLint GlslProg::getUniformBlockLocation( const std::string &name ) const
{
	auto found = findUniformBlock( name );
	if( found )
		return found->mLoc;
	else
		return -1;
}

GLint GlslProg::getUniformBlockSize( GLint blockBinding ) const
{
	auto found = find_if( mUniformBlocks.begin(),
						 mUniformBlocks.end(),
						 [=]( const UniformBlock &block ){
							 return block.mBlockBinding == blockBinding;
						 });
	if( found != mUniformBlocks.end() )
		return found->mSize;
	else
		return -1;
}
	
const GlslProg::TransformFeedbackVaryings* GlslProg::findTransformFeedbackVaryings( const std::string &name ) const
{
	const TransformFeedbackVaryings* ret = nullptr;
	for( const auto & varying : mTransformFeedbackVaryings ) {
		if( varying.mName == name ) {
			ret = &varying;
		}
	}
	return ret;
}
	
GlslProg::TransformFeedbackVaryings* GlslProg::findTransformFeedbackVaryings( const std::string &name )
{
	TransformFeedbackVaryings* ret = nullptr;
	for( auto & varying : mTransformFeedbackVaryings ) {
		if( varying.mName == name ) {
			ret = &varying;
		}
	}
	return ret;
}
#endif // ! defined( CINDER_GL_ES_2 )
	
bool GlslProg::checkUniformValue( const Uniform &uniform, const void *val, int count ) const
{
	if( mUniformBuffer ) {
		return mUniformBuffer->shouldBuffer( uniform.mBytePointer, glTypeToBytes( uniform.mType ) * count, val );
	}
	else {
		return false;
	}
}

// bool
void GlslProg::uniform( int location, bool data ) const
{
	ScopedGlslProg shaderBind( shared_from_this() );
	glUniform1i( location, data );
}
#if ! defined( CINDER_GL_ES_2 )
// uint32_t
void GlslProg::uniform( int location, uint32_t data ) const
{
	ScopedGlslProg shaderBind( shared_from_this() );
	glUniform1ui( location, data );
}
	
// uvec2
void GlslProg::uniform( int location, const uvec2 &data ) const
{
	ScopedGlslProg shaderBind( shared_from_this() );
	glUniform2ui( location, data.x, data.y );
}

// uvec3
void GlslProg::uniform( int location, const uvec3 &data ) const
{
	ScopedGlslProg shaderBind( shared_from_this() );
	glUniform3ui( location, data.x, data.y, data.z );
}
	
// uvec4
void GlslProg::uniform( int location, const uvec4 &data ) const
{
	ScopedGlslProg shaderBind( shared_from_this() );
	glUniform4ui( location, data.x, data.y, data.z, data.w );
}
#endif
	
// int
void GlslProg::uniform( int location, int data ) const
{
    ScopedGlslProg shaderBind( shared_from_this() );
    glUniform1i( location, data );
}

// ivec2
void GlslProg::uniform( int location, const ivec2 &data ) const
{
    ScopedGlslProg shaderBind( shared_from_this() );
    glUniform2i( location, data.x, data.y );
}

// int *, count
void GlslProg::uniform( int location, const int *data, int count ) const
{
    ScopedGlslProg shaderBind( shared_from_this() );
    glUniform1iv( location, count, data );
}

// ivec2 *, count
void GlslProg::uniform( int location, const ivec2 *data, int count ) const
{
    ScopedGlslProg shaderBind( shared_from_this() );
    glUniform2iv( location, count, &data[0].x );
}

// float
void GlslProg::uniform( int location, float data ) const
{
    ScopedGlslProg shaderBind( shared_from_this() );
    glUniform1f( location, data );
}

// vec2
void GlslProg::uniform( int location, const vec2 &data ) const
{
    ScopedGlslProg shaderBind( shared_from_this() );
    glUniform2f( location, data.x, data.y );
}

// vec3
void GlslProg::uniform( int location, const vec3 &data ) const
{
    ScopedGlslProg shaderBind( shared_from_this() );
    glUniform3f( location, data.x, data.y, data.z );
}

// vec4
void GlslProg::uniform( int location, const vec4 &data ) const
{
    ScopedGlslProg shaderBind( shared_from_this() );
    glUniform4f( location, data.x, data.y, data.z, data.w );
}

// mat3
void GlslProg::uniform( int location, const mat3 &data, bool transpose ) const
{
    ScopedGlslProg shaderBind( shared_from_this() );
    glUniformMatrix3fv( location, 1, ( transpose ) ? GL_TRUE : GL_FALSE, glm::value_ptr( data ) );
}

// mat4
void GlslProg::uniform( int location, const mat4 &data, bool transpose ) const
{
    ScopedGlslProg shaderBind( shared_from_this() );
    glUniformMatrix4fv( location, 1, ( transpose ) ? GL_TRUE : GL_FALSE, glm::value_ptr( data ) );
}

// Color
void GlslProg::uniform( int location, const Color &data ) const
{
    ScopedGlslProg shaderBind( shared_from_this() );
    glUniform3f( location, data.r, data.g, data.b );
}

// ColorA
void GlslProg::uniform( int location, const ColorA &data ) const
{
    ScopedGlslProg shaderBind( shared_from_this() );
    glUniform4f( location, data.r, data.g, data.b, data.a );
}

// float*, count
void GlslProg::uniform( int location, const float *data, int count ) const
{
    ScopedGlslProg shaderBind( shared_from_this() );
    glUniform1fv( location, count, data );
}

// vec2*, count
void GlslProg::uniform( int location, const vec2 *data, int count ) const
{
    ScopedGlslProg shaderBind( shared_from_this() );
    glUniform2fv( location, count, &data[0].x );
}

// vec3*, count
void GlslProg::uniform( int location, const vec3 *data, int count ) const
{
    ScopedGlslProg shaderBind( shared_from_this() );
    glUniform3fv( location, count, &data[0].x );
}

// vec4*, count
void GlslProg::uniform( int location, const vec4 *data, int count ) const
{
    ScopedGlslProg shaderBind( shared_from_this() );
    glUniform4fv( location, count, &data[0].x );
}

// mat3*, count
void GlslProg::uniform( int location, const mat3 *data, int count, bool transpose ) const
{
    ScopedGlslProg shaderBind( shared_from_this() );
    glUniformMatrix3fv( location, count, ( transpose ) ? GL_TRUE : GL_FALSE, glm::value_ptr( *data ) );
}

// mat4*, count
void GlslProg::uniform( int location, const mat4 *data, int count, bool transpose ) const
{
    ScopedGlslProg shaderBind( shared_from_this() );
    glUniformMatrix4fv( location, count, ( transpose ) ? GL_TRUE : GL_FALSE, glm::value_ptr( *data ) );
}

std::ostream& operator<<( std::ostream &os, const GlslProg &rhs )
{
	os << "ID: " << rhs.mHandle << std::endl;
	if( ! rhs.mLabel.empty() )
		os << "    Label: " << rhs.mLabel << std::endl;
	
	auto attribs = rhs.getActiveAttributes();
	os << "\tAttributes: " << std::endl;
	for( auto &attrib : attribs ) {
		os << "\t\t\"" << attrib.mName << "\":" << std::endl;
		os << "\t\t Loc: " << attrib.mLoc << std::endl;
		os << "\t\t Type: " << gl::constantToString( attrib.mType ) << std::endl;
		os << "\t\t Semantic: <" << geom::attribToString( attrib.mSemantic ) << ">" << std::endl;
	}
	
	os << "\tUniforms: " << std::endl;
	auto & uniforms = rhs.getActiveUniforms();
	for( auto &uniform : uniforms ) {
		os << "\t\t\"" << uniform.mName << "\":" << std::endl;
		os << "\t\t Loc: " << uniform.mLoc << std::endl;
		os << "\t\t Type: " << gl::constantToString( uniform.mType ) << std::endl;
		os << "\t\t Semantic: <" << gl::uniformSemanticToString( uniform.mSemantic ) << ">" << std::endl;
	}
	
#if ! defined( CINDER_GL_ES_2 )
	os << "\tUniform Blocks: " << std::endl;
	auto & uniformBlocks = rhs.getActiveUniformBlocks();
	for( auto & uniformBlock : uniformBlocks ) {
		os << "\t\t\"" << uniformBlock.mName << "\":" << std::endl;
		os << "\t\t Loc: " << uniformBlock.mLoc << std::endl;
		os << "\t\t Size: " << uniformBlock.mSize << std::endl;
		os << "\t\t BlockBinding: " << uniformBlock.mBlockBinding << std::endl;
		os << "\t\t Active Uniforms: " << endl;
		for( auto & uniform : uniformBlock.mActiveUniforms ) {
			os << "\t\t\t\"" << uniform.mName << "\":" << std::endl;
			os << "\t\t\t Loc: " << uniform.mLoc << std::endl;
			os << "\t\t\t Type: " << gl::constantToString( uniform.mType ) << std::endl;
			os << "\t\t\t Semantic: <" << gl::uniformSemanticToString( uniform.mSemantic ) << ">" << std::endl;
		}
	}
	
	os << "\tTransform Feedback Varyings: " << std::endl;
	auto & feedbackVaryings = rhs.getActiveTransformFeedbackVaryings();
	for( auto & varying : feedbackVaryings ) {
		os << "\t\t\"" << varying.mName << "\":" << std::endl;
		os << "\t\t Type: " << gl::constantToString( varying.mType ) << std::endl;
		os << "\t\t Count: " << varying.mCount << std::endl;
	}
#endif
	return os;
}

//////////////////////////////////////////////////////////////////////////
// GlslProgCompileExc
GlslProgCompileExc::GlslProgCompileExc( const std::string &log, GLint shaderType )
{
	string typeString;

	switch( shaderType ) {
		case GL_VERTEX_SHADER:			typeString = "VERTEX: "; break;
		case GL_FRAGMENT_SHADER:		typeString = "FRAGMENT: "; break;
#if ! defined( CINDER_GL_ES )
		case GL_GEOMETRY_SHADER:		typeString = "GEOMETRY: "; break;
		case GL_TESS_CONTROL_SHADER:	typeString = "TESSELLATION CONTROL: "; break;
		case GL_TESS_EVALUATION_SHADER:	typeString = "TESSELLATION EVALUATION: "; break;
#endif
		default:						typeString = "UNKNOWN: ";
	}

	setDescription( typeString + log );
}
	
} } // namespace cinder::gl