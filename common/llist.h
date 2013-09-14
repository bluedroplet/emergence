/**
 * @file
 * @copyright 1998-2004 Jonathan Brown <jbrown@bluedroplet.com>
 * @license https://www.gnu.org/licenses/gpl-3.0.html
 * @homepage https://github.com/bluedroplet/emergence
 */

#define LL_ALLOC(type, node0, node)\
	{\
		*node = (type*)malloc(sizeof(type));\
		*node->next = *node0;\
		*node0 = *node;\
	}

#define LL_CALLOC(type, node0, node)\
	{\
		*node = (type*)calloc(1, sizeof(type));\
		*node->next = *node0;\
		*node0 = *node;\
	}

		
#define LL_CALLOC_TAIL(type, node0, node)\
	{\
		type **cnode = node0;\
\
		while(*cnode)\
			cnode = &(*cnode)->next;\
		\
		*node = *cnode = (type*)calloc(1, sizeof(type));\
	}

		
#define LL_ADD(type, node0, node)\
	{\
		type *temp = (type*)malloc(sizeof(type));\
		memcpy(temp, node, sizeof(type));\
		temp->next = *node0;\
		*node0 = temp;\
	}

	
#define LL_ADD_TAIL(type, node0, node)\
	{\
		if(!*node0)\
		{\
			*node0 = (type*)malloc(sizeof(type));\
			memcpy(*node0, node, sizeof(type));\
			(*node0)->next = NULL;\
		}\
		else\
		{\
			type *cnode = *node0;\
	\
			while(cnode->next)\
				cnode = cnode->next;\
	\
			cnode->next = (type*)malloc(sizeof(type));\
			cnode = cnode->next;\
			memcpy(cnode, node, sizeof(type));\
			cnode->next = NULL;\
		}\
	}

	
#define LL_REMOVE(type, node0, node)\
	{\
		if(*node0 == node)\
		{\
			type *temp = (*node0)->next;\
			free(*node0);\
			*node0 = temp;\
		}\
		else\
		{\
			type *cnode = *node0;\
	\
			while(cnode->next)\
			{\
				if(cnode->next == node)\
				{\
					type *temp = cnode->next->next;\
					free(cnode->next);\
					cnode->next = temp;\
					break;\
				}\
	\
				cnode = cnode->next;\
			}\
		}\
	}

#define LL_REMOVE_ALL(type, node0)\
	{\
		while(*node0)\
		{\
			type *temp = (*node0)->next;\
			free(*node0);\
			*node0 = temp;\
		}\
	}
	
#define LL_NEXT(node) node = node->next
