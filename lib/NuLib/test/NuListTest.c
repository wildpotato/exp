
#include "NuUtil.h"

typedef struct _obj_t
{
	base_list_node_t Node;
	int              Cnt;
} obj_t;



int main()
{
	base_list_t *lst = (base_list_t *)malloc(sizeof(base_list_t));
	obj_t *o = NULL;

	
	base_list_init(lst);

	o = (obj_t *)malloc(sizeof(obj_t));
	o->Cnt = 1;

	base_list_insert_head(lst, &(o->Node));



	base_list_remove_node(&(o->Node));


	return 0;
}

