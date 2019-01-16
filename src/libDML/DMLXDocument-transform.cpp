#include <DMLXDocument.h>

#if 0
#undef Log
#define Log(...)
#endif

DMLX::Keyword keyw_xsl_stylesheet("xsl:stylesheet");
DMLX::Keyword keyw_xsl_template("xsl:template");
DMLX::Keyword keyw_match("match");
DMLX::Keyword keyw_test("test");
DMLX::Keyword keyw_select("select");
DMLX::Keyword keyw_xsl_if("xsl:if");   
DMLX::Keyword keyw_xsl_apply_templates("xsl:apply-templates");
DMLX::Keyword keyw_xsl_for_each("xsl:for-each");
DMLX::Keyword keyw_xsl_value_of("xsl:value-of");
DMLX::Keyword keyw_xsl_choose("xsl:choose");
DMLX::Keyword keyw_xsl_when("xsl:when");
DMLX::Keyword keyw_xsl_otherwise("xsl:otherwise");

void DMLX::Document::transform(int fd, DMLX::Document * xsl)
{
  Log ( "Operating transform...\n" );	
  Node * stylesheet = xsl->getNode ( keyw_xsl_stylesheet );
  AssertBug ( stylesheet != NULL, "Could not get stylesheet\n" );
  Node * template0 = stylesheet->getNode ( keyw_xsl_template );
  AssertBug ( template0 != NULL, "Could not get initial template !\n" );
  char * match = template0->getAttribute ( keyw_match );
  // Shall reimplement this i guess..
  match++;
  DMLX::Keyword key(match);
  std::list<Node*> * nodes = getNodeList ( key );
  for ( std::list<Node*>::iterator node = nodes->begin () ;
	node != nodes->end () ; node ++ )
    (*node)->transform ( fd, stylesheet, template0 );
  Log ( "Transform ended..\n" );
}

char * DMLX::Node::getValueOf ( const char * selection )
{
  Log ( "node=%p('%s') : selection = '%s'\n", this, getName(), selection );
  if ( selection[0] == '.'
       && 	selection[1] == '.'
       && 	selection[2] == '/' )
    {
      Log ( "FATHER VALUE\n" );
      AssertFatal ( father != NULL, "Got no father, but asked for a ../ arg\n" );
      return father->getValueOf ( selection + 3 );
    }
  AssertFatal(selection[0] == '@', "Selection shall start with @!!\n" );
  Keyword k(selection+1);
  if ( getAttribute ( k ) == NULL )
    Warn ( "No (null) selection !\n" );
  return getAttribute (k);
}

bool DMLX::Node::checkCondition ( const char * _cond )
// This is the ugliest lasy code i have ever written...
{
  AssertBug ( _cond[0] == '@', "Only knowns the @... condition\n" );
  AssertFatal ( ! isText (), "Can not apply a checkCondition on a text node !\n" );
  char cond[strlen(_cond) + 1];
  strcpy ( cond, _cond );
  char * param = cond + 1;
  char * paramEnd = strchr(param, ' ' );
  if ( paramEnd == NULL )
    {
      Keyword k(param);
      return (getAttribute(k) != NULL);
    }
  paramEnd[0] = '\0';
  char * value = strchr ( paramEnd+1, '\'' ) + 1;
  strchr ( value, '\'' )[0] = '\0';
  Log ( "Condition : node '%s', param '%s', value '%s'\n", getName(), param, value );
  Keyword k(param);
  char * attrValue = getAttribute ( k );
  Log ( "Node param '%s' is '%s'\n", param, attrValue );
  bool res;
  if ( attrValue == NULL )
    res =  false;
  res = (strcmp ( attrValue, value ) == 0);
  if ( strstr ( _cond, "!=" ) )
    res = !res;
  Log ( "Condition : evaled to '%s'\n", res ? "true" : "false" );
  return res;
}

#define __writeText(_s_)						\
  { if ( _s_ != NULL ) { char * _s = _s_; int l = strlen ( _s ); ::write ( fd, _s, l ); } }

void DMLX::Node::transform(int fd, Node * xslStylesheet, Node * xslTemplate)
{
  Log ( "Apply transform '%s' to node '%s'\n", 
	xslTemplate->getName(), getName () );
  for ( Node * _action = xslTemplate->getFirst() ; _action != NULL ; _action = _action->getNext() )
    {
      Node & action = *(_action);
      if ( action.isText () )
	{
	  __writeText	( action.getText() );
	  continue;
	}
      Keyword & key = *(action.name);
      Log ( "Key '%s' (hash 0x%x)\n", key.getKeyword(), key.getHash() );
      if ( key.isEqual ( keyw_xsl_if ) )
	{
	  char * cond = action.getAttribute ( keyw_test );	
	  if ( checkCondition ( cond ) )
	    transform ( fd, xslStylesheet, &action );
	  continue;
	} 
      else if ( key.isEqual ( keyw_xsl_value_of ) )
	{
	  char * value = getValueOf ( action.getAttribute ( keyw_select ) );
	  __writeText(value);
	  Log ( "value-of (select='%s') gave '%s'\n",
		action.getAttribute ( keyw_select ), value );
	  continue;
	}
      else if ( key.isEqual ( keyw_xsl_apply_templates ) )
	{
	  char * actualMatch = xslTemplate->getAttribute(keyw_match);
	  char * extraMatch = action.getAttribute(keyw_select);
	  char newMatch[strlen(actualMatch)+strlen(extraMatch)+2];
	  strcpy ( newMatch, actualMatch );
	  strcat ( newMatch, "/" );
	  strcat ( newMatch, extraMatch );
	  Log ( "New match : '%s'\n", newMatch );
	  // Must find template now
	  Node * newXslTemplate = NULL;
	  std::list<Node*> * templates = xslStylesheet->getNodeList ( keyw_xsl_template );
	  for ( std::list<Node*>::iterator tm = templates->begin () ;
		tm != templates->end () ; tm++ )
	    if ( strcmp ((*tm)->getAttribute ( keyw_match ), newMatch ) == 0 )
	      { newXslTemplate = (*tm); break; }
	  AssertFatal ( newXslTemplate != NULL, "Could not get template for match '%s'\n", newMatch );

	  Keyword k(action.getAttribute ( keyw_select ));
	  std::list<Node*> * sons = getNodeList ( k );
	  Log ( "Got %d sons for this match\n", sons->size() );				
	  for ( std::list<Node*>::iterator iter = sons->begin () ;
		iter != sons->end () ; iter++ )
	    (*iter)->transform ( fd, xslStylesheet, newXslTemplate );
	  continue;
	}			
      else if ( key.isEqual ( keyw_xsl_for_each ) )
	{
	  Keyword k(action.getAttribute ( keyw_select ));
	  std::list<Node*> * sons = getNodeList ( k );
	  Log ( "Got %d sons match '%s'\n", sons->size(), action.getAttribute ( keyw_select ) );
	  for ( std::list<Node*>::iterator iter = sons->begin () ;
		iter != sons->end () ; iter++ )
	    (*iter)->transform ( fd, xslStylesheet, &action );
	  continue;					
	}
      else if ( key.isEqual ( keyw_xsl_choose ) )
	{
	  bool found = false;
	  for ( Node * when = action.getFirst () ; when != NULL ; when = when->getNext() )
	    {
	      if ( ! when->isName ( keyw_xsl_when ) )
		continue;
	      char * condition = when->getAttribute ( keyw_test );
	      if ( checkCondition ( condition ) )
		{
		  Log ( "Choose : run 'when' (condition '%s')\n", condition );
		  transform ( fd, xslStylesheet, when );
		  found = true;
		  break;
		}
	      
	    }
	  if ( ! found )
	    {
	      Node * otherwise = action.getNode ( keyw_xsl_otherwise );	
	      AssertFatal ( otherwise != NULL, "Choose has no otherwise !\n" );
	      Log ( "Choose : come to 'otherwise'\n" );
	      transform ( fd, xslStylesheet, otherwise );
	    }
	  continue;
	}
      Warn ( "Action not handled : '%s'\n", action.getName () );
    }
}
