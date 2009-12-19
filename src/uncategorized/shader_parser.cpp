#include <iomanip>
#include "shader_parser.h"
#include "scanner.h"
#include "parser.h"


//=====================================================================================================================================
// FindShaderVar                                                                                                                      =
//=====================================================================================================================================
void shader_parser_t::PrintSourceLines() const
{
	for( uint i=0; i<source_lines.size(); ++i )
	{
		PRINT( setw(3) << i+1 << ": " << source_lines[i] );
	}
}


//=====================================================================================================================================
// PrintShaderVars                                                                                                                    =
//=====================================================================================================================================
void shader_parser_t::PrintShaderVars() const
{
	PRINT( "TYPE" << setw(20) << "NAME" << setw(4) << "LOC" );
	for( uint i=0; i<uniforms.size(); ++i )
	{
		PRINT( setw(4) << "U" << setw(20) << uniforms[i].name << setw(4) << uniforms[i].custom_loc );
	}
	for( uint i=0; i<attributes.size(); ++i )
	{
		PRINT( setw(4) << "A" << setw(20) << attributes[i].name << setw(4) << attributes[i].custom_loc );
	}
}


//=====================================================================================================================================
// FindShaderVar                                                                                                                      =
//=====================================================================================================================================
vec_t<shader_parser_t::shader_var_t>::iterator shader_parser_t::FindShaderVar( vec_t<shader_var_t>& vec, const string& name ) const
{
	vec_t<shader_var_t>::iterator it = vec.begin();
	while( it != vec.end() && it->name != name )
	{
		++it;
	}
	return it;
}


//=====================================================================================================================================
// ParseFileForPragmas                                                                                                                =
//=====================================================================================================================================
bool shader_parser_t::ParseFileForPragmas( const string& filename, int id )
{
	// load file in lines
	vec_t<string> lines = GetFileLines( filename.c_str() );
	if( lines.size() < 1 )
	{
		ERROR( "Cannot parse file \"" << filename << "\"" );
		return false;
	}

	// scanner
	scanner_t scanner( false );
	if( !scanner.LoadFile( filename.c_str() ) ) return false;
	const scanner_t::token_t* token;

	
	do
	{
		token = &scanner.GetNextToken();

		if( token->code == scanner_t::TC_SHARP )
		{
			token = &scanner.GetNextToken();
			if( token->code == scanner_t::TC_IDENTIFIER && strcmp(token->value.string, "pragma") == 0 )
			{
				token = &scanner.GetNextToken();
				if( token->code == scanner_t::TC_IDENTIFIER && strcmp(token->value.string, "anki") == 0 )
				{
					token = &scanner.GetNextToken();
/* vert_shader_begins */
					if( token->code == scanner_t::TC_IDENTIFIER && strcmp(token->value.string, "vert_shader_begins") == 0 )
					{
						// play
						if( frag_shader_begins.defined_in_line != -1 ) // check if frag shader allready defined
						{
							PARSE_ERR( "vert_shader_begins must precede frag_shader_begins defined at " << frag_shader_begins.defined_in_file << 
							           ":" << frag_shader_begins.defined_in_line );
							return false;
						}
						
						if( vert_shader_begins.defined_in_line != -1 ) // allready defined elseware so throw error
						{
							PARSE_ERR( "vert_shader_begins allready defined at " << vert_shader_begins.defined_in_file << ":" << 
							           vert_shader_begins.defined_in_line );
							return false;
						}
						vert_shader_begins.defined_in_file = filename;
						vert_shader_begins.defined_in_line = scanner.GetLineNmbr();
						vert_shader_begins.global_line = source_lines.size() + 1;
						source_lines.push_back( string("#line ") + IntToStr(scanner.GetLineNmbr()) + ' ' + IntToStr(id) + " // " + lines[scanner.GetLineNmbr()-1] );
						// stop play
					}
/* frag_shader_begins */
					else if( token->code == scanner_t::TC_IDENTIFIER && strcmp(token->value.string, "frag_shader_begins") == 0 )
					{
						// play
						if( vert_shader_begins.defined_in_line == -1 )
						{
							PARSE_ERR( "frag_shader_begins should be defined after vert_shader_begins" );
							return false;
						}
						
						if( frag_shader_begins.defined_in_line != -1 ) // if allready defined elseware throw error
						{
							PARSE_ERR( "frag_shader_begins allready defined at " << frag_shader_begins.defined_in_file << ":" << 
							           frag_shader_begins.defined_in_line );
							return false;
						}
						frag_shader_begins.defined_in_file = filename;
						frag_shader_begins.defined_in_line = scanner.GetLineNmbr();
						frag_shader_begins.global_line = source_lines.size() + 1;
						source_lines.push_back( string("#line ") + IntToStr(scanner.GetLineNmbr()) + ' ' + IntToStr(id) + " // " + lines[scanner.GetLineNmbr()-1] );
						// stop play
					}
/* include */
					else if( token->code == scanner_t::TC_IDENTIFIER && strcmp(token->value.string, "include") == 0 )
					{
						token = &scanner.GetNextToken();
						if( token->code == scanner_t::TC_STRING )
						{
							// play
							//int line = source_lines.size();
							source_lines.push_back( string("#line 0 ") + IntToStr(id+1) + " // " + lines[scanner.GetLineNmbr()-1] );
							if( !ParseFileForPragmas( token->value.string, id+1 ) ) return false;
							source_lines.push_back( string("#line ") + IntToStr(scanner.GetLineNmbr()) + ' ' + IntToStr(id) +  " // end of " + lines[scanner.GetLineNmbr()-1] );
							// stop play
						}
						else
						{
							PARSE_ERR_EXPECTED( "string" );
							return false;
						}
					}
/* uniform */
					else if( token->code == scanner_t::TC_IDENTIFIER && strcmp(token->value.string, "uniform") == 0 )
					{
						token = &scanner.GetNextToken();
						if( token->code == scanner_t::TC_IDENTIFIER )
						{
							string var_name = token->value.string;
							token = &scanner.GetNextToken();
							if( token->code == scanner_t::TC_NUMBER && token->type == scanner_t::DT_INT )
							{
								// play
								vec_t<shader_var_t>::iterator uniform = FindShaderVar( uniforms, var_name );
								if( uniform != uniforms.end() )
								{
									PARSE_ERR( "Uniform allready defined at " << uniform->defined_in_file << ":" << uniform->defined_in_line );
									return false;
								}
								
								uniforms.push_back( shader_var_t( filename, scanner.GetLineNmbr(), var_name, token->value.int_ ) );
								source_lines.push_back( lines[scanner.GetLineNmbr()-1] );
								// stop play
							}
							else
							{
								PARSE_ERR_EXPECTED( "integer" );
								return false;
							}
						}
						else
						{
							PARSE_ERR_EXPECTED( "identifier" );
							return false;
						}
					}
/* attribute */
					else if( token->code == scanner_t::TC_IDENTIFIER && strcmp(token->value.string, "attribute") == 0 )
					{
						token = &scanner.GetNextToken();
						if( token->code == scanner_t::TC_IDENTIFIER )
						{
							string var_name = token->value.string;
							token = &scanner.GetNextToken();
							if( token->code == scanner_t::TC_NUMBER && token->type == scanner_t::DT_INT )
							{
								// play
								vec_t<shader_var_t>::iterator attrib = FindShaderVar( attributes, var_name );
								if( attrib != attributes.end() )
								{
									PARSE_ERR( "Attribute allready defined at " << attrib->defined_in_file << ":" << attrib->defined_in_line );
									return false;
								}
								
								attributes.push_back( shader_var_t( filename, scanner.GetLineNmbr(), var_name, token->value.int_ ) );
								source_lines.push_back( lines[scanner.GetLineNmbr()-1] );
								// stop play
							}
							else
							{
								PARSE_ERR_EXPECTED( "integer" );
								return false;
							}
						}
						else
						{
							PARSE_ERR_EXPECTED( "identifier" );
							return false;
						}
					}
					else
					{
						PARSE_WARN( "Ignoring incorrect #pragma anki" );
					}
				} // end if anki
				
				token = &scanner.GetNextToken();
				if( token->code!=scanner_t::TC_NEWLINE && token->code!=scanner_t::TC_EOF )
				{
					PARSE_ERR_EXPECTED( "newline or end of file" );
					return false;
				}
				
				if( token->code == scanner_t::TC_EOF )
					break;
				
			} // end if pragma
		} // end if #
/* newline */		
		else if( token->code == scanner_t::TC_NEWLINE )
		{
			source_lines.push_back( lines[ scanner.GetLineNmbr() - 2 ] );
			//PRINT( lines[ scanner.GetLineNmbr() - 2 ] )
		}
/* EOF */
		else if( token->code == scanner_t::TC_EOF )
		{
			source_lines.push_back( lines[ scanner.GetLineNmbr() - 1 ] );
			//PRINT( lines[ scanner.GetLineNmbr() - 1 ] )
			break;
		}
/* error */
		else if( token->code == scanner_t::TC_ERROR )
		{
			return false;
		}

	} while( true );
	
	return true;
}


//=====================================================================================================================================
// Load                                                                                                                               =
//=====================================================================================================================================
bool shader_parser_t::ParseFile( const char* filename )
{
	// parse master file
	if( !ParseFileForPragmas( filename ) ) return false;

	// sanity checks
	if( vert_shader_begins.global_line == -1 )
	{
		ERROR( "Entry point \"vert_shader_begins\" is not defined in file \"" << filename << "\"" );
		return false;
	}
	if( frag_shader_begins.global_line == -1 )
	{
		ERROR( "Entry point \"frag_shader_begins\" is not defined in file \"" << filename << "\"" );
		return false;
	}
	
	// construct shader's source code
	vert_shader_source = "";
	frag_shader_source = "";
	for( int i=0; i<vert_shader_begins.global_line-1; ++i )
	{
		vert_shader_source += source_lines[i] + "\n";
		frag_shader_source += source_lines[i] + "\n";
	}	
	for( int i=vert_shader_begins.global_line-1; i<frag_shader_begins.global_line-1; ++i )
	{
		vert_shader_source += source_lines[i] + "\n";
	}
	for( int i=frag_shader_begins.global_line-1; i<int(source_lines.size()); ++i )
	{
		frag_shader_source += source_lines[i] + "\n";
	}
	
	//PRINT( "vert_shader_begins.global_line: " << vert_shader_begins.global_line )
	//PRINT( "frag_shader_begins.global_line: " << frag_shader_begins.global_line )
	//PrintSourceLines();
	//PrintShaderVars();

	return true;
}
