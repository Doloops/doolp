#include <glogers.h>
#include <DMLXParser.h>
#include <DMLXDocument.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/*
 * Generates a C++-compilable version of an XML file
 */

setGlog ( "generateDMLXDocument" );

void generateCode ( DMLX::Node * node, FILE * fp )
{
  fprintf ( fp, "\n\t// Node '%p'\n", node );
  static char text[2048];
  if ( node->isText() )
    {
      // Must protected the '"'
      int i,j;
      char * fromText = node->getText();
      for ( i = 0, j = 0 ; fromText[i] != '\0' ; i++, j++ )
	{
	  if ( fromText[i] == '"' )
	    {
	      text[j++] = '\\';
	      text[j] = '"';
	      continue;
	    }
	  else if ( fromText[i] == '\n' )
	    {
	      text[j++] = '\\';
	      text[j] = 'n';
	      continue;
	    }
	  else if ( fromText[i] == '\\' )
	    {
	      //	      Log ( "Found new protected \\ (next %c)\n", fromText[i+1] );
	      text[j++] = '\\';
	      text[j] = '\\';
	      continue;
	    }

	  text[j] = fromText[i];
	}
      text[j] = '\0';
      fprintf ( fp, "\tDMLX::Node * __node__%p = new DMLX::Node ( \"%s\", false );\n",
		node, text );
      return;
    }
  fprintf ( fp, "\t// Node '%s'\n", node->getName () );
  fprintf ( fp, "\tDMLX::Node * __node__%p = new DMLX::Node ( &__keyword__0x%x );\n",
	    node, node->getHash () );
  for ( DMLX::Attribute * attr = node->getFirstAttribute () ; attr != NULL ; attr = attr->getNext() )
    {
      fprintf ( fp, "\t// Attribute '%s'\n", attr->getName() );
      fprintf ( fp, "\tDMLX::Attribute * __attr__%p = new DMLX::Attribute ( &__keyword__0x%x, \"%s\" );\n",
		attr, attr->getHash (), attr->getValue() );
      fprintf ( fp, "\t__node__%p->addAttribute ( __attr__%p );\n",
		node, attr );
    }
  for ( DMLX::Node * subNode = node->getFirst () ; subNode != NULL ; subNode = subNode->getNext() )
    {
      generateCode ( subNode, fp );
      fprintf ( fp, "\t__node__%p->addNode ( __node__%p );\n",
		node, subNode );
    }
}

void generateCode ( DMLX::Document * doc, char * funcName, FILE * fp )
{
  fprintf ( fp, "#include <DMLXDocument.h>\n" );
  fprintf ( fp, "// Keys\n" );
  std::list<DMLX::Keyword *> * l = doc->getHashTree()->getKeywordList ();
  for ( std::list<DMLX::Keyword *>::iterator k = l->begin () ;
	k != l->end () ; k++ )
      fprintf ( fp, "DMLX::Keyword __keyword__0x%x(\"%s\");\n", 
	       (*k)->getHash(), (*k)->getKeyword() );
  fprintf ( fp, "\n" );
  fprintf ( fp, "DMLX::Document * %s()\n", funcName );
  fprintf ( fp, "{\n" );
  fprintf ( fp, "\tDMLX::Document * document = new DMLX::Document();\n" );
  for ( std::list<DMLX::Keyword *>::iterator k = l->begin () ;
	k != l->end () ; k++ )
    fprintf ( fp, "\tdocument->getHashTree()->add ( &__keyword__0x%x );\n",
	      (*k)->getHash() );
  for ( DMLX::Node * node = doc->getFirst() ; node != NULL ; node = node->getNext() )
    {
      generateCode ( node, fp );
      fprintf ( fp, "\tdocument->addNode ( __node__%p );\n", node );
    }
  fprintf ( fp, "\treturn document;\n" );
  fprintf ( fp, "}\n" );
  fprintf ( fp, "// Done\n" );
}


int main ( int argc, char ** argv )
{
  addGlog ( new Gloger_File ( stderr ) );
  if ( argc != 3 )
    {
      Fatal ( "Usage : generateDMLXDocument 'document.xml' 'function name'\n" );
    }
  DMLX::ParserFile parserxml( argv[1] );
  parserxml.setParseKeepAllText ( true );
  DMLX::Document docxml;
  docxml.buildFromParser ( parserxml );
  generateCode ( &docxml, argv[2], stdout );
  return 0;
}
