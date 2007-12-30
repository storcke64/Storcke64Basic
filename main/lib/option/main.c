

#define __MAIN_C

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "getoptions.h"

#include "main.h"

typedef  void (*type_hook_old)(int*,char**);
static type_hook_old old_hook;
char** cmd_arg;
int arg_count;



GB_INTERFACE GB EXPORT;



GB_DESC *GB_CLASSES[] EXPORT =
{
  GetOptionsDesc,
  NULL
};
void hook_main(int *argc,char ** argv)
{
	int i;
	char **tmp;

	if(old_hook!=NULL)
	{
		old_hook(argc,argv);
	}

	arg_count=*argc;
			
	GB.NewArray((void*)(&cmd_arg),sizeof(*cmd_arg),0);

	for(i=0;i<*argc;i++)
	{
		tmp=(char **)GB.Add((void*)(&cmd_arg));
		GB.NewString(tmp,argv[i],0);

	}
	*argc=1;
	return;
}

int EXPORT GB_INIT(void)
{
  old_hook= (type_hook_old) GB.Hook ( GB_HOOK_MAIN, (void *)hook_main );
  return 0;
}


void EXPORT GB_EXIT()
{
	int i;
	for(i=0;i<arg_count;i++)
	{
		GB.FreeString( (void *) &cmd_arg[i]);
//		tmp=(char **)GB.Array.Get((void*)(cmd_arg),i);
//		GB.FreeString(tmp);
	}
	GB.FreeArray( (void *) &cmd_arg );

}

