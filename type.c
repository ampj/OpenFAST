#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
# define rindex(X,Y) strrchr(X,Y)
# define index(X,Y) strchr(X,Y)
#else
# include <strings.h>
#endif
 

#include "registry.h"
#include "protos.h"
#include "data.h"

int
init_type_table()
{
  node_t *p ;
  p = new_node(TYPE) ; p->type_type = SIMPLE  ; strcpy( p->name , "integer" )         ;
                                                strcpy( p->mapsto, "INTEGER(IntKI)")  ;
                                                add_node_to_end ( p , &Type )         ;
  p = new_node(TYPE) ; p->type_type = SIMPLE  ; strcpy( p->name , "intki" )           ;
                                                strcpy( p->mapsto, "INTEGER(IntKI)")  ;
                                                add_node_to_end ( p , &Type )         ;
  p = new_node(TYPE) ; p->type_type = SIMPLE  ; strcpy( p->name , "b4ki" )           ;
                                                strcpy( p->mapsto, "INTEGER(IntKI)")  ;
                                                add_node_to_end ( p , &Type )         ;
  p = new_node(TYPE) ; p->type_type = SIMPLE  ; strcpy( p->name , "real" )            ;
  p = new_node(TYPE) ; p->type_type = SIMPLE  ; strcpy( p->name , "real" )            ;
                                                strcpy( p->mapsto, "REAL(ReKi)")      ;
                                                add_node_to_end ( p , &Type )         ;
  p = new_node(TYPE) ; p->type_type = SIMPLE  ; strcpy( p->name , "reki" )            ;
                                                strcpy( p->mapsto, "REAL(ReKi)")      ;
                                                add_node_to_end ( p , &Type )         ;
  p = new_node(TYPE) ; p->type_type = SIMPLE  ; strcpy( p->name , "logical" )         ;
                                                strcpy( p->mapsto, "LOGICAL")         ;
                                                add_node_to_end ( p , &Type )         ;
#if 0
// these have to be handled individually because people can and will put lengths after them
// so can't make a generic type node here
  p = new_node(TYPE) ; p->type_type = SIMPLE  ; strcpy( p->name , "character" )       ;
                                                strcpy( p->mapsto, "CHARACTER") /**/  ;
                                                add_node_to_end ( p , &Type )         ;
#endif
  p = new_node(TYPE) ; p->type_type = SIMPLE  ; strcpy( p->name , "doubleprecision" ) ;
                                                strcpy( p->mapsto, "REAL(DbKi)")      ;
                                                add_node_to_end ( p , &Type )         ;
  p = new_node(TYPE) ; p->type_type = SIMPLE  ; strcpy( p->name , "dbki" )            ;
                                                strcpy( p->mapsto, "REAL(DbKi)")      ;
                                                add_node_to_end ( p , &Type )         ;
  p = new_node(TYPE) ; p->type_type = SIMPLE  ; strcpy( p->name , "r8Ki" )            ;
                                                strcpy( p->mapsto, "REAL(DbKi)")      ;
                                                add_node_to_end ( p , &Type )         ;
  p = new_node(TYPE) ; p->type_type = DERIVED ; strcpy( p->name , "meshtype" )        ;
                                                strcpy( p->mapsto, "MeshType")  ;
                                                add_node_to_end ( p , &Type )         ;
  return(0) ;
}

int
set_state_dims ( char * dims , node_t * node )
{
  int modifiers ;
  node_t *d, *d1 ;
  char *c ;
  char dspec[NAMELEN] ;
  int inbrace ;
  int i, deferred ;

  if ( dims == NULL ) dims = "-" ;
  modifiers = 0 ;
  node->ndims = 0 ;
  node->boundary_array = 0 ;

  inbrace = 0 ;
  node->subgrid = 0 ;
  strcpy(dspec,"") ;
  for ( c = dims ; *c ; c++ )
  {
    if ( *c == '-' && ! inbrace )
    {
      break ;
    }
    else if ( *c == '{' && ! inbrace )
    {
      inbrace = 1 ;
      continue ;
    }
    else if ( modifiers == 0 )
    {
      if ( *c == '}' && inbrace )  { inbrace = 0 ; }
      else                         { int n = strlen(dspec) ; dspec[n] = *c ; dspec[n+1]='\0' ; }
      if ( inbrace ) {
        continue ;
      }
      d1 = new_node(DIM) ;  /* make a copy */
      if (( d = get_dim_entry ( dspec )) != NULL ) {
         *d1 = *d ;
      } else {
         set_dim_len( dspec , d1 ) ;
      }
      node->dims[node->ndims++] = d1 ;
      strcpy(dspec,"") ;
    }
  }
  // check to make sure that if any dimension is deferred they all most be 

  if ( node->ndims > 0 ) {
    deferred = node->dims[0]->deferred ;
    for ( i = 1 ; i < node->ndims ; i++ )
    {
      if ( deferred != node->dims[i]->deferred ) {
        if ( node->dims[i]->deferred ) {
          fprintf(stderr, 
            "Registry warning: dimension %d of %s is allocatable while others are not.\n",i,node->name) ;
        } else {
          fprintf(stderr, 
            "Registry warning: dimension %d of %s is not allocatable while others are.\n",i,node->name) ;
        }
      }
      if ( node->dims[i]->deferred ) deferred = 1 ;
    }
  }

  return (0) ;
}
 
#if 0
node_t *
get_4d_entry ( char * name )
{
  node_t *p ;
  if ( name == NULL ) return (NULL)  ;
  for ( p = FourD ; p != NULL ; p = p->next4d )
  {
    if ( !strcmp( p->name , name ) )
    {
      return(p) ;
    }
  }
  return(NULL) ;
}
#endif

node_t *
get_type_entry ( char * typename )
{
  node_t * retval ;
  retval = get_entry(typename,Type) ;
  return(retval) ;
}

node_t *
get_modname_entry ( char * modname )
{
  return(get_entry(modname,ModNames)) ;
}

node_t *
get_rconfig_entry ( char * name )
{
  node_t * p ;
  if ((p=get_entry(name,Domain.fields))==NULL) return(NULL) ;
  if (p->node_kind & RCONFIG) return(p) ;
  return(NULL) ;
}

node_t *
get_entry ( char * name , node_t * node )
{
  node_t *p ;
  if ( name == NULL ) return (NULL)  ;
  if ( node == NULL ) return (NULL)  ;
  for ( p = node ; p != NULL ; p = p->next )
  {
    if ( !strncmp( name , "character", 9 ) )
    {
      if ( !strncmp( p->name , name, 9 ) )
      {
        return(p) ;
      }
    } else {
//fprintf(stderr,"name |%s|, p->name |%s|\n",name,p->name) ;
      if ( !strcmp( make_lower_temp(p->name) , make_lower_temp(name) ) )
      {
        return(p) ;
      }
    }
  }
  return(NULL) ;
}

/* this gets the entry for the node even if it           */
/* is a derived data structure; does this by following   */
/* the fully specified f90 reference.  For example:      */
/* "xa%f" for the field of derived type xa.              */
/* note it will also take care to ignore the _1 or _2    */
/* suffixes from variables that have ntl > 1             */
/* 11/10/2001 -- added use field; if the entry has a use */
/* that starts with "dyn_" and use doesn't correspond to */
/* that, skip that entry and continue                    */

node_t *
get_entry_r ( char * name , char * use , node_t * node )
{
  node_t *p ;
  char tmp[NAMELEN], *t1, *t2 ;

  if ( name == NULL ) return (NULL)  ;
  if ( node == NULL ) return (NULL)  ;

  for ( p = node ; p != NULL ; p = p->next )
  {
    strcpy( tmp, name ) ;

    /* first check for exact match */
    if ( !strcmp( p->name , tmp ) )
    {
      return(p) ;
    }

    t1 = NULL ;
    if ((t1 = index(tmp,'%'))!= NULL ) *t1 = '\0' ;

    if ( p->ntl > 1 )
    {
      if (( t2 = rindex( tmp , '_' )) != NULL )
      {
         /* be sure it really is an integer that follows the _ and that */
         /* that is that is the last character                          */
         if ((*(t2+1) >= '0' && *(t2+1) <= '9') && *(t2+2)=='\0') *t2 = '\0' ;
      }
    }

    /* also allow _tend */
    if (( t2 = rindex( tmp , '_' )) != NULL ) {
         if (!strcmp(t2,"_tend")) *t2 = '\0' ;
    }

    /* also allow _tend */
    if (( t2 = rindex( tmp , '_' )) != NULL ) {
         if (!strcmp(t2,"_old")) *t2 = '\0' ;
    }

    if ( !strcmp( p->name , tmp ) )
    {
      if ( t1 != NULL ) return( get_entry_r( t1+1 , use , p->type->fields ) ) ;
      return(p) ;
    }
  }
  return(NULL) ;
}

node_t *
get_dimnode_for_coord ( node_t * node , int coord_axis )
{
  int i ;
  if ( node == NULL ) return(NULL) ;
  for ( i = 0 ; i < node->ndims ; i++ )
  {
    if ( node->dims[i] == NULL ) continue ;
    if ( node->dims[i]->coord_axis == coord_axis )
    {
      return(node->dims[i]) ;
    }
  }
  return(NULL) ;
}

int 
get_index_for_coord ( node_t * node , int coord_axis )
{
  int i ;
  if ( node == NULL ) return( -1 ) ;
  for ( i = 0 ; i < node->ndims ; i++ )
  {
    if ( node->dims[i] == NULL ) continue ;
    if ( node->dims[i]->coord_axis == coord_axis )
    {
      return(i) ;
    }
  }
  return(-1) ;
}


char *
set_mem_order( node_t * node , char * str , int n )
{
  int i ;
  node_t * p ;
  
  if ( str == NULL || node == NULL ) return(NULL) ;
  strcpy(str,"") ;
  if ( node->boundary_array )
  {
     strcpy(str, "C") ;  /* if this is called for a boundary array, just give it a   */
                         /* "reasonable" value and move on.                          */
  }
  else
  {
    if ( node->ndims <= 0 )
    {
      strcat(str,"0") ; return(str) ;
    }
    for ( i = 0 ; i < node->ndims && i < n  ; i++ )
    {
      p = node->dims[i] ;
      switch( p->coord_axis )
      {
      case(COORD_X) : strcat(str,"X") ; break ;
      case(COORD_Y) : strcat(str,"Y") ; break ;
      case(COORD_Z) : strcat(str,"Z") ; break ;
      case(COORD_C) : strcat(str,"C") ; break ;
      default : break ;
      }
    }
  }
  return(str) ;
}
